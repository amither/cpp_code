/**
  * FileName: fund_spaccount_freeze_service.cpp
  * Author: sivenli
  * Version :1.0
  * Date: 2015-3-31
  * Description: �����׷��� ���ͨ�˻�����ⶳ�ӿ� Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_account_freeze_service.h"



FundAccountFreeze::FundAccountFreeze(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));

    m_fund_bind_exist =false;				
    m_optype = 0;                      
}

/**
  * service step 1: ����������� �Ƿ���ҪУ��IP
  */
void FundAccountFreeze::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
  
    TRACE_DEBUG("[fund_account_freeze_service] receives: %s", szMsg);

    //�������˺Ŷ�Ӧid�����
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "cre_type", 0, 5);//֧�ֻ���֤
    m_params.readStrParam(szMsg, "cre_id", 0, 32);//����֤��֪���೤
    m_params.readStrParam(szMsg, "name", 0, 64);
    //�������ͣ������
    m_params.readIntParam(szMsg, "op_type", 1, 3);
    //����
    m_params.readIntParam(szMsg, "frozen_channel", 1, 3);
    m_params.readStrParam(szMsg, "caller_name", 0, 64);
    m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_optype = m_params.getInt("op_type");
}

/*
 * ���ɻ���ע����token
 */
string FundAccountFreeze::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uin|spid|cre_type|cre_id|op_type|frozen_channel|key
    // ��������ԭ��
    ss << m_params["uin"] << "|" ;
    ss << m_params["cre_type"] << "|" ;
    ss << m_params["cre_id"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << m_params["frozen_channel"] << "|" ;
    ss << gPtrConfig->m_AppCfg.account_freeze_key;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundAccountFreeze::CheckToken() throw (CException)
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
void FundAccountFreeze::CheckParams() throw (CException)
{
    // ��������
	//std::size_t found  = gPtrConfig->m_AppCfg.account_freeze_white_ip.find(SRC_IP);

	//if (found == std::string::npos)
	//{
		//throw CException(ERR_INVALID_IP,"��Ч��ip", __FILE__, __LINE__);
	//}

    // ��֤token
    CheckToken();
}

/**
  * ִ���깺����
  */
void FundAccountFreeze::excute() throw (CException)
{
    try
    {
        CheckParams();

         /* �������� */
        m_pFundCon->Begin();
		 	 /* �������˻��󶨻���˾�����˻���¼ */
		CheckFundBind();

        switch (m_optype)
	    {
	        case ACCOUNT_FREEZE_QUERY:
		        do_query();
		        break;

	        case ACCOUNT_FREEZE_DO_FREEZE:
		        do_freeze();
		        break;

	        case ACCOUNT_FREEZE_UNDO_FREEZE:
		        undo_freeze();
		        break;
	        default:
		        throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		        break;
	    }
        /* �ύ���� */
        m_pFundCon->Commit();
        //�����ύ�ɹ����ڸ���CKV

        if(m_optype != ACCOUNT_FREEZE_QUERY)
        {
            setFundBindToKV(m_pFundCon, m_fund_bind, false);
        }
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
*����Ƿ�󶨻���˾�ʺţ����ҿɽ���
*/
void FundAccountFreeze::CheckFundBind() throw (CException)
{
    m_fund_bind_exist = QueryFundBindByUin(m_pFundCon, m_params.getString("uin").c_str(), &m_fund_bind, true);

	if(!m_fund_bind_exist)
	{
	    throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);	
	}
    else
    {
        if( m_optype != ACCOUNT_FREEZE_QUERY )
        if(!isCreidEqual(m_params.getString("cre_id"),string(m_fund_bind.Fcre_id)))
        {
           throw CException(ERR_BAD_PARAM, "input cre_id error", __FILE__, __LINE__);
        }
    }
}

void FundAccountFreeze::do_freeze()
{
	if(m_fund_bind.Flstate == 1)
    {
        m_fund_bind.Flstate = 3;
        m_fund_bind.Ffrozen_channle = m_params.getInt("frozen_channel");
        strncpy(m_fund_bind.Fmodify_time,m_params["systime"],sizeof(m_fund_bind.Fmodify_time)-1);
        UpdateFundBindFlstate(m_pFundCon,m_fund_bind);
        ST_FUND_ACCOUNT_FREEZE_LOG account_freeze_info;
        memset(&account_freeze_info,0,sizeof(ST_FUND_ACCOUNT_FREEZE_LOG));
        strncpy(account_freeze_info.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(account_freeze_info.Ftrade_id)-1);
        strncpy(account_freeze_info.Fqqid,m_fund_bind.Fqqid,sizeof(account_freeze_info.Fqqid)-1);
        account_freeze_info.Fuid = m_fund_bind.Fuid;
        account_freeze_info.Fop_type = m_params.getInt("op_type");
        strncpy(account_freeze_info.Fcreate_time,m_params["systime"],sizeof(account_freeze_info.Fcreate_time)-1);
        account_freeze_info.Fchannel_type = m_params.getInt("frozen_channel");
        strncpy(account_freeze_info.Fop_name,m_params["caller_name"],sizeof(account_freeze_info.Fop_name)-1);
        insertFundAccountFreeze(m_pFundCon,account_freeze_info);
    }
    else
    {
        throw CException(ERR_ALREADY_FREEZE, "the uin already frozen ", __FILE__, __LINE__);    
    }
}

void FundAccountFreeze::undo_freeze()
{
	if(m_fund_bind.Flstate == 3)
    {
        m_fund_bind.Flstate = 1;
        m_fund_bind.Ffrozen_channle = m_params.getInt("frozen_channel");
        strncpy(m_fund_bind.Fmodify_time,m_params["systime"],sizeof(m_fund_bind.Fmodify_time)-1);
        UpdateFundBindFlstate(m_pFundCon,m_fund_bind);
        ST_FUND_ACCOUNT_FREEZE_LOG account_freeze_info;
        memset(&account_freeze_info,0,sizeof(ST_FUND_ACCOUNT_FREEZE_LOG));
        strncpy(account_freeze_info.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(account_freeze_info.Ftrade_id)-1);
        strncpy(account_freeze_info.Fqqid,m_fund_bind.Fqqid,sizeof(account_freeze_info.Fqqid)-1);
        account_freeze_info.Fuid = m_fund_bind.Fuid;
        account_freeze_info.Fop_type = m_params.getInt("op_type");
        strncpy(account_freeze_info.Fcreate_time,m_params["systime"],sizeof(account_freeze_info.Fcreate_time)-1);
        account_freeze_info.Fchannel_type = m_params.getInt("frozen_channel");
        strncpy(account_freeze_info.Fop_name,m_params["caller_name"],sizeof(account_freeze_info.Fop_name)-1);
        insertFundAccountFreeze(m_pFundCon,account_freeze_info);
    }
    else
    {
        throw CException(ERR_NOT_FROZEN, "the uin not frozen ", __FILE__, __LINE__);    
    }
}

void FundAccountFreeze::do_query()
{
	return;
}

/**
  * ����������
  */
void FundAccountFreeze::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    if(m_optype == ACCOUNT_FREEZE_QUERY)
    {
        if(m_fund_bind.Flstate ==1)
        {
            CUrlAnalyze::setParam(rqst->odata, "isfrozen", "0");
        }
        else
        {
            CUrlAnalyze::setParam(rqst->odata, "isfrozen", "1");
            CUrlAnalyze::setParam(rqst->odata, "frozen_channle", m_fund_bind.Ffrozen_channle);
        }
    }

    rqst->olen = strlen(rqst->odata);
    return;
}


