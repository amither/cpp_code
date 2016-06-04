/**
  * FileName: factory_redeem_sp_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 基金赎回请求工厂类
  */

#include "fund_commfunc.h"
#include "factory_redeem_sp_ack_service.h"

FactoryRedeemSpAck::FactoryRedeemSpAck(CMySQL* mysql)
{
    m_pFundCon = mysql;
}

/*
 * 生成基金注册用token
 */
string FactoryRedeemSpAck::GenFundToken(CParams& params)
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|cft_bank_billno|spid|sp_billno|total_fee|key
    // 规则生成原串
    ss << params["uid"] << "|" ;
    ss << params["cft_bank_billno"] << "|" ;
    ss << params["spid"] << "|" ;
    ss << params["sp_billno"] << "|" ;
    ss << params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FactoryRedeemSpAck::CheckToken(CParams& params) throw (CException)
{
	// 生成token
    string token = GenFundToken(params);
    string inputToken = params.getString("token");
    
    if (StrUpper(inputToken) != StrUpper(token))
    {   
        TRACE_DEBUG("[FactoryRedeemSpAck]fund authen token check failed, input=%s", 
	                inputToken.c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}

void FactoryRedeemSpAck::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szSpId[MAX_SPID_LEN] = {0};
    char szMsg[MAX_MSG_LEN] = {0};
    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    TRACE_DEBUG("[FactoryRedeemSpAck] receives: %s", szMsg);
  
    CParams params;
    params.readStrParam(szMsg, "spid", 10, 15);
    params.readStrParam(szMsg, "fund_code", 0, 64);
    params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
    params.readStrParam(szMsg, "cft_bank_billno", 1, 32);
    params.readStrParam(szMsg, "sp_billno", 0, 32);
    params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    params.readStrParam(szMsg, "token", 1, 32);   // 接口token
    
    // 检查token
    CheckToken(params);
    
    // 检查spid 及fund_code 是否有效
    FundSpConfig fundSpConfig;
    memset(&fundSpConfig,0,sizeof(fundSpConfig));
    strncpy(fundSpConfig.Fspid, params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
    checkSpidAndFundcode(m_pFundCon,fundSpConfig);
    
    switch(fundSpConfig.Ftype)
    {
    	case SPCONFIG_TYPE_DEMAND:
    		redeemSpAck = new RedeemDemandAck(m_pFundCon);
    		break;
    	case SPCONFIG_TYPE_CLOSE:
    		redeemSpAck = new RedeemCloseAck(m_pFundCon);
    		break;
    	case SPCONFIG_TYPE_INSURE:
    		redeemSpAck = new RedeemDemandAck(m_pFundCon);
    		break;
    	case SPCONFIG_TYPE_ETF:
    		redeemSpAck = new RedeemIndexAck(m_pFundCon);
    		break;
    	default:
			char szErrMsg[128];
			snprintf(szErrMsg,sizeof(szErrMsg),"[FactoryRedeemSpAck]input spid[%s] has error type [%d]", fundSpConfig.Fspid,fundSpConfig.Ftype);
    		throw CException(ERR_BAD_PARAM, szErrMsg, __FILE__, __LINE__);
    };
    redeemSpAck->parseInputMsg(rqst,szMsg);
    redeemSpAck->setSpConfig(fundSpConfig);
  
}
void FactoryRedeemSpAck::excute() throw (CException)
{
	redeemSpAck->excute();
}
void FactoryRedeemSpAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
	redeemSpAck->packReturnMsg(rqst);
}

