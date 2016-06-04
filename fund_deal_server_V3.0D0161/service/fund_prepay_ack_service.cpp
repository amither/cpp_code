/**
  * FileName: fund_prepay_ack_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-11-07
  * Description: �����׷��� ��ʵ����֤�û�(���޷���ͨ����˻�)Ԥ֧��ȷ�Ͻӿ�
  */

#include "fund_commfunc.h"
#include "fund_prepay_ack_service.h"

FundPrepayAck::FundPrepayAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

	memset(&m_fund_prepay, 0, sizeof(FundPrepay));    
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));

}

/**
  * service step 1: �����������
  */
void FundPrepayAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	
    char *pMsg = (char*)(rqst->idata);

	m_params.readStrParam(pMsg, "uin", 0, 64);
	//m_params.readIntParam(pMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(pMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(pMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(pMsg, "spid", 10, 15);
	//m_params.readLongParam(pMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(pMsg, "op_type", 1, 2);
	//m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(pMsg, "token", 1, 32);   // �ӿ�token
	

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * ���ɻ���ע����token
 */
string FundPrepayAck::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uid|fund_trans_id|spid|sp_billno|total_fee|key
    // ��������ԭ��
    ss << m_params["uin"] << "|" ;
    ss << m_params["uid"] << "|" ;
    ss << m_params["fund_trans_id"] << "|" ;
	ss << m_params["cft_trans_id"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundPrepayAck::CheckToken() throw (CException)
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
void FundPrepayAck::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();

}

/**
  * ִ���깺����
  */
void FundPrepayAck::excute() throw (CException)
{
    try
    {
        CheckParams();

         /* �������� */
        m_pFundCon->Begin();
		 
		CheckFundPrepay();

		UpdateFundPrepay();

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


/*
*���Ԥ֧��������Ϣ
*/
void FundPrepayAck::CheckFundPrepay() throw (CException)
{
	strncpy(m_fund_prepay.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(m_fund_prepay.Flistid) - 1);
	
    if(!queryFundPrepay(m_pFundCon, m_fund_prepay, true))
	{
		gPtrAppLog->error("prepay record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_PREPAY_NOT_EXIST, "prepay record not exist! ", __FILE__, __LINE__);
	}

	// ���ؼ�����
    if( (0 != strcmp(m_fund_prepay.Fspid, m_params.getString("spid").c_str())))
    {
        TRACE_ERROR("prepay record exists, spid is different! spid in db[%s], spid input[%s]", 
			m_fund_prepay.Fspid, m_params.getString("spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, spid is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("uin").empty() && 0 != strcmp(m_fund_prepay.Fuin, m_params.getString("uin").c_str()))
    {
        TRACE_ERROR("prepay record exists, uin is different! uin in db[%s], uin input[%s] ", 
			m_fund_prepay.Fuin, m_params.getString("uin").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "prepay record exists, uin is different!", __FILE__, __LINE__);
    }

	// �������
	if(m_params.getInt("op_type")==m_fund_prepay.Fauthen_state&&
		0==strcmp(m_params.getString("fund_trans_id").c_str(),m_fund_prepay.Flistid)&&
		0==strcmp(m_params.getString("cft_trans_id").c_str(),m_fund_prepay.Fcft_trans_id))
	{
		throw EXCEPTION(ERR_REPEAT_ENTRY, "repeat entry");
	}

}

void FundPrepayAck::UpdateFundPrepay()
{
	
	if(m_params.getInt("op_type") == OP_TYPE_AUTHEN_OK || m_params.getInt("op_type") == OP_TYPE_AUTHEN_FAIL)
	{
		UpdatePrepayForAuthen();
	}
	/*
	else if(m_params.getInt("op_type") == OP_TYPE_PREPAY_SUC)
	{
		UpdatePrepayForPayOk();
	}
	else if(m_params.getInt("op_type") == OP_TYPE_PREPAY_REFUND)
	{
		UpdatePrepayForRefund();
	}
	*/
	else
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input op_type error");    
	}

}


void FundPrepayAck::UpdatePrepayForAuthen()
{
	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
	//�п����״�MQ��֤ʵ��ʧ�ܣ�cgi�����˰��������ڶ�����֤ʵ����Ϊ�ǳɹ���
	stRecord.Fauthen_state= m_params.getInt("op_type");

	updateFundPrepay(m_pFundCon, stRecord);
}
void FundPrepayAck::UpdatePrepayForPayOk()
{
	// û�й����¼������
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_PURCHASE, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}
	
	//����֧���ɹ�
	if(PREPAY_OK == m_fund_prepay.Fstate)
	{
		//���뷵��
		throw CException(ERR_REPEAT_ENTRY, "the prepay record has pay! ", __FILE__, __LINE__);
	}

	//�ǳ�ʼ״̬�Ĳ����ڸ��³ɹ�������Ҳ���ܱ����������Ϊ�����Ѿ�ת���˿ֻ�ܵ������˿�����ڽ����κβ���
	if(PREPAY_INIT != m_fund_prepay.Fstate)
	{
		throw CException(ERR_PREPAY_CANNOT_UPDATE, "the prepay connot be update to pay success! ", __FILE__, __LINE__);
	}

	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);
	stRecord.Fuid = m_params.getInt("uid");
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);  
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Fstate = PREPAY_OK;
	
	updateFundPrepay(m_pFundCon, stRecord);
}
void FundPrepayAck::UpdatePrepayForRefund()
{
	//����Ϊ�˿�
	if(PREPAY_REFUND_INIT == m_fund_prepay.Fstate || PREPAY_REFUND_SUC == m_fund_prepay.Fstate)
	{
		//���뷵��
		throw CException(ERR_REPEAT_ENTRY, "the prepay record has refund! ", __FILE__, __LINE__);
	}
	
	if(PREPAY_OK != m_fund_prepay.Fstate)
	{
		//��֧���ɹ������˿�
		throw CException(ERR_PREPAY_CANNOT_UPDATE, "the prepay connot be update to pay success! ", __FILE__, __LINE__);
	}

	//���ý��׵��Ƿ񴴽��깺��,���׵������ڻ��������깺ʧ�ܲ����˿���������˿�
	ST_TRADE_FUND  trade_fund;
    memset(&trade_fund, 0, sizeof(ST_TRADE_FUND));
	if(QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_PURCHASE, &trade_fund, true))
	{
		if(PUR_REQ_FAIL != trade_fund.Fstate)
		{
			gPtrAppLog->error("fund trade record not purchase request fail cannot refund,listid[%s]", trade_fund.Flistid);
			throw CException(ERR_PREPAY_CANNOT_REFUND, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
		}
	}

	FundPrepay stRecord;
	memset(&stRecord, 0, sizeof(FundPrepay));
	
	strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Frefund_id, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Frefund_id) - 1);//TODO
	stRecord.Fstate = PREPAY_REFUND_INIT;
	strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);

	updateFundPrepay(m_pFundCon, stRecord);
}


/**
  * ����������
  */
void FundPrepayAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "acct_type", m_fund_prepay.Facct_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_fund_prepay.Ffund_code);
	CUrlAnalyze::setParam(rqst->odata, "total_fee", toString(m_fund_prepay.Ftotal_fee).c_str());
	CUrlAnalyze::setParam(rqst->odata, "openidA", m_fund_prepay.Fopenid);
	CUrlAnalyze::setParam(rqst->odata, "uin", m_fund_prepay.Fuin);
    CUrlAnalyze::setParam(rqst->odata, "channel_id", m_fund_prepay.Fstandby3);
    rqst->olen = strlen(rqst->odata);
    return;
}


