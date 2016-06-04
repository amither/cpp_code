/**
  * FileName: fund_transfer_req_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-16
  * Description: �ݶ�ת������ӿ�
  */

#include "fund_commfunc.h"
#include "fund_transfer_req_service.h"

FundTransferReq::FundTransferReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fund_bind_orisp_acc, 0, sizeof(FundBindSp));
    memset(&m_fund_bind_newsp_acc, 0, sizeof(FundBindSp));
    memset(&m_transferOrder, 0, sizeof(m_transferOrder));
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    memset(&m_fund_newsp_config, 0, sizeof(FundSpConfig));
}

/**
  * service step 1: �����������
  */
void FundTransferReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_transfer_req_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readStrParam(szMsg, "trade_id", 10,32);
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "fund_exchange_id", 10, 32); //ת������
    m_params.readStrParam(szMsg, "ori_spid", 10, 15);
    m_params.readStrParam(szMsg, "new_spid", 10, 15);
    m_params.readStrParam(szMsg, "ori_fund_code", 1, 64);
    m_params.readStrParam(szMsg, "new_fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "buy_id", 10, 32);
    m_params.readStrParam(szMsg, "redem_id", 10, 32);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * ���ɻ���ע����token
 */
string FundTransferReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����trade_id|uin|fund_exchange_id|Fori_spid|Fnew_spid|total_fee|key
    // ��������ԭ��
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["uin"] << "|" ;
    ss << m_params["fund_exchange_id"] << "|" ;
    ss << m_params["ori_spid"] << "|" ;
    ss << m_params["new_spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.transfer_reqkey;

    //TRACE_DEBUG("token src=%s", ss.str().c_str());
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundTransferReq::CheckToken() throw (CException)
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
void FundTransferReq::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();
    if(m_params.getString("fund_exchange_id").substr(0,10) != m_params.getString("ori_spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error check with ori_spid"); 
    }

    if(m_params.getString("buy_id").substr(0,10) != m_params.getString("new_spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input buy_id error check with new_spid"); 
    }
    
    if(m_params.getString("redem_id").substr(0,10) != m_params.getString("ori_spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input redem_id error check with ori_spid"); 
    }

    if(m_params.getLong("total_fee") > gPtrConfig->m_AppCfg.max_transfer_fee_one_time)
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input total_fee exceed max fee limit"); 
    }
    
    //����λͬtradeid
    if(m_params.getString("fund_exchange_id").substr(m_params.getString("fund_exchange_id").length()-3,3) 
          != m_params.getString("trade_id").substr(m_params.getString("trade_id").length()-3,3))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error check with trade_id"); 
    }

    //���spid ��fund_code �Ƿ���Ч
    strncpy(m_fund_orisp_config.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params.getString("ori_fund_code").c_str(), sizeof(m_fund_orisp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_orisp_config, false);
	if(m_fund_orisp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
	{	
		//���ڲ�Ʒ��ʱ�������
		throw EXCEPTION(ERR_BAD_PARAM, "Do not support the redemption of the Fund's");    
	}

    strncpy(m_fund_newsp_config.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_newsp_config.Fspid) - 1);
    strncpy(m_fund_newsp_config.Ffund_code, m_params.getString("new_fund_code").c_str(), sizeof(m_fund_newsp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_newsp_config, false);
}

/**
  * ִ���깺����
  */
void FundTransferReq::excute() throw (CException)
{
    try
    {
        CheckParams();

        // �������˻���¼ 
        CheckFundBind();

        //�������˻��󶨻���˾�����˻���¼ 
        CheckFundBindSpAcc();

        //����û����
        checkUserBalance();

        //����û�ת������
        checkTransferTimes();

        //���ת�������ܷݶ�����
        checkSpRedemRate();

		// �������Ƿ���Խ����깺 
		checkFundcodePurchaseValid(m_fund_newsp_config.Fbuy_valid);

		// ����޶� 
		checkFundcodeToScopeUpperLimit(m_fund_bind.Fuid,m_params["systime"],m_params.getLong("total_fee"), m_fund_newsp_config, true);

        // �������� /
        m_pFundCon->Begin();

        // ��ѯ��У��ת���� 
        CheckFundTransfer();

		/* ����û��Ƿ���Թ�������Ʋ�Ʒ */
		checkPermissionBuyCloseFund(m_fund_bind.Ftrade_id, m_fund_newsp_config, m_params.getString("systime"), true);

        // ��¼����ת����¼ 
        RecordFundTransfer();

        // �ύ���� */
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
}

/*
 * ��ѯ�����˻��Ƿ����
 */
void FundTransferReq::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    
    if (m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "check fund bind record trade_id fail! ", __FILE__, __LINE__);
    }
}

/*
*����Ƿ�󶨻���˾�ʺţ����ҿɽ���
*/
void FundTransferReq::CheckFundBindSpAcc() throw (CException)
{
    strncpy(m_fund_bind_orisp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_orisp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_orisp_acc.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_bind_orisp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_orisp_acc, false);

    strncpy(m_fund_bind_newsp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_newsp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_newsp_acc.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_bind_newsp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_newsp_acc, false);
}


/**
  * ������ת����¼�Ƿ��Ѿ�����
  */
void FundTransferReq::CheckFundTransfer() throw (CException)
{
    strncpy(m_transferOrder.Fchange_id,m_params["fund_exchange_id"],sizeof(m_transferOrder.Fchange_id)-1);
    bool bBuyTradeExist = queryFundTransfer(m_pFundCon, m_transferOrder, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", bBuyTradeExist);

    if(!bBuyTradeExist)
        return;

    // ���ؼ�����
    if( (0 != strcmp(m_transferOrder.Fori_spid, m_params.getString("ori_spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! ori_spid in db[%s], ori_spid input[%s]", 
			m_transferOrder.Fori_spid, m_params.getString("ori_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, ori_spid is different!", __FILE__, __LINE__);
    }

    if( (0 != strcmp(m_transferOrder.Fnew_spid, m_params.getString("new_spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! new_spid in db[%s], new_spid input[%s]", 
			m_transferOrder.Fnew_spid, m_params.getString("new_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Ftrade_id, m_params.getString("trade_id").c_str()))
    {
        gPtrAppLog->error("fund trade exists, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
			m_transferOrder.Ftrade_id, m_params.getString("trade_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, trade_id is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Fori_fund_code, m_params.getString("ori_fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! ori_fund_code in db[%s], ori_fund_code input[%s] ", 
			m_transferOrder.Fori_fund_code, m_params.getString("ori_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, ori_fund_code is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! ori_fund_code in db[%s], new_fund_code input[%s] ", 
			m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_fund_code is different!", __FILE__, __LINE__);
    }

    if(m_transferOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_transferOrder.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, total_fee is different!", __FILE__, __LINE__);
    }

    if( m_params.getString("buy_id") != m_transferOrder.Fbuy_id)
    {
        gPtrAppLog->error("fund trade exists, Fbuy_id is different! buy_id in db[%s], Fbuy_id input[%s] ", 
			m_transferOrder.Fbuy_id, m_params.getString("Fbuy_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, Fbuy_id is different!", __FILE__, __LINE__);
    }

    if(m_params.getString("redem_id") != m_transferOrder.Fredem_id)
    {
        gPtrAppLog->error("fund trade exists, redem_id is different! redem_id in db[%s], redem_id input[%s] ", 
			m_transferOrder.Fredem_id, m_params.getString("redem_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, redem_id is different!", __FILE__, __LINE__);
    }

    // ��¼���ڣ�����״̬��Ч������
    if(LSTATE_INVALID == m_transferOrder.Flstate)
    {
        gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] ", 
			m_transferOrder.Fchange_id, m_transferOrder.Ftrade_id);
        throw CException(ERR_TRADE_INVALID, "fund transfer exists, lstate is invalid. ", __FILE__, __LINE__);
    }

    // ֻ�г�ʼ̬�ĵ���������
    if(FUND_TRANSFER_INIT != m_transferOrder.Fstate)
    {
        throw CException(ERR_BUY_RECORD_INVALID, "fund transfer record state invalid. ", __FILE__, __LINE__);
    }

    throw CException(ERR_REPEAT_ENTRY,"fund tratransferde record already exist. ", __FILE__, __LINE__);

}

/**
  * ���ɻ������¼��״̬: �ȴ�����
  */
void FundTransferReq::RecordFundTransfer()throw (CException)
{
    ST_TRANSFER_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRANSFER_FUND));

    strncpy(stRecord.Fchange_id, m_params.getString("fund_exchange_id").c_str(), sizeof(stRecord.Fchange_id)-1);
    strncpy(stRecord.Fori_spid, m_params.getString("ori_spid").c_str(), sizeof(stRecord.Fori_spid)-1);
    strncpy(stRecord.Fnew_spid, m_params.getString("new_spid").c_str(), sizeof(stRecord.Fnew_spid)-1);
    strncpy(stRecord.Fori_fund_code, m_params.getString("ori_fund_code").c_str(), sizeof(stRecord.Fori_fund_code)-1);
    strncpy(stRecord.Fnew_fund_code, m_params.getString("new_fund_code").c_str(), sizeof(stRecord.Fnew_fund_code)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = CREATE_INIT;
    stRecord.Flstate = LSTATE_VALID;
    stRecord.Fsubacc_state = CREATE_INIT;
    stRecord.Fcur_type = 1;

    strncpy(stRecord.Fbuy_id, m_params["buy_id"], sizeof(stRecord.Fbuy_id)-1);
    strncpy(stRecord.Fredem_id, m_params["redem_id"], sizeof(stRecord.Fredem_id)-1);

    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    strncpy(stRecord.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(stRecord.Fchannel_id)-1);
	
    insertFundTransfer(m_pFundCon, stRecord);
}


void FundTransferReq::checkUserBalance() throw (CException)
{
    //TODO ���������ĸ��ֶ�,���˻�ֻ�������ˣ���ʱ�����ڶ��Ჿ��
    LONG balance = querySubaccBalance(m_fund_bind.Fuid,querySubaccCurtype(m_pFundCon, m_params.getString("ori_spid")));

    if(balance < m_params.getLong("total_fee"))
    {
        throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
    }
}


//����û�ת������
void FundTransferReq::checkTransferTimes() throw (CException)
{
    //�����ת���еļ�¼�������µ�ת������
    if (checkIfExistTransferIngBill(m_pFundCon,m_params["trade_id"]))
    {
        throw CException(ERR_TRANSFERINT_EXIST, "user exist transfering record ,please try later!", __FILE__, __LINE__);
    }
    
    //�������û������ƴ���
    if (gPtrConfig->m_AppCfg.transfer_limit_white_list.find(m_params["uin"]) != string::npos)
    {
        return;
    }
    
    ST_FUND_DYNAMIC data;
    memset(&data,0,sizeof(data));
    strncpy(data.Ftrade_id,m_params["trade_id"],sizeof(data.Ftrade_id)-1);
    if (queryFundDynamic(m_pFundCon,data,false) && m_params.getString("systime")<changeDatetimeFormat(string(data.Fredem_day)+"150000"))
    {
        if ((data.Fdyn_status_mask & USER_STOP_TRANSFER)==1  //TODO  �������Ҫ�����������������Ҫ15��֮�������Ч
            || data.Fredem_times_day >= gPtrConfig->m_AppCfg.max_transfer_times_oneday)
        {
            throw CException(ERR_CANNOT_CHANGESP_AGAIN, "exceed max transfer times one day", __FILE__, __LINE__);
        }
    }
}

//���ת�������ܷݶ�����
void FundTransferReq::checkSpRedemRate() throw (CException)
{
    memset(&m_fund_newsp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_newsp_config.Fspid, m_params["new_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_newsp_config.Ffund_code, m_params["new_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, m_fund_newsp_config, false))
    {
    	//��Ӧ�÷���
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
    //��ת��Ȩ��
    /*
    * �Ѿ���������һ���ֶα�ʾת�룬��������ط�ȥ�����������
    if (m_fund_newsp_config.Fredem_valid&SP_NOT_ALLOW_TRANSFER_BUY)
    {
        throw EXCEPTION(ERR_TRANSFER_BUY_NOT_ALLOWED, "new spid transfer buy not allowed"); 
    }
    */

    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_orisp_config.Fspid, m_params["ori_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params["ori_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);

    checkSpRedemRateLimit(m_pFundCon, m_fund_orisp_config,m_params["systime"],m_params.getLong("total_fee"));
   
}


/**
  * ����������
  */
void FundTransferReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "new_sp_user", m_fund_bind_newsp_acc.Fsp_user_id);
    CUrlAnalyze::setParam(rqst->odata, "new_sp_trans_id", m_fund_bind_newsp_acc.Fsp_trans_id);
    CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
    CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
    CUrlAnalyze::setParam(rqst->odata, "new_fund_code", m_params.getString("new_fund_code").c_str());
    CUrlAnalyze::setParam(rqst->odata, "buy_id", m_params.getString("buy_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "total_fee", m_params.getString("total_fee").c_str());
    rqst->olen = strlen(rqst->odata);
    return;
}


