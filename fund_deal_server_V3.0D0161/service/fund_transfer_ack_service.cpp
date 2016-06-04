/**
  * FileName: fund_transfer_ack_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-16
  * Description: �ݶ�ת��ȷ�Ͻӿ�
  */


#include "fund_commfunc.h"
#include "fund_transfer_ack_service.h"

FundTransferAck::FundTransferAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_transferOrder, 0, sizeof(m_transferOrder));
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_buyOrder, 0, sizeof(m_buyOrder));
    memset(&m_redemOrder, 0, sizeof(m_redemOrder));
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    memset(&m_fund_newsp_config, 0, sizeof(FundSpConfig));
    memset(&m_fund_bind_orisp_acc, 0, sizeof(FundBindSp));
    memset(&m_fund_bind_newsp_acc, 0, sizeof(FundBindSp));
    memset(&m_dynamic_info,0,sizeof(m_dynamic_info));
	memset(&m_fundCloseTrans, 0, sizeof(FundCloseTrans));
	
	m_close_fund_seqno = 0;
    m_request = NULL;
}

/**
  * service step 1: �����������
  */
void FundTransferAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    TRACE_DEBUG("[fund_transfer_ack_service] parseInputMsg start: ");

    // Ҫ�����������ݣ��ײ��ʹ��
    m_request = rqst;
    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_transfer_ack_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "op_type", 1,5);
    m_params.readStrParam(szMsg, "trade_id", 10,32);
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "fund_exchange_id", 10, 32); //ת������
    m_params.readStrParam(szMsg, "ori_spid", 10, 15);
    m_params.readStrParam(szMsg, "new_spid", 10, 15);
    m_params.readStrParam(szMsg, "ori_fund_code", 1, 64);
    m_params.readStrParam(szMsg, "new_fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token
    m_params.readStrParam(szMsg, "memo", 0, 128);
    m_params.readStrParam(szMsg, "sp_billno_redem", 0,32);
    m_params.readStrParam(szMsg, "sp_billno_buy", 0,32);
    m_params.readStrParam(szMsg, "redem_id", 0,32);
    m_params.readStrParam(szMsg, "buy_id", 0,32);
    m_params.readStrParam(szMsg, "redem_result_sign", 0,32);

	m_params.readStrParam(szMsg, "close_end_day", 0, 8);	
	m_params.readIntParam(szMsg, "user_end_type", 0, 3);
	m_params.readIntParam(szMsg, "end_sell_type", 0, 3);
	m_params.readLongParam(szMsg,"end_plan_amt",0, MAX_LONG);
	m_params.readStrParam(szMsg, "end_transfer_spid", 0, 15);
	m_params.readStrParam(szMsg, "end_transfer_fundcode", 0, 64);

    m_optype = m_params.getInt("op_type");
    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

    if (m_optype == FUND_TRANSFER_REDEM_TIMEOUT
        && 	("true" == gPtrConfig->m_AppCfg.redem_timeout_conf))//�������þ�����س�ʱ���ɹ�����ʧ�ܣ�������ɹ�������Ҫ�����������ܳ���Գ�ʱ��ص����в���
    {
        m_optype = FUND_TRANSFER_REDEM_SUC;
    }

}

/*
 * ���ɻ���ע����token
 */
string FundTransferAck::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����trade_id|uin|fund_exchange_id|ori_spid|new_spid|total_fee|sp_billno_redem|sp_billno_buy|redem_result_sign|op_type|key
    // ��������ԭ��
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["uin"] << "|" ;
    ss << m_params["fund_exchange_id"] << "|" ;
    ss << m_params["ori_spid"] << "|" ;
    ss << m_params["new_spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["sp_billno_redem"] << "|" ;
    ss << m_params["sp_billno_buy"] << "|" ;
    ss << m_params["redem_result_sign"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << gPtrConfig->m_AppCfg.transfer_ackkey;
    //TRACE_DEBUG("FundTransferAck, sourceStr=[%s]", ss.str().c_str());

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

//���ǰ�û���ؽ��ǩ��
void FundTransferAck::checkResultSign()throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
       // ��������ԭ��
    ss << m_params["redem_id"] << "|" ;
    ss << m_params["ori_spid"] << "|" ;
    ss << m_params["sp_billno_redem"] << "|" ; 
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["ori_fund_code"] << "|" ;
    ss << m_params["trade_id"] << "|" ;
    ss << gPtrConfig->m_AppCfg.transfer_redem_result_key;
    
    //TRACE_DEBUG("checkResultSign, sourceStr=[%s]", ss.str().c_str());
    
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    if (StrUpper(m_params.getString("redem_result_sign")) != StrUpper(buff))
    {   
        TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("redem_result_sign").c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "input redem_result_sign error");    
    } 
}

/*
 * ����token
 */
void FundTransferAck::CheckToken() throw (CException)
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
void FundTransferAck::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();

    if(OP_TYPE_TRANSFER_REDEM_SUC == m_params.getInt("op_type"))
    { 
        CHECK_PARAM_EMPTY("sp_billno_redem"); 
        checkResultSign();

		if(CLOSE_FUND_SELL_TYPE_ANOTHER_FUND == m_params.getInt("end_sell_type"))
		{
			CHECK_PARAM_EMPTY("end_transfer_spid"); 
			CHECK_PARAM_EMPTY("end_transfer_fundcode"); 
		}

		if(CLOSE_FUND_END_TYPE_PATRIAL_REDEM == m_params.getInt("user_end_type") && 0 == m_params.getLong("end_plan_amt"))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "user_end_type not found, or empty"); 
		}
    }
    else if (OP_TYPE_TRANSFER_BUY_REQ == m_optype)
    {
        CHECK_PARAM_EMPTY("sp_billno_buy"); 
    }

    if (OP_TYPE_TRANSFER_REDEM_SUC == m_optype)
    {
        checkResultSign();
    }
}

void FundTransferAck::queryFundBindNewSpInfo()throw (CException)
{
    strncpy(m_fund_bind_newsp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_newsp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_newsp_acc.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_bind_newsp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_newsp_acc, false);    
}

/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundTransferAck::CheckFundTransfer() throw (CException)
{
    strncpy(m_transferOrder.Fchange_id,m_params["fund_exchange_id"],sizeof(m_transferOrder.Fchange_id)-1);

    if (false == queryFundTransfer(m_pFundCon, m_transferOrder, true))
    {
        gPtrAppLog->error("transfer record not exist, listid[%s]  ", m_params.getString("fund_exchange_id").c_str());
        throw CException(ERR_BUYPAY_NOLIST, "transfer record not exist! ", __FILE__, __LINE__);
    }

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
        gPtrAppLog->error("fund trade exists, fund_code is different! new_fund_code in db[%s], new_fund_code input[%s] ", 
			m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_fund_code is different!", __FILE__, __LINE__);
    }

    if(m_transferOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_transferOrder.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, total_fee is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("buy_id").empty() && m_params.getString("buy_id") != m_transferOrder.Fbuy_id)
    {
        gPtrAppLog->error("fund trade exists, Fbuy_id is different! buy_id in db[%s], Fbuy_id input[%s] ", 
			m_transferOrder.Fbuy_id, m_params.getString("Fbuy_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, Fbuy_id is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("redem_id").empty() && m_params.getString("redem_id") != m_transferOrder.Fredem_id)
    {
        gPtrAppLog->error("fund trade exists, redem_id is different! redem_id in db[%s], redem_id input[%s] ", 
			m_transferOrder.Fredem_id, m_params.getString("redem_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, redem_id is different!", __FILE__, __LINE__);
    }

    //У��ת��״̬
    checkTransferState();

}


/**
  * ת����״̬ȷ��
  */
void FundTransferAck::checkTransferState() throw (CException)
{
    if (m_optype != OP_TYPE_TRANSFER_BUY_REQ)
    {
        //�����ص�״̬
        checkRedemTrade();

        //����깺��״̬
        checkBuyTrade();
    }

    if (m_optype == OP_TYPE_TRANSFER_BUY_REQ)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REQ && m_transferOrder.Fstate != FUND_TRANSFER_INIT )
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        if (m_transferOrder.Fstate == FUND_TRANSFER_REQ)
        {
            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter . ", __FILE__, __LINE__);
        }
    }
    else if (m_optype == OP_TYPE_TRANSFER_REDEM_SUC)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REQ && m_transferOrder.Fstate != FUND_TRANSFER_REDEM_SUC 
           && m_transferOrder.Fstate != FUND_TRANSFER_TRANS_SUC)
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        //�������:
        if (m_transferOrder.Fstate == FUND_TRANSFER_REDEM_SUC || m_transferOrder.Fstate == FUND_TRANSFER_TRANS_SUC)
        {
			if(m_buyOrder.Fclose_listid > 0&&m_buyOrder.Ftrade_date[0]!=0)
			{
				// ���룬���ڶ��ڲ�ƷҪ��ѯ���������ڷ��ط�տ�ʼʱ��
				m_fundCloseTrans.Fid = m_buyOrder.Fclose_listid;
				strncpy(m_fundCloseTrans.Ftrade_id, m_buyOrder.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
				queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);
				//���ڲ������� m_params.setParam("trans_date", m_fundCloseTrans.Ftrans_date);
				m_params.setParam("trans_date", m_buyOrder.Ftrade_date);
				
			}
            //����˾��س�ʱ�Ĳ���
            if (m_params.getInt("op_type") == OP_TYPE_TRANSFER_REDEM_SUC && m_redemOrder.Fspe_tag==1)
            {
                m_optype =FUND_TRANSFER_REDEM_SPTIMEOUT_REDO_OK ;
                return;
            }
            //���˻���ʧ�ܵĲ���
            if ((m_transferOrder.Fsubacc_state&FUND_TRANSFER_SUBACC_SAVE) == 0 
                && (m_buyOrder.Fstate == PAY_OK || m_buyOrder.Fstate == PURCHASE_SUC))
            {
                m_optype =FUND_TRANSFER_SUBACC_SAVE_REDO ;
                return;
            }

            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter . ", __FILE__, __LINE__);
        }
        
    }
    else if (m_optype == OP_TYPE_TRANSFER_REDEM_FAIL)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REQ && m_transferOrder.Fstate != FUND_TRANSFER_REDEM_FAIL )
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        if (m_transferOrder.Fstate == FUND_TRANSFER_REDEM_FAIL)
        {
            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter. ", __FILE__, __LINE__);
        }
    }
    else if (m_optype == OP_TYPE_TRANSFER_BUY_SUC)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REDEM_SUC && m_transferOrder.Fstate != FUND_TRANSFER_TRANS_SUC )
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        if (m_transferOrder.Fstate == FUND_TRANSFER_TRANS_SUC)
        {
            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter. ", __FILE__, __LINE__);
        }
    }
    else
    {
        throw CException(ERR_BAD_PARAM, "optype invalid. ", __FILE__, __LINE__);
    }

}

/**
  * ִ���깺����
  */
void FundTransferAck::excute() throw (CException)
{
    try
    {
        CheckParams();

        CheckFundBind();

         // �������� 
        m_pFundCon->Begin();

        // ��ѯ��У��ת���� 
        CheckFundTransfer();

        //����op_type����ת����״̬�����˻�����
        dealTransfer();

        // �ύ���� 
        m_pFundCon->Commit();

        // ���¸���ckv ,��������֮���Ǳ�������ع�ȴд��ckv������
        updateCkvs();
		
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
void FundTransferAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params["uin"], &m_fund_bind, true))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    if (m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "check fund bind record trade_id fail! ", __FILE__, __LINE__);
    }
}

/**
*����û����
*/
void FundTransferAck::checkUserBalance() throw (CException)
{
    //TODO ���������ĸ��ֶ�,���˻�ֻ�������ˣ���ʱ�����ڶ��Ჿ��
    LONG balance = querySubaccBalance(m_fund_bind.Fuid,querySubaccCurtype(m_pFundCon, m_params.getString("ori_spid")));

    if(balance < m_params.getLong("total_fee"))
    {
        throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
    }
}

/**
*��¼�깺��
*/
void FundTransferAck::RecordFundBuy() throw (CException)
{
    ST_TRADE_FUND &stRecord = m_buyOrder;
    strncpy(stRecord.Flistid, m_transferOrder.Fbuy_id, sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_transferOrder.Fnew_spid, sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno_buy").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_fund_bind.Fuid;
    strncpy(stRecord.Ffund_name, m_fund_newsp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_transferOrder.Fnew_fund_code, sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    stRecord.Fpur_type = PURTYPE_TRANSFER_PURCHASE;
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = PAY_INIT;
    stRecord.Flstate = LSTATE_VALID;

    string trade_date;
    string fund_vdate;
	string end_date;
	if(m_fund_newsp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		getTradeDate(m_pFundCon,m_params.getString("systime"), trade_date,fund_vdate);
	}
	else
	{
		// �������ڼ���
		FundCloseCycle fundCloseCycle;
		memset(&fundCloseCycle, 0, sizeof(FundCloseCycle));

		strncpy(fundCloseCycle.Fdate, calculateFundDate(m_params.getString("systime")).c_str(), sizeof(fundCloseCycle.Fdate) - 1);
		strncpy(fundCloseCycle.Ffund_code, m_transferOrder.Fnew_fund_code, sizeof(fundCloseCycle.Ffund_code) - 1);
		queryFundCloseCycle(m_pFundCon, fundCloseCycle, false);
		trade_date = fundCloseCycle.Ftrans_date;
		fund_vdate = fundCloseCycle.Ffirst_profit_date;
		end_date = fundCloseCycle.Fdue_date;
	}
	if(	trade_date.empty()){
		//����������
		gPtrAppLog->error("trade_date unfound[%s], systime[%s]", stRecord.Flistid, m_params.getString("systime").c_str());
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound! ");
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}
    
    strncpy(stRecord.Ftrade_date,trade_date.c_str(), sizeof(stRecord.Ftrade_date) - 1);//������
    strncpy(stRecord.Fend_date,end_date.c_str(), sizeof(stRecord.Fend_date) - 1);//���ڽ��׽�����
    strncpy(stRecord.Ffund_vdate, fund_vdate.c_str(), sizeof(stRecord.Ffund_vdate) - 1);//����ֵ����,�ñ��깺�״β������������
    strncpy(stRecord.Frela_listid, m_transferOrder.Fchange_id, sizeof(stRecord.Frela_listid)-1);
    strncpy(stRecord.Ffetchid, "buy_no_fetchid", sizeof(stRecord.Ffetchid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("new_spid")); // ��������
    strncpy(stRecord.Fsub_trans_id, m_transferOrder.Fbuy_id, sizeof(stRecord.Fsub_trans_id)-1);

    //�ݶ�ת���ļ�¼�깺��;
    stRecord.Fpurpose= PURPOSE_CHANGE_SP;
    
    strncpy(stRecord.Fchannel_id, m_transferOrder.Fchannel_id, sizeof(stRecord.Fchannel_id)-1);
	
    InsertTradeFund(m_pFundCon, &stRecord);
    if(m_fund_bind.Fuid >= 10000)
    {
        InsertTradeUserFund(m_pFundCon, &stRecord);
    }
}

/**
*����ת����״̬
*/
void FundTransferAck::UpdateTransferState(int state,int sub_acc_state,const string&acc_time)throw (CException)
{
    ST_TRANSFER_FUND transOrder;
    memset(&transOrder,0,sizeof(transOrder));
    strncpy(transOrder.Fchange_id,m_transferOrder.Fchange_id,sizeof(transOrder.Fchange_id)-1);
    strncpy(transOrder.Fmodify_time,m_params["systime"],sizeof(transOrder.Fmodify_time)-1);
    strncpy(transOrder.Fmemo,m_params["memo"],sizeof(transOrder.Fmemo)-1);
    if (state)
    {
        transOrder.Fstate = state;
    }
    if (sub_acc_state)
    {
        transOrder.Fsubacc_state = sub_acc_state;
    }
    if (!acc_time.empty())
    {
        strncpy(transOrder.Facc_time,acc_time.c_str(),sizeof(transOrder.Facc_time)-1);
        strncpy(m_transferOrder.Facc_time,acc_time.c_str(),sizeof(m_transferOrder.Facc_time)-1);
    }
    
    if(OP_TYPE_TRANSFER_REDEM_TIMEOUT == m_params.getInt("op_type"))
    {
        transOrder.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
    }
    else if (OP_TYPE_TRANSFER_BUY_SUC == m_params.getInt("op_type"))
    {
        transOrder.Fspe_tag=m_transferOrder.Fspe_tag;
    }
    
    updateFundTransfer(m_pFundCon,transOrder);    
}

/**
*��¼��ص�
*/
void FundTransferAck::RecordFundRedem() throw (CException)
{
    ST_TRADE_FUND  &stRecord = m_redemOrder;

    string drawid = string(m_transferOrder.Fredem_id).substr(10,18);

    strncpy(stRecord.Flistid, m_transferOrder.Fredem_id, sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_params.getString("ori_spid").c_str(), sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno_redem").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_fund_bind.Fuid;
    strncpy(stRecord.Ffund_name, m_fund_orisp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("ori_fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    stRecord.Fpur_type = PURTYPE_TRANSFER_REDEEM;
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = REDEM_ININ;
    stRecord.Flstate = LSTATE_VALID;
    strncpy(stRecord.Frela_listid, m_transferOrder.Fchange_id, sizeof(stRecord.Frela_listid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("ori_spid"));; // ��������
    strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
    strncpy(stRecord.Fsub_trans_id, drawid.c_str(), sizeof(stRecord.Fsub_trans_id)-1);
    stRecord.Fpurpose = PURPOSE_CHANGE_SP;
    strncpy(stRecord.Fchannel_id, m_transferOrder.Fchannel_id, sizeof(stRecord.Fchannel_id)-1);

    stRecord.Floading_type = DRAW_NOT_USE_LOADING;

    InsertTradeFund(m_pFundCon, &stRecord);
    InsertTradeUserFund(m_pFundCon, &stRecord);

}

//�����ص�״̬
void FundTransferAck::checkRedemTrade() throw (CException)
{
    // û�н��׼�¼������
    if(!QueryTradeFund(m_pFundCon, m_transferOrder.Fredem_id, PURTYPE_TRANSFER_REDEEM,&m_redemOrder, true))
    {
        gPtrAppLog->error("redem record not exist, redem_id[%s]  ", m_transferOrder.Fredem_id);
        throw CException(ERR_BUYPAY_NOLIST, "redem record not exist! ", __FILE__, __LINE__);
    }

    // ����״̬��Ч������
    if(LSTATE_INVALID == m_redemOrder.Flstate)
    {
        gPtrAppLog->error("fund redem , lstate is invalid. listid[%s], uid[%d] ", m_redemOrder.Flistid, m_redemOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  lstate is invalid. ", __FILE__, __LINE__);
    }

    // У��ؼ�����
    if(m_redemOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("redem total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_redemOrder.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "redem total_fee is different!", __FILE__, __LINE__);
    }

    if( m_redemOrder.Fuid!= m_fund_bind.Fuid)
    {
        TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_redemOrder.Fuid, m_fund_bind.Fuid);
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with fundbind");
    }

    if (0 != strcmp(m_redemOrder.Fspid, m_params.getString("ori_spid").c_str()))
    {
        gPtrAppLog->error("redem trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_redemOrder.Fspid, m_params.getString("ori_spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "redem trade exists, spid is different!", __FILE__, __LINE__);
    }

    if (m_redemOrder.Fstate != REDEM_ININ && m_redemOrder.Fstate != REDEM_FINISH)
    {
        gPtrAppLog->error("fund redem , state=%d is invalid. listid[%s], uid[%d] ", m_redemOrder.Fstate,m_redemOrder.Flistid, m_redemOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  state is invalid. ", __FILE__, __LINE__);   
    }
    
    if (!m_params.getString("sp_billno_redem").empty() && m_redemOrder.Fcoding[0] != 0
        && m_params.getString("sp_billno_redem") != m_redemOrder.Fcoding)
    {
        gPtrAppLog->error("fund trade exists, sp_billno_redem is different! sp_billno_redem in db[%s], sp_billno_redem input[%s] ", 
			m_redemOrder.Fcoding, m_params.getString("sp_billno_redem").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, sp_billno_redem is different!", __FILE__, __LINE__);
    } 
    // �õ����������Ϣ
    m_params.setParam("uid", m_redemOrder.Fuid);
}

//����깺��״̬
void FundTransferAck::checkBuyTrade() throw (CException)
{
    // û�н��׼�¼������
    if(!QueryTradeFund(m_pFundCon, m_transferOrder.Fbuy_id, PURTYPE_TRANSFER_PURCHASE,&m_buyOrder, true))
    {
        gPtrAppLog->error("buy record not exist, Fbuy_id[%s]  ", m_transferOrder.Fbuy_id);
        throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
    }

    // ����״̬��Ч������
    if(LSTATE_INVALID == m_buyOrder.Flstate)
    {
        gPtrAppLog->error("fund buy , lstate is invalid. listid[%s], uid[%d] ", m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "buy list  lstate is invalid. ", __FILE__, __LINE__);
    }

    // У��ؼ�����
    if(m_buyOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("buy total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_buyOrder.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "buy total_fee is different!", __FILE__, __LINE__);
    }

    if( m_buyOrder.Fuid!= m_fund_bind.Fuid)
    {
        TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_buyOrder.Fuid, m_fund_bind.Fuid);
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with fundbind");
    }

    if (0 != strcmp(m_buyOrder.Fspid, m_params.getString("new_spid").c_str()))
    {
        gPtrAppLog->error("buy trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_buyOrder.Fspid, m_params.getString("new_spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "buy trade exists, spid is different!", __FILE__, __LINE__);
    }

    if (m_buyOrder.Fstate != PAY_INIT && m_buyOrder.Fstate != PAY_OK && m_buyOrder.Fstate != PURCHASE_SUC)
    {
        gPtrAppLog->error("fund buy , state=%d is invalid. listid[%s], uid[%d] ", m_buyOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  state is invalid. ", __FILE__, __LINE__);   
    }
    
    // У���̻�������
    if (!m_params.getString("sp_billno_buy").empty() && m_buyOrder.Fcoding[0] != 0
        && m_params.getString("sp_billno_buy") != m_buyOrder.Fcoding)
    {
        gPtrAppLog->error("fund trade exists, sp_billno_buy is different! sp_billno_buy in db[%s], sp_billno_buy input[%s] ", 
			m_buyOrder.Fcoding, m_params.getString("sp_billno_buy").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, sp_billno_buy is different!", __FILE__, __LINE__);
    }
    // �õ����������Ϣ
    m_params.setParam("uid", m_buyOrder.Fuid);
}


/**
*�ۼ��̻���������ܶ�
*/
void FundTransferAck::UpdateFundSpRedemInfo()throw (CException)
{
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_orisp_config.Fspid, m_params["ori_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params["ori_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, m_fund_orisp_config, true))
    {
    	//��Ӧ�÷���
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
    strncpy(m_fund_orisp_config.Fmodify_time, m_params["systime"], sizeof(m_fund_orisp_config.Fmodify_time) - 1);
    updateFundSpRedomTotal(m_pFundCon, m_fund_orisp_config, m_transferOrder.Ftotal_fee,DRAW_NOT_USE_LOADING,m_transferOrder.Facc_time);    
}

/**
*������ص�״̬
*/
void FundTransferAck::UpdateFundRedem(int state)throw (CException)
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

    stRecord.Fstate = state;
    stRecord.Fpur_type = m_redemOrder.Fpur_type;
    stRecord.Fuid = m_redemOrder.Fuid;    
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_redemOrder.Ftrade_id);
    strncpy(stRecord.Flistid, m_redemOrder.Flistid, sizeof(stRecord.Flistid) - 1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno_redem").c_str(), sizeof(stRecord.Fcoding) - 1);
    if(OP_TYPE_TRANSFER_REDEM_TIMEOUT == m_params.getInt("op_type"))
    {
        stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//��ʱ���		
    }
    
    UpdateFundTrade(m_pFundCon, stRecord, m_redemOrder, m_params.getString("systime"));
}

/**
*�����깺��״̬
*/
void FundTransferAck::UpdateFundBuy(int state, LONG close_listid)throw (CException)
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    stRecord.Fstate = state;
    strncpy(stRecord.Flistid, m_buyOrder.Flistid, sizeof(stRecord.Flistid) - 1);
    stRecord.Fpur_type = m_buyOrder.Fpur_type;
    stRecord.Fuid = m_buyOrder.Fuid;    
    //����trade_id,���½��׼�¼ʱ��Ҫʹ��
    SCPY(stRecord.Ftrade_id, m_buyOrder.Ftrade_id);
	stRecord.Fclose_listid = close_listid;
	//strncpy(stRecord.Fend_date, m_buyOrder.Fend_date, sizeof(stRecord.Fend_date) - 1);
    UpdateFundTrade(m_pFundCon, stRecord, m_redemOrder, m_params.getString("systime"));
}

/**
*�ۼ��û�����ת������
*/
void FundTransferAck::RecordTransferTimes() throw (CException)
{
    strncpy(m_dynamic_info.Ftrade_id,m_params["trade_id"],sizeof(m_dynamic_info.Ftrade_id)-1);
    strncpy(m_dynamic_info.Fmodify_time,m_params["systime"],sizeof(m_dynamic_info.Fmodify_time)-1);
    if (queryFundDynamic(m_pFundCon,m_dynamic_info,true))
    {
        if (m_transferOrder.Facc_time<changeDatetimeFormat(string(m_dynamic_info.Fredem_day)+"150000"))
        {
            if (m_transferOrder.Facc_time>=changeDatetimeFormat(addDays(m_dynamic_info.Fredem_day,-1)+"150000"))
            {
                m_dynamic_info.Fredem_times_day++;
                if (m_dynamic_info.Fredem_times_day >= gPtrConfig->m_AppCfg.max_transfer_times_oneday)
                {
                    m_dynamic_info.Fdyn_status_mask = (m_dynamic_info.Fdyn_status_mask|USER_STOP_TRANSFER);
                }
            }
        }
        else  
        {
            strncpy(m_dynamic_info.Fredem_day,addDays(getYYYYMMDDFromStdTime(m_transferOrder.Facc_time),1).c_str(),sizeof(m_dynamic_info.Fredem_day)-1);
            m_dynamic_info.Fredem_times_day=1;
            m_dynamic_info.Fdyn_status_mask = (m_dynamic_info.Fdyn_status_mask&(~USER_STOP_TRANSFER));
        }
        updateFundDynamic(m_pFundCon,m_dynamic_info);
    }
    else
    {
        string redemDay=getYYYYMMDDFromStdTime(m_transferOrder.Facc_time);
        if (string(m_transferOrder.Facc_time).substr(11,2)>="15")
        {
             redemDay = addDays(redemDay,1);
        }
        strncpy(m_dynamic_info.Fredem_day,redemDay.c_str(),sizeof(m_dynamic_info.Fredem_day)-1);
        m_dynamic_info.Fredem_times_day=1;
        m_dynamic_info.Fdyn_status_mask=0;
        m_dynamic_info.Flstate=1;
        strncpy(m_dynamic_info.Fcreate_time,m_params["systime"],sizeof(m_dynamic_info.Fcreate_time)-1);
        insertFundDynamic(m_pFundCon,m_dynamic_info);
    }
}


/**
*ת��ȷ��(�깺�ɹ�ȷ��)
*/
void FundTransferAck::processTransferBuyReqSuc() throw (CException)
{
    //����û��ݶ�
    checkUserBalance();

    //���spid ��fund_code �Ƿ���Ч����ȡ�������ƴ��������׵�ʱ�õ�
    strncpy(m_fund_orisp_config.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params.getString("ori_fund_code").c_str(), sizeof(m_fund_orisp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_orisp_config, false);

    strncpy(m_fund_newsp_config.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_newsp_config.Fspid) - 1);
    strncpy(m_fund_newsp_config.Ffund_code, m_params.getString("new_fund_code").c_str(), sizeof(m_fund_newsp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_newsp_config, false);

    //����̻���ض��
    checkSpRedemRateLimit(m_pFundCon, m_fund_orisp_config,m_params["systime"],m_params.getLong("total_fee"));
    
    //����ת����acctime��״̬Ϊ�깺����ɹ�
    UpdateTransferState(FUND_TRANSFER_REQ,0,m_params["systime"]);
    //�����깺��
    RecordFundBuy();
    //������ص�
    RecordFundRedem();

    //��ѯ�����˻���Ϣ��itg�����
    strncpy(m_fund_bind_orisp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_orisp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_orisp_acc.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_bind_orisp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_orisp_acc, false);
}


/**
*��سɹ�����
*/
void FundTransferAck::processTransferRedemSuc() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_INIT || m_redemOrder.Fstate != REDEM_ININ)
    {
        gPtrAppLog->error("fund buy state=%d,redem state=%d. invalid for processTransferRedemSuc listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_redemOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem or buy list  state is invalid. ", __FILE__, __LINE__);      
    }

    //�����ض��
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_orisp_config.Fspid, m_params["ori_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params["ori_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);
    checkSpRedemRateLimit(m_pFundCon, m_fund_orisp_config,m_params["systime"],m_params.getLong("total_fee"));

	queryNewFundSpAndFundcodeInfo();

	checkUserPermissionBuyCloseFund();
    
    //��ѯת�뽻���˻���itg���깺
    queryFundBindNewSpInfo();
    
    //�����˻�
    doDraw();

    //������ص�״̬
    UpdateFundRedem(REDEM_FINISH);

	//���ڲ�Ʒ��¼
	LONG close_listid =0;
	recordCloseFund(m_buyOrder.Ftrade_date, close_listid);

    //�����깺��״̬
    UpdateFundBuy(PAY_OK, close_listid);	
	
    //�����˻����
    bool saveRet = doSave(false);
    //����ת����״̬
    int subacc_state = FUND_TRANSFER_SUBACC_DRAW;
    if (saveRet)
    {
        subacc_state = (FUND_TRANSFER_SUBACC_DRAW|FUND_TRANSFER_SUBACC_SAVE);
    }
    UpdateTransferState(FUND_TRANSFER_REDEM_SUC,subacc_state);

    //�ۼ�ת������
    RecordTransferTimes();

    //�ۼ���طݶ�
    UpdateFundSpRedemInfo();

	//����CKV���Ӽ�¼
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    stRecord.Fstate = PAY_OK;
    stRecord.Fpur_type = m_buyOrder.Fpur_type;
	stRecord.Fclose_listid = close_listid;
	strncpy(stRecord.Ftrade_id, m_buyOrder.Ftrade_id, sizeof(stRecord.Ftrade_id)-1);
	strncpy(stRecord.Ffund_code, m_buyOrder.Ffund_code, sizeof(stRecord.Ffund_code)-1);
	strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
	stRecord.Ftotal_fee = m_buyOrder.Ftotal_fee;
	updateUserAcc(stRecord);

    /*catch(CException &e)
    {
        gPtrAppLog->error("in processTransferRedemSuc call  RecordTransferTimes or UpdateFundSpRedemInfo throw exp:errcode=%d,errinfo=%s"
                                        ,e.error(),e.what());
        alert(e.error(), (string("ת����سɹ����ۼӴ������޶���쳣:%s")+e.what()).c_str());
    }*/
    
}

/**
*���ʧ�ܴ���
*/
void FundTransferAck::processTransferRedemFail() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_INIT || m_redemOrder.Fstate != REDEM_ININ)
    {
        gPtrAppLog->error("fund buy state=%d,redem state=%d. invalid for processTransferRedemFail listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_redemOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem or buy list  state is invalid. ", __FILE__, __LINE__);      
    }
    //������ص�״̬
    UpdateFundRedem(REDEM_FAIL);

    //����ת����״̬
    UpdateTransferState(FUND_TRANSFER_REDEM_FAIL);
}

void FundTransferAck::processTransferBuySuc() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_OK || m_redemOrder.Fstate != REDEM_FINISH)
    {
        gPtrAppLog->error("fund buy state=%d,redem state=%d. invalid for processTransferBuySuc listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_redemOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem or buy list  state is invalid. ", __FILE__, __LINE__);      
    }

	checkCloseEndDate();

    //�����깺��״̬
    UpdateFundBuy(PURCHASE_SUC);

    //����ת����״̬
    UpdateTransferState(FUND_TRANSFER_TRANS_SUC);
}

void FundTransferAck::processTransferSubaccSaveRedo() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_OK && m_buyOrder.Fstate != PURCHASE_SUC)
    {
        gPtrAppLog->error("fund buy state=%d. invalid for processTransferSubaccSaveRedo listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);      
    }

    //����ת�������˻�״̬
    if (doSave(true))
    {
        UpdateTransferState(0,(m_transferOrder.Fsubacc_state|FUND_TRANSFER_SUBACC_SAVE));
    }
}

void FundTransferAck::processTransferSpRedemRedoSuc() throw (CException)
{
    if (m_redemOrder.Fstate != REDEM_FINISH)
    {
        gPtrAppLog->error("redem state=%d. invalid for processTransferSpRedemRedoSuc listid[%s], uid[%d] "
                                        , m_redemOrder.Fstate,m_redemOrder.Flistid, m_redemOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  state is invalid. ", __FILE__, __LINE__);      
    }
    //������ص���spe_tag���Ϊ0
    UpdateFundRedem(REDEM_FINISH);
    
    //����ת������spe_tag���Ϊ0
    UpdateTransferState(m_transferOrder.Fstate);
}

void FundTransferAck::queryNewFundSpAndFundcodeInfo()
{
	strncpy(m_fund_newsp_config.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_newsp_config.Fspid) - 1);
	strncpy(m_fund_newsp_config.Ffund_code, m_params.getString("new_fund_code").c_str(), sizeof(m_fund_newsp_config.Ffund_code) - 1);
	queryFundSpAndFundcodeFromCkv(m_fund_newsp_config, true);
}

void FundTransferAck::checkUserPermissionBuyCloseFund()
{
	//�Ƿ�ղ�Ʒ�����
	if(m_fund_newsp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}
	
	strncpy(m_fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	strncpy(m_fundCloseTrans.Ffund_code, m_buyOrder.Ffund_code, sizeof(m_fundCloseTrans.Ffund_code) - 1);

	// ��m_buyOrder������Ϊ׼
	m_close_fund_seqno = checkPermissionBuyCloseFund(m_fundCloseTrans, m_fund_newsp_config, m_buyOrder.Ftrade_date, m_buyOrder.Fend_date, true);
    gPtrAppLog->debug("checkUserPermissionBuyCloseFund : m_close_fund_seqno=%d,m_fundCloseTrans.Fseq=%d", m_close_fund_seqno, m_fundCloseTrans.Fseqno);

	m_params.setParam("trans_date", m_buyOrder.Ftrade_date);
	
}

void FundTransferAck::recordCloseFund(string trade_date, LONG& close_listid)
{
	//�Ƿ�ղ�Ʒֱ�ӷ���
	if(m_fund_newsp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}

	bool sale_close_fund_seqno = m_close_fund_seqno == m_fundCloseTrans.Fseqno;
	
	// ��յ����ռ���
	FundCloseCycle fundCloseCycle;
	memset(&fundCloseCycle, 0, sizeof(FundCloseCycle));
	strncpy(fundCloseCycle.Fdate, calculateFundDate(m_buyOrder.Facc_time).c_str(), sizeof(fundCloseCycle.Fdate) - 1);
	strncpy(fundCloseCycle.Ffund_code, m_buyOrder.Ffund_code, sizeof(fundCloseCycle.Ffund_code) - 1);
	bool hasCycle = queryFundCloseCycle(m_pFundCon, fundCloseCycle, false);

	if(strcmp(m_buyOrder.Ftrade_date,fundCloseCycle.Ftrans_date)!=0||!hasCycle)
	{
		//���μ��㲻һ�¸澯
		alert(ERR_DIFF_END_DATE, (string("date:") + toString(fundCloseCycle.Fdate) +string("fund_code:") + toString(fundCloseCycle.Ffund_code)
			+ "�깺������Ľ������붨�ڼ�¼����Ĳ�һ��").c_str());
		throw CException(ERR_DIFF_END_DATE, "trans date diff.", __FILE__, __LINE__); 
	}
	
	FundCloseTrans fundCloseTrans;
	
	//����������£����򴴽��¼�¼
	if(sale_close_fund_seqno)
	{
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		fundCloseTrans.Fid= m_fundCloseTrans.Fid;
		// ���δ���ò�����
		if(m_params.getInt("user_end_type")!=0){
			fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type");
			fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		}
		if(m_params.getInt("end_sell_type")!=0){
			fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO ���ݲ�Ʒ�������
			strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
			strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		}
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);
	
		fundCloseTrans.Fpay_type = m_fundCloseTrans.Fpay_type | CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_fundCloseTrans.Fstart_total_fee + m_buyOrder.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_fundCloseTrans.Fcurrent_total_fee +  m_buyOrder.Ftotal_fee;
		
		saveFundCloseTrans(fundCloseTrans,m_fundCloseTrans,m_buyOrder.Flistid,PURTYPE_PURCHASE);

		close_listid = m_fundCloseTrans.Fid;
		
	}
	else
	{
		memset(&fundCloseTrans, 0, sizeof(FundCloseTrans));
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		strncpy(fundCloseTrans.Fspid, m_buyOrder.Fspid, sizeof(fundCloseTrans.Fspid) - 1);
		strncpy(fundCloseTrans.Ffund_code, m_buyOrder.Ffund_code, sizeof(fundCloseTrans.Ffund_code) - 1);
		fundCloseTrans.Fuid =  m_params.getInt("uid");
		fundCloseTrans.Fseqno = m_close_fund_seqno;
		fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type");
		fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO ���ݲ�Ʒ�������
		fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
		strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);
		
		fundCloseTrans.Fpay_type = CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_buyOrder.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_buyOrder.Ftotal_fee;
		//�´������δ������ʹ��Ĭ��ֵ
		fundCloseTrans.Fuser_end_type = (m_params.getInt("user_end_type") != 0) ? m_params.getInt("user_end_type") : CLOSE_FUND_END_TYPE_ALL_EXTENSION; //TODO ���ݲ�Ʒ�������

		//ʹ�������깺����acctime���Ա�֤����һ��
		strncpy(fundCloseTrans.Facc_time, m_buyOrder.Facc_time, sizeof(fundCloseTrans.Facc_time) - 1);	    
	    strncpy(fundCloseTrans.Ftrans_date, fundCloseCycle.Ftrans_date, sizeof(fundCloseTrans.Ftrans_date) - 1);
		strncpy(fundCloseTrans.Ffirst_profit_date, fundCloseCycle.Ffirst_profit_date, sizeof(fundCloseTrans.Ffirst_profit_date) - 1);
		strncpy(fundCloseTrans.Fopen_date, fundCloseCycle.Fopen_date, sizeof(fundCloseTrans.Fopen_date) - 1);
		strncpy(fundCloseTrans.Fbook_stop_date, fundCloseCycle.Fbook_stop_date, sizeof(fundCloseTrans.Fbook_stop_date) - 1);
		strncpy(fundCloseTrans.Fstart_date, fundCloseCycle.Fstart_date, sizeof(fundCloseTrans.Fstart_date) - 1);
		strncpy(fundCloseTrans.Fend_date, fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fend_date) - 1);
		strncpy(fundCloseTrans.Fdue_date, fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fdue_date) - 1);
		strncpy(fundCloseTrans.Fprofit_end_date, fundCloseCycle.Fprofit_end_date, sizeof(fundCloseTrans.Fprofit_end_date) - 1);
		strncpy(fundCloseTrans.Fchannel_id, m_buyOrder.Fchannel_id, sizeof(fundCloseTrans.Fchannel_id) - 1);
		fundCloseTrans.Fstate= CLOSE_FUND_STATE_PENDING;
		fundCloseTrans.Flstate = LSTATE_VALID;
		strncpy(fundCloseTrans.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fcreate_time) - 1);

		createFundCloseTrans(fundCloseTrans,m_buyOrder.Flistid,PURTYPE_PURCHASE);
		
		close_listid = fundCloseTrans.Fid;
	}

	//���ڷ��ز���ʹ��
	m_params.setParam("trans_date", fundCloseCycle.Ftrans_date);
	m_params.setParam("first_profit_date", fundCloseCycle.Ffirst_profit_date);
	m_params.setParam("open_date", fundCloseCycle.Fopen_date);
	m_params.setParam("book_stop_date", fundCloseCycle.Fbook_stop_date);
	m_params.setParam("start_date", fundCloseCycle.Fstart_date);
	m_params.setParam("end_date", fundCloseCycle.Fdue_date);
	m_params.setParam("profit_end_date", fundCloseCycle.Fprofit_end_date);
    
	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));

	strncpy(fundSpConfig.Fspid, m_buyOrder.Fspid, sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_buyOrder.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//��Ӧ�÷���
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}
	
	//��������Fstat_flag=1ʱ����Ȼ�ռ���޶�
	string strStatDay=( SPCONFIG_STAT_DDAY & fundSpConfig.Fstat_flag )? nowdate(m_params.getString("systime").c_str()):m_buyOrder.Ftrade_date;       

       //ת��������ʱ�������˿�
	checkAndUpdateFundScope(fundSpConfig, m_buyOrder.Ftotal_fee,strStatDay);

	//��¼ckv
	setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_buyOrder.Ffund_code);

	
}

void FundTransferAck::checkCloseEndDate()
{
	if(OP_TYPE_TRANSFER_BUY_SUC != m_optype)
	{
		return;
	}

	if(m_buyOrder.Fclose_listid <= 0)
	{
		//�Ƿ�ջ��𲻼��
		return;
	}
		
	m_fundCloseTrans.Fid = m_buyOrder.Fclose_listid;
	strncpy(m_fundCloseTrans.Ftrade_id, m_buyOrder.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);

	if(m_params.getString("close_end_day") != m_fundCloseTrans.Fend_date)
	{
		alert(ERR_DIFF_END_DATE, (string("Fid:") + toString(m_fundCloseTrans.Fid) + "����˾���صķ�ս����պͱ��ؼ��㲻һ��").c_str());
		throw CException(ERR_DIFF_END_DATE, "end date diff.", __FILE__, __LINE__); 
	}
}



/**
*�����������ͣ�����Ӧ�Ĵ��� 
*/
void FundTransferAck::dealTransfer() throw (CException)
{
    switch (m_optype)  //���ﲻ�ܸĳ� m_param.getInt("op_type") ,״̬У�麯���л��m_optype�����ڲ���������
    {       
        case FUND_TRANSFER_REQ: //�깺����ɹ�
            processTransferBuyReqSuc();
            break;
			
        case FUND_TRANSFER_REDEM_SUC: //��سɹ�
            processTransferRedemSuc();
            break;
			
        case FUND_TRANSFER_REDEM_FAIL: //���ʧ��
            processTransferRedemFail();
            break;

        case FUND_TRANSFER_TRANS_SUC: //�깺ȷ�ϳɹ�
            processTransferBuySuc();
            break;
            
        case FUND_TRANSFER_REDEM_SPTIMEOUT_REDO_OK://����˾��ز��������ɹ�
            processTransferSpRedemRedoSuc();
            break;
            
        case FUND_TRANSFER_SUBACC_SAVE_REDO: //���˻���ʧ�ܲ���
            processTransferSubaccSaveRedo();
            break; 
            
        default:
            throw CException(ERR_BAD_PARAM, "op_type invalid", __FILE__, __LINE__);
            break;

    }
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
void FundTransferAck::doDraw() throw (CException)
{
    gPtrAppLog->debug("doDraw, listid[%s]  ", m_redemOrder.Fsub_trans_id);

    try
    {
        SubaccDraw(gPtrSubaccRpc, m_params.getString("ori_spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_redemOrder.Fsub_trans_id, m_params.getLong("total_fee"), m_redemOrder.Facc_time);
    }
    catch(CException& e)
    {
        //��ز��������ܷ��������жϲ����
        //���������˻���Ǯ����10����û�ɹ��ĸ澯�������ǲ���������ⲿ���ܲ���
        if((time_t)(toUnixTime(m_redemOrder.Facc_time))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval <(time(NULL)))	
        {
            char szErrMsg[256] = {0};
            snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.ת����س���10�������˻���δ���ɹ�");		  
            alert(ERR_BUDAN_TOLONG, szErrMsg);

            if(ERR_TYPE_MSG)
            {
                throw ; //�����һ��ʱ��δ�ɹ��ķ��͸澯���ٲ�����������ѭ������ѩ��,ͨ�������������в���
            }
        }
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        callErrorRpc(m_request, gPtrSysLog);
        
        throw;//�����˻�ʧ���׳��쳣������ع�
    }
}


/**
 * ���˻���ֵ
 */
bool FundTransferAck::doSave(bool exp) throw(CException)
{
    gPtrAppLog->debug("doSave, listid[%s]  ", m_buyOrder.Flistid);
    try
    {
        //�����˻��ӳٵ�֧���ɹ������ٻʱ�깺�����ѹ��
        createSubaccUser(m_request,m_buyOrder.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"));

        SubaccSave(gPtrSubaccRpc, m_buyOrder.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_buyOrder.Fsub_trans_id, m_params.getLong("total_fee"),"����ת��", m_buyOrder.Facc_time, 1);
		
    }
    catch(CException& e)
    {
        //���˻���Ӧ��ʧ�ܣ����м�Ǯʧ�ܵĶ��������
        //���֧���ص�����һ�����ʱ�����ϣ����˻������쳣�����澯
        if ((time_t)(toUnixTime(m_redemOrder.Facc_time))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval < (time(NULL)))	
        {
            char szErrMsg[256] = {0};
            snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.ת����س���10�������˻�����δ�ɹ�");        
            alert(ERR_BUDAN_TOLONG, szErrMsg);
            if(ERR_TYPE_MSG)
            {
                return false; //�����һ��ʱ��δ�ɹ��ķ��͸澯���ٲ�����������ѭ������ѩ��,ͨ�������������в���
            }
        }
		
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        
        // ���˻���ʧ�ܣ���Ӱ�콻�׵�״̬�ɹ��������ͨ�������Ϣ���Լ����˻�����
        callErrorRpc(m_request, gPtrSysLog);

        if(exp)
        {
            throw;
        }
        else
        {
            return false;
        }
    }

    return true;
}

/**
  * �����ύ�����Ҫ���µ�ckv�������ύ
  */
void FundTransferAck::updateCkvs()
{
    if (m_dynamic_info.Ftrade_id[0] != 0)
    {
        setUserDynamicInfoToCkv(m_pFundCon,m_dynamic_info,false);
    }
}


/**
  * ����������
  */
void FundTransferAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "total_fee", m_params.getString("total_fee").c_str());
	
    if (m_params.getInt("op_type") == FUND_TRANSFER_REDEM_SUC || m_params.getInt("op_type")  == FUND_TRANSFER_REDEM_TIMEOUT ) //��سɹ����߳�ʱ
    {
        if(m_fund_bind_newsp_acc.Fsp_user_id[0] ==0 && (!ERR_TYPE_MSG))
        {
            queryFundBindNewSpInfo();
        }
        CUrlAnalyze::setParam(rqst->odata, "new_sp_user", m_fund_bind_newsp_acc.Fsp_user_id);
        CUrlAnalyze::setParam(rqst->odata, "new_sp_trans_id", m_fund_bind_newsp_acc.Fsp_trans_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
        CUrlAnalyze::setParam(rqst->odata, "new_fund_code", m_params.getString("new_fund_code").c_str());
        CUrlAnalyze::setParam(rqst->odata, "buy_id", m_transferOrder.Fbuy_id);
        CUrlAnalyze::setParam(rqst->odata, "sp_billno_buy", m_buyOrder.Fcoding);
        CUrlAnalyze::setParam(rqst->odata, "acc_time", m_transferOrder.Facc_time);

		if(m_fund_newsp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
		{
			//���ص�trans_dateΪ�ⲿ�����close_start_day��ǰ�û��ͻ���˾Э������⵼��
			CUrlAnalyze::setParam(rqst->odata, "close_start_day", m_params.getString("trans_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "trans_date", m_params.getString("trans_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "first_profit_date", m_params.getString("first_profit_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "open_date", m_params.getString("open_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "book_stop_date", m_params.getString("book_stop_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "start_date", m_params.getString("start_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "end_date", m_params.getString("end_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "profit_end_date", m_params.getString("profit_end_date").c_str());
			//CUrlAnalyze::setParam(rqst->odata, "acc_time", m_params.getString("acc_time").c_str()); //�ͽ��׼�¼�ĳ�ͻ��
		}
    }
    else if (m_params.getInt("op_type") == FUND_TRANSFER_REQ) //�깺����ɹ�
    {
        CUrlAnalyze::setParam(rqst->odata, "old_sp_user", m_fund_bind_orisp_acc.Fsp_user_id);
        CUrlAnalyze::setParam(rqst->odata, "old_sp_trans_id", m_fund_bind_orisp_acc.Fsp_trans_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
        CUrlAnalyze::setParam(rqst->odata, "old_fund_code", m_params.getString("ori_fund_code").c_str());
        CUrlAnalyze::setParam(rqst->odata, "redem_id", m_transferOrder.Fredem_id);
        CUrlAnalyze::setParam(rqst->odata, "acc_time", m_transferOrder.Facc_time);
    }

    rqst->olen = strlen(rqst->odata);
    return;
}


