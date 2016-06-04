/**
* FileName: fund_buy_sp_ack_service.cpp
* Author: wenlonwang
* Version :1.0
* Date: 2013-8-13
* Description: 基金交易服务 基金申购确认 源文件
*/

#include "fund_commfunc.h"
#include "fund_buy_sp_ack_service.h"

extern CftLog* gPtrSysLog;

FundBuySpAck::FundBuySpAck(CMySQL* mysql)
{
	m_pFundCon = mysql;

	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));
	memset(&m_fundCloseTrans, 0, sizeof(FundCloseTrans));
	memset(&m_fundCloseCycle, 0, sizeof(FundCloseCycle));
	memset(&m_fundSpConfig, 0, sizeof(FundSpConfig));
	memset(&m_controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    memset(&m_freeze_fund,0,sizeof(ST_FREEZE_FUND));

	need_refund = false;
	pay_card_notequal = false;
	m_fund_bind_exist =false;
	refund_desc = "";
	need_updateKVFundBind = false;
	m_close_fund_seqno = 0;
    m_bCloseBuyTotalAdded = false;
	refund_reason = FUND_REFUND_REASON_0;

}

/**
* service step 1: 解析输入参数
*/
void FundBuySpAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szMsg[MAX_MSG_LEN] = {0};
	char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
	m_request = rqst;

	// 解密原始消息
	getDecodeMsg(rqst, szMsg, szSpId);
	m_spid = szSpId;

	TRACE_DEBUG("[fund_buy_sp_ack_service] receives: %s", szMsg);


	// 读取参数
	m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "uin", 0, 64);
	m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readStrParam(szMsg, "sp_billno", 0, 32);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "op_type", 1, 6);
	m_params.readStrParam(szMsg, "bind_serialno", 0, 64);
	m_params.readStrParam(szMsg, "desc", 0, 128);
	m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "pay_time", 0, 20);//格式:2013-12-30 15:21:41
	m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	m_params.readStrParam(szMsg, "close_end_day", 0, 8);	
	m_params.readIntParam(szMsg, "user_end_type", 0, 3);
	m_params.readIntParam(szMsg, "end_sell_type", 0, 3);
	m_params.readStrParam(szMsg, "end_transfer_spid", 0, 15);
	m_params.readStrParam(szMsg, "end_transfer_fundcode", 0, 64);
	m_params.readLongParam(szMsg,"end_plan_amt",0, MAX_LONG);

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);

	m_acc_time = szTimeNow;

	m_optype = m_params.getInt("op_type");

	if(m_optype == INF_PAY_OK)
	{
		//支付确认时uid必填
		m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	}

}

/*
* 生成基金注册用token
*/
string FundBuySpAck::GenFundToken()
{
	stringstream ss;
	char buff[128] = {0};

	// 按照uid|fund_trans_id|spid|sp_billno|total_fee|key
	// 规则生成原串
	ss << m_params["uid"] << "|" ;
	ss << m_params["fund_trans_id"] << "|" ;
	ss << m_params["spid"] << "|" ;
	ss << m_params["sp_billno"] << "|" ;
	ss << m_params["total_fee"] << "|" ;
	ss << gPtrConfig->m_AppCfg.pre_regkey;

	getMd5(ss.str().c_str(), ss.str().size(), buff);

	return buff;
}

/*
* 检验token
*/
void FundBuySpAck::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

	if (StrUpper(m_params.getString("token")) != StrUpper(token))
	{   
		TRACE_DEBUG("fund authen token check failed, input=%s", 
			m_params.getString("token").c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
	}   
}


/**
* 检查参数，获取内部参数
*/
void FundBuySpAck::CheckParams() throw (CException)
{
	// 验证token
	CheckToken();
	if(INF_PAY_OK == m_optype)
	{    
		if(CLOSE_FUND_SELL_TYPE_ANOTHER_FUND == m_params.getInt("end_sell_type"))
		{
			CHECK_PARAM_EMPTY("end_transfer_spid"); 
			CHECK_PARAM_EMPTY("end_transfer_fundcode"); 
		}

		if(CLOSE_FUND_END_TYPE_PATRIAL_REDEM == m_params.getInt("user_end_type") && 0 == m_params.getLong("end_plan_amt"))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "user_end_type not found, or empty"); 
		}
             //查询并校验绑定序列号参数,新用户查询绑卡接口要放到事物之外，避免事物耗时过长
             ckeckBindserialno();
	}
	else if(INF_PUR_SP_REQ_SUC == m_optype)
	{
		CHECK_PARAM_EMPTY("sp_billno");   
	}

    
}

void FundBuySpAck::ckeckBindserialno()throw (CException)
{
    if (m_params.getString("bind_serialno").empty() || m_params.getString("uin").empty() )
    {
        return;
    }
    
    FundPayCard fund_pay_card;
    memset(&fund_pay_card, 0, sizeof(FundPayCard));
    strncpy(fund_pay_card.Fqqid, m_params["uin"], sizeof(fund_pay_card.Fqqid) - 1);
    bool isFundPayCard = queryFundPayCard(m_pFundCon,fund_pay_card, false);

    //如果存在且绑定序列号一致
    if(isFundPayCard && m_params.getString("bind_serialno") == fund_pay_card.Fbind_serialno)
    {
        m_params.setParam("pay_bank_type", fund_pay_card.Fbank_type);
        m_params.setParam("pay_card_tail", fund_pay_card.Fcard_tail);
        return ;
    }
    
    if(!queryBindCardInfo(fund_pay_card.Fqqid, m_params.getString("bind_serialno"), m_bindCareInfo))
    {
        m_bindCareInfo.clear(); //异常情况在事物中处理，此处只做简单校验
    }
    else
    {
        m_params.setParam("pay_bank_type", m_bindCareInfo["bank_type"]);
        m_params.setParam("pay_card_tail", m_bindCareInfo["card_tail"]);
    }
}


/**
* 执行申购请求
*/
void FundBuySpAck::excute() throw (CException)
{
	try
	{
		CheckParams();

		/* 开启事务 */
		m_pFundCon->Begin();

             /* ckv操作放到事物之后真正提交 */
             gCkvSvrOperator->beginCkvtrans();

		/* 查询基金交易记录 */
		CheckFundTrade();

		/* 检查基金账户记录，对用户加锁，后续操作依赖对单个用户的不可并发操作 */
		CheckFundBind();

		/* 检查基金账户绑定基金公司交易账户记录 */
		CheckFundBindSpAcc();

		/* 更新基金交易记录 */
		UpdateTradeState();

		/* 提交事务 */
		m_pFundCon->Commit();

		/* 更新各类ckv ,放在事务之后是避免事务回滚却写入ckv的问题*/
             gCkvSvrOperator->commitCkvtrans();
        
		//updateCkvs();

		if(INF_PAY_OK == m_optype)
		{
			if(need_refund)
			{
				//给前置机抛出特殊异常，阻止前置机通知基金公司支付成功，但前置机需要返回支付回调为成功，后续由批跑来完成退款
				throw CException(ERR_BUY_RECORD_NEED_REFUND, "fund purchase record need refund! ", __FILE__, __LINE__);
			}
			else
			{
				//更新当前交易基金为默认基金，为了不影响主流程该函数中会单独启动事物失败不抛异常
				//已无默认主基金概念，不再更新主基金
				//UpdateDefaultTradeAcc();

				//调用子账户增加基金账户余额,第一次调用子账户失败会报错出去，但是申购单状态已修改成支付成功，当支付再次回调的时候没有在触发子账户
				doSave();
			}
		}

		//对份额转换做处理
		//updateChangeSp();

	}
	catch (CException& e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s,rollback ckv and db ops", e.file(), e.line(), e.error(), e.what());

             //回滚db前先回滚本地ckv
             gCkvSvrOperator->rollBackCkvtrans();
             
		m_pFundCon->Rollback();

		if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
			&& (ERR_REGOK_ALREADY != (unsigned)e.error()))
		{
			throw;
		}
	}
}

/*
//已无默认主基金概念，不再更新主基金
void FundBuySpAck::UpdateDefaultTradeAcc()
{
	if (need_refund  || m_optype != INF_PAY_OK)
	{
		return;
	}

	if (m_fund_bind_sp_acc.Facct_type == BIND_SPACC_MASTER)
	{
		//已经是默认交易账户则返回
		return;
	}

	if (m_fund_bind_sp_acc.Ftrade_id[0] == 0) //绑定表没有查询到tradeid直接返回
	{
		return;
	}

	try
	{
		// 开启事务
		m_pFundCon->Begin();
		ST_FUND_BIND tmpFundBind;
		//加全局锁
		if (QueryFundBindByTradeid(m_pFundCon, m_fund_bind_sp_acc.Ftrade_id, &tmpFundBind, true) == false)
		{
			throw  CException(ERR_UNKNOWN, "in UpdateDefaultTradeAcc QueryFundBindByTradeid not exist! ", __FILE__, __LINE__);
		}

		changeDefaultTradeAcc(m_pFundCon, m_fund_bind_sp_acc.Ftrade_id, m_fund_bind_sp_acc.Fspid, "", m_params["systime"]);

		//提交事务
		m_pFundCon->Commit();

		//更新ckv数据
		//setFundBindAllSpFromKV(m_pFundCon,m_fund_bind_sp_acc.Ftrade_id);
	}
	catch(CException e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		m_pFundCon->Rollback();
	}

	return;
}
*/

/*
* 查询基金账户是否存在
*/
void FundBuySpAck::CheckFundBind() throw (CException)
{
	if(need_refund)
	{
		return;
	}

	if(	m_optype ==	INF_PAY_SP_INFO_SUC || m_optype == INF_PAY_SP_INFO_TIMEOUT)
	{
		//支付确认的不检查，一定要成功
		return;
	}

	if(!m_params.getString("trade_id").empty()) 
	{
		//使用forupdate 查询，防止并发修改账户信息
		m_fund_bind_exist = QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &m_fund_bind, true);
	}
	else
	{
		m_fund_bind_exist = QueryFundBindByUin(m_pFundCon, m_params.getString("uin").c_str(), &m_fund_bind, true);
	}

	if(!m_fund_bind_exist)
	{
		// 支付通知接口判断退款,  非支付接口异常
		if(m_optype !=	INF_PAY_OK)
		{
			throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
		}
		// 判断退款
		//如果用户已经注销则退款
		if (m_params.getInt("uid") !=0 && checkIsUserUnbind(m_pFundCon, m_params.getInt("uid")))
		{ 
			TRACE_ERROR("user unbind ,so refund");
			refund_desc = "用户已经注销";
			need_refund = true; 
			refund_reason = FUND_REFUND_REASON_1;
		}      
		return; //没查询到可能是MQ还没到，不能报错也不能退款，退款在检查基金公司信息的地方统一处理
		
	}
	// 账户基金信息不一样，直接打退款标记
	if(m_params.getInt("uid") != 0 && m_fund_bind.Fuid != 0 && m_params.getInt("uid") != m_fund_bind.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
			m_fund_bind.Fuid, m_params.getInt("uid"));
		refund_desc = "申购请求和支付uid不一致";
		need_refund = true; 
		refund_reason = FUND_REFUND_REASON_2;
	}
	if( !(m_params.getString("uin").empty()) && m_params.getString("uin") != m_fund_bind.Fqqid)
	{
		TRACE_ERROR("Fqqid in db=%s diff with input=%s", 
			m_fund_bind.Fqqid, m_params.getString("uin").c_str());
		refund_desc = "申购请求和支付uin不一致";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_3;
	}
	if( !(m_params.getString("trade_id").empty()) && m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
	{
		TRACE_ERROR("Ftrade_id in db=%s diff with input=%s", 
			m_fund_bind.Ftrade_id, m_params.getString("trade_id").c_str());
		refund_desc = "申购请求和支付tradeId不一致";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_4;
	}else{
		m_params.setParam("trade_id",m_fund_bind.Ftrade_id);
	}

}

static bool payNotifyOvertime(string pay_suc_time)
{
	if(pay_suc_time.size() == 14)
	{
		//YYYYMMDDHHMMSS 转YYYY-MM-DD HH:MM:SS
		pay_suc_time = changeDatetimeFormat(pay_suc_time);
	}
	int pay_time = toUnixTime(pay_suc_time.c_str());
	if(pay_time + gPtrConfig->m_AppCfg.paycb_overtime_inteval < (int)(time(NULL)) )
	{
		return true;	
	}

	return false;
}

/*
*检查是否绑定基金公司帐号，并且可交易
*/
void FundBuySpAck::CheckFundBindSpAcc() throw (CException)
{
	if(need_refund && m_optype == INF_PAY_OK)
	{
		//前面发现交易单支付用户和发起用户不一致的，会退款，不要在处理此处，当两个账户都已经开通了理财户时，后面的代码已经无法拦截
		//而且可能导致变更主交易账户的风险
		return; 
	}

	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	bool bind_spacc_exist = queryFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false); 
	
	char bindErrMsg[256] = {0};
	unsigned bindErrCode = 0;
	string refundNowMsg = ""; //  立即退款说明
	// 交易帐号账户不存在
	if(!bind_spacc_exist||BIND_SPACC_INIT==m_fund_bind_sp_acc.Fstate)
	{
		snprintf(bindErrMsg, sizeof(bindErrMsg), "用户[%s]绑定商户[%s]帐户不存在", m_params.getString("trade_id").c_str(),m_params.getString("spid").c_str());
		bindErrCode=ERR_NOT_BIND_SP_ACC;
	}	
	// 交易账户被冻结
	else if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate)
	{
		snprintf(bindErrMsg, sizeof(bindErrMsg), "用户[%s]绑定商户[%s]帐户已冻结", m_params.getString("trade_id").c_str(),m_params.getString("spid").c_str());
		bindErrCode=ERR_SP_ACC_FREEZE;
		refundNowMsg = "账户被冻结";
		refund_reason = FUND_REFUND_REASON_5;
	}	
	// 交易账户开户失败
	else if(BIND_SPACC_SUC!= m_fund_bind_sp_acc.Fstate)
	{
		snprintf(bindErrMsg, sizeof(bindErrMsg), "用户[%s]绑定商户[%s]帐户开户失败", m_params.getString("trade_id").c_str(),m_params.getString("spid").c_str());
		bindErrCode=ERR_FUND_BIND_SPACC_FAIL;
		refundNowMsg = "账户开户失败";
		refund_reason = FUND_REFUND_REASON_6;
	}	
	// 绑定正常返回
	else{		
		return;
	}

	// 统一输出失败日志,处理绑定失败逻辑
	TRACE_ERROR("%s",bindErrMsg);
	
	// 支付通知立即退款
	if(INF_PAY_OK == m_optype&&refundNowMsg!=""){ //立即退款
		refund_desc = refundNowMsg;
		need_refund = true;
	}
	//支付通知超时退款
	else if(INF_PAY_OK == m_optype&&payNotifyOvertime(m_params.getString("pay_time"))){
		refund_desc = "支付超过一定时间仍未开户成功转退款";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_7;
	}
	//直接抛错,支付通知延迟等待支付回调
	else{ 
		throw EXCEPTION(bindErrCode, bindErrMsg);
	}

}


/**
* 检查基金交易记录是否已经生成,检查订单前置状态
*/
void FundBuySpAck::CheckFundTrade() throw (CException)
{
	// 没有购买记录，报错
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_BUY, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	if (m_stTradeBuy.Fpurpose != PURPOSE_BALANCE_BUY ) //理财通余额支付
	{
		if ((INF_PAY_OK == m_optype)) // 余额申购
		{
			CHECK_PARAM_EMPTY("cft_trans_id");  
		} 
	}

	// 物理状态无效，报错
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy record, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy record, lstate is invalid. ", __FILE__, __LINE__);
	}

	// 校验关键参数
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}

	if(m_params.getString("spid") != m_stTradeBuy.Fspid)
	{
		gPtrAppLog->error("fund buy, spid is different! spid in db[%s], spid input[%s] ", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy, spid is different!", __FILE__, __LINE__);
	}

	//理财通余额支付是通过余额总账户做的支付，acctime 需要和余额流水单中的acctime一致
	if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY && (m_optype == INF_PAY_OK || m_optype == INF_PAY_SP_INFO_SUC || m_optype==INF_PAY_SP_INFO_TIMEOUT))
	{
		ST_BALANCE_ORDER fetch_data;
		memset(&fetch_data,0,sizeof(ST_BALANCE_ORDER));
		strncpy(fetch_data.Flistid,m_params["fund_trans_id"],sizeof(fetch_data.Flistid)-1);
		strncpy(fetch_data.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(fetch_data.Ftrade_id)-1);
		fetch_data.Ftype = OP_TYPE_BA_BUY;
		if (false == queryFundBalanceOrder(m_pFundCon, fetch_data,  false))
		{
			throw CException(ERR_BA_ORDER_NOT_EXIST, "query FundBalance Order fail!", __FILE__, __LINE__);
		}
		m_params.setParam("pay_time", fetch_data.Facc_time);
		m_acc_time = fetch_data.Facc_time;
	}

	//购买账户信息不一样，直接打退款标记，串cookie 用户支付回调的时候交易单中的uid 和支付回调的不一样，而且用户可能未开理财账户
	//这种类型的退款要非常小心，因为交易单的中的uid 和输入的uid 不一致，是否可以直接退款
	if(m_params.getInt("uid") != 0 && m_stTradeBuy.Fuid != 0 && m_params.getInt("uid") != m_stTradeBuy.Fuid)
	{
		ST_FUND_BIND m_fund_bind_query; 
		bool bind_exist = QueryFundBindByUid(m_pFundCon, m_stTradeBuy.Fuid, &m_fund_bind_query, false);
		if(!bind_exist)
		{
			throw CException(ERR_FIND_BIND_NOT_EXIST, "QueryFundBindByUid fail!", __FILE__, __LINE__);
		}
		else
		{
			//有记录，成功，QueryFundBindByUid查到的uid,fqqid设置到参数里
			if(true == checkInnerBalancePayForReBuy(m_pFundCon,m_params["fund_trans_id"],m_params["total_fee"],m_fund_bind_query.Fqqid))
			{
				m_params.setParam("uin", m_fund_bind_query.Fqqid);
				m_params.setParam("uid", m_fund_bind_query.Fuid);
			}
			else
			{
				//没有记录，走退款逻辑
				TRACE_ERROR("uid in db=%d diff with input=%d", m_stTradeBuy.Fuid, m_params.getInt("uid"));
				refund_desc = "申购记录uid和支付uid不一致";
				need_refund = true; //不能抛出异常，否则会阻止申购请求，导致申购确认无法到达
				refund_reason = FUND_REFUND_REASON_8;
			}	
		}
	}

	// 得到账户相关信息
	m_params.setParam("trade_id", m_stTradeBuy.Ftrade_id);
	m_params.setParam("sub_trans_id", m_stTradeBuy.Flistid);//用于更新子账户

}

/**
*根据前置状态和请求类型，做相应的处理 
*/
void FundBuySpAck::UpdateTradeState()
{
	if(ERR_TYPE_MSG && (m_stTradeBuy.Fstate == PAY_OK || m_stTradeBuy.Fstate == PURCHASE_SUC ))
	{
		//差错补单并且申购记录已成功则只补子账户，如果交易单没成功直接补子账户，会出现Facc_time为空的情况，也不合理
		return;
	}
	switch (m_optype)
	{
	case INF_PUR_SP_REQ_SUC:
	case INF_PUR_SP_REQ_FAIL:
		UpdateFundTradeForReq();
		break;

	case INF_PAY_OK:
		UpdateFundTradeForPay();
		break;

	case INF_PAY_SP_INFO_SUC:
	case INF_PAY_SP_INFO_TIMEOUT:
		UpdateFundTradeForAck();
		//申购确认成功后，给券服务发异步请求，尽量把券充值到账
		UpdateFundTradeForCoupon();
		break;



	default:
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;

	}

}

/**
* 券消费接口
* 控制券申购单状态、发券状态和实际充值等操作
*/
void FundBuySpAck::UpdateFundTradeForCoupon() throw (CException)
{
	gPtrAppLog->debug("UpdateFundTradeForCoupon called, listid [%s] cft_trans_id [%s]",m_params.getString("fund_trans_id").c_str(),m_params.getString("cft_trans_id").c_str());

	//确认发券状态为未使用，否则报错
	//先不做了，免得增加券服务负担

	//CheckFundTrade()中已经检查申购单存在，且已经获取到couponid字段

	//如果没有couponid，则不给券服务发异步消息
	if("" == string(m_stTradeBuy.Fcoupon_id))
	{
		//TRACE_ERROR("UpdateFundTradeForCoupon, coupon_id empty!");
		return;
	}

	//如果没有微信支付accid，则无法进行券消费
	if(m_params.getString("uin").empty())
	{
		//TRACE_ERROR("UpdateFundTradeForCoupon, uin empty!");
		return;
	}

	//先删除ckv中活动有券的记录,如果删除失败，就不尝试进行券消费了。
	/*string trialkey = "lct_action_20001_" +  m_params.getString("uin");
	if(0 != gCkvSvrOperator->del(trialkey))
	{
		TRACE_ERROR("UpdateFundTradeForCoupon, del ckv [%s] failed!",trialkey.c_str());
		return;
	}*/

	//发送异步请求给券服务，将此券消费
	char szMsg[MAX_MSG_LEN + 1] = {0};
	//判断账户类型 手Q-1，微信-2
    string strAcctType="1";
	string::size_type idx = m_params.getString("uin").find("@wx.tenpay.com");
    if ( idx != string::npos ) strAcctType="2";
	// 组装关键参数
	CUrlAnalyze::setParam(szMsg, "acct_type", strAcctType.c_str(), true);
	CUrlAnalyze::setParam(szMsg, "acct_id", m_params.getString("uin").c_str());
	CUrlAnalyze::setParam(szMsg, "coupon_id", m_stTradeBuy.Fcoupon_id);
	CUrlAnalyze::setParam(szMsg, "listid", m_stTradeBuy.Flistid);
	CUrlAnalyze::setParam(szMsg, "spid", m_stTradeBuy.Fspid);
	CUrlAnalyze::setParam(szMsg, "fund_code", m_stTradeBuy.Ffund_code);
	CUrlAnalyze::setParam(szMsg, "channel_id", m_stTradeBuy.Fchannel_id);
	CUrlAnalyze::setParam(szMsg, "client_ip", m_params.getString("client_ip").c_str());
	CUrlAnalyze::setParam(szMsg, "total_fee", m_stTradeBuy.Ftotal_fee);

	string token = strAcctType +"|";
	token = token + m_params.getString("uin") +"|";
	token = token + m_stTradeBuy.Fcoupon_id + "|";
	token = token + m_stTradeBuy.Flistid + "|";
	token = token + m_stTradeBuy.Fspid + "|";
	token = token + m_stTradeBuy.Ffund_code + "|";
	token = token + m_stTradeBuy.Fchannel_id + "|";
	token = token + m_params.getString("client_ip") + "|";
	token = token + toString(m_stTradeBuy.Ftotal_fee) + "|";
	token = token + gPtrConfig->m_AppCfg.coupon_key;

	char md5token[32+1] = {0};
	getMd5(token.c_str(),token.length(),md5token);
	CUrlAnalyze::setParam(szMsg, "token", md5token);

	sendCouponMsg2Mq(szMsg);

	return;
}

/**
* 申购请求结果变更
*/

void FundBuySpAck::UpdateFundTradeForReq() throw (CException)
{	
	gPtrAppLog->debug("fund buy pay, state [%d] m_optype[%d]", m_stTradeBuy.Fstate,m_optype);

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	int state =0;
	if(INF_PUR_SP_REQ_SUC == m_optype)
	{
		if(PAY_INIT == m_stTradeBuy.Fstate)
		{
			//重入错误
			gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
			throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
		}
		state = PAY_INIT;//申购请求成功，转入待付款
		//strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	}
	else
	{
		if(PUR_REQ_FAIL == m_stTradeBuy.Fstate)
		{
			//重入错误
			gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
			throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
		}
		state = PUR_REQ_FAIL;
	}

	if(CREATE_INIT != m_stTradeBuy.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	stRecord.Fstate = state;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

	if(m_fund_bind.Fuid != 0 && m_stTradeBuy.Fuid != 0)
	{
		//用户更新按uid进行的分库分表
		stRecord.Fuid = m_fund_bind.Fuid;
	}

	//保存trade_id,更新交易记录时需要使用
	SCPY(stRecord.Ftrade_id, m_fund_bind.Ftrade_id);

	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (m_fund_bind.Fuid == 0 || m_stTradeBuy.Fuid == 0) ? false : true);
}

/**
* 支付成功结果变更,先更新交易单为付款成功，在增加基金账户余额，更新余额失败通过补单完成。
* 支付失败结果不接收
*/
void FundBuySpAck::UpdateFundTradeForPay() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	//余额申购退款的重入情况
	if (m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC && m_stTradeBuy.Fpurpose==PURPOSE_BALANCE_BUY)
	{
		need_refund = true;
		return;
	}

	if(PAY_OK == m_stTradeBuy.Fstate || PURCHASE_SUC == m_stTradeBuy.Fstate)
	{
		if(m_stTradeBuy.Fclose_listid > 0&&m_stTradeBuy.Ftrade_date[0]!=0)
		{
			// 重入，对于定期产品要查询出数据用于返回封闭开始时间
			m_fundCloseTrans.Fid = m_stTradeBuy.Fclose_listid;
			strncpy(m_fundCloseTrans.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
			queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);
			// 应该使用交易表的trans_date.  m_params.setParam("trans_date", m_fundCloseTrans.Ftrans_date);
			m_params.setParam("trans_date", m_stTradeBuy.Ftrade_date);
		}

		//重入错误
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//余额申购如果支付时间在15点之前，当前时间大于15点且超过了指定时间，或者2者相差6小时以上需要退款
	if (m_stTradeBuy.Fpurpose==PURPOSE_BALANCE_BUY && ((m_acc_time.substr(11,2)<"15"
		&& m_params.getString("systime").substr(11,2) >= "15" 
		&& (payNotifyOvertime(m_params.getString("pay_time")))) 
		|| (toUnixTime(m_params["systime"])>6*3600+toUnixTime(m_params["pay_time"]))) )
	{
		refund_desc = "余额冻结成功超过一定时间未申购成功退款";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_9;
	} 

	//用于返回结果及传给子账户使用,不能放在检查重入之间，否则会导致重入返回时间和数据库不一致
	if (PURPOSE_BALANCE_BUY == m_stTradeBuy.Fpurpose)
	{
		// 余额申购必须用余额流水表的时间
		strncpy(m_stTradeBuy.Facc_time, m_acc_time.c_str(), sizeof(m_stTradeBuy.Facc_time) - 1);
	}
	else
	{
		strncpy(m_stTradeBuy.Facc_time, m_params.getString("systime").c_str(), sizeof(m_stTradeBuy.Facc_time) - 1);
	}

	if(need_refund)
	{
		//已在前面判断了需要转退款的，什么都不做	
	}
	else if(PUR_REQ_FAIL == m_stTradeBuy.Fstate)
	{
		//订单为预申购失败的，直接转入退款
		refund_desc = "申购失败转退款";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_10;
	}
	else if(PAY_INIT != m_stTradeBuy.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	//全新用户首次申购，交易记录的Ftrade_id 可能因为不满足开户条件而为空，标记退款是便于可能的用户查看
	//m_stTradeBuy.Fuid=0 是全新用户就会为0，根据实际情况发现，当用户并发支付(相差几秒)，可能后支付的先到，将m_fund_bind.Fuid写入真实值，
	//先支付的订单后到，导致m_stTradeBuy.Fuid=0而m_fund_bind.Fuid!=0
	if((m_fund_bind_exist && m_fund_bind.Fuid == 0) || (0 == strcmp("", m_stTradeBuy.Ftrade_id)) || 0 == m_stTradeBuy.Fuid)
	{
		//开户时没有uid 的无法在用户交易表中创建，在支付成功时再创建
		m_stTradeBuy.Fuid = m_params.getInt("uid");
		try
		{
			//插入失败不报错
			InsertTradeUserFund(m_pFundCon, &m_stTradeBuy);
		}
		catch(...)
		{
			gPtrAppLog->error("InsertTradeUserFund error once");
			try
			{
				//插入失败不报错
				InsertTradeUserFund(m_pFundCon, &m_stTradeBuy);
			}
			catch(...)
			{
				gPtrAppLog->error("InsertTradeUserFund error two");
			}

		}
	}

	//检查用户支付的绑定序列号和数据库中记录的是否一致
	checkSamePayCard();

	//检查用户持有的份额,超过份额最大限制转退款
	checkUserTotalShare();

	//查询基金代码配置
	queryFundSpAndFundcodeInfo();

	//计算交易日期
	string trade_date;
	string fund_vdate;
	string end_date;
	if(m_fundSpConfig.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		getTradeDate(m_pFundCon,m_acc_time, trade_date,fund_vdate);
	}
	else
	{
		strncpy(m_fundCloseCycle.Fdate, calculateFundDate(m_acc_time).c_str(), sizeof(m_fundCloseCycle.Fdate) - 1);
		strncpy(m_fundCloseCycle.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundCloseCycle.Ffund_code) - 1);
		queryFundCloseCycle(m_pFundCon, m_fundCloseCycle, false);
		trade_date = m_fundCloseCycle.Ftrans_date;
		fund_vdate = m_fundCloseCycle.Ffirst_profit_date;
	}
	if(	trade_date.empty()){
		//无配置日期
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound:"+string(m_stTradeBuy.Flistid));
		gPtrAppLog->error("trade_date unfound[%s], systime[%s]", m_stTradeBuy.Flistid, m_params.getString("systime").c_str());
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}

	checkUserPermissionBuyCloseFund();
	// 预付卡购买检查
	checkFreezePrepayCard();

	strncpy(stRecord.Facc_time, m_acc_time.c_str(), sizeof(stRecord.Facc_time) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	strncpy(stRecord.Fsub_trans_id, m_stTradeBuy.Flistid, sizeof(stRecord.Fsub_trans_id) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	//strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;

	//当需要退款的时候，支付回调的uid 可能是和发起请求的时候的uid不一致，如串cookie的问题
	stRecord.Fuid = (need_refund && m_stTradeBuy.Fuid !=0 ) ? m_stTradeBuy.Fuid : m_params.getInt("uid");

	strncpy(stRecord.Ftrade_date,trade_date.c_str(), sizeof(stRecord.Ftrade_date) - 1);//交易日
	strncpy(stRecord.Ffund_vdate, fund_vdate.c_str(), sizeof(stRecord.Ffund_vdate) - 1);//基金净值日期,该笔申购首次产生收益的日期
	stRecord.Fbank_type=m_params.getInt("pay_bank_type");
       strncpy(stRecord.Fcard_no, m_params["pay_card_tail"], sizeof(stRecord.Fcard_no) - 1);
      //检查是否已经超卖，如果已经超卖就退款。
      checkCloseDayTotalBuyOverFull(stRecord);
	//定期产品记录
	recordCloseFund(stRecord);

       strncpy(stRecord.Fmemo, need_refund ? refund_desc.c_str() : m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);//退款的记录退款原因

	//前置条件判断需要退款的，登记交易单状态为转入退款
	//延迟到此处更新是为防止交易单已成功，却给用户退了款
	if(need_refund)
	{
		stRecord.Fstate = PURCHASE_APPLY_REFUND;
		stRecord.Frefund_reason = refund_reason;
		stRecord.Frefund_type = REFUND_CARD;
		if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY)
		{
			stRecord.Fstate = PURCHASE_REFUND_SUC;
			stRecord.Frefund_type = REFUND_BALANCE;
		}
	}
	else
	{	
		if(m_fund_bind.Fuid == 0)
		{
			//开户时没有uid 的无法在用户交易表中创建，在支付成功时再创建
			m_stTradeBuy.Fuid = m_params.getInt("uid");

			stRecord.Fuid = m_params.getInt("uid");

			//更新基金账户关联表，写入uid
			ST_FUND_BIND fundBind;
			memset(&fundBind, 0, sizeof(ST_FUND_BIND));
			strncpy(fundBind.Ftrade_id,m_fund_bind.Ftrade_id, sizeof(fundBind.Ftrade_id) - 1);
			fundBind.Fuid = m_params.getInt("uid");
			UpdateFundBind(m_pFundCon, fundBind, m_fund_bind, m_params.getString("systime"));

        		memset(&fundBind, 0, sizeof(ST_FUND_BIND));
        		strncpy(fundBind.Fqqid,m_fund_bind.Fqqid, sizeof(fundBind.Fqqid) - 1);
        		setFundBindToKV(m_pFundCon, fundBind, true);
       

			//开子账户延迟到支付成功，所以每次必开，此处无需处理
			//createSubaccUser(m_request, m_params.getString("spid"), m_params.getString("uin"), m_params.getString("client_ip"));
		}

		stRecord.Fstate = PAY_OK;
	}

	//保存trade_id,更新交易记录时需要使用
	SCPY(stRecord.Ftrade_id, m_fund_bind.Ftrade_id);

	//交易记录没有等到预申购，超过一定时间转退款的，不更新用户交易表，防止多单交叉问题导致没有trade_id也没有记录按用户分表数据
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (0 == strcmp("", m_stTradeBuy.Ftrade_id)) ? false : true);

	if(need_refund)
	{
		return;
	}

	

	//计算赠送收益的部分到收益记录中
	/*
	* 不再支持
	if(m_stTradeBuy.Fpur_type == PURTYPE_REWARD_PROFIT)
	{
	recordRewardToProfit();
	}
	*/
	//对于购买合约机的申购需要修改冻结单状态为支付成功
	if (m_stTradeBuy.Fpurpose == PURPOSE_FREEZE)
	{
	       updateFundFreezeBill(fund_vdate);
	}   

	//交易记录发MQ,组装变化的参数
	m_stTradeBuy.Fuid = m_params.getInt("uid");
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcft_trans_id, stRecord.Fcft_trans_id, sizeof(m_stTradeBuy.Fcft_trans_id) - 1);
	strncpy(m_stTradeBuy.Fsub_trans_id, stRecord.Fsub_trans_id, sizeof(m_stTradeBuy.Fsub_trans_id) - 1);
	strncpy(m_stTradeBuy.Ftrade_date, stRecord.Ftrade_date, sizeof(m_stTradeBuy.Ftrade_date) - 1);
	strncpy(m_stTradeBuy.Ffund_vdate, stRecord.Facc_time, sizeof(m_stTradeBuy.Ffund_vdate) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fclose_listid = stRecord.Fclose_listid;

	//更新用户子账户余额CKV与支付成功在一个事务
	//避免支付成功CKV更新失败,用户看不到份额
	updateUserAcc(m_stTradeBuy);

	sendFundBuy2MqMsg(m_stTradeBuy);

       /*更新定期日限额，定期日限额不严格控制超卖，所以超卖不退款，
       更新日限额会锁定基金配置表所以必须放在事物的最后*/
       updateCloseDayTotalBuy(stRecord,false);
	
}

void FundBuySpAck::checkUserTotalShare() throw (CException)
{
		//已经是需要退款不处理
    if(need_refund)
    {
        return;
    }
	
    int uid = (m_fund_bind.Fuid == 0) ? m_params.getInt("uid") : m_fund_bind.Fuid;

    //余额申购和活动赠送都不检查限额
    if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY || m_stTradeBuy.Fpurpose == PURPOSE_ACTION_BUY)
    {
        return;
    }
      

    LONG currentTotalAsset = queryUserTotalAsset(uid,m_fund_bind.Ftrade_id);
    if (true == isUserAssetOverLimit(m_fund_bind.Fasset_limit_lev,currentTotalAsset, m_params.getLong("total_fee")))
    {
        //提现失败重新申购不限额
        if (true == checkInnerBalancePayForReBuy(gPtrFundDB, m_params.getString("fund_trans_id"), m_params.getString("total_fee"),""))
        {
            return;
        }
        
        TRACE_ERROR("user total Asset Over Limit!");
        refund_desc = "超过总资产限额";
        need_refund = true; //用户持有的份额,超过份额最大限制，转退款
        refund_reason = FUND_REFUND_REASON_11;
    }
}


/**
* 记录赠送收益记录
*/
/*
void FundBuySpAck::recordRewardToProfit()
{
FundProfit fund_profit;
memset(&fund_profit, 0, sizeof(FundProfit));

strncpy(fund_profit.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(fund_profit.Ftrade_id) - 1);
fund_profit.Fcurtype = CUR_FUND_SP;

bool fund_profit_exist = queryFundProfit(m_pFundCon, fund_profit, true);
if(!fund_profit_exist)
{
//赠送收益必须在基金公司收益到账后赠送
gPtrAppLog->error("profit record not exist! trade_id=[%s]", m_stTradeBuy.Ftrade_id);
throw CException(ERR_PROFIT_NOT_EXIST, "profit record not exist! ", __FILE__, __LINE__);
}

FundProfitRecord  stRecord;
memset(&stRecord, 0, sizeof(FundProfitRecord));

strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid)-1);
strncpy(stRecord.Fsub_trans_id, m_stTradeBuy.Flistid, sizeof(stRecord.Fsub_trans_id)-1);
strncpy(stRecord.Ftrade_id,  m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id)-1);
stRecord.Fcurtype= CUR_FUND_SP; // 币种类型
strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid)-1);
stRecord.Ftotal_profit =((fund_profit_exist) ? fund_profit.Ftotal_profit : 0) + m_params.getLong("total_fee");
stRecord.Fprofit = m_params.getLong("total_fee");
strncpy(stRecord.Fday, addDays(toString(GetDateToday()), -1).c_str(), sizeof(stRecord.Fday)-1);
stRecord.Fprofit_type = PROFIT_TYPE_AWARD;
//stRecord.F1day_profit_rate= m_params.getLong("day_profit_rate");
stRecord.F7day_profit_rate= gPtrConfig->m_AppCfg.reward_profit_rate * 1000000; //百分值在乘以10的6次方，数据库保存的是真实收益的10的8次方
strncpy(stRecord.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(stRecord.Flogin_ip)-1);
strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);

insertFundProfitRecord(m_pFundCon, stRecord);

//累计收益处理
fund_profit.Freward_profit= m_params.getLong("total_fee");
fund_profit.Ftotal_profit =((fund_profit_exist) ? fund_profit.Ftotal_profit : 0) + m_params.getLong("total_fee");
strncpy(fund_profit.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fund_profit.Fmodify_time)-1);
//TODO Fsign

updateFundProfitForReward(m_pFundCon, fund_profit);

//ckv处理
setFundProfitRecordToKV(m_pFundCon,stRecord);
setTotalProfit(fund_profit,m_fund_bind.Fuid, 60*60*16);//实时数据更新，可以设置超时时间长点

}
*/

/**
* 申购确认结果变更
* 到基金公司的申购确认因为有预申购请求作为第一步，申购确认不能出现失败，合同上保证。
* 申购确认超时当成功处理，打超时标记，等补单。
*/
void FundBuySpAck::UpdateFundTradeForAck() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	//成功且不为超时状态
	if(PURCHASE_SUC == m_stTradeBuy.Fstate && TRADE_RECORD_TIMEOUT != m_stTradeBuy.Fspe_tag)
	{
		//重入错误
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	if(!(PAY_OK == m_stTradeBuy.Fstate || (PURCHASE_SUC == m_stTradeBuy.Fstate && TRADE_RECORD_TIMEOUT == m_stTradeBuy.Fspe_tag)))
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(INF_PAY_SP_INFO_TIMEOUT == m_optype)
	{		
		if(PURCHASE_SUC == m_stTradeBuy.Fstate)
		{
			//重入错误
			gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
			throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
		}
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记
	}
	else
	{
		if(PURCHASE_SUC == m_stTradeBuy.Fstate && TRADE_RECORD_TIMEOUT != m_stTradeBuy.Fspe_tag)
		{
			//重入错误
			gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
			throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
		}
		stRecord.Fspe_tag = 0;//超时补单成功需要将超时状态修改，否则导致不停补单
	}

	checkCloseEndDate();

	stRecord.Fstate = PURCHASE_SUC;//申购确认成功超时订单状态都变更为成功
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));


}

void FundBuySpAck::checkAndUpdateChangeSp()
{
	ChangeSpRecord change_sp_record;
	memset(&change_sp_record, 0, sizeof(ChangeSpRecord));

	strncpy(change_sp_record.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(change_sp_record.Ftrade_id) - 1);

	if(!queryChangingSpRecord(m_pFundCon, change_sp_record, true))
	{
		//不存在，可能是重入，直接返回
		gPtrAppLog->normal("change sp record not exist. trade_id[%s]", 
			m_params.getString("trade_id").c_str());
		return;
	}

	if(0 != strcmp(m_stTradeBuy.Fspid, change_sp_record.Fnew_spid)
		|| 0 != strcmp(m_stTradeBuy.Flistid, change_sp_record.Fbuy_id))
	{
		//关键信息不一致记录信息返回;
		gPtrAppLog->error("change sp account info diff. buy record spid[%s] listid[%s],changing record spid[%s] listid[%s]", 
			m_stTradeBuy.Fspid,m_stTradeBuy.Flistid, change_sp_record.Fnew_spid, change_sp_record.Fbuy_id);
		return;
	}	

	//更新转换记录为成功
	ChangeSpRecord change_sp;
	memset(&change_sp, 0, sizeof(ChangeSpRecord));

	change_sp.Fimt_id = change_sp_record.Fimt_id;
	change_sp.Fstate = CHANGE_SP_SUC;//转换成功

	updateChangeSpRecord(m_pFundCon, change_sp);
}

void FundBuySpAck::checkCloseDayTotalBuyOverFull(ST_TRADE_FUND& stRecord)
{
	if(need_refund)
	{
		return;
	}

	//非封闭产品直接返回
	if(m_fundSpConfig.Fclose_flag == CLOSE_FLAG_NORMAL&&m_fundSpConfig.Fbuyfee_tday_limit==0)
	{
		return;
	}

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));

	strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, false))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}

       bool overfull=false;

       //有日限额，如果已经超出，那么标记为超限
       if (fundSpConfig.Fbuyfee_tday_limit>0 && (m_stTradeBuy.Ftotal_fee+fundSpConfig.Ftotal_buyfee_tday> fundSpConfig.Fbuyfee_tday_limit)) 
       {
           overfull = true;
       }

        //有总限额，如果已经超出，那么标记为超限
       if (fundSpConfig.Fscope_upper_limit > 0) 
       {
           if(fundSpConfig.Fscope + m_stTradeBuy.Ftotal_fee> fundSpConfig.Fscope_upper_limit)
           {
               overfull = true;
           }
       }

       //判断为已经超限，需要先累加限额或者退款
       if (overfull==true)
       {
        	updateCloseDayTotalBuy(stRecord,true);
       }
}

void FundBuySpAck::updateCloseDayTotalBuy(ST_TRADE_FUND& stRecord,bool RefunAllowed)
{
	if(need_refund)
	{
		return;
	}

	//非封闭产品直接返回
	if(m_fundSpConfig.Fclose_flag == CLOSE_FLAG_NORMAL&&m_fundSpConfig.Fbuyfee_tday_limit==0)
	{
		return;
	}

      //如果已经累加过，就不用再累加直接返回
      if (m_bCloseBuyTotalAdded == true)
      {
          return;
      }

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));

	strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}

	//基金配置Fstat_flag=1时按自然日检查限额
	string strStatDay=( SPCONFIG_STAT_DDAY & fundSpConfig.Fstat_flag )? nowdate(m_params.getString("systime").c_str()):stRecord.Ftrade_date;
	checkAndUpdateFundScope(fundSpConfig, m_stTradeBuy.Ftotal_fee,strStatDay,(RefunAllowed?(&need_refund):NULL),(RefunAllowed?(&refund_desc):NULL)
                                                    ,(m_fundSpConfig.Fclose_flag!= CLOSE_FLAG_NORMAL && m_stTradeBuy.Fpurpose==101));

      m_bCloseBuyTotalAdded = true;

}

void FundBuySpAck::updateChangeSp()
{
	if(ERR_TYPE_MSG)
	{
		//差错补单只补子账户
		return;
	}

	//如果申购单为份额转换申购，则更新份额转换为成功;申购确认超时的先不处理，等确认成功时在修改，避免再次转换无份额可赎回
	if(m_stTradeBuy.Fpurpose != PURPOSE_CHANGE_SP || m_optype != INF_PAY_SP_INFO_SUC)
	{
		return;
	}

	/* 开启事务 */
	//份额转换失败不能影响前面业务，因此开独立事务
	m_pFundCon->Begin();


	checkAndUpdateChangeSp();

	/* 提交事务 */
	m_pFundCon->Commit();

}

void FundBuySpAck::checkSamePayCard()
{
	//已经是需要退款不处理
	if(need_refund)
	{
		return;
	}

	if(checkWhitePayUser(m_params.getString("uin")))
	{
		//如果白名单用户一定处理原卡进出，此处无需逻辑
	}
	else if(gPtrConfig->m_AppCfg.check_same_pay_card == 0)
	{
		return ; //配置不检查
	}

	if(m_stTradeBuy.Fpur_type == PURTYPE_REWARD_PROFIT || m_stTradeBuy.Fpur_type == PURTYPE_REWARD_SHARE || m_stTradeBuy.Fpur_type == PURTYPE_TRANSFER_PURCHASE)
	{
		return ;
	}

	//提现失败重新申购是余额支付bind_serialno为空放过
	//理财通余额申购也是走余额支付bind_serialno 为空
	if (m_params.getString("bind_serialno").empty()
		&& (true == checkInnerBalancePayForReBuy(m_pFundCon,m_params["fund_trans_id"],m_params["total_fee"],"")
		|| m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY))
	{
		return;
	}

	//检查用户支付的绑定序列号和数据库中记录的是否一致
	FundPayCard fund_pay_card;
	memset(&fund_pay_card, 0, sizeof(FundPayCard));
	strncpy(fund_pay_card.Fqqid, m_fund_bind.Fqqid, sizeof(fund_pay_card.Fqqid) - 1);
	strncpy(fund_pay_card.Ftrade_id,  m_fund_bind.Ftrade_id, sizeof(fund_pay_card.Ftrade_id) - 1);
	fund_pay_card.Fuid = m_fund_bind.Fuid;
	strncpy(fund_pay_card.Fcreate_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fcreate_time) - 1);
	strncpy(fund_pay_card.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);
	if(!checkPayCard(m_pFundCon, fund_pay_card,  m_params.getString("bind_serialno"),m_bindCareInfo))
	{
		gPtrAppLog->warning("must use the same card payment.");
		if(m_params.getString("bind_serialno").empty())
		{
			refund_desc = "绑定序列号为空导致退款";
			refund_reason = FUND_REFUND_REASON_12;
		}
		else
		{
			refund_desc = "非安全卡支付";
			refund_reason = FUND_REFUND_REASON_13;
		}

		need_refund = true;
		pay_card_notequal = true;
	}
}

/**
 * 检查预付卡购买情况
 */
void FundBuySpAck::checkFreezePrepayCard() throw (CException)
{
	// 冻结购买情况才检查预付卡和合约机
	if (m_stTradeBuy.Fpurpose != PURPOSE_FREEZE)
	{
		return;
	}

    strncpy(m_freeze_fund.Ffreeze_id,m_stTradeBuy.Flistid,sizeof(m_freeze_fund.Ffreeze_id)-1);
	if(!queryFundFreeze(m_pFundCon,m_freeze_fund,true))
	{
        //异常情况，为了不影响主流程，这里只告警
        alert(ERR_UPDATE_FREEZE_BILL, (string(m_stTradeBuy.Flistid)+" purpose=5 but payed Ok but query Freeze bill fail!").c_str());
        memset(&m_freeze_fund,0,sizeof(ST_FREEZE_FUND));
        return;
	}
	// 非王府井预付卡商户不检查
    if (string(m_freeze_fund.Fspid) != gPtrConfig->m_AppCfg.wx_wfj_spid)
    {
    	return;
    }
	ST_FUND_CONTROL_INFO controlParams;	
    //设置微信王府井参数
    memset(&controlParams,0,sizeof(ST_FUND_CONTROL_INFO));
	strncpy(controlParams.Fspid,m_freeze_fund.Fspid,sizeof(controlParams.Fspid)-1);
	strncpy(controlParams.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controlParams.Ftrade_id)-1);
	strncpy(controlParams.Fcard_no,m_freeze_fund.Fpre_card_no,sizeof(controlParams.Fcard_no)-1);
	// 支付通知发现预付卡卡号冲突，需退款
	if(!checkWxPreCardBuy(m_pFundCon,controlParams,m_controlInfo,true))
	{
		need_refund = true;
	}
	
}

void FundBuySpAck::queryFundSpAndFundcodeInfo()
{
	memset(&m_fundSpConfig, 0, sizeof(FundSpConfig));
	strncpy(m_fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(m_fundSpConfig.Fspid) - 1);
	strncpy(m_fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundSpConfig.Ffund_code) - 1);
	//queryFundSpAndFundcodeFromCkv(m_fundSpConfig, true);
       if (false == queryFundSpAndFundcodeConfig(m_pFundCon,m_fundSpConfig,false))
       {
           throw EXCEPTION(ERR_BAD_PARAM, "query spid and fund_code from db failure.");   
       }
}

void FundBuySpAck::checkUserPermissionBuyCloseFund()
{
	//已经是需要退款不处理
	if(need_refund)
	{
		return;
	}

	//非封闭产品不检查
	if(m_fundSpConfig.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}

	strncpy(m_fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	strncpy(m_fundCloseTrans.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(m_fundCloseTrans.Ffund_code) - 1);
	try
	{
		m_close_fund_seqno = checkPermissionBuyCloseFund(m_fundCloseTrans, m_fundSpConfig, m_fundCloseCycle.Ftrans_date,m_fundCloseCycle.Fdue_date, true);
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

void FundBuySpAck::recordCloseFund(ST_TRADE_FUND& stRecord)
{
	if(need_refund)
	{
		return;
	}

	//非封闭产品直接返回
	if(m_fundSpConfig.Fclose_flag == CLOSE_FLAG_NORMAL)
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

	//记录ckv
	setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_stTradeBuy.Ffund_code);

}

void FundBuySpAck::checkCloseEndDate()
{
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

void FundBuySpAck::updateFundFreezeBill(const string& fund_vdate ) throw(CException)
{
	// 没查到冻结单
	if(m_freeze_fund.Ffreeze_id[0]==0)
	{
        //异常情况，为了不影响主流程，这里只告警
        alert(ERR_UPDATE_FREEZE_BILL, (string(m_stTradeBuy.Flistid)+" purpose=5 but payed Ok but query Freeze bill fail!").c_str());
        return;
	}
    ST_FREEZE_FUND freezeData;
    memset(&freezeData,0,sizeof(ST_FREEZE_FUND));
    queryFundFreeze(m_pFundCon,freezeData,false);
    
    ST_FREEZE_FUND freezeDataSet;
    memset(&freezeDataSet,0,sizeof(ST_FREEZE_FUND));
    freezeDataSet.Fstate = FUND_FREEZE_PAY_BUYED;
    strncpy(freezeDataSet.Ffreeze_id,m_stTradeBuy.Flistid,sizeof(freezeDataSet.Ffreeze_id)-1);
    strncpy(freezeDataSet.Fmodify_time,m_params["systime"],sizeof(freezeDataSet.Fmodify_time)-1);
    if (0 == updateFundFreeze(m_pFundCon,freezeDataSet, freezeData))
    {
        //异常情况,不应该出现
        return;
    }
	// 非王府井预付卡商户不更新
    if (string(m_freeze_fund.Fspid) != gPtrConfig->m_AppCfg.wx_wfj_spid)
    {
    	return;
    }
	
    //微信王府井预付卡商户临时方案
    if(m_controlInfo.Ftrade_id[0]==0)
    {
        ST_FUND_CONTROL_INFO controldata;
        memset(&controldata,0,sizeof(ST_FUND_CONTROL_INFO));
        strncpy(controldata.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controldata.Ftrade_id));
        controldata.Ftype=1;
        strncpy(controldata.Fuin,m_fund_bind.Fqqid,sizeof(controldata.Fuin));
        strncpy(controldata.Ffund_spid,m_stTradeBuy.Fspid,sizeof(controldata.Ffund_spid));
        controldata.Fcur_type=m_stTradeBuy.Fcur_type;
        strncpy(controldata.Fspid,m_freeze_fund.Fspid,sizeof(controldata.Fspid));
        strncpy(controldata.Ffirst_profit_day,fund_vdate.c_str(),sizeof(controldata.Ffirst_profit_day));
        strncpy(controldata.Fcreate_time,m_params["systime"],sizeof(controldata.Fcreate_time));
        strncpy(controldata.Fmodify_time,m_params["systime"],sizeof(controldata.Fmodify_time));
        strncpy(controldata.Fcard_no,m_freeze_fund.Fpre_card_no,sizeof(controldata.Fcard_no));
        strncpy(controldata.Fcard_partner,m_freeze_fund.Fpre_card_partner,sizeof(controldata.Fcard_partner));
        controldata.Ftotal_fee=m_stTradeBuy.Ftotal_fee;
        controldata.Flstate=1;
		insertFundControlInfo(m_pFundCon,controldata);
    }else{
    	addFundControlBalance(m_pFundCon,m_stTradeBuy.Ftotal_fee,m_params.getString("systime"),m_fund_bind.Ftrade_id);
    }
}

/**
* 子账户充值
*/
void FundBuySpAck::doSave() throw(CException)
{
	gPtrAppLog->debug("doSave, listid[%s]  ", m_params.getString("sub_trans_id").c_str());

	try
	{
		if (ERR_SUBACC_NOT_EXIST == SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), m_params.getLong("total_fee"),"基金申购", m_stTradeBuy.Facc_time, 1))
             {
        		//开子账户延迟到支付成功，减少活动时申购请求的压力
        		createSubaccUser(m_request, m_params.getString("spid"), m_params.getString("uin"), m_params.getString("client_ip"));

                    SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), m_params.getLong("total_fee"),"基金申购", m_stTradeBuy.Facc_time, 1);
             }      

	}
	catch(CException& e)
	{
		//子账户不应该失败，所有加钱失败的都发给差错

		//如果支付回调超过一定间隔时间以上，子账户还报异常，发告警
		//使用订单成功时间为条件，否则支付回调堵了，直接告警而没有补单10分钟
		if(payNotifyOvertime(m_stTradeBuy.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.申购支付回调超过10分钟子账户仍未成功");        
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			if(ERR_TYPE_MSG)
			{
				return; //差错补单一定时间未成功的发送告警后不再补单，避免死循环或发生雪崩
			}

		}

		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);

		if(ERR_TYPE_MSG)
		{
			throw;//来自差错补单的直接抛出异常，阻止后面继续执行
		}

	}

}

void FundBuySpAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	char szMsg[MAX_MSG_LEN + 1] = {0};

	// 组装关键参数
	CUrlAnalyze::setParam(szMsg, "Flistid", fundTradeBuy.Flistid, true);
	CUrlAnalyze::setParam(szMsg, "Fspid", fundTradeBuy.Fspid);
	CUrlAnalyze::setParam(szMsg, "Fuin", m_params.getString("uin").c_str());
	CUrlAnalyze::setParam(szMsg, "Fcoding", fundTradeBuy.Fcoding);
	CUrlAnalyze::setParam(szMsg, "Ftrade_id", fundTradeBuy.Ftrade_id);
	CUrlAnalyze::setParam(szMsg, "Fuid", fundTradeBuy.Fuid);
	CUrlAnalyze::setParam(szMsg, "Ffund_code", fundTradeBuy.Ffund_code);
	CUrlAnalyze::setParam(szMsg, "Fpur_type", fundTradeBuy.Fpur_type);
	CUrlAnalyze::setParam(szMsg, "Ftotal_fee", fundTradeBuy.Ftotal_fee);
	CUrlAnalyze::setParam(szMsg, "Fstate", fundTradeBuy.Fstate);
	CUrlAnalyze::setParam(szMsg, "Ftrade_date", fundTradeBuy.Ftrade_date);
	CUrlAnalyze::setParam(szMsg, "Ffund_vdate", fundTradeBuy.Ffund_vdate);
	CUrlAnalyze::setParam(szMsg, "Fcreate_time", fundTradeBuy.Fcreate_time);
	CUrlAnalyze::setParam(szMsg, "Fmodify_time", fundTradeBuy.Fmodify_time);
	CUrlAnalyze::setParam(szMsg, "Fcft_trans_id", fundTradeBuy.Fcft_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_charge_ctrl_id", fundTradeBuy.Fcft_charge_ctrl_id);
	CUrlAnalyze::setParam(szMsg, "Fsp_fetch_id", fundTradeBuy.Fsp_fetch_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_bank_billno", fundTradeBuy.Fcft_bank_billno);
	CUrlAnalyze::setParam(szMsg, "Fsub_trans_id", fundTradeBuy.Fsub_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcur_type", fundTradeBuy.Fcur_type);
	CUrlAnalyze::setParam(szMsg, "Fpurpose", fundTradeBuy.Fpurpose);
	CUrlAnalyze::setParam(szMsg, "Facc_time", fundTradeBuy.Facc_time);
	CUrlAnalyze::setParam(szMsg, "Fchannel_id", fundTradeBuy.Fchannel_id);
	CUrlAnalyze::setParam(szMsg, "Fmemo", fundTradeBuy.Fmemo);

	sendMsg2Mq(szMsg);
}


/**
* 打包输出参数
*/
void FundBuySpAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
	CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time",m_stTradeBuy.Facc_time);
	CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
	CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
	CUrlAnalyze::setParam(rqst->odata, "sp_billno", m_stTradeBuy.Fcoding);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_stTradeBuy.Ffund_code);
	//本地的trans_date为外部定义的close_start_day，前置机和基金公司协议的问题导致
	CUrlAnalyze::setParam(rqst->odata, "close_start_day", m_params.getString("trans_date").c_str());

	rqst->olen = strlen(rqst->odata);
	return;
}


