/**
  * FileName: fund_redeem_fetch_ack.cpp	
  * Version :1.0
  * Date: 2015-3-21
  * Description: ����������ֻص��ӿ�
  */

#include "fund_commfunc.h"
#include "fund_redeem_fetch_ack_service.h"

FundRedeemFetchAck::FundRedeemFetchAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_balanceOrder, 0, sizeof(ST_BALANCE_ORDER));
}

/**
  * service step 1: �����������
  */
void FundRedeemFetchAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	TRACE_DEBUG("[fund_redem_fetch_ack_service] parseInputMsg start: ");

	// Ҫ�����������ݣ��ײ��ʹ��
    m_request = rqst;

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_fetch_ack_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readStrParam(szMsg, "uin", 1,64);
    m_params.readStrParam(szMsg, "cft_bank_billno", 0, 32);
    m_params.readStrParam(szMsg, "cft_fetch_no", 0, 32);
    m_params.readStrParam(szMsg, "spid", 10, 15);
    m_params.readIntParam(szMsg, "op_type", 1, 2);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "fetch_arrival_time", 0, 32);
    m_params.readIntParam(szMsg , "fetch_result", FETCH_RESULT_BANK_SUCCESS, FETCH_RESULT_BANK_FAIL);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token
	
    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

/**
  * ����������ȡ�ڲ�����
  */
void FundRedeemFetchAck::CheckParams() throw (CException)
{
    // ��֤token
    checkToken();
	CHECK_PARAM_EMPTY("cft_bank_billno");
	CHECK_PARAM_EMPTY("fetch_arrival_time");
	// ��������,���ݷ�������,��������ɾ��if�ж�
	if(m_params.getInt("op_type")==INF_FETCH_ARRIVAL_BALANCE)
	{	
		CHECK_PARAM_EMPTY("cft_fetch_no");
	}

}

void FundRedeemFetchAck::checkToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
       // ��������ԭ��
    ss << m_params["uin"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ; 
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << m_params["fetch_result"] << "|";
    ss << gPtrConfig->m_AppCfg.fetch_service_key;

    
    //TRACE_DEBUG("check token, sourceStr=[%s]", ss.str().c_str());
    
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    if (StrUpper(m_params.getString("token")) != StrUpper(buff))
    {   
        TRACE_DEBUG("token check failed, input=%s", 
	                m_params.getString("token").c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    } 
}

/**
  * ִ���깺����
  */
void FundRedeemFetchAck::excute() throw (CException)
{
    try
    {
        CheckParams();

        CheckFundBind();

         /* �������� */
        m_pFundCon->Begin();

		updateFetchArrival();
		
        /* �ύ���� */
        m_pFundCon->Commit();

		updateCkvs();
		
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


/*
 * ��ѯ�����˻��Ƿ����
 */
void FundRedeemFetchAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}

void FundRedeemFetchAck::updateFetchArrival() throw (CException)
{
	switch (m_params.getInt("op_type"))
	{
	case INF_FETCH_ARRIVAL_BALANCE:
		updateBalanceFetchArrival();
		break;

	case INF_FETCH_ARRIVAL_REDEEM:
		updateRedeemFetchArrival();
		break;
		
	default:
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;
	}
}
void FundRedeemFetchAck::updateBalanceFetchArrival() throw (CException)
{
	/* ��ѯ����������ּ�¼ */
	CheckFundBalanceOrder();
	
	/* ���»����������״̬ */
	UpdateFundBalanceOrder();
}

/**
  * �����������ˮ�Ƿ��Ѿ�����
  */
void FundRedeemFetchAck::CheckFundBalanceOrder() throw (CException)
{
	char errMsg[128]={0};
	strncpy(m_balanceOrder.Flistid,m_params["cft_fetch_no"],sizeof(m_balanceOrder.Flistid)-1);
	strncpy(m_balanceOrder.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_balanceOrder.Ftrade_id)-1);
	// û�н��׼�¼������
	if(!queryFundBalanceFetchByListid(m_pFundCon,m_balanceOrder,true))
	{
		snprintf(errMsg,sizeof(errMsg),"������ֻص��鲻�������ˮ[%s][%s]",m_balanceOrder.Flistid,m_balanceOrder.Ftrade_id);
		throw CException(ERR_FETCH_ARRIVAL_BALANCE_UNEXIST, errMsg, __FILE__, __LINE__);
	}
	if(m_balanceOrder.Ftype!=OP_TYPE_BA_FETCH&&m_balanceOrder.Ftype!=OP_TYPE_BA_FETCH_T1)
	{
		snprintf(errMsg,sizeof(errMsg),"������ֻص������ˮ���Ͳ���ȷ[%s][%s][%d]",m_balanceOrder.Flistid,m_balanceOrder.Ftrade_id,m_balanceOrder.Ftype);
		throw CException(ERR_FETCH_ARRIVAL_BALANCE_INVALID, errMsg, __FILE__, __LINE__);
	}
	if(m_balanceOrder.Fstate!=FUND_FETCH_OK)
	{
		snprintf(errMsg,sizeof(errMsg),"������ֻص������ˮ״̬����ȷ[%s][%s][%d]",m_balanceOrder.Flistid,m_balanceOrder.Ftrade_id,m_balanceOrder.Fstate);
		throw CException(ERR_FETCH_ARRIVAL_BALANCE_INVALID, errMsg, __FILE__, __LINE__);
	}
}

/**
  * ���»��������ˮ
  */
void FundRedeemFetchAck::UpdateFundBalanceOrder() throw (CException)
{
	ST_BALANCE_ORDER data;
	memset(&data, 0, sizeof(ST_BALANCE_ORDER));
	data.Ftype=m_balanceOrder.Ftype;
	strncpy(data.Ftrade_id,m_balanceOrder.Ftrade_id,sizeof(m_stTradeBuy.Ftrade_id)-1);
	strncpy(data.Flistid,m_balanceOrder.Flistid,sizeof(m_stTradeBuy.Flistid)-1);
	data.Ffetch_result=m_params.getInt("fetch_result");
	strncpy(data.Ffetch_arrival_time,m_params.getString("fetch_arrival_time").c_str(),sizeof(data.Ffetch_arrival_time) -1);
	updateFundBalanceOrder(m_pFundCon,data);
	
}

void FundRedeemFetchAck::updateRedeemFetchArrival() throw (CException)
{	
	/* ��ѯ�����׼�¼ */
	CheckFundTrade();
	
	/* ���»�����״̬ */
	UpdateFetchArrivalTime();
}

/**
  * �������׼�¼�Ƿ��Ѿ�����
  */
void FundRedeemFetchAck::CheckFundTrade() throw (CException)
{
	// û�н��׼�¼������
	if(!QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		0, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, cft_bank_billno[%s]  ", m_params.getString("cft_bank_billno").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	// ����״̬��Ч������
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy pay, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
	}

	// ���״̬�Ϸ���
	if(m_stTradeBuy.Fstate!= REDEM_FINISH)
	{   
		char errMsg[128] = {0};
		snprintf(errMsg,sizeof(errMsg),"��ض���״̬�쳣[%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Fstate);
		throw CException(ERR_TRADE_INVALID, errMsg, __FILE__, __LINE__);
	}

	if(0 != strcmp(m_stTradeBuy.Ftrade_id,m_fund_bind.Ftrade_id))
	{
		TRACE_ERROR("fund trade exists, trade_id is different!trade_id in db[%s], trade_id input[%s]", 
					m_stTradeBuy.Ftrade_id, m_fund_bind.Ftrade_id);
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}

	if( 0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str()))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}
	// �õ����������Ϣ
	m_params.setParam("uid", m_stTradeBuy.Fuid);
	m_params.setParam("trade_id", m_stTradeBuy.Ftrade_id);

}

/**
 *  ʵʱ֪ͨ����˾�ɹ�
 */ 
void FundRedeemFetchAck::UpdateFetchArrivalTime() throw (CException)
{

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fspe_tag = m_stTradeBuy.Fspe_tag; //�����޸ĳ�ʱ���
	stRecord.Fuid = m_stTradeBuy.Fuid;
	stRecord.Ffetch_result = m_params.getInt("fetch_result");
	stRecord.Fstate = m_stTradeBuy.Fstate;
	strncpy(stRecord.Ffetch_arrival_time,m_params.getString("fetch_arrival_time").c_str(),sizeof(stRecord.Ffetch_arrival_time) -1);
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
	
	// ���½��׵����̱�
	FundTransProcess fundIndexTrans;
	strncpy(fundIndexTrans.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(fundIndexTrans.Ftrade_id)-1);
	strncpy(fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(fundIndexTrans.Flistid)-1);
	if(queryFundTransProcess(m_pFundCon,fundIndexTrans,true))
	{
		FundTransProcess data;
		data.Fid = fundIndexTrans.Fid;
		strncpy(data.Ftrade_id,fundIndexTrans.Ftrade_id,sizeof(data.Ftrade_id)-1);
		data.Fstate = PROCESS_TRANS_STATE_REDEEM_ARRIVAL;
		strncpy(data.Ffinish_time,m_params.getString("systime").c_str(),sizeof(data.Ffinish_time)-1);
		strncpy(data.Fmodify_time,m_params.getString("systime").c_str(),sizeof(data.Fmodify_time)-1);
		//updateFundTransProcess(m_pFundCon,data);
		updateFundTransProcessWithSign(m_pFundCon,data,fundIndexTrans);
	}
}

void FundRedeemFetchAck::updateCkvs()
{
	switch (m_params.getInt("op_type"))
	{
	case INF_FETCH_ARRIVAL_BALANCE:
		setCashInTransitCKV(m_pFundCon,m_fund_bind.Ftrade_id);
		break;

	case INF_FETCH_ARRIVAL_REDEEM:
		setFundUnfinishTransCKV(m_pFundCon,m_stTradeBuy.Ftrade_id);
		break;
		
	default:
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;
	}
}



/**
  * ����������
  */
void FundRedeemFetchAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());

    rqst->olen = strlen(rqst->odata);
    return;
}


