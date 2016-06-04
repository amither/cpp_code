/**
  * FileName: fund_reg_user_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-13
  * Description: �����׷��� ���𿪻� Դ�ļ�
  */
	  
#include "fund_common.h"
#include "fund_commfunc.h"
#include "fundacc_bind_sp_service.h"

#include "db_code.h"


FundBindSpAcc::FundBindSpAcc(CMySQL* mysql)
{
    m_fund_conn = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
  	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
    m_bind_exist = false;
	m_bind_spacc_exist=false;
  m_user_assess_exist=false;
    m_optype = 0;
    m_is_recovery = false;
   memset(&user_risk, 0, sizeof(ST_USER_RISK));
}


/**
  * service step 1: �����������
  */
void FundBindSpAcc::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// Ҫ�����������ݣ��ײ��ʹ��
    m_request = rqst;

    // ����ԭʼ��Ϣ
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fundacc_bind_sp_service] receives: %s", szMsg);

    // ��ȡ����
    m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "uin", 1, 64);
	m_params.readStrParam(szMsg, "openidA", 0, 64);
	m_params.readIntParam(szMsg, "acct_type", 0, 2);
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
	m_params.readIntParam(szMsg, "op_type", 1, 2);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readStrParam(szMsg, "sp_user_id", 0, 64);
	m_params.readStrParam(szMsg, "sp_trans_id", 0, 64);
	m_params.readIntParam(szMsg, "state", 0, 4);
    m_params.readStrParam(szMsg, "cre_type", 1, 5);//֧�ֻ���֤
    m_params.readStrParam(szMsg, "cre_id", 1, 23);//����֤��֪���೤��DB���ܻҶ��ڼ��޸�֧�ֳ���
    m_params.readStrParam(szMsg, "true_name", 1, 64);
    m_params.readStrParam(szMsg, "phone", 0, 21);
    m_params.readStrParam(szMsg, "mobile", 0, 21); //TODO
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readIntParam(szMsg, "cft_auth_type", 0, 4); // �Ƹ�ͨʵ��״̬
    m_params.readIntParam(szMsg, "fund_auth_type", 0, 6); // ���ͨ�û�ʵ�����
    m_params.readStrParam(szMsg, "token", 1, 32);   // �ӿ�token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

    m_optype = m_params.getInt("op_type");
	m_uin = m_params.getString("uin");
}

/*
 * ���ɻ���ע����token
 */
string FundBindSpAcc::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // ����uid|cre_type|cre_id|op_type|true_name|spid|sp_user_id|key
    // ��������ԭ��
    ss << m_params["uid"] << "|" ;
    ss << m_params["cre_type"] << "|" ;
    ss << m_params["cre_id"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << m_params["true_name"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_user_id"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * ����token
 */
void FundBindSpAcc::CheckToken() throw (CException)
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

/*
 * ���ʵ����֤��Ҫ�Ĳ���
 */
void FundBindSpAcc::CheckAuthParam()
{
    // ʵ����֤�������������Ϣ
    CHECK_PARAM_EMPTY("cre_type");
    CHECK_PARAM_EMPTY("cre_id");
    CHECK_PARAM_EMPTY("true_name");


    // ����֤�����ͱ��������֤��֧�ֹ�˾�ϰ�Ļ���֤��krixtanȷ��
    /*
    if (CRE_IDENTITY != m_params.getInt("cre_type") && CRE_RE_CARD != m_params.getInt("cre_type"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "cre_type must be identity card(=1) or re card(=5)");
    }
    */
	if (CRE_IDENTITY == m_params.getInt("cre_type") && m_params.getString("cre_id").size() != 18 )
    {
        throw EXCEPTION(ERR_CRE_ID_INVALID, "cre_id must be length 18.");
    }

	if(m_params.getInt("acct_type") == 0)
	{
		m_params.setParam("acct_type", 1);
	}

	 // �������֤���������cre_id_input��
    m_params.setParam("cre_id_input",  m_params.getString("cre_id"));
	 
	// fund��ߵĴ�������18λ�����֤����,��ڲ��������������֤�ų��ȱ���Ϊ18λ
    //m_params.setParam("cre_id", TransIdentityNumber((char*)m_params.getString("cre_id").c_str()));
}

/**
  * ����������ȡ�ڲ�����
  */
void FundBindSpAcc::CheckParams() throw (CException)
{
    // ��֤token
    CheckToken();

	// �û���Ϣ���
	CheckAuthParam();

	if(BIND_ACK == m_optype)
	{
		if(BIND_SPACC_SUC == m_params.getInt("state") )
		{
			CHECK_PARAM_EMPTY("sp_user_id");
			CHECK_PARAM_EMPTY("sp_trans_id");
		}
		
		if(BIND_SPACC_SUC != m_params.getInt("state") && BIND_SPACC_FAIL != m_params.getInt("state"))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "state invalid");
		}
		
	}
}

/**
  * ִ�л����˻�����
  */
void FundBindSpAcc::excute() throw (CException)
{
    try
    {
        CheckParams();

        /* �������� */
        m_fund_conn->Begin();

        /* �������˻���¼ */
        CheckFundBind();

        /* �������˻��󶨻���˾�����˻���¼ */
        CheckFundBindSpAcc();

        /* ע�ᴦ�� */
        DoRegProcess();

        /* �ύ���� */
        m_fund_conn->Commit();


		/*
		//�ӳٵ�֧���ɹ�ʱ����
		if(BIND_ACK == m_optype || ERR_TYPE_MSG)
		{
			//Ϊ������ʷ�û����ڰ󶨻���˾ȷ�ϵ�ʱ�������˻������������ӿ����뼴�൱�ڲ�ѯ
			//�����˻�ϵͳ��Ϊ�����û�ע��
			create_user();
		}
		*/
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_fund_conn->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }

    if (m_is_recovery)
    {
        setFundBindToKV(m_fund_conn,m_fund_bind,false);
        throw EXCEPTION(ERR_FUND_BIND_SPACC_OK, "hava bind the sp account ok.");
    }
}

/*
 * ���ͨע�����ǲƸ�ͨû��ע�����û����¿���ʱ�Զ��ָ�
 */
void FundBindSpAcc::recoveryUserBindsp(ST_FUND_UNBIND &unbindInfo) throw (CException)
{
    //���°󶨱�״̬���ָ�qqid
    strncpy(unbindInfo.Fmodify_time,m_params["systime"],sizeof(unbindInfo.Fmodify_time)-1);
    recoveryFundBind(m_fund_conn,unbindInfo);
    disableFundUnBind(m_fund_conn,unbindInfo);
    FundPayCard data;
    memset(&data,0,sizeof(FundPayCard));
    strncpy(data.Fqqid,unbindInfo.Ftrade_id,sizeof(data.Fqqid)-1);
    if (queryFundPayCard(m_fund_conn,data,true))
    {
        //memset(&data,0,sizeof(FundPayCard)); ˢ��Fsign��Ҫ��db��ȡ����������
        strncpy(data.Ftrade_id,unbindInfo.Ftrade_id,sizeof(data.Ftrade_id)-1);
        strncpy(data.Fqqid,unbindInfo.Fqqid,sizeof(data.Fqqid)-1);
        recoveryFundPayCard(m_fund_conn, data);
    }
    m_is_recovery = true;
}

/*
 * ��ѯ�����˻��Ƿ���ڣ��Լ���֤������һ����
 */
void FundBindSpAcc::CheckFundBind() throw (CException)
{
    m_bind_exist = QueryFundBindByUin(m_fund_conn, m_params.getString("uin"), &m_fund_bind, true);

    if(!m_bind_exist)
    {
        if(BIND_ACK == m_optype)
        {
            TRACE_ERROR("the fund bind record not exist by bind_ack");
            throw EXCEPTION(ERR_FUNDBIND_NOTREG, "the fund bind record not exist");
        }

        //�״ο�ͨ�����˻����ֻ��������
        CHECK_PARAM_EMPTY("mobile");

        if (m_params.getInt("uid") !=0 && checkIsUserUnbind(m_fund_conn, m_params.getInt("uid")))
        {
            ST_FUND_UNBIND unbindInfo;
            memset(&unbindInfo,0,sizeof(ST_FUND_UNBIND));
            strncpy(unbindInfo.Fqqid,m_params["uin"],sizeof(unbindInfo.Fqqid)-1);
            getLastUnbindUid(m_fund_conn,unbindInfo);
            if (m_params.getInt("uid") == unbindInfo.Fuid && m_params.getString("uin") ==unbindInfo.Fqqid  )
            {
                recoveryUserBindsp(unbindInfo);
                QueryFundBindByUin(m_fund_conn, m_params.getString("uin"), &m_fund_bind, false);
                //ˢ��Fsign,Ϊ������һ���ԣ���Ӱ��һ�����ܣ�������ʹ��unbind��(��������һ��������)���߸���QueryFundBindByUin�Ķ�̫��
                updateSignForFundBind(m_fund_conn, m_fund_bind);
            }
            else
            {
                TRACE_ERROR("user already unbind uid=%d,qqid=%s",m_params.getInt("uid"),unbindInfo.Fqqid);
                throw EXCEPTION(ERR_USER_UNBINDED, "user already unbind ");
            }
        }
        else
        {
            // ��¼�����ڣ�����������trade_id
            m_params.setParam("trade_id", GenerateTradeid(m_params.getInt("uid")).c_str());
            return;
        }
    }

    // ���ؼ�����
    if (m_params.getString("true_name") != m_fund_bind.Ftrue_name)
    {
        TRACE_ERROR("true_name in db=%s diff with input=%s", 
                    m_fund_bind.Ftrue_name, m_params.getString("true_name").c_str());
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "true_name in db diff with input");
    }
	string creId=m_params.getString("cre_id");
	if (toUpper(creId) != toUpper(m_fund_bind.Fcre_id))
    {
        TRACE_ERROR("cre_id in db=%s diff with input=%s", 
                    m_fund_bind.Fcre_id, m_params.getString("cre_id").c_str());
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "cre_id in db diff with input");
    }

	//��Ȩ���첽MQ�ص���û��uid ,�˴�����
	if(m_params.getInt("uid") != 0 && m_fund_bind.Fuid !=0 && m_params.getInt("uid") != m_fund_bind.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_fund_bind.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}

	if(m_params.getString("uin") != m_fund_bind.Fqqid)
	{
		TRACE_ERROR("uin in db=%s diff with input=%s", 
                    m_fund_bind.Fqqid, m_params.getString("uin").c_str());
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uin in db diff with input");
	}

	/**
	*�Ȳ���֤�ֻ������һ���ԣ�����ϵͳ������ֻ���������ο����Ƹ�ͨ�޸��˻ᵼ���û��״ο���ʧ���ٴο����޷����е����⣬����������α�����Ƹ�ͨ�ֻ������һ����
	if(!m_params.getString("mobile").empty() && m_params.getString("mobile") != m_fund_bind.Fmobile)
	{
		TRACE_ERROR("mobile in db=%s diff with input=%s", 
                    m_fund_bind.Fmobile, m_params.getString("mobile").c_str());
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "mobile in db diff with input");
	}
	*/

    // ��¼���ڣ�������¼�е�trade_id��mobile
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
	//ʹ���ѿ�����¼�е��ֻ���������˾���������һ���˾���ֻ����벻һ��
	//TODO�Ƿ�������������
	m_params.setParam("mobile", m_fund_bind.Fmobile);

	//TODO �ֻ��������һ�£��Ƿ�Ӧ���޸Ļ����˻��ֻ���?
}

void FundBindSpAcc::CheckFundBindSpAcc() throw (CException)
{

	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);
	m_bind_spacc_exist = queryFundBindSp(m_fund_conn, m_fund_bind_sp_acc, true);

	TRACE_DEBUG("result: Facc_time=[%s], Fstate=[%d]",m_fund_bind_sp_acc.Facc_time,m_fund_bind_sp_acc.Fstate);
	
    if(!m_bind_spacc_exist)
    {
		if(BIND_ACK == m_optype)
		{
			TRACE_ERROR("the fund bind sp account record not exist.spid:%s",m_params.getString("spid").c_str());
        	throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
		}
		
        return;
    }

	if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate)
	{
		//�˻������᲻�����������
		TRACE_ERROR("the fund bind sp account record has been frozen.");
        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
	}

	if(BIND_SPACC_FAIL== m_fund_bind_sp_acc.Fstate)
	{
		//�˻��Ѿ�����ʧ��,�������������
		TRACE_ERROR("the fund bind sp account record has been failed.");
        throw EXCEPTION(ERR_FUND_BIND_SPACC_FAIL, "the fund bind sp account record has been failed.");
	}

	TRACE_DEBUG("query fund bind sp account exist.");
	
	// ��¼���ڣ�������¼�е�����imt_id
	m_params.setParam("imt_id", m_fund_bind_sp_acc.Fimt_id);

	// ���ؼ�����
	if (m_params.getString("spid") != m_fund_bind_sp_acc.Fspid)
	{
		TRACE_ERROR("spid in db=%s diff with input=%s", 
					m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str());
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "spid in db diff with input");
	}

	if (!m_params.getString("sp_user_id").empty() && !string(m_fund_bind_sp_acc.Fsp_user_id).empty()
		&& m_params.getString("sp_user_id") != m_fund_bind_sp_acc.Fsp_user_id)
    {
        TRACE_ERROR("sp_user_id in db=%s diff with input=%s", 
                    m_fund_bind_sp_acc.Fsp_user_id, m_params.getString("sp_user_id").c_str());
        throw EXCEPTION(ERR_BIND_SPACC_INFO_DIFF, "sp_user_id in db diff with input");
    }
	if (!m_params.getString("sp_trans_id").empty() && !string(m_fund_bind_sp_acc.Fsp_trans_id).empty()
		&& m_params.getString("sp_trans_id") != m_fund_bind_sp_acc.Fsp_trans_id)
    {
        TRACE_ERROR("sp_trans_id in db=%s diff with input=%s", 
                    m_fund_bind_sp_acc.Fsp_trans_id, m_params.getString("sp_trans_id").c_str());
        throw EXCEPTION(ERR_BIND_SPACC_INFO_DIFF, "sp_trans_id in db diff with input");
    }

	//�ȼ�����һ�£��ٱ��ѳɹ�����,���ڸ�ǰ�û���ȷ�������ⲿ�����Ƿ��ڵ��û���˾�������������
	if(BIND_SPACC_SUC == m_fund_bind_sp_acc.Fstate && !ERR_TYPE_MSG && !m_is_recovery)
	{
		throw EXCEPTION(ERR_FUND_BIND_SPACC_OK, "hava bind the sp account ok.");
	}
}



void FundBindSpAcc::DoRegProcess() throw (CException)
{
	if(ERR_TYPE_MSG || m_is_recovery)
	{
		//�����ֻ�����Ŀ���ʧ��
		//�����ע����Ļָ�Ҳֱ�ӷ���
		return;
	}
	/* ����ǰ��״̬���������ͣ�����Ӧ�Ĵ��� */
    switch (m_optype)
    {
        case PRE_BIND:
            DoAuthenRegAndPreBindSpAcc();
            break;
            
        case BIND_ACK:
            DoBindSpAck();
            break;
                    
        default:
            throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
            break;
    }
}
/**
  *������������
  */
 void FundBindSpAcc::CheckAssessRisk()
{

  if(m_fund_sp_config.Frisk_ass_flag== 0 ){

      TRACE_DEBUG("risk_ass_flag is 0");
      return;
  }
  strncpy(user_risk.Fqqid, m_params.getString("uin").c_str(), sizeof(user_risk.Fqqid) - 1);
  m_user_assess_exist=queryFundUserRisk(m_fund_conn,user_risk);
  if(!m_user_assess_exist)
  {
    	 TRACE_DEBUG("user assess result not exist");
        throw EXCEPTION(ERR_USER_ASSESS, "user assess result not exist");;
   }
   m_params.setParam("risk_type", user_risk.Frisk_type);
   m_params.setParam("risk_modify_time", user_risk.Fcreate_time);
}

/**
  * ��ʵ����֤ͨ������
  */
void FundBindSpAcc::DoAuthenRegAndPreBindSpAcc() throw (CException)
{
	// ������˾��
	//��ǿ�Ʊ�������Ч����˾���û��ѿ�ͨ�˸û���˾����ʹ�ô����ƵĻ���˾
	strncpy(m_fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_sp_config.Fspid) - 1);
	checkSpidAndFundcode(m_fund_conn,m_fund_sp_config);
	
	// ����������
	CheckAssessRisk();
	
    if (!m_bind_exist)
    {
        // ���������¼��¼�����ڣ�������¼
        AddFundBind();
    }else{
    
    	// ���·�������
        UpdateRiskAssess();
    }

	if(!m_bind_spacc_exist)
	{
    	// δ����ֻ��ʹ�úϷ�����˾
    	if(m_fund_sp_config.Flstate!=LSTATE_VALID)
    	{
    		char errMsg[64]={0};
			snprintf(errMsg,sizeof(errMsg),"�̻�����״̬�����¿���.spid[%s]lstate[%d]",m_fund_sp_config.Fspid,m_fund_sp_config.Flstate);
			throw EXCEPTION(ERR_BAD_PARAM, errMsg);    
    	}
		//�����˻������˾�˻��󶨼�¼�����ڣ�������¼
		AddFundBindSpAcc();
	}else
	{
		//�Ѱ󶨳ɹ���ǰ��ᱨ���˴����м�״̬�������޸�acc_time;
		UpdateBindSpStateAcctime();
	}
}

 void FundBindSpAcc::UpdateRiskAssess()
{
	
	if(m_fund_sp_config.Frisk_ass_flag== 0 || !m_user_assess_exist){
		return;
	}
	ST_FUND_BIND fundBind;
       memset(&fundBind, 0, sizeof(ST_FUND_BIND));
	strncpy( fundBind.Ftrade_id,  m_fund_bind.Ftrade_id,  sizeof(fundBind.Ftrade_id)-1);
	fundBind.Fassess_risk_type=m_params.getInt("risk_type");
	strncpy( fundBind.Fassess_modify_time,  m_params.getString("risk_modify_time").c_str(), sizeof( fundBind.Fassess_modify_time)-1);
	UpdateFundBind(m_fund_conn, fundBind, m_fund_bind, m_params.getString("systime"));        

       m_fund_bind.Fassess_risk_type=m_params.getInt("risk_type");
       strncpy( m_fund_bind.Fassess_modify_time,  fundBind.Fassess_modify_time, sizeof( m_fund_bind.Fassess_modify_time)-1); 
	
       //���µ�ckv
       setFundBindToKV(m_fund_conn,m_fund_bind,false);		 

}
		

/**
  * ���������˻�: ���ӻ����˻�������ϵ��¼������������
  */
void FundBindSpAcc::AddFundBind()
{
    ST_FUND_BIND stFundAcc;
    memset(&stFundAcc, 0, sizeof(ST_FUND_BIND));

    /* ��������˻�������ϵ���¼ */ 
    stFundAcc.Fcre_type = m_params.getInt("cre_type");
    strncpy(stFundAcc.Fcre_id, m_params.getString("cre_id").c_str(), sizeof(stFundAcc.Fcre_id)-1);
    strncpy(stFundAcc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stFundAcc.Ftrade_id)-1);
    strncpy(stFundAcc.Fqqid, m_uin.c_str(), sizeof(stFundAcc.Fqqid) - 1);
    stFundAcc.Fuid = m_params.getInt("uid");
    strncpy(stFundAcc.Ftrue_name, m_params.getString("true_name").c_str(), sizeof(stFundAcc.Ftrue_name)-1);
    strncpy(stFundAcc.Fcre_id_orig, m_params.getString("cre_id_input").c_str(), sizeof(stFundAcc.Fcre_id_orig)-1);
    strncpy(stFundAcc.Fphone, m_params.getString("phone").c_str(), sizeof(stFundAcc.Fphone)-1);
    strncpy(stFundAcc.Fmobile, m_params.getString("mobile").c_str(), sizeof(stFundAcc.Fmobile)-1);
    stFundAcc.Fstate = REG_INIT;
    stFundAcc.Flstate = LSTATE_VALID;
	stFundAcc.Facct_type = m_params.getInt("acct_type");
	strncpy(stFundAcc.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(stFundAcc.Fchannel_id)-1);
    strncpy(stFundAcc.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stFundAcc.Fcreate_time)-1);
    strncpy(stFundAcc.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stFundAcc.Fmodify_time)-1);
	strncpy(stFundAcc.Fopenid, m_params.getString("openidA").c_str(), sizeof(stFundAcc.Fopenid)-1);
	
	stFundAcc.Fcft_auth_type = m_params.getInt("cft_auth_type")==0?CFT_AUTHEN_INIT:m_params.getInt("cft_auth_type");
	stFundAcc.Ffund_auth_type = m_params.getInt("fund_auth_type")==0?FUND_AUTHEN_INIT:m_params.getInt("fund_auth_type");
       stFundAcc.Fassess_risk_type=m_params.getInt("risk_type");
	strncpy(stFundAcc.Fassess_modify_time, m_params.getString("risk_modify_time").c_str(), sizeof(stFundAcc.Fassess_modify_time)-1);
   
       InsertFundBind(m_fund_conn, &stFundAcc);

	setFundBindToKV(m_fund_conn,stFundAcc);
}

void FundBindSpAcc::AddFundBindSpAcc()
{
	FundBindSp fund_bind_sp_acc;
	memset(&fund_bind_sp_acc, 0, sizeof(FundBindSp));

	strncpy(fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(fund_bind_sp_acc.Fspid) - 1);
	fund_bind_sp_acc.Facct_type = BIND_SPACC_GENERAL;
    fund_bind_sp_acc.Fstate = m_params.getInt("state");
	fund_bind_sp_acc.Flstate = LSTATE_VALID;

    strncpy(fund_bind_sp_acc.Facc_time, m_params.getString("systime").c_str(), sizeof(fund_bind_sp_acc.Facc_time)-1);
	strncpy(fund_bind_sp_acc.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fund_bind_sp_acc.Fcreate_time)-1);
    strncpy(fund_bind_sp_acc.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fund_bind_sp_acc.Fmodify_time)-1);
	strncpy(fund_bind_sp_acc.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(fund_bind_sp_acc.Fchannel_id)-1);
	
	insertFundBindSp(m_fund_conn, fund_bind_sp_acc);
}

void FundBindSpAcc::DoBindSpAck()
{
	
	int acct_type = BIND_SPACC_GENERAL;
	//���û���������˻�(Ĭ�ϻ���)����ע����˻�Ϊ�������˻�
	if(!ExistMasterSpAcc() && BIND_SPACC_SUC == m_params.getInt("state"))
	{
		acct_type = BIND_SPACC_MASTER;
	}
	UpdateFundBindSpAcc(acct_type);
}


bool FundBindSpAcc::ExistMasterSpAcc()
{
	FundBindSp fund_bind_sp_acc;
	memset(&fund_bind_sp_acc, 0, sizeof(FundBindSp));
	strncpy(fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(fund_bind_sp_acc.Ftrade_id) - 1);

	if(queryMasterSpAcc(m_fund_conn, fund_bind_sp_acc, true))
		return true;
	
	return false;
}


void FundBindSpAcc::UpdateFundBindSpAcc(int acct_type)
{
	FundBindSp fund_bind_sp_acc;
	memset(&fund_bind_sp_acc, 0, sizeof(FundBindSp));
	strncpy(fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(fund_bind_sp_acc.Ftrade_id) - 1);
    strncpy(fund_bind_sp_acc.Fsp_user_id, m_params.getString("sp_user_id").c_str(), sizeof(fund_bind_sp_acc.Fsp_user_id)-1);
	strncpy(fund_bind_sp_acc.Fsp_trans_id, m_params.getString("sp_trans_id").c_str(), sizeof(fund_bind_sp_acc.Fsp_trans_id)-1);
    fund_bind_sp_acc.Fstate = m_params.getInt("state");               
	strncpy(fund_bind_sp_acc.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fund_bind_sp_acc.Fmodify_time)-1);
	fund_bind_sp_acc.Fimt_id = m_params.getLong("imt_id");
	fund_bind_sp_acc.Facct_type = acct_type;

	updateFundBindSp(m_fund_conn, fund_bind_sp_acc);

	//���»���
	if(BIND_SPACC_SUC == m_params.getInt("state"))
	{
		setFundBindAllSpToKVFromDB(m_fund_conn,m_params.getString("trade_id"));
	}
}

void FundBindSpAcc::UpdateBindSpStateAcctime()
{
	if(m_params.getString("systime") == m_fund_bind_sp_acc.Facc_time)
	{
		return; //����������������ʱ����ͬ�Ĳ��������ݿ�
	}
	
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);

	strncpy(m_fund_bind_sp_acc.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_fund_bind_sp_acc.Fmodify_time)-1);
	strncpy(m_fund_bind_sp_acc.Facc_time, m_params.getString("systime").c_str(), sizeof(m_fund_bind_sp_acc.Facc_time)-1);
	m_fund_bind_sp_acc.Fstate= m_params.getInt("state");
	m_fund_bind_sp_acc.Fimt_id = m_params.getLong("imt_id");
	
	updateBindSpStateAcctime(m_fund_conn, m_fund_bind_sp_acc);
}

void FundBindSpAcc::create_user()
{
	if(m_params.getInt("uid") != 0)
	{
		createSubaccUser(m_request, m_params.getString("spid"), m_params.getString("uin"), m_params.getString("client_ip"));
	}
}


/**
  * ����������
  */
void FundBindSpAcc::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "mobile", m_params.getString("mobile").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_bind_spacc_exist ? m_fund_bind_sp_acc.Facc_time: m_params.getString("systime").c_str());

    rqst->olen = strlen(rqst->odata);
    return;
}


