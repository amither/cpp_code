/**
  * FileName: fund_manage_config_service.cpp
  * Author: elijahyang
  * Version :1.0
  * Date: 2014-12-18
  * Description:理财通后台管理系统配置管理接口
  */

#include "fund_commfunc.h"
#include "fund_manage_config_service.h"

FundManageConfig::FundManageConfig(CMySQL* mysql)
{
    m_pFundCon = mysql;       
}

/**
  * service step 1: 解析输入参数
  */
void FundManageConfig::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szMsg[MAX_MSG_LEN] = {0};
	char szSpId[MAX_SPID_LEN] = {0};
	 char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 解密原始消息
	getDecodeMsg(rqst, szMsg, szSpId);

	TRACE_DEBUG("[fund_manage_config_service] receives: %s", szMsg);

	// 读取参数
	m_params.readIntParam(szMsg, "op_type", OP_TYPE_MANAGE_WHITE_LIST,OP_TYPE_MANAGE_USER_WHITELIST);	// 新增功能时可修改范围
	m_params.readIntParam(szMsg, "op_subtype", 0, OP_SUBTYPE_DEL_WHITE_LIST); //新增功能时可修改范围
	m_params.readStrParam(szMsg, "key", 0,500);
	m_params.readStrParam(szMsg, "value", 0,50000);

	m_params.readStrParam(szMsg, "spid", 0, 15);
	m_params.readStrParam(szMsg, "fund_code", 0,64);
	m_params.readLongParam(szMsg,"buyfee_tday_limit",0,MAX_LONG);
	m_params.readLongParam(szMsg,"buyfee_tday_limit_offset",0,MAX_LONG);

	 m_params.readStrParam(szMsg, "uin", 0, 64);
	 m_params.readIntParam(szMsg,"asset_limit_lev", 0 , 2);

	  m_params.readStrParam(szMsg, "op_name", 0, 64);
	   m_params.readStrParam(szMsg, "token", 0, 32);   // 接口签名

	 

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);

}

/**
  * 执行请求
  */
void FundManageConfig::excute() throw (CException)
{
	CheckParams();

	ManageConfig();
}

/**
  * 打包输出参数
  */
void FundManageConfig::packReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
	CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "key", m_params.getString("key").c_str());
	CUrlAnalyze::setParam(rqst->odata, "value", m_params.getString("value").c_str());

	rqst->olen = strlen(rqst->odata);
	return;
}

/**
  * 检查参数
  */
void FundManageConfig::CheckParams() throw (CException)
{
	int op_type = m_params.getInt("op_type");


	switch (op_type)
	{
	case OP_TYPE_MANAGE_WHITE_LIST:
		CheckWhiteListParams();
		break;
	case OP_TYPE_MANAGE_SP_CONFIG:
		CheckSpConfigParams();
		break;	
	case OP_TYPE_MANAGE_ASSET_LIMIT:
		CheckUserAssetLimitParams();
		break;	
	case OP_TYPE_MANAGE_PAY_CARD:
		CheckPayCardParams();
		break;
        case OP_TYPE_MANAGE_USER_WHITELIST:
             CheckUserWhiteParams();
		break;

	default:
		TRACE_WARN("unexcept op_type=[%s][%d]", m_params.getString("op_type").c_str(),m_params.getInt("op_type"));
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;
	}
}

void FundManageConfig::CheckUserWhiteParams()
{
    stringstream ss;
    char buff[128] = {0};
    
    ss << m_params["key"] << "|" ;
    ss << m_params["value"] << "|" ;
    ss << "e10adc3949ba59abbe56e057f20f883e";

    string token=getMd5(ss.str().c_str(), ss.str().size(), buff);

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_ERROR("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
    
    TStr2StrMap tmpMapStr;
    Tools::StrToMap(tmpMapStr,m_params["value"]);
    if (tmpMapStr["byte_index"].empty() || tmpMapStr["byte_value"].empty())
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input value error");    
    }
    
}

void FundManageConfig::updateUserWhiteList()
{
    if (false ==updateUserWhiteListValue(m_params.getString("key"),m_params.getString("value")))
    {
        throw EXCEPTION(ERR_SET_CKV, "update ckv fail!");
    }
}


void FundManageConfig::CheckWhiteListParams() throw (CException)
{
	// 检查op_subtype
	int op_subtype = m_params.getInt("op_subtype");
	if (OP_SUBTYPE_GET_WHITE_LIST != op_subtype
	&& OP_SUBTYPE_SET_WHITE_LIST != op_subtype
		&& OP_SUBTYPE_DEL_WHITE_LIST != op_subtype)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "op_subtype invalid!");
		}

		// key不能为空
		if (m_params.getString("key").empty())
		{
			throw EXCEPTION(ERR_BAD_PARAM, "key is empty!");
		}

		// key的前缀必须是white_list_
		if (m_params.getString("key").find("white_list_") != 0)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "key must be start with \"white_list_\" !");
		}

		// set时value不能为空
		if (OP_SUBTYPE_SET_WHITE_LIST == op_subtype && m_params.getString("value").empty())
		{
			throw EXCEPTION(ERR_BAD_PARAM, "set white list but value is empty!");
	}
}

void FundManageConfig::CheckSpConfigParams() throw (CException)
{
	CHECK_PARAM_EMPTY("spid");
	CHECK_PARAM_EMPTY("fund_code");
}

void FundManageConfig::CheckUserAssetLimitParams() throw (CException)
{
	 CHECK_PARAM_EMPTY("uin");
}

void FundManageConfig::CheckPayCardParams() throw (CException)
{
	 CHECK_PARAM_EMPTY("uin");
	 CHECK_PARAM_EMPTY("op_name");
	 CheckToken();
	 
}

string FundManageConfig::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uin|op_name|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["op_name"] << "|" ;
    ss << gPtrConfig->m_AppCfg.nopass_reset_paycard_key;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundManageConfig::CheckToken() throw (CException)
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

void FundManageConfig::ManageConfig() throw (CException)
{
	switch (m_params.getInt("op_type"))
	{
	case OP_TYPE_MANAGE_WHITE_LIST:
		ConfigWhiteList();
		break;
	case OP_TYPE_MANAGE_SP_CONFIG:
		ConfigSpConfig();
		break;
	case OP_TYPE_MANAGE_ASSET_LIMIT:
		ConfigUserAssetLimit();
		break;
	case OP_TYPE_MANAGE_PAY_CARD:
		ConfigPayCard();
		break;
	case OP_TYPE_MANAGE_USER_WHITELIST:
		updateUserWhiteList();
		break;
	default:
		TRACE_WARN("unexcept op_type=[%s][%d]", m_params.getString("op_type").c_str(),m_params.getInt("op_type"));
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;
	}
}
void FundManageConfig::ConfigWhiteList() throw (CException)
{
	int ret = -1;
	switch (m_params.getInt("op_subtype"))
	{
	case OP_SUBTYPE_SET_WHITE_LIST:
		ret = setWhiteListToCkv();
		break;

	case OP_SUBTYPE_GET_WHITE_LIST:
		ret = getWhiteListFromCkv();
		break;

	case OP_SUBTYPE_DEL_WHITE_LIST:
		ret = delWhiteListByKey();
		break;		

	default:
		TRACE_WARN("unexcept op_subtype=[%s][%d]", m_params.getString("op_subtype").c_str(),m_params.getInt("op_subtype"));
		throw EXCEPTION(ERR_BAD_PARAM, "op_subtype invalid");
		break;
	}

	if(ret != 0)
	{
		if (ret == ERR_KEY_NOT_EXIST)
		{
			throw EXCEPTION(ERR_MANAGE_NOT_FOUND_DATA, "key不存在!");
		}
		throw EXCEPTION(ERR_MANAGE_ERROR, "操作ckv 失败!");
	}
}	

int FundManageConfig::setWhiteListToCkv()
{
	string key = m_params.getString("key");
	string value = m_params.getString("value");
	return gCkvSvrOperator->set(CKV_KEY_WHITE_LIST, key, value);
}

int FundManageConfig::getWhiteListFromCkv()
{
	string key = m_params.getString("key");
	CParams value;
	int ret = gCkvSvrOperator->get(key, value);
	m_params.setParam("value", value.pack());
	return ret;
}

int FundManageConfig::delWhiteListByKey()
{
	string key = m_params.getString("key");
	return gCkvSvrOperator->del(NOT_REUPDATE_CKV(CKV_KEY_WHITE_LIST),key);
}

void FundManageConfig::ConfigSpConfig() throw (CException)
{
	try{
		 	    
		 FundSpConfig fundSpConfig;
	        memset(&fundSpConfig, 0, sizeof(FundSpConfig));
	
	        strncpy(fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
	        strncpy(fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
	
		
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, false))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}
	
	        fundSpConfig.Fbuyfee_tday_limit = m_params.getLong("buyfee_tday_limit") ;
	        fundSpConfig.Fbuyfee_tday_limit_offset = m_params.getLong("buyfee_tday_limit_offset") ;
	        strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
	
               updateSpBuyfeeTday(m_pFundCon, fundSpConfig);      
	     //   setAllSupportSpConfig(m_pFundCon); 不需要更新到ckv，因为两个限值在ckv中没有字段
    
		 
		 }	 
	catch(CException &e) {
		 
		 TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		 throw;
		}
}

void FundManageConfig::ConfigUserAssetLimit() throw (CException)
{
	try{
			 	    
		 ST_FUND_BIND fundBind;
               memset(&fundBind, 0, sizeof(ST_FUND_BIND));

	        if (!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &fundBind, false))
              {
                    throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
               }

               fundBind.Fasset_limit_lev = m_params.getInt("asset_limit_lev");
	        strncpy(fundBind.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundBind.Fmodify_time)-1);
               UpdateFundBindAssetLimit(m_pFundCon, fundBind);
                setFundBindToKV(m_pFundCon,fundBind,false);
		  
		 }	 
	catch(CException &e) {
		 
		 TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		 throw;
		}
}


void FundManageConfig::ConfigPayCard() throw (CException)
{
	try{
		 m_pFundCon->Begin(); 
		 
		 DeleteUserPayCard();
		 
		  m_pFundCon->Commit();
		 }	 
	catch(CException &e) {
		 m_pFundCon->Rollback();
		 TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		 throw;
		}
}


void FundManageConfig::DeleteUserPayCard() throw(CException){
	
       ST_FUND_BIND fund_bind; 
	memset(&fund_bind, 0, sizeof(ST_FUND_BIND));
	if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &fund_bind, false))
	{
		 throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
	}
       FundPayCard old_pay_card;
	memset(&old_pay_card, 0, sizeof(FundPayCard));
	strncpy(old_pay_card.Fqqid, m_params.getString("uin").c_str(), sizeof(old_pay_card.Fqqid) - 1);

	if(!queryFundPayCard(m_pFundCon,old_pay_card,true))
	{
		return;
	}

	//把支付卡信息都清空
	FundPayCard fund_pay_card;
	memset(&fund_pay_card, 0, sizeof(FundPayCard));
	strncpy(fund_pay_card.Fqqid, m_params.getString("uin").c_str(), sizeof(fund_pay_card.Fqqid) - 1);
	strncpy(fund_pay_card.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);
    //补齐刷新Fsign需要的字段
    strncpy(fund_pay_card.Ftrade_id, old_pay_card.Ftrade_id, sizeof(fund_pay_card.Ftrade_id) - 1);
	
	saveFundPayCardLog(m_pFundCon, old_pay_card, " ", "DeleteUserPayCard[fund_manage_config_service by ]"+m_params.getString("op_name"));
	updateFundPayCard(m_pFundCon, fund_pay_card);
	delPayCardToKV(m_params.getString("uin"));
}

