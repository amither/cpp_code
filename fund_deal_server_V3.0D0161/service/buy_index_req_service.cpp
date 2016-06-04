/**
  * FileName: buy_index_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-03-03
  * Description: 活期基金申请申购
  */
  
#include "fund_commfunc.h"
#include "buy_index_req_service.h"

BuyIndexReq::BuyIndexReq(CMySQL* mysql):AbstractBuySpReq(mysql)
{
    TRACE_DEBUG("[BuyIndexReq] init");
}

void BuyIndexReq::parseBizInputMsg(char* szMsg) throw (CException)
{
	// 没有特殊的业务参数
	return;
}
void BuyIndexReq::CheckParams() throw (CException)
{
	AbstractBuySpReq::CheckParams();
	char szErrMsg[128]={0};
	if(!m_params.getString("coupon_id").empty())
	{
		snprintf(szErrMsg,sizeof(szErrMsg),"[BuyIndexReq]listid[%s] not allow use coupon", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_INDEX_BAD_PARAM, szErrMsg, __FILE__, __LINE__);
	}
	// 不支持合约机
	if(m_stTradeBuy.Fpurpose == PURPOSE_FREEZE)
	{
		snprintf(szErrMsg,sizeof(szErrMsg),"[BuyIndexAck]listid[%s] not allow freeze purpose", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_INDEX_BAD_PARAM, szErrMsg, __FILE__, __LINE__);
	}

	if(m_fund_sp_config.Fstandby1 == SP_TYPE_HK_STOCK)//沪港通需要判断交易日和时间段
	{
		CheckTradeLimit(m_pFundCon, getTradeDate(m_pFundCon, getSysTime()));
	}
}
/**
  * 检查基金资产
  * ETF基金申购需校验净申购限额
  */
void BuyIndexReq::CheckFundSpLimit() throw (CException)
{	
    /* 需要根据是否存在基金交易单以及uid为0判断是否是全新用户支付后的开户，如果是，请求需要放过*/
    if ((m_bBuyTradeExist==false ||  m_stTradeBuy.Fuid != 0) && m_params.getInt("purpose") != 101) //赠送申购不检查限额
    {
        CheckETFBuyDayLimit(m_fund_bind.Fuid,m_params["systime"],m_params.getLong("total_fee"), m_fund_sp_config);
    }
}

void BuyIndexReq::CheckETFBuyDayLimit(int uid ,const string &sys_time,LONG total_fee, FundSpConfig& data) throw (CException)
{
	TRACE_DEBUG("CheckETFBuyDayLimit.[%d][%ld][%s]",uid,total_fee,data.Fspid);
	// 限制购买金额和最小步长
	if(total_fee < data.Fbuy_lower_limit || 0 != total_fee % data.Fbuy_add_limit)
	{
		//小于购买金额限制或者不满足步长要求，报错
		throw EXCEPTION(ERR_BAD_PARAM, "input total_fee error"); 
	}
	
	if(!queryFundSpAndFundcodeConfig(gPtrFundDB, data, false))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}
	string strStatDay=( SPCONFIG_STAT_DDAY & data.Fstat_flag )? 
		nowdate(sys_time):
		getCacheTradeDate(gPtrFundSlaveDB,sys_time);

	// 计算净申购金额
	if(data.Fbuyfee_tday_limit > 0)
	{
		if (string(data.Fstat_buy_tdate)< strStatDay)
        {
        	return;
        }
		// 获取T日赎回金额
		LONG redeemFee = 0;
		if (string(data.Fstat_redeem_tdate)>= strStatDay)
		{
			LONG net = getCacheETFNet(gPtrFundDB,data.Fspid,data.Ffund_code);
		   	redeemFee = data.Ftotal_redeem_tday*net/10000;
		}
        if(data.Ftotal_buyfee_tday + total_fee - redeemFee > data.Fbuyfee_tday_limit-data.Fbuyfee_tday_limit_offset|| (data.Fbuy_valid&FUNDCODE_BUY_DAY_LIMIT))
        {
        	gPtrAppLog->debug("超过申购限额[%ld+%ld-%ld>%ld-%ld]",data.Ftotal_buyfee_tday,total_fee,redeemFee,data.Fbuyfee_tday_limit,data.Fbuyfee_tday_limit_offset);
            throw EXCEPTION(ERR_SCOPE_UPPER_LIMIT, "over day buy upper limit.");
        }
	}
}

void BuyIndexReq::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	// 没有特殊的业务参数
	return;
}


