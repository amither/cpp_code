/**
  * FileName: redeem_index_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金赎回确认
  */
  
#include "fund_commfunc.h"
#include "redeem_index_ack_service.h"

RedeemIndexAck::RedeemIndexAck(CMySQL* mysql):AbstractRedeemSpAck(mysql)
{
    TRACE_DEBUG("[RedeemIndexAck] init");
}

void RedeemIndexAck::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 指数基金默认只使用前端收费
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_FRONT);
	}

	m_params.readStrParam(szMsg,"biz_attach",0,MAX_PARAM_LEN);
	const char* bizAttach = m_params.getString("biz_attach").c_str();
	if(strcmp(bizAttach,"")==0)
	{
		return;
	}
	m_params.readStrParam(bizAttach,"confirm_date",0,14);
	m_params.readStrParam(bizAttach,"fund_net",0,16);
	m_params.readStrParam(bizAttach,"net_date",0,14);
    return;
}
void RedeemIndexAck::CheckFundTrade() throw (CException)
{
	AbstractRedeemSpAck::CheckFundTrade();
	
	// 检查指数基金关联数据
	strncpy(m_fundIndexTrans.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	bool hasProcess = queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);
	
	//TODO:fundIndexTrans增加逻辑:第二步增加检查新表数据
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
	    char szMsg[128] = {0};
		if(!hasProcess)
		{
			snprintf(szMsg, sizeof(szMsg), "指数赎回确认,找不到关联指数数据[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
		
		if(m_fundIndexTrans.Flstate==PROCESS_TRANS_LSTATE_INVALID)
		{
			snprintf(szMsg, sizeof(szMsg), "指数赎回确认,关联指数数据无效[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id);  
			throw CException(ERR_INDEX_BUY_TRANS, szMsg, __FILE__, __LINE__);
		}
	}
}

/**
  * 赎回份额确认检查
  */
bool RedeemIndexAck::CheckRedemTradeForAckSuc() throw (CException)
{
	// 先检查公共参数
	bool result =  AbstractRedeemSpAck::CheckRedemTradeForAckSuc();
	// 检查业务参数
	CHECK_PARAM_EMPTY("fund_net");
	CHECK_PARAM_EMPTY("net_date");

    char szErrMsg[128] = {0};
	// 检查净值日期等于交易日期
	string netDate = m_params.getString("net_date");
	if(strcmp(netDate.c_str(),m_stTradeBuy.Ftrade_date)!=0)
	{
        snprintf(szErrMsg, sizeof(szErrMsg), "赎回确认净值日期与交易日期不一致[%s][%s][%s]" ,m_stTradeBuy.Flistid,netDate.c_str(),m_stTradeBuy.Ftrade_date);
		throw CException(ERR_REDEM_NET_UNCONSISTENT, szErrMsg, __FILE__, __LINE__);
	}
	// 检查净值等于最新净值
	LONG netDB = getCacheETFNet(gPtrFundDB,m_fund_sp_config.Fspid,m_fund_sp_config.Ffund_code);
	LONG net;
	string netStr = m_params.getString("fund_net");
	// 计算净值:乘以10的4次方
	if(str2Long(netStr,net,4)==-1)
	{
        snprintf(szErrMsg, sizeof(szErrMsg), "赎回确认净值计算失败[%s][%s][%ld]" ,m_stTradeBuy.Flistid,netStr.c_str(),netDB);
		throw CException(ERR_REDEM_NET_UNCONSISTENT, szErrMsg, __FILE__, __LINE__);
	}
	// 比较净值
	if(netDB!=net)
	{
        snprintf(szErrMsg, sizeof(szErrMsg), "赎回确认净值不一致[%s][%s][%ld][%ld]" ,m_stTradeBuy.Flistid,netDate.c_str(),netDB,net);
		throw CException(ERR_REDEM_NET_UNCONSISTENT, szErrMsg, __FILE__, __LINE__);
	}

	return result;
}


/**
  * 赎回确认构建参数
  */
void RedeemIndexAck::BuildRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::BuildRedemTradeForAckSuc(stRecord);
	
	// 指数型基金特性参数
	// 基金净值
	strncpy(stRecord.Ffund_value,m_params.getString("fund_net").c_str(),sizeof(stRecord.Ffund_value)-1);
	
	// 更新关联指数数据
	m_update_fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(m_update_fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(m_update_fundIndexTrans.Ftrade_id)-1);
	
	m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_REDEEM_CONFIRM;
	m_update_fundIndexTrans.Ftotal_fee = m_params.getLong("redeem2usr_fee");
	strncpy(m_update_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_update_fundIndexTrans.Ffund_net)-1);
	strncpy(m_update_fundIndexTrans.Fconfirm_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fconfirm_time)-1);
	strncpy(m_update_fundIndexTrans.Fsubacc_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fsubacc_time)-1);
	strncpy(m_update_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fmodify_time)-1);
}

void RedeemIndexAck::RecordRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::RecordRedemTradeForAckSuc(stRecord);
	//updateFundTransProcess(m_pFundCon,m_update_fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,m_update_fundIndexTrans,m_fundIndexTrans);
}
void RedeemIndexAck::BuildRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::BuildRedemTradeForInfoFail(stRecord);
	
	// 更新关联指数数据
	m_update_fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(m_update_fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(m_update_fundIndexTrans.Ftrade_id)-1);
	
	m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_REDEEM_INFO_FAIL;
	strncpy(m_update_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_update_fundIndexTrans.Ffund_net)-1);
	strncpy(m_update_fundIndexTrans.Fconfirm_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fconfirm_time)-1);
	strncpy(m_update_fundIndexTrans.Fsubacc_time,m_fundIndexTrans.Facc_time,sizeof(m_update_fundIndexTrans.Fsubacc_time)-1);
	strncpy(m_update_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fmodify_time)-1);
	strncpy(m_update_fundIndexTrans.Ffinish_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Ffinish_time)-1);
}
void RedeemIndexAck::RecordRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::RecordRedemTradeForInfoFail(stRecord);	
	//updateFundTransProcess(m_pFundCon,m_update_fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,m_update_fundIndexTrans,m_fundIndexTrans);
}

void RedeemIndexAck::BuildRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::BuildRedemTradeForAckFail(stRecord);
	
	// 更新关联指数数据
	m_update_fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(m_update_fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(m_update_fundIndexTrans.Ftrade_id)-1);
	
	m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_REDEEM_CONFIRM_FAIL;
	strncpy(m_update_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_update_fundIndexTrans.Ffund_net)-1);
	strncpy(m_update_fundIndexTrans.Fconfirm_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fconfirm_time)-1);
	strncpy(m_update_fundIndexTrans.Fsubacc_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fsubacc_time)-1);
	strncpy(m_update_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fmodify_time)-1);
	strncpy(m_update_fundIndexTrans.Ffinish_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Ffinish_time)-1);
}

void RedeemIndexAck::RecordRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::RecordRedemTradeForAckFail(stRecord);
	//updateFundTransProcess(m_pFundCon,m_update_fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,m_update_fundIndexTrans,m_fundIndexTrans);
}

void RedeemIndexAck::BuildRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::BuildRedemTradeForFinish(stRecord);
	
	// 更新关联指数数据
	m_update_fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(m_update_fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(m_update_fundIndexTrans.Ftrade_id)-1);
	
	// 赎回到余额直接更新到完成
	if(m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA||m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA_T1)
	{
		m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_REDEEM_ARRIVAL;
		strncpy(m_update_fundIndexTrans.Ffinish_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Ffinish_time)-1);
	}else{
		m_update_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_REDEEM_FETCH;
	}
	strncpy(m_update_fundIndexTrans.Ffetch_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Ffetch_time)-1);
	strncpy(m_update_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_update_fundIndexTrans.Fmodify_time)-1);
}

void RedeemIndexAck::RecordRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException)
{
	AbstractRedeemSpAck::RecordRedemTradeForFinish(stRecord);
	//updateFundTransProcess(m_pFundCon,m_update_fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,m_update_fundIndexTrans,m_fundIndexTrans);
}

void RedeemIndexAck::updateCkvs()
{
	AbstractRedeemSpAck::updateCkvs();
	// 更新过程处理单
	setFundUnfinishTransCKV(m_pFundCon,m_fundIndexTrans.Ftrade_id);
}

void RedeemIndexAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


