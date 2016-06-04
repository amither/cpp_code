/**
  * FileName: fund_risk_assess_service.cpp
  * Version :1.0
  * Date: 2015-02-28
  * Description: 风险等级评测接口
  */

#include "fund_commfunc.h"
#include "db_fund_user_risk.h"
#include "fund_risk_assess_service.h"

FundRiskAssess::FundRiskAssess(CMySQL* mysql)
{
    m_pFundCon = mysql;
    risk_type=0;
    agree_risk=0;
}

/**
  * service step 1: 解析输入参数
  */
void FundRiskAssess::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_risk_assess_service] receives: %s", szMsg);

	m_params.readStrParam(szMsg, "uin", 1, 64);
	m_params.readStrParam(szMsg, "spid", 0, 15);
	m_params.readIntParam(szMsg, "risk_score", 1,MAX_INTEGER);//是否必填，是否可能0分
	m_params.readStrParam(szMsg, "risk_subject_no", 1, 32);
	m_params.readStrParam(szMsg, "risk_answer", 1,1024);
	m_params.readStrParam(szMsg, "client_ip", 1, 16);
       m_params.readStrParam(szMsg, "token", 1, 32);   // 接口签名
       m_params.readStrParam(szMsg, "memo", 0, 255);   //备注信息
       m_params.readStrParam(szMsg, "qlskey", 0, 64);
       m_params.readStrParam(szMsg, "qluin", 0, 64);

	GetTimeNow(szTimeNow);
       m_params.setParam("systime", szTimeNow);

}

/*
 * 生成基金注册用token
 */
string FundRiskAssess::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照(uin|spid| risk_score| risk_subject_no|key)
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["risk_score"] << "|" ;
    ss << m_params["risk_subject_no"] << "|" ;
    ss << gPtrConfig->m_AppCfg.risk_assess_key;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundRiskAssess::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_ERROR("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * 检查参数，获取内部参数
  */
void FundRiskAssess::CheckParams() throw (CException)
{

    checkSession(m_params["qlskey"], m_params["qluin"], "100911");
    // 验证token
    CheckToken();
    if(m_params.getLong("risk_score")<0 ||m_params.getLong("risk_score")>100 )
   {   
	    TRACE_ERROR("input risk_score[0,100] error, input=%ld", 
	                m_params.getLong("risk_score"));
	    throw EXCEPTION(ERR_BAD_PARAM, "input risk_score error");    
    } 

}


/**
  * 执行
  */
void FundRiskAssess::excute() throw (CException)
{

    CheckParams();
    CheckUserAssessRisk(m_params.getLong("risk_score"));

    try
    {
         /* 开启事务 */
        m_pFundCon->Begin();
		 
	/* 新增用户评测流水 */	 
	 insertFundUserRiskLog();
	
        /* 更新用户评测级别 */
        UpdateRiskAssess(); 
		
        /* 提交事务 */
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
    	TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
    	m_pFundCon->Rollback();
    	throw;
    }

     CheckSpAgreeRisk();
    
}

void FundRiskAssess::CheckSpAgreeRisk()
{
    if(m_params.getString("spid").empty() )
    {
       agree_risk=0;
    	return;
    }
   vector<FundSpConfig> fundSpConfigVec;
  if( !queryFundSpConfig(m_pFundCon, fundSpConfigVec,m_params.getString("spid"), false))
  {
	TRACE_DEBUG("no fund sp config");
	agree_risk=0;
    	return;
   }
   if(fundSpConfigVec.size()>1)
   {
	TRACE_DEBUG("more than one sp config");
	agree_risk=0;
	throw EXCEPTION(ERR_BAD_PARAM, "input spid has muti fund_code"); 
    }
	FundSpConfig& fundSpConfig = fundSpConfigVec[0];
    if(fundSpConfig.Frisk_ass_flag == 0 || risk_type >= fundSpConfig.Frisk_type )
		agree_risk=1;
    else
		agree_risk=0;
}

void FundRiskAssess::CheckUserAssessRisk(int risk_score)
{
    
    vector<int> assessRiskTypeVec = gPtrConfig->m_AppCfg.AssessRiskTypeVec;

    int i=0;
	//找到第一个大于risk_score的下标值
	for(; i < assessRiskTypeVec.size(); i++)
	{
		if(assessRiskTypeVec[i] > risk_score)
		{
			break;
		}
	}
    risk_type = i ;//计算属于哪个区间
	
   if(risk_type == 0)
   {   
	    TRACE_ERROR("check risk_type error");
	    throw EXCEPTION(ERR_RISK_TYPE, "check risk_type error");    
    } 
    
}

void FundRiskAssess::insertFundUserRiskLog()
{
    ST_USER_RISK assess_user_risk;
    memset(&assess_user_risk, 0, sizeof(assess_user_risk));
    strncpy(assess_user_risk.Fqqid, m_params.getString("uin").c_str(), sizeof(assess_user_risk.Fqqid) - 1);
    strncpy(assess_user_risk.Fspid,  m_params.getString("spid").c_str(), sizeof(assess_user_risk.Fspid) - 1);
    assess_user_risk.Frisk_score=m_params.getInt("risk_score") ;
    strncpy(assess_user_risk.Fsubject_no,  m_params.getString("risk_subject_no").c_str(), sizeof(assess_user_risk.Fsubject_no) - 1);
    strncpy(assess_user_risk.Fanswer,  m_params.getString("risk_answer").c_str(), sizeof(assess_user_risk.Fanswer) - 1);
    assess_user_risk.Frisk_type=risk_type;
    strncpy(assess_user_risk.Fmemo,  m_params.getString("memo").c_str(), sizeof(assess_user_risk.Fmemo) - 1);
    strncpy(assess_user_risk.Fcreate_time,  m_params.getString("systime").c_str(), sizeof(assess_user_risk.Fcreate_time) - 1);
    strncpy(assess_user_risk.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(assess_user_risk.Fmodify_time) - 1);
    strncpy(assess_user_risk.Fclient_ip, m_params.getString("client_ip").c_str(), sizeof(assess_user_risk.Fclient_ip) - 1);
    saveFundUserRisk(m_pFundCon,assess_user_risk);
    
}
void FundRiskAssess::UpdateRiskAssess()
{
	ST_FUND_BIND fund_bind; 
	memset(&fund_bind, 0, sizeof(ST_FUND_BIND));
	if(QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &fund_bind, true))
	{
	        ST_FUND_BIND fundBind;
		 memset(&fundBind, 0, sizeof(ST_FUND_BIND));
               strncpy( fundBind.Ftrade_id,  fund_bind.Ftrade_id,  sizeof(fundBind.Ftrade_id)-1);
		 fundBind.Fassess_risk_type=risk_type;
		 strncpy( fundBind.Fassess_modify_time,  m_params.getString("systime").c_str(), sizeof( fundBind.Fassess_modify_time)-1);
		 UpdateFundBind(m_pFundCon, fundBind, fund_bind, m_params.getString("systime"));        
               fund_bind.Fassess_risk_type=risk_type;
	        strncpy( fund_bind.Fassess_modify_time,  fundBind.Fassess_modify_time, sizeof( fund_bind.Fassess_modify_time)-1);
		//更新到ckv
               setFundBindToKV(m_pFundCon,fund_bind,false);		 
	}
		
}

/**
  * 打包输出参数
  */
void FundRiskAssess::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "risk_type", risk_type);
    if(!m_params.getString("spid").empty()){
		CUrlAnalyze::setParam(rqst->odata, "spid", m_params.getString("spid").c_str());
              CUrlAnalyze::setParam(rqst->odata, "agree_risk", agree_risk);
    	}
    rqst->olen = strlen(rqst->odata);
    return;
}


