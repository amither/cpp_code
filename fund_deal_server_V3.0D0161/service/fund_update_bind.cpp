/**
  * FileName: fund_update_bind.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2015-04-15
  * Description: �����׷��� ����û�����Ϣ Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_update_bind.h"

FundUpdateBind::FundUpdateBind(CMySQL* mysql)
{
    m_pFundCon = mysql;    
	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));

}

/**
  * service step 1: �����������
  */
void FundUpdateBind::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// Ҫ�����������ݣ��ײ��ʹ��
    m_request = rqst;

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_update_bind] receives: %s", szMsg);
	
    m_params.readStrParam(szMsg, "trade_id", 1, 32);
    m_params.readStrParam(szMsg, "email", 0, 128);
    m_params.readStrParam(szMsg, "address", 0, 128);
    m_params.readStrParam(szMsg, "phone", 0, 16);

    m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token
    m_params.readStrParam(szMsg, "qlskey", 0, 64);
    m_params.readStrParam(szMsg, "qluin", 0, 64);

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * ���ɻ���ע����token
 */
string FundUpdateBind::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����trade_id|email|address|phone|key
    // ��������ԭ��
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["email"] << "|" ;
    ss << m_params["address"] << "|" ;
    ss << m_params["phone"] << "|" ;
    ss << gPtrConfig->m_AppCfg.update_fund_bind_key;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundUpdateBind::CheckToken() throw (CException)
{
    // ����token
    string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw CException(ERR_BAD_PARAM, "input token error", __FILE__, __LINE__);    
    }   
}


/**
  * ����������ȡ�ڲ�����
  */
void FundUpdateBind::CheckParams() throw (CException)
{
    checkSession(m_params["qlskey"], m_params["qluin"], "100986");
    // ��֤token
    CheckToken();

	// ���Ҫ���µ�����
	if(m_params.getString("email").empty()&&m_params.getString("address").empty()&&m_params.getString("phone").empty()){
		throw CException(ERR_REPEAT_ENTRY, "û��Ҫ�����û��˻�������", __FILE__, __LINE__); 
	}
	// ����ֻ���
	if(!isDigitString(m_params.getString("phone").c_str()))
	{
		throw CException(ERR_BAD_PARAM, "�ֻ��ŷǷ�,��������"+m_params.getString("phone"), __FILE__, __LINE__); 
	}
}
/*
 * ��ѯ�����˻��Ƿ����
 */
void FundUpdateBind::CheckFundBind() throw (CException)
{
	//����û��˻�
	bool bind_exist = QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &m_fund_bind,true);
	
	if(!bind_exist)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "�û��ʺŷǷ�.", __FILE__, __LINE__);
    }

	// ��¼���ڣ�������¼�е�trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);

	// �������һ����
	if(!m_params.getString("phone").empty()&&strcmp(m_fund_bind.Fphone,m_params.getString("phone").c_str())==0)
	{
		m_params.setParam("phone","");
	}
	if(!m_params.getString("address").empty()&&strcmp(m_fund_bind.Faddress,m_params.getString("address").c_str())==0)
	{
		m_params.setParam("address","");
	}
	if(!m_params.getString("email").empty()&&strcmp(m_fund_bind.Femail,m_params.getString("email").c_str())==0)
	{
		m_params.setParam("email","");
	}

	//  �����������
	if(m_params.getString("email").empty()&&m_params.getString("address").empty()&&m_params.getString("phone").empty())
	{
		throw CException(ERR_REPEAT_ENTRY, "û��Ҫ�޸ĵ�����", __FILE__, __LINE__); 
	}
}

/**
  * ִ�б��
  */
void FundUpdateBind::excute() throw (CException)
{
    try
    {
        CheckParams();

		/* �������� */
		m_pFundCon->Begin();

		CheckFundBind();

        updateFundBind();

		/* �ύ���� */
        m_pFundCon->Commit();
		
        updateCKV();
		
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
void FundUpdateBind::updateCKV() throw(CException)
{
	ST_FUND_BIND stFundAcc;
	memset(&stFundAcc, 0, sizeof(ST_FUND_BIND));
	strncpy(stFundAcc.Fqqid, m_fund_bind.Fqqid, sizeof(stFundAcc.Fqqid) - 1);
	setFundBindToKV(m_pFundCon, stFundAcc, true);

}
void FundUpdateBind::updateFundBind() throw (CException)
{
	ST_FUND_BIND fundBind;
	memset(&fundBind, 0, sizeof(ST_FUND_BIND));

	strncpy(fundBind.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundBind.Ftrade_id) - 1);
	strncpy(fundBind.Femail, m_params.getString("email").c_str(), sizeof(fundBind.Femail) - 1);
	strncpy(fundBind.Faddress, m_params.getString("address").c_str(), sizeof(fundBind.Faddress) - 1);
	strncpy(fundBind.Fphone, m_params.getString("phone").c_str(), sizeof(fundBind.Fphone) - 1);
	strncpy(fundBind.Fmobile, m_params.getString("phone").c_str(), sizeof(fundBind.Fmobile) - 1);

	UpdateFundBind(m_pFundCon, fundBind, m_fund_bind, m_params.getString("systime"));
	
} 


/**
  * ����������
  */
void FundUpdateBind::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
  
    rqst->olen = strlen(rqst->odata);
    return;
}


