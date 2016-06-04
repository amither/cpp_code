/**
  * FileName: redeem_index_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */
  
#include "fund_commfunc.h"
#include "redeem_index_req_service.h"

RedeemIndexReq::RedeemIndexReq(CMySQL* mysql):AbstractRedeemSpReq(mysql)
{
    TRACE_DEBUG("[RedeemIndexReq] init");
}

void RedeemIndexReq::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 没有特殊的业务参数
	return;
}

/**
  * 检查参数
  */
void RedeemIndexReq::CheckParams() throw (CException)
{
	AbstractRedeemSpReq::CheckParams();

	if(m_fund_sp_config.Fstandby1 == SP_TYPE_HK_STOCK)//沪港通需要判断交易日和时间段
	{
		CheckTradeLimit(m_pFundCon, getTradeDate(m_pFundCon, getSysTime()));
	}
	
	char szMsg[128]={0};
	//只支持T+1和赎回到余额
    if(m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1){
		snprintf(szMsg, sizeof(szMsg), "指数型基金赎回只支持T+1.[%s][%d]",m_params.getString("cft_bank_billno").c_str(),m_params.getInt("fetch_type"));
		throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
    }
	
	// 计算份额确认日期:T+1
	string t1Date = getCacheT1Date(m_pFundCon,m_params.getString("systime"));
	if(t1Date.empty())
	{
		snprintf(szMsg, sizeof(szMsg), "指数型基金赎回T+1日未配置[%s][%s]",m_params.getString("cft_bank_billno").c_str(),m_params.getString("systime").c_str());
		throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
	}else{
		m_params.setParam("confirm_date",t1Date);
	}
}

void RedeemIndexReq::BuildFundTrade()  throw (CException)
{
	// 构建公共特性
	AbstractRedeemSpReq::BuildFundTrade();
	
	// 检查指数基金关联数据
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeRedem.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	char szMsg[128]={0};
	if(queryFundTransProcess(m_pFundCon,m_fundIndexTrans))
	{
		snprintf(szMsg, sizeof(szMsg), "指数型基金新建赎回存在关联指数数据[%s][%s]",m_stTradeRedem.Flistid,m_fund_bind.Ftrade_id);
		throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
	}
	
	// 记录指数关联单
	strncpy(m_fundIndexTrans.Flistid,m_stTradeRedem.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Fspid,m_fund_sp_config.Fspid,sizeof(m_fundIndexTrans.Fspid)-1);
	m_fundIndexTrans.Ftype=m_fund_sp_config.Ftype;
	strncpy(m_fundIndexTrans.Ffund_code,m_fund_sp_config.Ffund_code,sizeof(m_fundIndexTrans.Ffund_code)-1);
	strncpy(m_fundIndexTrans.Ftrade_date,m_stTradeRedem.Ftrade_date,sizeof(m_fundIndexTrans.Ftrade_date)-1);
	strncpy(m_fundIndexTrans.Fconfirm_date, m_params.getString("confirm_date").c_str(),sizeof(m_fundIndexTrans.Fconfirm_date)-1);
	m_fundIndexTrans.Fpur_type=m_stTradeRedem.Fpur_type;
	m_fundIndexTrans.Fpurpose=m_stTradeRedem.Fpurpose;
	m_fundIndexTrans.Ftotal_fee=0; // 金额未确认
	m_fundIndexTrans.Ffund_units = m_stTradeRedem.Ftotal_fee; // 赎回份额
	strncpy(m_fundIndexTrans.Ffund_net,m_params.getString("fund_net").c_str(),sizeof(m_fundIndexTrans.Ffund_net)-1);
	m_fundIndexTrans.Fstate = PROCESS_TRANS_STATE_REDEEM_UNCONFIRM;
	m_fundIndexTrans.Flstate = PROCESS_TRANS_LSTATE_VALID;
	strncpy(m_fundIndexTrans.Fcreate_time,m_params.getString("systime").c_str(),sizeof(m_fundIndexTrans.Fcreate_time)-1);
	strncpy(m_fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_fundIndexTrans.Fmodify_time)-1);
	strncpy(m_fundIndexTrans.Ffinish_time,"9999-12-31 23:59:59",sizeof(m_fundIndexTrans.Ffinish_time)-1);
	strncpy(m_fundIndexTrans.Facc_time,m_stTradeRedem.Facc_time,sizeof(m_fundIndexTrans.Facc_time)-1);
}
void RedeemIndexReq::RecordFundTrade()  throw (CException)
{
	AbstractRedeemSpReq::RecordFundTrade();
	// 创建指数基金关联单
	insertFundTransProcess(m_pFundCon,m_fundIndexTrans);
}
void RedeemIndexReq::updateCkvs()
{
	AbstractRedeemSpReq::updateCkvs();
	// 更新过程处理单
	setFundUnfinishTransCKV(m_pFundCon,m_fund_bind.Ftrade_id);
}

void RedeemIndexReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


