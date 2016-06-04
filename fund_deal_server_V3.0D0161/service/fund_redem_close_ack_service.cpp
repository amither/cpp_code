/**
  * FileName: fund_redem_close_ack_service.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2014-9-6
  * Description: �����׷��� ���ڲ�Ʒʵʱ���ȷ�� Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_redem_close_ack_service.h"

FundRedemCloseAck::FundRedemCloseAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

	memset(&m_stTradeRedem, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));
	memset(&m_close_trans, 0, sizeof(FundCloseTrans));

	m_draw_arrive_type = DRAW_ARRIVE_TYPE_T1; //Ĭ��t+1 ����
	m_loading_type = DRAW_NOT_USE_LOADING; //Ĭ��Ϊ����Ҫ����
    m_stop_fetch = false;
	m_need_updateExauAuthLimit = false;
}

/**
  * service step 1: �����������
  */
void FundRedemCloseAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	TRACE_DEBUG("[fund_redem_close_ack_service] parseInputMsg start: ");

	// Ҫ�����������ݣ��ײ��ʹ��
    m_request = rqst;

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_close_ack_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(szMsg, "fund_trans_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readStrParam(szMsg, "sp_billno", 0, 32);
    m_params.readStrParam(szMsg, "fund_name", 0, 64);
    m_params.readStrParam(szMsg, "fund_code", 0, 64);
	m_params.readIntParam(szMsg, "op_type", 1, 4);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_optype = m_params.getInt("op_type");

}

/*
 * ���ɻ���ע����token
 */
string FundRedemCloseAck::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uid|cft_bank_billno|spid| sp_billno | total_fee |key
    // ��������ԭ��
    ss << m_params["uid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;
	TRACE_DEBUG("FundRedemCloseAck, sourceStr=[%s]", 
						ss.str().c_str());

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundRedemCloseAck::CheckToken() throw (CException)
{
	// ����token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * ����������ȡ�ڲ�����
  */
void FundRedemCloseAck::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();

	if(INF_REDEM_SP_ACK_SUC == m_optype)
	{
		CHECK_PARAM_EMPTY("sp_billno");   
	}

}

/**
  * ִ���깺����
  */
void FundRedemCloseAck::excute() throw (CException)
{
    try
    {
        CheckParams();

		CheckFundBind();

         /* �������� */
        m_pFundCon->Begin();

        /* ckv�����ŵ�����֮�������ύ */
        gCkvSvrOperator->beginCkvtrans();

		/* ��ѯ�����׼�¼ */
        CheckFundTrade();

        /* ���˻������»�����״̬ */
        UpdateTradeState();

        /* �ύ���� */
        m_pFundCon->Commit();

        gCkvSvrOperator->commitCkvtrans();
		
		/* ����MQ */
		sendFundBuy2MqMsg(m_stTradeRedem);
		
		/* ����EXAU */
		updateExauAuthLimitNoExcp();

		/* ���¸���ckv ,��������֮���Ǳ�������ع�ȴд��ckv������*/
		updateCkvs();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
         //�ع�dbǰ�Ȼع�����ckv
         gCkvSvrOperator->rollBackCkvtrans();
         
        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_CLOSE_REDEM_DRAW_SUCC != (unsigned)e.error()))
        {
            throw;
        }
    }
}


/*
 * ��ѯ�����˻��Ƿ����
 */
void FundRedemCloseAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}


/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundRedemCloseAck::CheckFundTrade() throw (CException)
{
	// û�н��׼�¼������
	if(!QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeRedem, true))
	{
		gPtrAppLog->error("buy record not exist, cft_bank_billno[%s]  ", m_params.getString("cft_bank_billno").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	// ����״̬��Ч������
	if(LSTATE_INVALID == m_stTradeRedem.Flstate)
	{
		gPtrAppLog->error("fund buy pay, lstate is invalid. listid[%s], uid[%d] ", m_stTradeRedem.Flistid, m_stTradeRedem.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
	}

	if(m_params.getInt("uid") != 0 && m_stTradeRedem.Fuid!=0 && m_params.getInt("uid") != m_stTradeRedem.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_stTradeRedem.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}

	if( (0 != strcmp(m_stTradeRedem.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeRedem.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}

	// �õ����������Ϣ
	m_params.setParam("trade_id", m_stTradeRedem.Ftrade_id);
	m_params.setParam("close_id",m_stTradeRedem.Fclose_listid);
}


/**
  * ����ѿۼ��ɹ��Ķ��ڽ��׼�¼
  */
void FundRedemCloseAck::CheckAckSucTrans() throw (CException)
{
	//��ȡ���ڼ�¼( �����鲻������¼)
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,false)){
	    TRACE_ERROR("unfound fund_close_trans close_id");
	    throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "unfound fund_close_trans close_id");
	}
	// ���ȫ����صĹ�������
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_ALL_REDEM
		&&0!=strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)){
	    TRACE_ERROR("all redem close_trans sellid diff");
	    throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem close_trans sellid diff");
	}
	// ��鲿����ع�������
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM
		&&0==strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)){
			TRACE_ERROR("all redem fund trade exists, stTradeRedem opt_type invalid");
			throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem fund trade exists, stTradeRedem opt_type invalid");
	}
	// �������У����: 5״̬ʹ��total_fee���
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM&&m_stTradeRedem.Ftotal_fee != m_params.getLong("total_fee"))
	{
		TRACE_ERROR("fund redem, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
}


/**
  * �����˻�ǰ��鶨�ڽ��׼�¼
  */
void FundRedemCloseAck::CheckForUpdateSuc() throw (CException)
{
	if( REDEM_ININ != m_stTradeRedem.Fstate){
		//��Ϊ��س�ʼ״̬���������ٴθ��³ɹ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	
	//��ȡ���ڼ�¼
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,true)){
	    TRACE_ERROR("unfound fund_close_trans close_id");
	    throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "unfound fund_close_trans close_id");
	}
	// ���total_feeӦ�����
	if(m_stTradeRedem.Ftotal_fee != m_params.getLong("total_fee")){
		TRACE_ERROR("fund partial redem, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ",m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}

    // ���ʱ��    
	string dueDate=string(m_close_trans.Fdue_date);
	string dueTime=dueDate+"150000";	
	string bookStopDate=string(m_close_trans.Fbook_stop_date);
	string bookStopTime=bookStopDate+"150000"; 	
	if(m_params.getString("systime")>=changeDatetimeFormat(dueTime)||
		m_params.getString("systime")<changeDatetimeFormat(bookStopTime)){
		TRACE_ERROR("cannot redem close_trans right now");
		throw EXCEPTION(ERR_CLOSE_REDEM_TIME, "cannot redem close_trans right now");
	}
	// �ѷ���ɨβ���,�����ٷ���������ʽ���
	if(m_close_trans.Fsell_listid[0]!=0){
		TRACE_ERROR("close_trans sell_listid exists, cannot redem anymore");
		throw EXCEPTION(ERR_CLOSE_REDEM_HAS_ALL_REDEM, "close_trans sell_listid exists, cannot redem anymore");
	}
	// ԤԼ������غ�ȫ��˳�Ӳſ��Է������
	if(m_close_trans.Fuser_end_type!=CLOSE_FUND_END_TYPE_PATRIAL_REDEM
		&&m_close_trans.Fuser_end_type!=CLOSE_FUND_END_TYPE_ALL_EXTENSION){
		TRACE_ERROR("close_trans user_end_type cannot redem anymore");
		throw EXCEPTION(ERR_CLOSE_REDEM_USER_END, "close_trans user_end_type cannot redem anymore");
	}
	// ���״̬
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING&&m_close_trans.Fstate!=CLOSE_FUND_STATE_REDEM_SUC){
		TRACE_ERROR("close_trans status cannot redem");
		throw EXCEPTION(ERR_CLOSE_REDEM_STATE, "close_trans status cannot redem");
	}
	// ���ʣ����
	LONG leftFee=m_close_trans.Fcurrent_total_fee;
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_PATRIAL_REDEM&&m_close_trans.Fstate==CLOSE_FUND_STATE_PENDING){
		leftFee=leftFee-m_close_trans.Fend_plan_amt;
	}
	if(leftFee<m_params.getLong("total_fee")){		
		TRACE_ERROR("close_trans status cannot redem");
		throw EXCEPTION(ERR_CLOSE_REDEM_NOT_ENOUGH_MONEY, "close_trans status cannot redem");
	}
	//ɨβ��ؽ��Ӧ�õ���ʣ��ȫ�����
	if(leftFee!=m_params.getLong("total_fee")&&m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		TRACE_ERROR("close_trans all redem money[%ld][%ld] invalid",leftFee,m_params.getLong("total_fee"));
		throw EXCEPTION(ERR_CLOSE_REDEM_INVALID_MONEY, "close_trans all redem money invalid");
	}
	// ���ԤԼ��ؽ�ֹ�յ����������Ƿ��Ѿ����
	string bookStopProfitTime=bookStopDate+gPtrConfig->m_AppCfg.close_redem_ack_stop_time;
	if(m_params.getString("systime")>=changeDatetimeFormat(bookStopProfitTime)){
		FundReconLog recon;
		memset(&recon,0,sizeof(FundReconLog));
		recon.Frecon_type=RECON_TYPE_PROFIT;
		strncpy(recon.Fspid,m_close_trans.Fspid,sizeof(recon.Fspid));
		strncpy(recon.Frecon_date,m_close_trans.Fbook_stop_date,sizeof(recon.Frecon_date));
		bool existRecon = queryFundSpReconLog(m_pFundCon,recon);
		if(!existRecon){
			TRACE_ERROR("profit is recording,cannot redem right now[%s][%s]",m_close_trans.Fspid,m_close_trans.Fbook_stop_date);
			throw EXCEPTION(ERR_CLOSE_REDEM_PROFIT_RECORDING, "profit is recording,cannot redem right now");
		}
	}
}


/**
*�����������ͣ�����Ӧ�Ĵ��� 
*/
void FundRedemCloseAck::UpdateTradeState()
{
	switch (m_optype)
    {       
        case INF_REDEM_SP_ACK_SUC:
			UpdateRedemTradeForSuc();
            break;
			
		case INF_REDEM_SP_ACK_TIMEOUT:
            UpdateRedemTradeForTimeout();
            break;
			
		case INF_REDEM_SP_ACK_FAIL:
            UpdateRedemTradeForFail();
            break;

		case INF_REDEM_SP_ACK_FINISH:
            UpdateRedemTradeForFinish();
            break;
                    
        default:
            throw CException(ERR_BAD_PARAM, "op_type invalid", __FILE__, __LINE__);
            break;

    }
}

/**
*	������˾��سɹ�����
*	�������˻����
*/
void FundRedemCloseAck::UpdateRedemTradeForSuc() throw (CException)
{
	if(TRADE_RECORD_TIMEOUT == m_stTradeRedem.Fspe_tag){
		//��ʱ�����Ķ���ֱ��ȥ����
		UpdateRedemTradeForBudan();
		return;
	}
	
	if(REDEM_SUC == m_stTradeRedem.Fstate || REDEM_FINISH == m_stTradeRedem.Fstate){
		// ������
		CheckAckSucTrans();
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}
	// �������˻�ǰ��鵥״̬
	CheckForUpdateSuc();

	//�ȼ��˻��������ʧ��ֱ�ӱ����ع�����
	doDraw();

	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	if(INF_REDEM_SP_ACK_TIMEOUT == m_optype){
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
	}else{
		stRecord.Fspe_tag = 0;//��ʱ�����ɹ���Ҫ����ʱ״̬�޸ģ������²�ͣ����
	}
	stRecord.Fstate = REDEM_SUC;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fuid = m_stTradeRedem.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	stRecord.Fpurpose = m_stTradeRedem.Fpurpose;
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);

	FundCloseTrans closeTrans;
	closeTrans.Fid = m_close_trans.Fid;
	strncpy(closeTrans.Ftrade_id, m_close_trans.Ftrade_id, sizeof(closeTrans.Ftrade_id)-1);
	strncpy(closeTrans.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(closeTrans.Fmodify_time)-1);
	// ɨβ��ؼ�¼����
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		strncpy(closeTrans.Fsell_listid, m_stTradeRedem.Flistid, sizeof(closeTrans.Fsell_listid)-1);
	}
	closeTrans.Fcurrent_total_fee=m_close_trans.Fcurrent_total_fee-m_stTradeRedem.Ftotal_fee;
	closeTrans.Fend_real_sell_amt=m_close_trans.Fend_real_sell_amt+m_stTradeRedem.Ftotal_fee;
	
	
	//���׼�¼��MQ,��װ�仯�Ĳ���
	m_stTradeRedem.Fstate= stRecord.Fstate;
	strncpy(m_stTradeRedem.Fcoding, stRecord.Fcoding, sizeof(m_stTradeRedem.Fcoding) - 1);
	strncpy(m_stTradeRedem.Fmemo, stRecord.Fmemo, sizeof(m_stTradeRedem.Fmemo) - 1);
	//�����˻��ɹ�������ҵ��ɹ���������ص�ʧ�ܷ���������������ɹ�
	try
	{
		// ���½��׵�
		UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));
		// ���¶��ڼ�¼
		saveFundCloseTrans(closeTrans,m_close_trans,m_stTradeRedem.Flistid,PURTYPE_REDEEM);
		// ͬ���ڴ�����
		m_close_trans.Fcurrent_total_fee = closeTrans.Fcurrent_total_fee;
		m_close_trans.Fend_real_sell_amt = closeTrans.Fend_real_sell_amt;

		//�����˻����͸��µ�����һ���������֤һ����ֻ����һ��
		//recordFundTotalaccDraw();
		
		//��������˾���
		checkSpLoaning();
		
		//���³ɹ�, ��Ҫ�ۼ�exau
		m_need_updateExauAuthLimit = true;
	}
	catch(CException& e)
	{
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// ����Ϣ�����
		callErrorRpc(m_request, gPtrSysLog);
		
		if(ERR_TYPE_MSG)
		{
			//���Բ����ʱ�����ص�����ʱ�䳬��10���ӵĸ澯
			int inteval = (int)(time(NULL) - toUnixTime(m_stTradeRedem.Facc_time));	
			gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
			//����10������δ�ɹ��澯,�澯���������ѹ��
			if(inteval >= 600 )
			{	
				char szErrMsg[256] = {0};
		        snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.��ز��������%d����δ�ɹ�" ,inteval);
		        
		        alert(ERR_BUDAN_TOLONG, szErrMsg);
			}	
			throw;//���Բ������ֱ���׳�
		}else{
			// �ǲ��ֱ�ӷ��سɹ�, ���ع�����
			throw CException(ERR_CLOSE_REDEM_DRAW_SUCC, "draw success need retry! ", __FILE__, __LINE__);
		}
	}
	catch(...)
	{
		//��Ӧ�÷����������˸澯
		char szErrMsg[256] = {0};
        snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.update close redem trade for success unexception.");
        
        alert(ERR_DB_UNKNOW, szErrMsg);
	}
	
	//���³ɹ�,�����û���������, ֻ����,�ǹؼ���Ϣ.����ʧ�ܲ��ع�
	//ʹ��forupdate ���û�����ֹ�����깺�޸�user_acc��Ϣ
	try{
		ST_FUND_BIND fund_bind; 
		QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &fund_bind, true);
		updateUserAcc(m_stTradeRedem);
	}catch(CException& e)
	{TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());}

}

void FundRedemCloseAck::checkSpLoaning() throw (CException)
{

    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, m_stTradeRedem.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stTradeRedem.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//��Ӧ�÷���
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    if ((fundSpConfig.Fredem_valid&0x07) ==2) // ֹͣ���
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
	
	/** ������**/
	/** �������ȫ������,��������沿��**/
	/** �㷨: ���������,����ر���**/
	LONG redemScope;
	// ����������,��ǰ�����ڱ���:ֻ�������, ���ۼ����
	if(m_close_trans.Fcurrent_total_fee>=m_close_trans.Fstart_total_fee){
		redemScope=0;
	// �������ǰ,��ǰ����Ѿ�С�ڱ���:֮ǰ�����Ѿ������,���ȫ���ۼ�
	}else if(m_params.getLong("total_fee")+m_close_trans.Fcurrent_total_fee<=m_close_trans.Fstart_total_fee){
		redemScope=m_params.getLong("total_fee");
	// ������ذ�����������,���ֱ���:ֻ��ر�����ٲ���
	}else{
		redemScope=m_close_trans.Fstart_total_fee-m_close_trans.Fcurrent_total_fee;
	}
	
    //��������ۼƶ��,t+1������ز��ۼƣ�����������ͨ��ء����ѡ�t+0��ض�Ҫ�ۼ�
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedom(m_pFundCon, fundSpConfig, m_stTradeRedem.Ftotal_fee,redemScope,m_stTradeRedem.Floading_type,m_stTradeRedem.Facc_time);
	
}


/**
*��س�ʱ����
*/
void FundRedemCloseAck::UpdateRedemTradeForTimeout() throw (CException)
{
	//�������þ�����س�ʱ���ɹ�����ʧ�ܣ�������ɹ�������Ҫ�����������ܳ���Գ�ʱ��ص����в���
	if("true" == gPtrConfig->m_AppCfg.redem_timeout_conf)
	{
		UpdateRedemTradeForSuc();
	}
	else
	{
		UpdateRedemTradeForFail();
	}
}

/**
*	���ʧ�ܴ���
*/
void FundRedemCloseAck::UpdateRedemTradeForFail() throw (CException)
{
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	if(REDEM_FAIL == m_stTradeRedem.Fstate)
	{
		//�������
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_ININ != m_stTradeRedem.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
	}

	stRecord.Fstate = REDEM_FAIL;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fuid = m_stTradeRedem.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	//����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);
    
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));
}

void FundRedemCloseAck::UpdateRedemTradeForFinish() throw (CException)
{
	// У��ؼ�����:����״̬���ʹ����ʵ���
	LONG checkFee=m_stTradeRedem.Freal_redem_amt>0?m_stTradeRedem.Freal_redem_amt:m_stTradeRedem.Ftotal_fee;
	if(checkFee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			checkFee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}	
	
	if(REDEM_FINISH == m_stTradeRedem.Fstate)
	{
		//����
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_SUC != m_stTradeRedem.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	ST_TRADE_FUND  stRecord;
	
	// ��ص����ֱ�Ӹ��µ���ʱ��͵���״̬
	if(m_stTradeRedem.Fpurpose==PURPOSE_REDEM_TO_BA||m_stTradeRedem.Fpurpose==PURPOSE_REDEM_TO_BA_T1)
	{
		strncpy(stRecord.Ffetch_arrival_time,m_params.getString("systime").c_str(),sizeof(stRecord.Ffetch_arrival_time));
		stRecord.Ffetch_result=FETCH_RESULT_BALANCE_SUCCESS;
	}
	
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fstate = REDEM_FINISH;
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fspe_tag = m_stTradeRedem.Fspe_tag; //�����޸ĳ�ʱ���
	stRecord.Fuid = m_stTradeRedem.Fuid;
	//����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);
    
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));

}

void FundRedemCloseAck::UpdateRedemTradeForBudan() throw (CException)
{
	// У��ؼ�����:�̻�����ʹ��total_feeУ����
	if(m_stTradeRedem.Ftotal_fee!= m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	if(REDEM_SUC != m_stTradeRedem.Fstate && REDEM_FINISH != m_stTradeRedem.Fstate)
	{
		//���ݿ�״̬���ܸ���
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(TRADE_RECORD_TIMEOUT != m_stTradeRedem.Fspe_tag || INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		//�ǳ�ʱ�������������,���߳�ʱ�����룬ֱ�ӷ���
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}	

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fspe_tag = 0;
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fuid = m_stTradeRedem.Fuid;
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));
}

static bool payNotifyOvertime(string pay_suc_time)
{
	if(pay_suc_time.size() == 14)
	{
		//YYYYMMDDHHMMSS תYYYY-MM-DD HH:MM:SS
		pay_suc_time = changeDatetimeFormat(pay_suc_time);
	}
	int pay_time = toUnixTime(pay_suc_time.c_str());
	if(pay_time + gPtrConfig->m_AppCfg.paycb_overtime_inteval < (int)(time(NULL)) )
	{
		return true;	
	}

	return false;
}


/**
 * ���˻���������
 * ���˻�ʧ���˲��ܲ�������û����������ٷ�����أ�������������ػ���ɲƸ�ͨ��ʧ
 * д������ģ���ص������Ի���˾�ɹ�Ϊ׼�����������˻�Ϊ׼��һֱ�����ۣ�ǰ�����۶������ķ������������������˾����ó���˵�¡�����
 * ����漰���ϵͳ ����˾+�����ֵϵͳ+���˻�+�Ƹ�ͨ +��������ز�����Ƶ�ϵͳ
 * һ�����ԭ���Ǽ�Ǯ�ɹ��ӻ����׵��ɹ�����Ϊ�ɹ���������ʧ�ܶ�ͨ����������ɡ�
 * ��Ǯ�漰������ϵͳ������˾ϵͳ���ݶ���˻���Ǯ���������ĸ�Ϊ׼? ��˭Ϊ׼��������:
 * ������Ϊ�����˻��ɹ�Ϊ׼��Ϊ���ף�����: 1)���˻��ǲƸ�ͨ�ɿ��Ƶģ��ɹ�����ɻ���˾������2)���˻��쳣�����ǿ���ͨ��ʵʱ���˷��֣�����ʱ������
 * ���ڵ�����:���˻�ʧ��δ��Ǯ������˾�Ѿ����ݶ�û��޷��ٴ���أ�����ͨ���ڶ��������˾�Ķ��˰��û��ʽ��ϣ����������Ż��������൥ʵʱ֪ͨ����˾���ʧ�ܣ�
 *
 */
void FundRedemCloseAck::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stTradeRedem.Fsub_trans_id);

	try
	{
	    SubaccDraw(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeRedem.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeRedem.Facc_time);
	}
	
	catch(CException& e)
	{

		//���Ҫ��������
		//���������˻���Ǯ����10����û�ɹ��ĸ澯���ڲ����������ǲ���������ⲿ���ܲ�����10����û�ɹ����쳣ʱ���ᴥ���澯
		if(payNotifyOvertime(m_stTradeRedem.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.��ز�������10�������˻���δ�ɹ�");		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// ����Ϣ�����
		callErrorRpc(m_request, gPtrSysLog);

		throw;//ֱ���׳��쳣����ֹ�������ִ�У����ܰ���ص����ɹ�
		
	}
	
}

void FundRedemCloseAck::updateCkvs()
{
	if(!m_need_updateExauAuthLimit)
	{
		return;
	}
	//���¶��ڼ�¼CKV
	setFundCloseTransToKV(m_close_trans.Ftrade_id,m_close_trans.Ffund_code);
}

void FundRedemCloseAck::updateExauAuthLimitNoExcp()
{
	if(!m_need_updateExauAuthLimit)
	{
		return;
	}

	// T+0  ������ز��޶�(����T+1��ز��޶�)
	if (m_stTradeRedem.Floading_type == 0)
	{
		return;
	}

	//�ۼ�exau ���������⣬���ⳬʱ���µ�����ع�
	//��������ʱ����exau���ƣ��ۼ�ʱʧ�ܲ������������ۼӣ�����ص��޷�����ɹ�������
	try
	{
		//�ۼ��û�����޶�
		int redem_type = (m_stTradeRedem.Floading_type == 0?DRAW_ARRIVE_TYPE_T1:DRAW_ARRIVE_TYPE_T0);
		updateExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,redem_type);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}


void FundRedemCloseAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	// ���½��ɹ��ŷ�MQ�������η���
	if(!m_need_updateExauAuthLimit)
	{
		return;
	}
	char szMsg[MAX_MSG_LEN + 1] = {0};

    // ��װ�ؼ�����
    CUrlAnalyze::setParam(szMsg, "Flistid", fundTradeBuy.Flistid, true);
    CUrlAnalyze::setParam(szMsg, "Fspid", fundTradeBuy.Fspid);
    CUrlAnalyze::setParam(szMsg, "Fcoding", fundTradeBuy.Fcoding);
	CUrlAnalyze::setParam(szMsg, "Ftrade_id", fundTradeBuy.Ftrade_id);
	CUrlAnalyze::setParam(szMsg, "Fuid", fundTradeBuy.Fuid);
	CUrlAnalyze::setParam(szMsg, "Ffund_code", fundTradeBuy.Ffund_code);
	CUrlAnalyze::setParam(szMsg, "Fpur_type", fundTradeBuy.Fpur_type);
	CUrlAnalyze::setParam(szMsg, "Ftotal_fee", fundTradeBuy.Ftotal_fee);
	CUrlAnalyze::setParam(szMsg, "Fstate", fundTradeBuy.Fstate);
	CUrlAnalyze::setParam(szMsg, "Ftrade_date", fundTradeBuy.Ftrade_date);
	CUrlAnalyze::setParam(szMsg, "Ffund_vdate", fundTradeBuy.Ffund_vdate);
	CUrlAnalyze::setParam(szMsg, "Fcreate_time", fundTradeBuy.Fcreate_time);
	CUrlAnalyze::setParam(szMsg, "Fmodify_time", fundTradeBuy.Fmodify_time);
	CUrlAnalyze::setParam(szMsg, "Fcft_trans_id", fundTradeBuy.Fcft_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_charge_ctrl_id", fundTradeBuy.Fcft_charge_ctrl_id);
	CUrlAnalyze::setParam(szMsg, "Fsp_fetch_id", fundTradeBuy.Fsp_fetch_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_bank_billno", fundTradeBuy.Fcft_bank_billno);
	CUrlAnalyze::setParam(szMsg, "Fsub_trans_id", fundTradeBuy.Fsub_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcur_type", fundTradeBuy.Fcur_type);
	CUrlAnalyze::setParam(szMsg, "Fpurpose", fundTradeBuy.Fpurpose);
	CUrlAnalyze::setParam(szMsg, "Facc_time", fundTradeBuy.Facc_time);
	CUrlAnalyze::setParam(szMsg, "Fchannel_id", fundTradeBuy.Fchannel_id);
	CUrlAnalyze::setParam(szMsg, "Fmemo", fundTradeBuy.Fmemo);

	sendMsg2Mq(szMsg);
}


/**
  * ����������
  */
void FundRedemCloseAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "fetch_type",DRAW_ARRIVE_TYPE_T1);
    CUrlAnalyze::setParam(rqst->odata, "loading_type", DRAW_NOT_USE_LOADING);

    rqst->olen = strlen(rqst->odata);
    return;
}


