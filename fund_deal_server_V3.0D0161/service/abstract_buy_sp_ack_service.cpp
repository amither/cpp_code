/**
  * FileName: abstract_buy_sp_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#include "fund_commfunc.h"
#include "abstract_buy_sp_ack_service.h"

AbstractBuySpAck::AbstractBuySpAck(CMySQL* mysql)
{
	m_pFundCon = mysql;

	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
	memset(&m_controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    memset(&m_freeze_fund,0,sizeof(ST_FREEZE_FUND));

	need_refund = false;
	m_fund_bind_exist =false;
	refund_desc = "";
	m_doSaveOnly = false;
	m_bBuyTotalAdded = false;
	refund_reason = FUND_REFUND_REASON_0;

}

/**
  * 传入已有的商户配置,避免重复查询
  */
void AbstractBuySpAck::setSpConfig(FundSpConfig fundSpConfig)
{
	m_fund_sp_config = fundSpConfig;
}

/**
  * service step 1: 解析输入参数
  */
void AbstractBuySpAck::parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg)  throw (CException)
{
	// 要保留请求数据，抛差错使用	
	m_request = rqst;

    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息    
    TRACE_DEBUG("[abstract_buy_sp_ack_service] receives: %s", szMsg);

	// 读取参数
	m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "uin", 0, 64);
	m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readStrParam(szMsg, "sp_billno", 0, 32);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "op_type", 1, 7);
	m_params.readStrParam(szMsg, "bind_serialno", 0, 64);
	m_params.readStrParam(szMsg, "desc", 0, 128);
	m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "pay_time", 0, 20);//格式:2013-12-30 15:21:41
	m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
    m_params.readStrParam(szMsg, "coupon_id", 0, 32);
	m_params.readLongParam(szMsg,"fund_units",0,MAX_LONG); // 申购确认份额
	m_params.readLongParam(szMsg,"charge_fee",0,MAX_LONG); // 手续费
	m_params.readStrParam(szMsg,"charge_type",0,1); // 收费方式

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);

	m_acc_time = szTimeNow;

	m_optype = m_params.getInt("op_type");

	if(m_optype == INF_PAY_OK)
	{
		//支付确认时uid必填
		m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	}

    parseBizInputMsgComm(szMsg);
    parseBizInputMsg(szMsg); // 读取业务参数

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);


}

/**
* 检查参数，获取内部参数
*/
void AbstractBuySpAck::CheckParams() throw (CException)
{	
	if(INF_PAY_OK == m_optype)
	{
	    //查询并校验绑定序列号参数,新用户查询绑卡接口要放到事物之外，避免事物耗时过长
        checkBindserialno();
		
	}else if(INF_PUR_SP_REQ_SUC == m_optype)
	{
		CHECK_PARAM_EMPTY("sp_billno");   
	}
	
	// 检查收费方式是数值类型
	if(!isDigitString(m_params.getString("charge_type").c_str()))
	{
        throw CException(ERR_BAD_PARAM, string("Param is not a int value:charge_type=")+ m_params.getString("charge_type"), __FILE__, __LINE__);
	}
}

void AbstractBuySpAck::checkBindserialno()throw (CException)
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
    }else
    {
        m_params.setParam("pay_bank_type", m_bindCareInfo["bank_type"]);
        m_params.setParam("pay_card_tail", m_bindCareInfo["card_tail"]);
    }
}


/**
* 执行申购请求
*/
void AbstractBuySpAck::excute() throw (CException)
{
	try
	{
		CheckParams();

		/* 开启事务 */
		m_pFundCon->Begin();
		
		/* ckv操作放到事务之后真正提交*/
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
		updateCkvs();

		/* 判断更新子账户 */
		doSave();

        /* 更新用户限额 */
        updateExauAuthLimitNoExcp();

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

void AbstractBuySpAck::updateExauAuthLimitNoExcp()
{
    //pc网银支付成功才加限额
    if ((m_stTradeBuy.Fpay_channel != PAY_TYPE_WEB) || (m_optype != INF_PAY_OK))
    {
        TRACE_DEBUG("pay_channel %d, op_type: %d", m_stTradeBuy.Fpay_channel, m_optype);
        return;
    }

	try
	{
		updateBalanceFetchExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid,m_params.getLong("total_fee"),m_fund_bind.Fcre_id, 
            FUND_BUY_PC_CHANNEL_EXAU_REQ_TYPE);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}

/*
* 查询基金账户是否存在
*/
void AbstractBuySpAck::CheckFundBind() throw (CException)
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
	if(m_params.getString("trade_id").empty())
	{
		m_params.setParam("trade_id",m_fund_bind.Ftrade_id);
	}

}

bool AbstractBuySpAck::payNotifyOvertime(string pay_suc_time)
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
void AbstractBuySpAck::CheckFundBindSpAcc() throw (CException)
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
void AbstractBuySpAck::CheckFundTrade() throw (CException)
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

	//检查状态合法性
	if(m_stTradeBuy.Fstate<0||m_stTradeBuy.Fstate>=TRADE_STATE_SIZE)
	{
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state is invalid", __FILE__, __LINE__);
	}
	// 检查退款单重入,返回特殊退款错误码
	if(m_stTradeBuy.Fstate==PURCHASE_APPLY_REFUND||m_stTradeBuy.Fstate==PURCHASE_REFUND_SUC)
	{
		throw CException(ERR_BUY_RECORD_NEED_REFUND, "fund purchase record has refund! ", __FILE__, __LINE__);
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
void AbstractBuySpAck::UpdateTradeState()
{
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
		UpdateFundTradeForPayAck();
		break;
	case INF_PUR_SP_ACK_SUC:
		UpdateFundTradeForSucAck();
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
void AbstractBuySpAck::UpdateFundTradeForCoupon() throw (CException)
{
	gPtrAppLog->debug("UpdateFundTradeForCoupon called, listid [%s] cft_trans_id [%s]",m_params.getString("fund_trans_id").c_str(),m_params.getString("cft_trans_id").c_str());

	//确认发券状态为未使用，否则报错
	//先不做了，免得增加券服务负担
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
	
	// 只支持支付实时确认份额用券
	if(m_fund_sp_config.Fbuy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		char szErrMsg[128] = {0};
		snprintf(szErrMsg, sizeof(szErrMsg), "[AbstractBuySpAck]商户[%s]申购确认类型[%d]不支持用券,单号[%s]",m_fund_sp_config.Fspid,m_fund_sp_config.Fbuy_confirm_type,m_params.getString("fund_trans_id").c_str());		  
		alert(ERR_BUDAN_TOLONG, szErrMsg);
		return;
	}

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

void AbstractBuySpAck::UpdateFundTradeForReq() throw (CException)
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
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_NONE);
	}
	stRecord.Fstate = state;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	strncpy(stRecord.Fcharge_type, m_params.getString("charge_type").c_str(), sizeof(stRecord.Fcharge_type) - 1);
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	stRecord.Fcharge_fee= m_params.getLong("charge_fee");

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
* 检查支付通知重入情况
* 检查成功,继续后续流程:return true
* 检查失败,不进行后续流程: return false
*/
bool AbstractBuySpAck::CheckPayRepeat() throw (CException)
{
	// 状态在支付成功状态之前,不是重入,正常返回
	if(PURCHASE_STATE_ORDER[PAY_OK]>PURCHASE_STATE_ORDER[m_stTradeBuy.Fstate])
	{
		return true;
	}
	// 申请申购失败退款
	if(m_stTradeBuy.Fstate==PUR_REQ_FAIL)
	{
		need_refund=true;
		refund_desc="申请申购失败退款";
		refund_reason = FUND_REFUND_REASON_15;
		return true;
	}
	// 检查重入

	// 退款单异常
	if(m_stTradeBuy.Fstate==PURCHASE_REFUND_SUC||m_stTradeBuy.Fstate==PURCHASE_APPLY_REFUND)
	{
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase can not update! ", __FILE__, __LINE__);
	}
	
	// 事务成功的差错补单不是重入，只更新子账户,不进行后续操作
	if(ERR_TYPE_MSG&& m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		//设置子账户确认份额
		m_params.setParam("subacc_units",m_stTradeBuy.Freal_redem_amt);
		m_params.setParam("subacc_time",m_stTradeBuy.Facc_time);
		m_doSaveOnly=true;
		return false;
	}

	//重入错误
	gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
	throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
}

/**
 * 检查余额支付情况
* 检查成功,继续后续流程:return true
* 检查失败,不进行后续流程: return false
 */
bool AbstractBuySpAck::CheckBalancePay() throw (CException)
{		
	char errMsg[128]={0};
	if (m_stTradeBuy.Fpurpose != PURPOSE_BALANCE_BUY)
	{
		strncpy(m_stTradeBuy.Facc_time, m_params.getString("systime").c_str(), sizeof(m_stTradeBuy.Facc_time) - 1);
		m_acc_time=m_params.getString("systime");
		return true;
	}

	//理财通余额支付是通过余额总账户做的支付	
	ST_BALANCE_ORDER fetch_data;
	memset(&fetch_data,0,sizeof(ST_BALANCE_ORDER));
	strncpy(fetch_data.Flistid,m_params["fund_trans_id"],sizeof(fetch_data.Flistid)-1);
	strncpy(fetch_data.Ftrade_id,m_stTradeBuy.Ftrade_id,sizeof(fetch_data.Ftrade_id)-1);
	fetch_data.Ftype = OP_TYPE_BA_BUY;
	//  检查余额流水单数据存在
	if (false == queryFundBalanceOrder(m_pFundCon, fetch_data,  false))
	{
		snprintf(errMsg, sizeof(errMsg), "余额申购单不存在[%s]", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BA_ORDER_NOT_EXIST, errMsg, __FILE__, __LINE__);
	}
	// 检查支付通知使用余额流水单中的acctime
	m_params.setParam("pay_time", fetch_data.Facc_time);
	m_acc_time = fetch_data.Facc_time;
	if(m_acc_time.empty()||m_acc_time.size()!=19)
	{
		snprintf(errMsg, sizeof(errMsg), "支付成功:余额申购单时间不正确[%s][%s]", fetch_data.Flistid,fetch_data.Facc_time);
		throw CException(ERR_BA_ORDER_NOT_EXIST, errMsg, __FILE__, __LINE__);
	}	
	//余额申购退款的重入:不进行后续流程
	if (m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC)
	{
		need_refund = true;
		return false;
	}
	// 检查余额流水单状态:5状态变更,6状态重入
	if(fetch_data.Fstate!=FUND_FETCH_SUBACC_OK&&fetch_data.Fstate!=FUND_FETCH_OK)
	{
		snprintf(errMsg, sizeof(errMsg), "支付成功:余额申购单状态不正确[%s][%d]", fetch_data.Flistid,fetch_data.Fstate);
		throw CException(ERR_BA_ORDER_NOT_EXIST, errMsg, __FILE__, __LINE__);
	}

	//余额申购如果支付时间在15点之前，当前时间大于15点且超过了指定时间，或者2者相差6小时以上需要退款
	if ((m_acc_time.substr(11,2)<"15"
		&& m_params.getString("systime").substr(11,2) >= "15" 
		&& (payNotifyOvertime(m_params.getString("pay_time")))) 
		|| (toUnixTime(m_params["systime"])>6*3600+toUnixTime(m_params["pay_time"])) )
	{
		refund_desc = "余额冻结成功超过一定时间未申购成功退款";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_9;
	} 
	strncpy(m_stTradeBuy.Facc_time, m_acc_time.c_str(), sizeof(m_stTradeBuy.Facc_time) - 1);
	
	return true;
}

/**
 * 支付通知记录新用户信息
 */
void AbstractBuySpAck::RecordFundBindPay() throw (CException)
{	
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
}

void AbstractBuySpAck::checkSamePayCard()
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

	if (m_stTradeBuy.Fpay_channel == PAY_TYPE_WEB)  //网银支付只校验银行类型
	{
		if (false == queryFundPayCard(m_pFundCon,fund_pay_card, false))
		{
			need_refund = true;
			refund_desc = "安全卡不存在";
			TRACE_ERROR("checkSamePayCard fail,pay card not exist refund!");
			refund_reason = FUND_REFUND_REASON_12;
		}
		else if (m_params.getInt("bank_type") != fund_pay_card.Fbank_type)
		{
			need_refund = true;
			refund_desc = "网银支付银行类型与安全卡银行类型不一致";
			TRACE_ERROR("checkSamePayCard fail,bank_type check fail  refund! input bank_type=%d,safe_bank_type=%d",
                m_params.getInt("bank_type"), fund_pay_card.Fbank_type);
			refund_reason = FUND_REFUND_REASON_13;
		}
	}
    else  //微信手q支付验安全卡
	{ 
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
    	}
    }
}

void AbstractBuySpAck::checkUserTotalShare() throw (CException)
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
 * 检查预付卡购买情况
 */
void AbstractBuySpAck::checkFreezePrepayCard() throw (CException)
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

/**
 * 检查并计算交易日,记录到m_params中
 */
void AbstractBuySpAck::CheckPayTransDate() throw(CException)
{	
	//计算交易日期
	string trade_date;
	string fund_vdate;
	getTradeDate(m_pFundCon,m_acc_time, trade_date,fund_vdate);
	if(	trade_date.empty()){
		//无配置日期
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound:"+string(m_stTradeBuy.Flistid));
		gPtrAppLog->error("trade_date unfound[%s], systime[%s]", m_stTradeBuy.Flistid, m_params.getString("systime").c_str());
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}
	m_params.setParam("trade_date",trade_date);
	m_params.setParam("fund_vdate",fund_vdate);

}
/**
 *  支付通知检查申购交易单参数
 */
bool AbstractBuySpAck::CheckParamsForPay() throw (CException)
{
	if(need_refund)
	{
		return true;
	}else if(PUR_REQ_FAIL == m_stTradeBuy.Fstate)
	{
		//订单为预申购失败的，直接转入退款
		refund_desc = "申购失败转退款";
		need_refund = true;
		refund_reason = FUND_REFUND_REASON_10;
		return true;
	}else if(PAY_INIT != m_stTradeBuy.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	
	// 延迟确认商户不支持购买合约机
	if (m_stTradeBuy.Fpurpose == PURPOSE_FREEZE && m_fund_sp_config.Fbuy_confirm_type != SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		char szErrMsg[128]={0};
		snprintf(szErrMsg,sizeof(szErrMsg),"freeze purpose buy[%s] not allow use delay confirm sp[%s]",m_stTradeBuy.Flistid,m_fund_sp_config.Fspid);
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, szErrMsg, __FILE__, __LINE__);
	}
	
	// 实时确认类型的份额初始化为金额
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM&&m_params.getLong("fund_units")==0)
	{
		m_params.setParam("fund_units",m_stTradeBuy.Ftotal_fee);
	}
	return true;
}

void AbstractBuySpAck::checkSpconfigBuyOverFull(const string& tradeDate)
{
	if(need_refund)
	{
		return;
	}

	//未设置上限直接返回
	if(m_fund_sp_config.Fbuyfee_tday_limit <= 0&&m_fund_sp_config.Fscope_upper_limit <= 0)
	{
		return;
	}
	if(tradeDate=="")
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "checkSpconfigBuyOverFull tradeDate is error"); 
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
	// 基金配置统计日期
	string strStatDay=( SPCONFIG_STAT_DDAY & fundSpConfig.Fstat_flag )? nowdate(m_params.getString("systime").c_str()):tradeDate;
	

	//有日限额，如果已经超出，那么标记为超限
	if (fundSpConfig.Fbuyfee_tday_limit>0) 
	{
		double total_redeem_tday=0;
		if((fundSpConfig.Fstat_flag&SPCONFIG_STAT_NET)&&fundSpConfig.Fstat_redeem_tdate>=strStatDay) 
		{   //计算净申购
			total_redeem_tday=fundSpConfig.Ftotal_redeem_tday*calRedeemRate();
		}
		if(m_stTradeBuy.Ftotal_fee+fundSpConfig.Ftotal_buyfee_tday-total_redeem_tday> fundSpConfig.Fbuyfee_tday_limit)
		{
	    	overfull = true;
		}
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
	 	updateSpconfigTotalBuy(strStatDay,true);
	}
}

void AbstractBuySpAck::updateSpconfigTotalBuy(const string& strStatDay,bool RefunAllowed)
{
	if(need_refund)
	{
		return;
	}

	// 未设置上限直接返回
	// 0 表示不需要记录
	// -1表示需要记录但不需要限制
	if(m_fund_sp_config.Fbuyfee_tday_limit == 0&&m_fund_sp_config.Fscope_upper_limit == 0)
	{
		return;
	}

    //如果已经累加过，就不用再累加直接返回
    if (m_bBuyTotalAdded == true)
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
	double redeemRate = calRedeemRate();
	checkAndUpdateFundScope(fundSpConfig, m_stTradeBuy.Ftotal_fee,strStatDay,(RefunAllowed?(&need_refund):NULL),(RefunAllowed?(&refund_desc):NULL)
                                                    ,(fundSpConfig.Fclose_flag!= CLOSE_FLAG_NORMAL && fundSpConfig.Fpurpose==101),redeemRate);

    m_bBuyTotalAdded = true;

}
/**
  * 计算申购额度限制使用赎回金额比率
  */
double AbstractBuySpAck::calRedeemRate()
{
	return 1;
}

/**
 * 支付通知检查数据
 */
bool AbstractBuySpAck::CheckFundTradeForPay() throw (CException)
{
	if(!CheckPayRepeat())
	{
		// 重入不进行后续更新
		return false;
	}
	if(!CheckBalancePay())
	{
		// 余额退款不进行后续更新
		return false;
	}
	// 检查参数一致性
	CheckParamsForPay();
	
	// 检查交易日
	CheckPayTransDate();
	
	// 记录新用户绑定信息
	RecordFundBindPay();

	//检查用户支付的绑定序列号和数据库中记录的是否一致
	checkSamePayCard();

	//检查用户持有的份额,超过份额最大限制转退款
	checkUserTotalShare();

	// 检查预付卡购买情况
	checkFreezePrepayCard();

	// 检查限额,需放在交易日检查之后
	checkSpconfigBuyOverFull(m_params.getString("trade_date"));
	return true;
	
}
// 组装支付结果交易单
void AbstractBuySpAck::BuildFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	strncpy(stRecord.Facc_time, m_acc_time.c_str(), sizeof(stRecord.Facc_time) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	strncpy(stRecord.Fsub_trans_id, m_stTradeBuy.Flistid, sizeof(stRecord.Fsub_trans_id) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;

	//当需要退款的时候，支付回调的uid 可能是和发起请求的时候的uid不一致，如串cookie的问题
	stRecord.Fuid = (need_refund && m_stTradeBuy.Fuid !=0 ) ? m_stTradeBuy.Fuid : m_params.getInt("uid");

	strncpy(stRecord.Ftrade_date,m_params.getString("trade_date").c_str(), sizeof(stRecord.Ftrade_date) - 1);//交易日
	strncpy(stRecord.Ffund_vdate,m_params.getString("fund_vdate").c_str(), sizeof(stRecord.Ffund_vdate) - 1);//基金净值日期,该笔申购首次产生收益的日期
	strncpy(stRecord.Fmemo, need_refund ? refund_desc.c_str() : m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);//退款的记录退款原因
    if (m_stTradeBuy.Fpay_channel == PAY_TYPE_WEB) //网银支付取biz_attach中的bank_type
        stRecord.Fbank_type=m_params.getInt("bank_type");
    else
        stRecord.Fbank_type=m_params.getInt("pay_bank_type");
	strncpy(stRecord.Fcard_no, m_params["pay_card_tail"], sizeof(stRecord.Fcard_no) - 1);
	//保存trade_id,更新交易记录时需要使用
	strncpy(stRecord.Ftrade_id, m_fund_bind.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
	// 支付通知不更新手续费:stRecord.Fcharge_fee= m_params.getLong("charge_fee");

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
		// 更新退款返回参数为特殊错误码
		int result = ERR_BUY_RECORD_NEED_REFUND;
		m_params.setParam("result",result);
		return;
	}
	
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

		// 更新用户fundBind 到CKV
		memset(&fundBind, 0, sizeof(ST_FUND_BIND));
		strncpy(fundBind.Fqqid,m_fund_bind.Fqqid, sizeof(fundBind.Fqqid) - 1);
		setFundBindToKV(m_pFundCon, fundBind, true);

	}

	stRecord.Fstate = PAY_OK;
	// 支付确认份额,直接更新份额
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		stRecord.Freal_redem_amt= m_params.getLong("fund_units");
		m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
		m_params.setParam("subacc_time",m_acc_time);
	}
	
	
}

// 更新合约机冻结状态
void AbstractBuySpAck::updateFundFreezeBill(const string& fund_vdate ) throw(CException)
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
    if (0 == updateFundFreeze(m_pFundCon,freezeDataSet,freezeData))
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

// 更新支付结果数据
void AbstractBuySpAck::RecordFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	//交易记录没有等到预申购，超过一定时间转退款的，不更新用户交易表，防止多单交叉问题导致没有trade_id也没有记录按用户分表数据
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (0 == strcmp("", m_stTradeBuy.Ftrade_id)) ? false : true);

	// 退款单不更新账户
	if(need_refund)
	{
		return;
	}

	//对于购买合约机的申购需要修改冻结单状态为支付成功
	if (m_stTradeBuy.Fpurpose == PURPOSE_FREEZE)
	{
	    updateFundFreezeBill(m_params.getString("fund_vdate"));
	}
}
// 同步支付结果数据到全局变量
void AbstractBuySpAck::SyncFundTradeForPay(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 组装变化的参数
	m_stTradeBuy.Fuid = m_params.getInt("uid");
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcft_trans_id, stRecord.Fcft_trans_id, sizeof(m_stTradeBuy.Fcft_trans_id) - 1);
	strncpy(m_stTradeBuy.Fsub_trans_id, stRecord.Fsub_trans_id, sizeof(m_stTradeBuy.Fsub_trans_id) - 1);
	strncpy(m_stTradeBuy.Ftrade_date, stRecord.Ftrade_date, sizeof(m_stTradeBuy.Ftrade_date) - 1);
	strncpy(m_stTradeBuy.Facc_time, stRecord.Facc_time, sizeof(m_stTradeBuy.Facc_time) - 1);
	strncpy(m_stTradeBuy.Ffund_vdate, stRecord.Ffund_vdate, sizeof(m_stTradeBuy.Ffund_vdate) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fclose_listid = stRecord.Fclose_listid;
}

/**
* 支付成功结果变更,先更新交易单为付款成功，在增加基金账户余额，更新余额失败通过补单完成。
* 支付失败结果不接收
*/
void AbstractBuySpAck::UpdateFundTradeForPay() throw (CException)
{
	if(!CheckFundTradeForPay())
	{	// 检查不通过，不更新数据
		return;
	}

	// 构建交易表和相关业务表的相关数据
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	BuildFundTradeForPay(stRecord);

	// 更新数据
	RecordFundTradeForPay(stRecord);

	// 数据更新成功同步变更到全局变量
	SyncFundTradeForPay(stRecord);

	// 发送MQ消息
	sendFundBuy2MqMsg(m_stTradeBuy);

   /*更新日限额，日限额不严格控制超卖，所以超卖不退款，
       更新日限额会锁定基金配置表所以必须放在事物的最后*/
   updateSpconfigTotalBuy(stRecord.Ftrade_date,false);
	
}


// 支付通知商户结果变更
// 检查参数
void AbstractBuySpAck::CheckFundTradeForPayAck() throw (CException)
{
	// 判断是否状态已经通知确认成功
	bool isStatePayAck = false;
	if(PAY_ACK_SUC==m_stTradeBuy.Fstate||PURCHASE_SUC==m_stTradeBuy.Fstate)
	{
		isStatePayAck=true;
	}

	//重入: 成功且不为超时状态
	if(isStatePayAck&& TRADE_RECORD_TIMEOUT != m_stTradeBuy.Fspe_tag)
	{
		//重入错误
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//重入: 成功且更新超时
	if(isStatePayAck&&INF_PAY_SP_INFO_TIMEOUT == m_optype)
	{
		//重入错误
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//  可以变更状态:1, 支付确认;2,成功并且超时
	if(!(PAY_OK == m_stTradeBuy.Fstate || 
		(isStatePayAck && TRADE_RECORD_TIMEOUT == m_stTradeBuy.Fspe_tag)))
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

}

void AbstractBuySpAck::BuildFundTradeForPayAck(ST_TRADE_FUND& stRecord) throw (CException)
{
	if(INF_PAY_SP_INFO_TIMEOUT == m_optype)
	{	
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记
	}
	else
	{
		stRecord.Fspe_tag = 0;//超时补单成功需要将超时状态修改，否则导致不停补单
	}
	// 更新支付通知结果
	if(m_fund_sp_config.Fbuy_confirm_type == SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		// 实时确认份额，更新为最终状态
		stRecord.Fstate = PURCHASE_SUC;
	}else{
		// 延迟确认份额，更新为支付通知状态
		stRecord.Fstate = PAY_ACK_SUC;
	}
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

	// 支付通知确认不更新手续费:stRecord.Fcharge_fee= m_params.getLong("charge_fee");
}

/**
* 支付通知商户结果变更
* 到基金公司的申购确认因为有预申购请求作为第一步，申购确认不能出现失败，合同上保证。
* 申购确认超时当成功处理，打超时标记，等补单。
*/
void AbstractBuySpAck::UpdateFundTradeForPayAck() throw (CException)
{
	// 检查支付通知参数
	CheckFundTradeForPayAck();

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	BuildFundTradeForPayAck(stRecord);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
	
	//申购确认成功后，给券服务发异步请求，尽量把券充值到账
	UpdateFundTradeForCoupon();
}

//延时商户结果变更
// 检查参数
bool AbstractBuySpAck::CheckFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	// 判断是否状态已经申购确认成功
	if(PURCHASE_SUC==m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==0)
	{
		// 事务成功的差错补单不是重入，只更新子账户,不进行后续操作
		if(ERR_TYPE_MSG)
		{
			// 设置子账户确认份额,重入使用modify_time重试
			// (业务类记录子账户时间在子类实现)
			m_params.setParam("subacc_units",m_stTradeBuy.Freal_redem_amt);
			m_params.setParam("subacc_time",m_stTradeBuy.Fmodify_time);
			m_doSaveOnly=true;
			return false;
		}
		//重入错误
		gPtrAppLog->error("repeat enter listid[%s], state[%d]", m_stTradeBuy.Flistid, m_stTradeBuy.Fstate);
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record has exist! ", __FILE__, __LINE__);
	}

	//  可以变更状态: 
	//  1, 支付通知成功
	//  2, 申购确认成功spe_tag不为0
	if(PAY_ACK_SUC != m_stTradeBuy.Fstate && m_stTradeBuy.Fspe_tag==0)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	return true;
}

void AbstractBuySpAck::BuildFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{

	stRecord.Fstate = PURCHASE_SUC;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);		
	if(strcmp(m_stTradeBuy.Fcoding,"")==0)
	{
    	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	}
	stRecord.Fcharge_fee= m_params.getLong("charge_fee");
	stRecord.Freal_redem_amt= m_params.getLong("fund_units");
	//设置子账户确认份额,默认使用当前时间增加子账户,记录到modify_time
	m_params.setParam("subacc_units",stRecord.Freal_redem_amt);
	m_params.setParam("subacc_time",m_params.getString("systime"));
}

void AbstractBuySpAck::RecordFundTradeForSucAck(ST_TRADE_FUND&  stRecord) throw (CException)
{
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}


/**
* 商户确认成功结果变更
* 状态从12到3
* 增加子账户
*/
void AbstractBuySpAck::UpdateFundTradeForSucAck() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	// 检查申购确认参数
	if(!CheckFundTradeForSucAck(stRecord))
	{
		return;
	}

	// 组装申购确认结果
	BuildFundTradeForSucAck(stRecord);

	// 更新申购确认结果
	RecordFundTradeForSucAck(stRecord);
}


/**
* 子账户充值
*/
void AbstractBuySpAck::doSave() throw(CException)
{
	// 检查子账户份额
	LONG subaccUnits = m_params.getLong("subacc_units");
	gPtrAppLog->debug("doSave, listid[%s],subacc_units[%ld]  ", m_params.getString("sub_trans_id").c_str(),subaccUnits);
	if(subaccUnits<=0)
	{
		return;
	}
	bool needCallErr=true;
	string subaccTime = m_params.getString("subacc_time");
	if(subaccTime.empty())
	{
		char szErrMsg[256] = {0};
		snprintf(szErrMsg, sizeof(szErrMsg), "[%s][%s]申购加子账户时间未赋值,使用当前时间,失败不抛差错,避免死循环",m_stTradeBuy.Flistid,subaccTime.c_str());
		alert(ERR_SUBACC_TIME_EMPTY, szErrMsg);
		subaccTime=m_params.getString("systime");
		needCallErr = false;
	}

	try
	{
		if (ERR_SUBACC_NOT_EXIST == SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), subaccUnits,"基金申购", subaccTime.c_str(), 1))
             {
        			//开子账户延迟到支付成功，减少活动时申购请求的压力
        			createSubaccUser(m_request, m_params.getString("spid"), m_params.getString("uin"), m_params.getString("client_ip"));

                    SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("sub_trans_id"), subaccUnits,"基金申购", subaccTime.c_str(), 1);
             }      

	}
	catch(CException& e)
	{
		if(!needCallErr) // 不拋差错直接throw 异常
		{
			throw;
		}
		//子账户不应该失败，所有加钱失败的都发给差错

		//如果支付回调超过一定间隔时间以上，子账户还报异常，发告警
		//使用订单成功时间为条件，否则支付回调堵了，直接告警而没有补单10分钟
		if(payNotifyOvertime(subaccTime.c_str()))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal[%s][%s]申购支付回调超过10分钟子账户仍未成功",m_stTradeBuy.Flistid,m_params.getString("sub_trans_id").c_str());
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

void AbstractBuySpAck::updateCkvs()
{
	if(m_doSaveOnly)
	{
		//差错补单只补子账户,不需要补ckv		
		return;
	}
	
	// 份额确认,增加子账户更新总账户和资产
	if(m_params.getLong("subacc_units")>0)
	{
		//更新用户轮播
		updateUserAcc(m_stTradeBuy);

	// 份额未确认,支付成功更新轮播
	}else if(INF_PAY_OK == m_optype &&  false == need_refund)	
	{
		//更新用户轮播待确认资产
		updateUserAcc(m_stTradeBuy);	
	}

}

void AbstractBuySpAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
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
void AbstractBuySpAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
		CUrlAnalyze::setParam(rqst->odata, "result", m_params.getInt("result"), true);
		CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
		CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
		CUrlAnalyze::setParam(rqst->odata, "acc_time",m_stTradeBuy.Facc_time);
		CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
		CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
		CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
		CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
		CUrlAnalyze::setParam(rqst->odata, "sp_billno", m_stTradeBuy.Fcoding);
		CUrlAnalyze::setParam(rqst->odata, "fund_code", m_stTradeBuy.Ffund_code);
		CUrlAnalyze::setParam(rqst->odata, "state", m_stTradeBuy.Fstate);
		CUrlAnalyze::setParam(rqst->odata, "agree_risk", getAgreeRiskType(m_fund_sp_config,m_fund_bind));

		packBizReturnMsg(rqst);
		
    rqst->olen = strlen(rqst->odata);
    return;
}

void AbstractBuySpAck::parseBizInputMsgComm(char* szMsg) throw (CException)
{
    m_params.readStrParam(szMsg,"biz_attach",0,MAX_PARAM_LEN);
	const char* bizAttach = m_params.getString("biz_attach").c_str();
	if(strcmp(bizAttach,"")==0)
	{
		return;
	}
    
	m_params.readIntParam(bizAttach,"bank_type",0,MAX_INTEGER);

}