/**
  * FileName: buy_close_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#include "fund_commfunc.h"
#include "buy_close_ack_service.h"

BuyCloseAck::BuyCloseAck(CMySQL* mysql):AbstractBuySpAck(mysql)
{
    TRACE_DEBUG("[BuyCloseAck] init");
    memset(&m_fundCloseTrans, 0, sizeof(FundCloseTrans));
    memset(&m_fundCloseCycle, 0, sizeof(FundCloseCycle));
    
    m_close_fund_seqno = 0;
    m_bCloseBuyTotalAdded = false;
}

/**
  * service step 1: 解析输入参数
  */
void BuyCloseAck::parseBizInputMsg(char* szMsg)  throw (CException)
{
	m_params.readStrParam(szMsg,"biz_attach",0,MAX_PARAM_LEN);
	const char* bizAttach = m_params.getString("biz_attach").c_str();
	if(strcmp(bizAttach,"")==0)
	{
		return;
	}
   
   // 读取定期业务参数:后续改成从attach字段中获取
   m_params.readStrParam(bizAttach, "close_end_day", 0, 8);	
   m_params.readIntParam(bizAttach, "user_end_type", 0, 3);
   m_params.readIntParam(bizAttach, "end_sell_type", 0, 3);
   m_params.readStrParam(bizAttach, "end_transfer_spid", 0, 15);
   m_params.readStrParam(bizAttach, "end_transfer_fundcode", 0, 64);
   m_params.readLongParam(bizAttach,"end_plan_amt",0, MAX_LONG);

}

/**
* 检查参数，获取内部参数
*/
void BuyCloseAck::CheckParams() throw (CException)
{
	// 先检查公用特性
	AbstractBuySpAck::CheckParams();
	
	// 检查定期特性
	if(INF_PAY_OK == m_optype)
	{    
		if(CLOSE_FUND_SELL_TYPE_ANOTHER_FUND == m_params.getInt("end_sell_type"))
		{
			CHECK_PARAM_EMPTY("end_transfer_spid"); 
			CHECK_PARAM_EMPTY("end_transfer_fundcode"); 
		}

		if(CLOSE_FUND_END_TYPE_PATRIAL_REDEM == m_params.getInt("user_end_type") && 0 == m_params.getLong("end_plan_amt"))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "部分赎回需指定赎回金额"); 
		}
	}
}
bool BuyCloseAck::CheckPayRepeat() throw (CException)
{
	try{
		return AbstractBuySpAck::CheckPayRepeat();
			
	}catch(CException& e)
	{
		if (ERR_REPEAT_ENTRY == (unsigned)e.error()) 
		{
			CheckClosePayRepeat();
		}
		throw;
	}
}

/**
 * 检查定期支付确认重入
 */
bool BuyCloseAck::CheckClosePayRepeat() throw (CException)
{
	char errMsg[128]={0};
	// 定期重入逻辑
	if(m_stTradeBuy.Fclose_listid<=0)
	{
		snprintf(errMsg, sizeof(errMsg), "定期基金支付通知重入，找不到close_listid[%s]",m_stTradeBuy.Flistid); 
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	// 重入，对于定期产品要查询出数据用于返回封闭开始时间
	m_fundCloseTrans.Fid = m_stTradeBuy.Fclose_listid;
	strncpy(m_fundCloseTrans.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	if(!queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false))
	{
		snprintf(errMsg, sizeof(errMsg), "定期基金支付通知重入，找不到关联close_trans[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_id); 
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	if(0!=strcmp(m_stTradeBuy.Fend_date,m_fundCloseTrans.Fend_date))
	{
		snprintf(errMsg, sizeof(errMsg), "定期基金支付通知重入，Fend_date不一致[%s][%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Fend_date,m_fundCloseTrans.Fend_date); 
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	if(0!=strcmp(m_stTradeBuy.Ftrade_date,m_fundCloseTrans.Ftrans_date))
	{
		snprintf(errMsg, sizeof(errMsg), "定期基金支付通知重入，Ftrans_date不一致[%s][%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Ftrade_date,m_fundCloseTrans.Ftrans_date);
		throw CException(ERR_REPEAT_ENTRY_DIFF, errMsg, __FILE__, __LINE__);
	}
	m_params.setParam("trans_date", m_stTradeBuy.Ftrade_date);
	throw CException(ERR_REPEAT_ENTRY, "定期基金支付通知重入! ", __FILE__, __LINE__);
}

/**
  *  支付通知检查参数
  */
bool BuyCloseAck::CheckParamsForPay() throw (CException)
{
	bool result = AbstractBuySpAck::CheckParamsForPay();
	// 兼容原来没有传入fund_units参数
	// 定期类型需在支付通知的时候记录份额
	if(m_params.getLong("fund_units")==0)
	{
		m_params.setParam("fund_units",m_params.getLong("total_fee"));
	}
	return result;
}

/**
 * 检查并计算交易日,记录到m_params中
 */
void BuyCloseAck::CheckPayTransDate() throw(CException)
{	
	//计算交易日期	
	strncpy(m_fundCloseCycle.Fdate, calculateFundDate(m_acc_time).c_str(), sizeof(m_fundCloseCycle.Fdate) - 1);
	strncpy(m_fundCloseCycle.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundCloseCycle.Ffund_code) - 1);
	queryFundCloseCycle(m_pFundCon, m_fundCloseCycle, false);
	string trade_date = toString(m_fundCloseCycle.Ftrans_date);
	string fund_vdate = toString(m_fundCloseCycle.Ffirst_profit_date);
		
	if(	trade_date.empty()){
		//无配置日期
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound:"+string(m_stTradeBuy.Flistid));
		gPtrAppLog->error("close cycle unfound[%s], systime[%s]", m_stTradeBuy.Flistid, m_params.getString("systime").c_str());
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}	
	m_params.setParam("trade_date",trade_date);
	m_params.setParam("fund_vdate",fund_vdate);

}
// 检查用户购买定期规则
void BuyCloseAck::checkUserPermissionBuyCloseFund()
{
	//已经是需要退款不处理
	if(need_refund)
	{
		return;
	}

	//非封闭产品不检查
	if(m_fund_sp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}

	strncpy(m_fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	strncpy(m_fundCloseTrans.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundCloseTrans.Ffund_code) - 1);
	try
	{
		m_close_fund_seqno = checkPermissionBuyCloseFund(m_fundCloseTrans, m_fund_sp_config, m_fundCloseCycle.Ftrans_date,m_fundCloseCycle.Fdue_date, true);
	}
	catch(CException& e)
	{
		//无权限购买，标记退款
		refund_desc = "不满足购买定期产品权限";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_14;
	}

	m_params.setParam("trans_date", m_fundCloseCycle.Ftrans_date);
}


bool BuyCloseAck::CheckFundTradeForPay() throw (CException)
{
	// 先检查公用特性
	if(!AbstractBuySpAck::CheckFundTradeForPay())
	{
		return false;
	}
	// 检查定期购买特性
	checkUserPermissionBuyCloseFund();
	return true;		
}

void BuyCloseAck::BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 先记录基本信息
	AbstractBuySpAck::BuildFundTradeForPay(stRecord);
	
	//定期产品记录
	if(need_refund)
	{
		return;
	}

	//非封闭产品直接返回
	if(m_fund_sp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}
	
	FundCloseTrans fundCloseTrans;	
	
	//如果相等则更新，否则创建新纪录
	if(m_close_fund_seqno == m_fundCloseTrans.Fseqno)
	{
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		fundCloseTrans.Fid=m_fundCloseTrans.Fid;
		// 如果未设置不更新
		if(m_params.getInt("user_end_type")!=0){
			fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type");
			fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		}
		if(m_params.getInt("end_sell_type")!=0){
			fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO 根据产品规则决定
			strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
			strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		}
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);

		fundCloseTrans.Fpay_type = m_fundCloseTrans.Fpay_type | CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_fundCloseTrans.Fstart_total_fee + m_stTradeBuy.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_fundCloseTrans.Fcurrent_total_fee +  m_stTradeBuy.Ftotal_fee;

		saveFundCloseTrans(fundCloseTrans,m_fundCloseTrans,stRecord.Flistid,PURTYPE_PURCHASE);

		//用户记录到基金交易单中
		strncpy(stRecord.Fend_date, m_fundCloseTrans.Fend_date, sizeof(stRecord.Fend_date) - 1);
		stRecord.Fclose_listid = m_fundCloseTrans.Fid;
	}
	else
	{
		memset(&fundCloseTrans,0,sizeof(FundCloseTrans));
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		strncpy(fundCloseTrans.Fspid, m_stTradeBuy.Fspid, sizeof(fundCloseTrans.Fspid) - 1);
		strncpy(fundCloseTrans.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundCloseTrans.Ffund_code) - 1);
		fundCloseTrans.Fuid =  m_params.getInt("uid");
		fundCloseTrans.Fseqno = m_close_fund_seqno;
		fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO 根据产品规则决定
		fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
		strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);

		fundCloseTrans.Fpay_type = CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_stTradeBuy.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_stTradeBuy.Ftotal_fee;
		//新创建如果未设置则使用默认值
		fundCloseTrans.Fuser_end_type = (m_params.getInt("user_end_type") != 0) ? m_params.getInt("user_end_type") : CLOSE_FUND_END_TYPE_ALL_EXTENSION; //TODO 根据产品规则决定

		m_params.setParam("trans_date", m_fundCloseCycle.Ftrans_date);//用于返回参数使用

		strncpy(fundCloseTrans.Ftrans_date, m_fundCloseCycle.Ftrans_date, sizeof(fundCloseTrans.Ftrans_date) - 1);
		strncpy(fundCloseTrans.Ffirst_profit_date, m_fundCloseCycle.Ffirst_profit_date, sizeof(fundCloseTrans.Ffirst_profit_date) - 1);
		strncpy(fundCloseTrans.Fopen_date, m_fundCloseCycle.Fopen_date, sizeof(fundCloseTrans.Fopen_date) - 1);
		strncpy(fundCloseTrans.Fbook_stop_date, m_fundCloseCycle.Fbook_stop_date, sizeof(fundCloseTrans.Fbook_stop_date) - 1);
		strncpy(fundCloseTrans.Fstart_date, m_fundCloseCycle.Fstart_date, sizeof(fundCloseTrans.Fstart_date) - 1);
		strncpy(fundCloseTrans.Fend_date, m_fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fend_date) - 1);
		strncpy(fundCloseTrans.Fdue_date, m_fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fdue_date) - 1);
		strncpy(fundCloseTrans.Fprofit_end_date, m_fundCloseCycle.Fprofit_end_date, sizeof(fundCloseTrans.Fprofit_end_date) - 1);
		strncpy(fundCloseTrans.Fchannel_id, m_stTradeBuy.Fchannel_id, sizeof(fundCloseTrans.Fchannel_id) - 1);
		fundCloseTrans.Fstate= CLOSE_FUND_STATE_PENDING;
		fundCloseTrans.Flstate = LSTATE_VALID;
		strncpy(fundCloseTrans.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fcreate_time) - 1);
		strncpy(fundCloseTrans.Facc_time, m_acc_time.c_str(), sizeof(fundCloseTrans.Facc_time) - 1);//首次才记录，更新不修改

		createFundCloseTrans(fundCloseTrans,stRecord.Flistid,PURTYPE_PURCHASE);

		//用户记录到基金交易单中,用到期日标识唯一日期
		strncpy(stRecord.Fend_date, m_fundCloseCycle.Fdue_date, sizeof(stRecord.Fend_date) - 1);
		stRecord.Fclose_listid = fundCloseTrans.Fid;
	}

}

// 支付通知商户结果通知
// 检查交易单配置
void BuyCloseAck::CheckFundTradeForPayAck() throw (CException)
{
	// 检查通用参数
	AbstractBuySpAck::CheckFundTradeForPayAck();

	// 检查定期到期日
	if(INF_PAY_SP_INFO_SUC != m_optype)
	{
		return;
	}
	if(m_stTradeBuy.Fclose_listid <=0)
	{
		//非封闭基金不检查
		return;
	}

	m_fundCloseTrans.Fid = m_stTradeBuy.Fclose_listid;
	strncpy(m_fundCloseTrans.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);

	if(m_params.getString("close_end_day") != m_fundCloseTrans.Fend_date)
	{
		alert(ERR_DIFF_END_DATE, (string("Fid:") + toString(m_fundCloseTrans.Fid) + "基金公司返回的封闭结束日和本地计算不一致").c_str());
		throw CException(ERR_DIFF_END_DATE, "end date diff.", __FILE__, __LINE__); 
	}
}


void BuyCloseAck::updateCkvs()
{
	AbstractBuySpAck::updateCkvs();
	
	if(m_doSaveOnly)
	{
		//差错补单只补子账户,不需要补ckv		
		return;
	}
	
	if(INF_PAY_OK == m_optype &&  false == need_refund)
	{
		//记录定期ckv
		setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_stTradeBuy.Ffund_code);
	}
}


/**
  * 打包输出参数
  */
void BuyCloseAck::packBizReturnMsg(TRPC_SVCINFO* rqst)
{
	//本地的trans_date为外部定义的close_start_day，前置机和基金公司协议的问题导致
	CParams bizRequestMsg;
    //设置要修改的数据szValue
	bizRequestMsg.setParam("close_start_day",m_params.getString("trans_date").c_str());
	CUrlAnalyze::setParam(rqst->odata, "biz_attach",bizRequestMsg.pack().c_str());
}


