/**
  * FileName: fund_redem_close_req_service.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2014-9-6
  * Description: �����׷��� ���ڲ�Ʒʵʱ������� Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_redem_close_req_service.h"

FundRedemCloseReq::FundRedemCloseReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
	memset(&m_stTradeRedem, 0, sizeof(ST_TRADE_FUND));
	memset(&m_close_trans, 0, sizeof(FundCloseTrans));

	m_redemTradeExist = false;

}

/**
  * service step 1: �����������
  */
void FundRedemCloseReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_close_req_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_charge_ctrl_id", 0, 32);
	// ���������˻����ֵ���
	// m_params.readStrParam(szMsg, "sp_fetch_id", 0, 32);
	m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
    m_params.readIntParam(szMsg, "fetch_type", 0,DRAW_ARRIVE_TYPE_T1);
	m_params.readStrParam(szMsg, "spid", 10, 15);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "purpose", 0,4);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
	m_params.readLongParam(szMsg, "close_id", 0, MAX_LONG);
	m_params.readIntParam(szMsg, "opt_type", 0,3); // 1:ָ��������, 2:ȫ�����
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * ���ɻ���ע����token
 */
string FundRedemCloseReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uid|cft_bank_billno|spid|sp_billno|total_fee|key
    // ��������ԭ��
    ss << m_params["uid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundRedemCloseReq::CheckToken() throw (CException)
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
void FundRedemCloseReq::CheckParams() throw (CException)
{	
	// ��֤�����ǿ�
	CHECK_PARAM_EMPTY("cft_bank_billno");
	CHECK_PARAM_EMPTY("cft_trans_id");
	CHECK_PARAM_EMPTY("cft_fetch_id");
	CHECK_PARAM_EMPTY("cft_charge_ctrl_id");
	CHECK_PARAM_EMPTY("close_id");
	CHECK_PARAM_EMPTY("opt_type");
    // ��֤token
    CheckToken();

	//���spid ��fund_code �Ƿ���Ч
	strncpy(m_fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_sp_config.Fspid) - 1);
	strncpy(m_fund_sp_config.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_sp_config.Ffund_code) - 1);
	checkFundSpAndFundcode(m_pFundCon,m_fund_sp_config, false);//��ǿ�Ʊ�������Ч����˾���û��ѿ�ͨ�˸û���˾����ʹ�ô����ƵĻ���˾

	if(m_fund_sp_config.Fclose_flag!=CLOSE_FLAG_ALL_CLOSE){
        throw CException(ERR_BAD_PARAM, "�ýӿ�ֻ֧�ֶ��ڲ�Ʒ", __FILE__, __LINE__);
	}

	//����Ƿ�֧����ص��Ƹ�ͨ���
	if(m_params.getInt("purpose") == PURPOSE_DEFAULT && !(gPtrConfig->m_AppCfg.support_redem_to_cft == 1)){
		throw CException(ERR_CANNOT_REDEM_TO_CFT, "not support redeem to cft balance.", __FILE__, __LINE__);
	}
	//ֻ֧��T+1���,Ĭ��T+1
	if(m_params.getString("fetch_type").empty()){
		m_params.setParam("fetch_type",DRAW_ARRIVE_TYPE_T1);
	}
    if(m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1){
        throw CException(ERR_BAD_PARAM, "����ʵʱ���ֻ֧��T+1.", __FILE__, __LINE__);
    }

	if ((m_fund_sp_config.Fredem_valid&0x07) ==2) // ֹͣ���
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
}


/**
  * ִ���깺����
  */
void FundRedemCloseReq::excute() throw (CException)
{
    try
    {
        CheckParams();
		

        /* ������󶨼�¼ */
        CheckFundBind();

		/* �������˻��󶨻���˾�����˻���¼ */
		CheckFundBindSpAcc();

         /* �������� */
        m_pFundCon->Begin();

		/** ��ѯ��ؽ��׼�¼ 
		* Ϊ��֧����ز������룬�����Ȳ�ѯ���Ƿ����
		*/
        CheckFundTrade();

		//��cgi �����ʱ��飬�������Ҳ�޷���ȫ��������
		//�޷ݶ����˾�����ʧ�ܣ���ʱ����˾�������������ȷ��ʱ�����˻����Ҳ��ʧ��
		CheckFundBalance();

		//�������޶�
		CheckAuthLimit();

        /* ��¼��ؽ��׼�¼ */
        RecordFundTrade();

        /* �ύ���� */
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }
}

void FundRedemCloseReq::CheckAuthLimit() throw (CException)
{
	UserClassify::UT user_type = g_user_classify->getUserType(m_params.getString("uin"));
	//�ǰ������û���������޶�
	if (user_type == UserClassify::NORMAL || m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1)
	{
		//�������޶�
		checkExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,m_params.getInt("fetch_type"));
	}
}


/*
 * ��ѯ�����˻��Ƿ���ڣ��Լ���֤������һ����
 */
void FundRedemCloseReq::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }

	// ��¼���ڣ�������¼�е�trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
	m_params.setParam("uin", m_fund_bind.Fqqid);
}

/*
*����Ƿ�󶨻���˾�ʺ�
*/
void FundRedemCloseReq::CheckFundBindSpAcc() throw (CException)
{
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	//��ز������Ʊ������������ʺţ�ֻ�����깺
	//queryValidMasterSpAcc(m_pFundCon, m_fund_bind_sp_acc,m_params.getString("spid"), false);
	queryValidFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);

}

/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundRedemCloseReq::CheckFundTrade() throw (CException)
{
	// ��鶨�ڼ�¼����
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,true)){
		TRACE_ERROR("unfound fund_close_trans close_id=%ld,trade_id=%s", m_close_trans.Fid,m_close_trans.Ftrade_id);
		throw EXCEPTION(ERR_BAD_PARAM, "unfound fund_close_trans");
	}
	// �ж��û�
	if(strcmp(m_params.getString("trade_id").c_str(),m_close_trans.Ftrade_id)!=0){	
		TRACE_ERROR("close_trans[%ld]trade_id[%s][%s] is different", m_close_trans.Fid,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "close_trans trade_id is different");
	}
	// �ж��̻��Ų���
	if(0!=strcmp(m_close_trans.Fspid,m_params.getString("spid").c_str())){
		TRACE_ERROR("close_trans[%ld] spid[%s] is not equal to input[%s]", m_close_trans.Fid,m_close_trans.Fspid,m_params.getString("spid").c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "close_trans spid is different");
	}
    m_redemTradeExist = QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeRedem, false);
	
	gPtrAppLog->debug("fund redem req trade record exist : %d",m_redemTradeExist);
	
	// ����ؼ�¼,������
	if(m_redemTradeExist){
		m_params.setParam("fund_trans_id", m_stTradeRedem.Flistid);
		CheckFundTradeRepeat();
		return;
	}
	/**
        * û����ؼ�¼,�״�������
        **/
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
		TRACE_ERROR("close_trans user_end_type[%d] cannot redem",m_close_trans.Fuser_end_type);
		throw EXCEPTION(ERR_CLOSE_REDEM_USER_END, "close_trans user_end_type cannot redem");
	}
	// ���״̬
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING&&m_close_trans.Fstate!=CLOSE_FUND_STATE_REDEM_SUC){
		TRACE_ERROR("close_trans status[%d] cannot redem",m_close_trans.Fstate);
		throw EXCEPTION(ERR_CLOSE_REDEM_STATE, "close_trans status cannot redem");
	}
	// ���ʣ����
	LONG leftFee=m_close_trans.Fcurrent_total_fee;
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_PATRIAL_REDEM&&m_close_trans.Fstate==CLOSE_FUND_STATE_PENDING){
		leftFee=leftFee-m_close_trans.Fend_plan_amt;
	}
	if(leftFee<m_params.getLong("total_fee")){		
		TRACE_ERROR("close_trans has not enough money[%ld] to redem[%ld]",leftFee,m_params.getLong("total_fee"));
		throw EXCEPTION(ERR_CLOSE_REDEM_NOT_ENOUGH_MONEY, "close_trans has not enough money to redem");
	}
	//ɨβ��ؽ��Ӧ�õ���ʣ��ȫ�����
	if(leftFee!=m_params.getLong("total_fee")&&m_params.getInt("opt_type")==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		TRACE_ERROR("close_trans all redem money[%ld][%ld] invalid",leftFee,m_params.getLong("total_fee"));
		throw EXCEPTION(ERR_CLOSE_REDEM_INVALID_MONEY, "close_trans all redem money invalid");
	}
	// ��鵱ǰ�Ƿ���δ��ɵ�ͬ�ڴ���ص�(������ԤԼ��ص�)
	bool existsInitRedem = isCloseInitRedemExists(m_pFundCon,m_close_trans.Fid,m_close_trans.Fuid,m_close_trans.Ffund_code,bookStopTime.c_str(),dueTime.c_str());
	if(existsInitRedem){
		TRACE_ERROR("exists unfinished redem[%ld][%d]",m_close_trans.Fid,m_close_trans.Fuid);
		throw EXCEPTION(ERR_CLOSE_REDEM_EXISTS_INIT_REDEM, "exists unfinished redem");
	}
	
	// ���ԤԼ��ؽ�ֹ�յ����������Ƿ��Ѿ����
	string bookStopProfitTime=bookStopDate+gPtrConfig->m_AppCfg.close_redem_req_stop_time;
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
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundRedemCloseReq::CheckFundTradeRepeat() throw (CException)
{
	if(0 != strcmp(m_stTradeRedem.Fspid, m_params.getString("spid").c_str())){
		TRACE_ERROR("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeRedem.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}
	if(0 != strcmp(m_stTradeRedem.Ffund_code, m_params.getString("fund_code").c_str())){
		TRACE_ERROR("fund trade exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_stTradeRedem.Ffund_code, m_params.getString("fund_code").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fund_code is different!", __FILE__, __LINE__);
	}

	if(m_stTradeRedem.Ftotal_fee != m_params.getLong("total_fee")){
		TRACE_ERROR("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, total_fee is different!", __FILE__, __LINE__);
	}

	if(0 != strcmp(m_stTradeRedem.Fcft_trans_id, m_params.getString("cft_trans_id").c_str())){
		TRACE_ERROR("fund trade exists, cft_trans_id is different! cft_trans_id in db[%s], cft_trans_id input[%s] ", 
			m_stTradeRedem.Fcft_trans_id, m_params.getString("cft_trans_id").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, cft_trans_id is different!", __FILE__, __LINE__);
	}
    
    if (m_stTradeRedem.Floading_type != DRAW_NOT_USE_LOADING){
        TRACE_ERROR("fund trade exists, Floading_type is confict! Floading_type in db[%d], fetch_type input[%s] ", 
    			m_stTradeRedem.Floading_type, m_params.getString("fetch_type").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fetch_type is confict!", __FILE__, __LINE__);
    }
	if(LSTATE_INVALID == m_stTradeRedem.Flstate){
		TRACE_ERROR("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeRedem.Flistid, m_stTradeRedem.Ftrade_id, m_stTradeRedem.Fpur_type);
		throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
	}
	if(m_params.getInt("opt_type")!=m_stTradeRedem.Fopt_type){
		TRACE_ERROR("all redem fund trade exists, opt_type diff");
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, opt_type diff");
	}
	if(m_params.getLong("close_id")!=m_stTradeRedem.Fclose_listid){
		TRACE_ERROR("all redem fund trade exists, close_id diff");
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, close_id diff");
	}
	if(m_close_trans.Fuid!=m_stTradeRedem.Fuid){
		TRACE_ERROR("all redem fund trade exists, uid diff");
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, uid diff");
	}
	
	// ɨβ�������������
	if(m_params.getInt("opt_type")==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		// �����������
		if(m_close_trans.Fsell_listid[0]==0&&m_stTradeRedem.Fstate==REDEM_ININ){
			gPtrAppLog->debug("all redem fund req has finished");
		// ���ȷ������
		}else if(0==strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)
			&&m_stTradeRedem.Fstate==REDEM_SUC){
			gPtrAppLog->debug("all redem fund ack has finished");
		}else{
			TRACE_ERROR("all redem fund trade exists, relate close_trans invalid");
			throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem fund trade exists, relate close_trans invalid");
		}
	}
	// �������������
	if(m_params.getInt("opt_type")==FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM
		&&0==strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)){
			TRACE_ERROR("all redem fund trade exists, input opt_type invalid");
			throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem fund trade exists, input opt_type invalid");
	}

    if(m_stTradeRedem.Fstate == REDEM_ININ || m_stTradeRedem.Fspe_tag == TRADE_RECORD_TIMEOUT)
    {
		//�ɹ�����ʱ�ĵ���Ҳ����������
		throw CException(ERR_REPEAT_ENTRY, "fund redem trade exist. ", __FILE__, __LINE__);
    }
	else if(m_stTradeRedem.Fstate == REDEM_SUC)
	{
		//������˾����ѳɹ��ĵ����뱨��������룬����ǰ�û�ʶ����ٷ��𵽻���˾������
		throw CException(ERR_REDEM_SP_SUC_REPEAT_ENTRY, "fund redem from sp success. ", __FILE__, __LINE__);
	}
	else
    {
		//����״̬����ز���������
		throw CException(ERR_REDEM_REPEAT_ENTRY, "fund redem trade exist.but state cannot be redem now()", __FILE__, __LINE__);
    }
}

/**
* ��ѯ���ͨ�˻����
*/
void FundRedemCloseReq::CheckFundBalance()
{
	//TODO ���������ĸ��ֶ�,���˻�ֻ�������ˣ���ʱ�����ڶ��Ჿ��
	//TODO �ж����ʽ������ݶ�ת��
	LONG balance = querySubaccBalance(m_params.getInt("uid"),querySubaccCurtype(m_pFundCon, m_params.getString("spid")));

	if(balance < m_params.getLong("total_fee"))
	{
		throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
	}

	//����Ƿݶ�ת����أ�����ʽ������˻��ʽ�һ��
	if(m_params.getInt("purpose") == PURPOSE_CHANGE_SP && balance != m_params.getLong("total_fee"))
	{
		throw CException(ERR_MUST_REDEM_ALL, "change sp must redeem all original fund.", __FILE__, __LINE__);
	}
}



/**
  * ���ɻ�����ؼ�¼��״̬: ��ʼ���״̬
  */
void FundRedemCloseReq::RecordFundTrade()
{
	if(m_redemTradeExist){
		return;
	}
	string drawid = genSubaccDrawid();
	string cft_bank_billno = m_params.getString("cft_bank_billno");
	//��ص����ڲ�����  10�̻���+8λ����+10���к�+cft_bank_billno��3λ����֤cft_bank_billno��listid�ķֿ�ֱ����һ�£������ڲ�ֱ�
	string listid =  m_params.getString("spid") + drawid + cft_bank_billno.substr(cft_bank_billno.size()-3);
	m_params.setParam("fund_trans_id", listid);

    strncpy(m_stTradeRedem.Flistid, listid.c_str(), sizeof(m_stTradeRedem.Flistid)-1);
    strncpy(m_stTradeRedem.Fspid, m_params.getString("spid").c_str(), sizeof(m_stTradeRedem.Fspid)-1);
    strncpy(m_stTradeRedem.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(m_stTradeRedem.Fcoding)-1);
    strncpy(m_stTradeRedem.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_stTradeRedem.Ftrade_id)-1);
    m_stTradeRedem.Fuid = m_params.getInt("uid");
    strncpy(m_stTradeRedem.Ffund_name, m_fund_sp_config.Ffund_name, sizeof(m_stTradeRedem.Ffund_name)-1);
    strncpy(m_stTradeRedem.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_stTradeRedem.Ffund_code)-1);
    m_stTradeRedem.Fbank_type = m_params.getInt("bank_type");
    strncpy(m_stTradeRedem.Fcard_no, m_params.getString("card_no").c_str(), sizeof(m_stTradeRedem.Fcard_no)-1);
    m_stTradeRedem.Fpur_type = PURTYPE_REDEEM;
    m_stTradeRedem.Ftotal_fee = m_params.getLong("total_fee");
    m_stTradeRedem.Fstate = REDEM_ININ;
    m_stTradeRedem.Flstate = LSTATE_VALID;
    //strncpy(m_stTradeRedem.Ftrade_date, m_params.getString("trade_date").c_str(), sizeof(m_stTradeRedem.Ftrade_date)-1);
    //strncpy(m_stTradeRedem.Ffund_value, m_params.getString("fund_value").c_str(), sizeof(m_stTradeRedem.Ffund_value)-1);
    //strncpy(m_stTradeRedem.Ffund_vdate, m_params.getString("fund_vdate").c_str(), sizeof(m_stTradeRedem.Ffund_vdate)-1);
    //strncpy(m_stTradeRedem.Ffund_type, m_params.getString("fund_type").c_str(), sizeof(m_stTradeRedem.Ffund_type)-1);
    //strncpy(m_stTradeRedem.Fnotify_url, m_params.getString("notify_url").c_str(), sizeof(m_stTradeRedem.Fnotify_url)-1);
    //strncpy(m_stTradeRedem.Frela_listid, "", sizeof(m_stTradeRedem.Frela_listid)-1);
    //strncpy(m_stTradeRedem.Fdrawid, "buy_no_drawid", sizeof(m_stTradeRedem.Fdrawid)-1);
    strncpy(m_stTradeRedem.Ffetchid, m_params.getString("cft_fetch_id").c_str(), sizeof(m_stTradeRedem.Ffetchid)-1);
    m_stTradeRedem.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(m_stTradeRedem.Fcreate_time, m_params.getString("systime").c_str(), sizeof(m_stTradeRedem.Fcreate_time)-1);
    strncpy(m_stTradeRedem.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_stTradeRedem.Fmodify_time)-1);
    //�Ƹ�ͨ�����Ƿ�֧������? m_stTradeRedem.Fstandby1 = 1; // ������¼
    m_stTradeRedem.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));; // ��������
    strncpy(m_stTradeRedem.Facc_time, m_params.getString("systime").c_str(), sizeof(m_stTradeRedem.Facc_time)-1);
	strncpy(m_stTradeRedem.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(m_stTradeRedem.Fcft_trans_id)-1);
	strncpy(m_stTradeRedem.Fcft_charge_ctrl_id, m_params.getString("cft_charge_ctrl_id").c_str(), sizeof(m_stTradeRedem.Fcft_charge_ctrl_id)-1);
	strncpy(m_stTradeRedem.Fsp_fetch_id, m_params.getString("sp_fetch_id").c_str(), sizeof(m_stTradeRedem.Fsp_fetch_id)-1);
	strncpy(m_stTradeRedem.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(m_stTradeRedem.Fcft_bank_billno)-1);
	strncpy(m_stTradeRedem.Fsub_trans_id, drawid.c_str(), sizeof(m_stTradeRedem.Fsub_trans_id)-1);
	m_stTradeRedem.Fpurpose = m_params.getInt("purpose");
	strncpy(m_stTradeRedem.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(m_stTradeRedem.Fchannel_id)-1);

    m_stTradeRedem.Floading_type = (m_params.getInt("fetch_type")==DRAW_ARRIVE_TYPE_T1?DRAW_NOT_USE_LOADING:DRAW_USE_LOADING);	
    // Ffund_vdate T+1��������: ��ص�����Ϊ���浽����+1��
	string vDate = addDays(m_close_trans.Fprofit_end_date,1);
    strncpy(m_stTradeRedem.Ffund_vdate, vDate.c_str(), sizeof(m_stTradeRedem.Ffund_vdate)-1);

	// ������ؼ�¼	
	m_stTradeRedem.Fclose_listid = m_close_trans.Fid;
	m_stTradeRedem.Fopt_type = m_params.getInt("opt_type");
	//Freal_redem_amt��ǰ����֪��,����total�����˵�ʱ���ټ���ɨβ
	m_stTradeRedem.Freal_redem_amt =m_params.getLong("total_fee");
	strncpy(m_stTradeRedem.Fend_date,m_close_trans.Fend_date, sizeof(m_stTradeRedem.Fend_date) - 1);
    InsertTradeFund(m_pFundCon, &m_stTradeRedem);
    InsertTradeUserFund(m_pFundCon, &m_stTradeRedem);
}

/**
  * ����������
  */
void FundRedemCloseReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_redemTradeExist ? m_stTradeRedem.Facc_time : m_params.getString("systime").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
	CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_trans_id", m_params.getString("fund_trans_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "spid", m_params.getString("spid").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_name", m_fund_sp_config.Fsp_name);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_params.getString("fund_code").c_str());
	CUrlAnalyze::setParam(rqst->odata, "fund_name", m_fund_sp_config.Ffund_name);
	CUrlAnalyze::setParam(rqst->odata, "end_date", m_stTradeRedem.Fend_date);

    rqst->olen = strlen(rqst->odata);
    return;
}


