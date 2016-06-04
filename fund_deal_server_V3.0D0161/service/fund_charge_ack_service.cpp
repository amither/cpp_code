/**
* FileName: fund_charge_ack_service.cpp
* Author: louisjiang
* Version :1.0
* Date: 2014-07-23
* Description: 理财通余额充值确认
*/

#include "fund_commfunc.h"
#include "fund_charge_ack_service.h"

FundChargeAck::FundChargeAck(CMySQL* mysql)
{
	m_pFundCon = mysql;
	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_charge_data,0, sizeof(ST_BALANCE_ORDER));

	m_bChargeSubaccSuc = true;
	m_subAccErrCode = 0;
	m_bNeedRefund = false;
	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fundBaCfg,0,sizeof(m_fundBaCfg));
}

void FundChargeAck::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
	char szMsg[MAX_MSG_LEN] = {0};
	char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 解密原始消息
	getDecodeMsg(rqst, szMsg, szSpId);

	// 要保留请求数据，抛差错使用
	m_request = rqst;

	TRACE_DEBUG("[fund_charge_req_service] receives: %s", szMsg);

	// 读取参数
	m_params.readStrParam(szMsg, "uin", 0, 64);
	m_params.readStrParam(szMsg, "trade_id", 0, 32);
	m_params.readIntParam(szMsg, "op_type", 1,MAX_INTEGER);
	m_params.readLongParam(szMsg, "total_fee", 1,MAX_LONG);
	m_params.readIntParam(szMsg, "cur_type", 1,1);
	m_params.readStrParam(szMsg, "spid", 0, 15);
	m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_transfer_id", 0, 32);
	m_params.readStrParam(szMsg, "redem_id", 0, 32);
	m_params.readStrParam(szMsg, "fetch_id", 0, 32);
	m_params.readStrParam(szMsg, "control_id", 0, 32);
	m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
	m_params.readStrParam(szMsg, "bind_serialno", 0, 64);
	m_params.readIntParam(szMsg, "bank_type", 0, 9999);
	m_params.readIntParam(szMsg, "check_refund_req", 0, 1);// 0 充值确认，1 pc和微信和手q充值转账前的退款判断

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);
	m_fund_trans_acc_time = szTimeNow;

}


/*
* 检验token
*/
void FundChargeAck::CheckToken() throw (CException)
{
	stringstream ss;
	char buff[128] = {0};
	//uin|op_type|total_fee|cft_trans_id|cft_transfer_id|key
	// 规则生成原串
	ss << m_params["uin"] << "|" ;
	ss << m_params["op_type"] << "|" ;
	ss << m_params["total_fee"] << "|" ;
	ss << m_params["cft_trans_id"] << "|" ;
	ss << m_params["cft_transfer_id"] << "|" ;
	ss << gPtrConfig->m_AppCfg.charge_service_cnf_key;

	TRACE_DEBUG("token src=%s", ss.str().c_str());
	getMd5(ss.str().c_str(), ss.str().size(), buff);

	if (StrUpper(m_params.getString("token")) != StrUpper(buff))
	{   
		TRACE_DEBUG("fund authen token check failed, input=%s, real=%s", 
			m_params.getString("token").c_str(), buff);
		throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
	}   
}



/**
* 检查参数，获取内部参数
*/
void FundChargeAck::CheckParams() throw (CException)
{
	// 验证token
	CheckToken();
	if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_PAY ||
		m_params.getInt("op_type") ==OP_TYPE_BA_TRANSFER_REFUND)
	{
		if (checkTransIdAndSpid(m_params["spid"],m_params["cft_trans_id"]) == false)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  cft_trans_id error "); 
		} 
		m_params.setParam("listid", m_params["cft_trans_id"]);
	}
	else if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_REDEM_T1 
		|| m_params.getInt("op_type") ==OP_TYPE_CHAEGE_REDEM_T0)
	{
		if (checkTransIdAndSpid(m_params["spid"],m_params["redem_id"]) == false)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  redem_id error "); 
		}
		m_params.setParam("listid", m_params["redem_id"]);
	}
	else if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_FETCH_FAIL)
	{
		if (m_params.getString("fetch_id").length()<18)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "input fetch_id check fail"); 
		}
		m_params.setParam("listid", m_params["fetch_id"]);
	}
	else if(m_params.getInt("op_type") == OP_TYPE_BA_ACTION_CHARGE)
  {
    if (checkTransIdAndSpid(m_params["spid"],m_params["cft_transfer_id"]) == false)
    {
      throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  cft_transfer_id error "); 
    } 
    m_params.setParam("listid", m_params["cft_transfer_id"]);
  }
	else
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input op_type check fail"); 
	}
	m_fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
	if (false == queryFundBalanceConfig(m_pFundCon,m_fundBaCfg,false))
	{
		throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
	}

	if (1 == m_params.getInt("check_refund_req") && (m_params.getInt("op_type") != OP_TYPE_CHAEGE_PAY))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input check_refund_req check with  op_type error "); 
	}
}

/*
* 查询基金账户是否存在
*/
void FundChargeAck::CheckFundBind() throw (CException)
{
	if(m_params.getString("uin").empty() && m_params.getString("trade_id").empty())
	{
		throw CException(ERR_BAD_PARAM, "uin and trade_id is NULL! ", __FILE__, __LINE__);
	}
	if(!m_params.getString("uin").empty())
	{
		if (!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
		{
			throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
		}
	}
	else if(!m_params.getString("trade_id").empty())
	{
		if (!QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &m_fund_bind, false))
		{
			throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
		}
	}
}

void FundChargeAck::RecordChargeOrder()throw (CException)
{
	strncpy(m_charge_data.Flistid,m_params["listid"],sizeof(m_charge_data.Flistid)-1);
	m_charge_data.Ftype = m_params.getInt("op_type");
	strncpy(m_charge_data.Fuin,m_params["uin"],sizeof(m_charge_data.Fuin)-1);
	strncpy(m_charge_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_charge_data.Ftrade_id)-1);
	m_charge_data.Ftotal_fee= m_params.getInt("total_fee");

	strncpy(m_charge_data.Fchannel_id,m_params["channel_id"],sizeof(m_charge_data.Fchannel_id)-1);
	m_charge_data.Fcur_type = m_params.getInt("cur_type");
	strncpy(m_charge_data.Fspid,m_params["spid"],sizeof(m_charge_data.Fspid)-1);
	strncpy(m_charge_data.Ftotal_acc_trans_id,m_params["cft_transfer_id"],sizeof(m_charge_data.Ftotal_acc_trans_id)-1);
	strncpy(m_charge_data.Fmemo,m_params["desc"],sizeof(m_charge_data.Fmemo)-1);
	strncpy(m_charge_data.Fcontrol_id,m_params["control_id"],sizeof(m_charge_data.Fcontrol_id)-1);
	strncpy(m_charge_data.Fcreate_time,m_params["systime"],sizeof(m_charge_data.Fcreate_time)-1);
	strncpy(m_charge_data.Fmodify_time,m_params["systime"],sizeof(m_charge_data.Fmodify_time)-1);

	//子账户单号要和总单号关联，确保db切换等异常情况下的重入
	if (m_params.getString("listid").length()==28) //商户号+8位日期+10位billno号
	{
		if(m_params.getInt("op_type") ==OP_TYPE_BA_TRANSFER_REFUND)
		{
			m_params.setParam("subacc_charge_id", m_params.getString("cft_transfer_id"));
		}
		else
		{
			m_params.setParam("subacc_charge_id", m_params.getString("listid"));
		}
	}
	else if (m_params.getString("listid").length()==31) //t1赎回单号
	{
		m_params.setParam("subacc_charge_id", m_params.getString("listid").substr(0,28));
	}
	else if (m_params.getString("listid").length()==21) //提现或者转账单号
	{
		m_params.setParam("subacc_charge_id", string(m_fundBaCfg.Fbalance_spid)+m_params.getString("listid"));
	}
	else
	{
		//正常情况不应该走到这个分支
		m_params.setParam("subacc_charge_id", GenerateIdsBySpid(m_fundBaCfg.Fbalance_spid));
	}
	strncpy(m_charge_data.Fsubacc_trans_id,m_params["subacc_charge_id"],sizeof(m_charge_data.Fsubacc_trans_id)-1);

	m_charge_data.Fstate = FUND_CHARGE_INIT;
	m_charge_data.Fuid = m_fund_bind.Fuid;
	insertFundBalanceOrder(m_pFundCon,m_charge_data);

	//记录单号和tradeid对应关系
	RecordRelationOrder();

}

void FundChargeAck::RecordRelationOrder()throw (CException)
{
	ST_ORDER_USER_RELA data;
	memset(&data,0,sizeof(ST_ORDER_USER_RELA));
	strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
	data.Ftype = m_params.getInt("op_type");
	strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
	strncpy(data.Fcreate_time,m_params["systime"],sizeof(data.Fcreate_time)-1);
	strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);

	insertOrderUserRelation(m_pFundCon,data);

	if (!m_params.getString("cft_transfer_id").empty()  && m_params.getString("cft_transfer_id") != m_params["listid"])
	{
		strncpy(data.Flistid,m_params["cft_transfer_id"],sizeof(data.Flistid)-1);
		insertOrderUserRelation(m_pFundCon,data);
	}

	if (m_params.getString("subacc_charge_id") != m_params["cft_transfer_id"]  && m_params.getString("subacc_charge_id") != m_params["listid"])
	{
		strncpy(data.Flistid,m_params["subacc_charge_id"],sizeof(data.Flistid)-1);
		insertOrderUserRelation(m_pFundCon,data);
	}

}

void FundChargeAck::CheckChargeOrder(bool LockQuery)throw (CException)
{
	if (true == LockQuery)
	{
		memset(&m_charge_data,0,sizeof(m_charge_data));
		strncpy(m_charge_data.Flistid,m_params["listid"],sizeof(m_charge_data.Flistid)-1);
		strncpy(m_charge_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_charge_data.Ftrade_id)-1);
		m_charge_data.Ftype = m_params.getInt("op_type");
		if (false == queryFundBalanceOrder(m_pFundCon, m_charge_data,  true))
		{
			throw CException(ERR_BAD_PARAM, "charge data not exists !", __FILE__, __LINE__);
		}
	}

	// 检查关键参数

	if (m_params.getInt("op_type") != m_charge_data.Ftype)
	{
		gPtrAppLog->error("charge data exists, op_type is different! op_type in db[%d], op_type input[%d]", 
			m_charge_data.Ftype, m_params.getInt("op_type"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, op_type is different!", __FILE__, __LINE__);
	}

	if (m_params.getString("uin") != m_charge_data.Fuin)
	{
		gPtrAppLog->error("charge data exists, uin is different! uin in db[%s], uin input[%s]", 
			m_charge_data.Fuin, m_params["uin"]);
		throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, uin is different!", __FILE__, __LINE__);
	}

	if (m_params.getLong("total_fee") != m_charge_data.Ftotal_fee)
	{
		gPtrAppLog->error("charge data exists, total_fee is different! total_fee in db[%zd], total_fee input[%zd]", 
			m_charge_data.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, total_fee is different!", __FILE__, __LINE__);
	}

	if (m_params.getString("listid") != m_charge_data.Flistid)
	{
		gPtrAppLog->error("charge data exists, listid is different! listid in db[%s], listid input[%s]", 
			m_charge_data.Flistid, m_params["listid"]);
		throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, listid is different!", __FILE__, __LINE__);
	}

	if (m_params.getString("cft_transfer_id") != m_charge_data.Ftotal_acc_trans_id)
	{
		gPtrAppLog->error("charge data exists, cft_transfer_id is different! cft_transfer_id in db[%s], cft_transfer_id input[%s]", 
			m_charge_data.Ftotal_acc_trans_id, m_params["cft_transfer_id"]);
		throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, cft_transfer_id is different!", __FILE__, __LINE__);
	}



	if (m_charge_data.Fstate == FUND_CHARGE_SUC || m_charge_data.Fstate == FUND_CHARGE_REFUND || m_charge_data.Fstate == FUND_CHARGE_REFUND_OK)
	{
		if (m_charge_data.Fstate == FUND_CHARGE_REFUND || m_charge_data.Fstate == FUND_CHARGE_REFUND_OK)
		{
			m_bNeedRefund = true;
		}
		throw CException(ERR_REPEAT_ENTRY, "charge data exists!", __FILE__, __LINE__);
	}

	if (m_charge_data.Fstate != FUND_CHARGE_INIT && m_charge_data.Fstate != FUND_CHARGE_PAYED_NOREFUND)
	{
		gPtrAppLog->error("harge record state =%d invalid",m_charge_data.Fstate);
		throw CException(ERR_INVALID_STATE, "charge record state invalid. ", __FILE__, __LINE__);
	}

}

void FundChargeAck::UpdateChargeOrder()throw (CException)
{
	ST_BALANCE_ORDER data;
	memset(&data,0,sizeof(ST_BALANCE_ORDER));
	strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
	data.Ftype = m_params.getInt("op_type");
	strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
	strncpy(data.Facc_time,m_params["systime"],sizeof(data.Facc_time)-1);
	strncpy(data.Fcard_tail,m_params["card_tail"],sizeof(data.Fcard_tail)-1);
	data.Fbank_type = m_params.getInt("bank_type");

	if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_REDEM_T0)
	{
		//赎回到余额，流水单中的acctime要和赎回单中acctime一致，确保二则属于同一个交易日
		strncpy(data.Facc_time,m_fund_trans_acc_time.c_str(),sizeof(data.Facc_time)-1);
	}

	strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);

	data.Fstate = (m_bNeedRefund==true?FUND_CHARGE_REFUND:FUND_CHARGE_SUC);

	if (FUND_CHARGE_PAYED_NOREFUND == m_charge_data.Fstate && m_bNeedRefund==true)
	{
		gPtrAppLog->error("charge record state invalid. can not change from 1(no refund) to 3(refund)");
		throw CException(ERR_INVALID_STATE, "charge record state invalid. can not change from 1 to 3 ", __FILE__, __LINE__);
	}

	updateFundBalanceOrder(m_pFundCon,data);
}

void FundChargeAck::doSubaccCharge()throw (CException)
{
	gPtrAppLog->debug("doSave, listid[%s]  ", m_charge_data.Fsubacc_trans_id);
	string acc_time = m_fund_trans_acc_time;

	try
	{
		//子账户充值
		int iRetCode= SubaccSave(gPtrSubaccRpc, m_fundBaCfg.Fbalance_spid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_charge_data.Fsubacc_trans_id, m_params.getLong("total_fee"),"理财通余额充值", acc_time, 1,CUR_FUND_BALANCE);
		if (iRetCode == ERR_SUBACC_NOT_EXIST)
		{
			//创建子账户
			createSubaccUser(m_request,m_fundBaCfg.Fbalance_spid, m_fund_bind.Fqqid, m_params.getString("client_ip"),CUR_FUND_BALANCE);
			iRetCode= SubaccSave(gPtrSubaccRpc, m_fundBaCfg.Fbalance_spid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
				m_charge_data.Fsubacc_trans_id, m_params.getLong("total_fee"),"理财通余额充值", acc_time, 1,CUR_FUND_BALANCE);
		}

		if (0 != iRetCode)
		{
			throw CException(iRetCode, "SubaccSave error ", __FILE__, __LINE__);
		}

	}
	catch(CException& e)
	{
		m_subAccErrCode = e.error();
		m_subAccErrInfo = e.what();
		//子账户不应该失败，所有加钱失败的都发给差错
		//如果支付回调超过一定间隔时间以上，子账户还报异常，发告警
		if ((time_t)(toUnixTime(acc_time.c_str()))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval < (time(NULL)))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.余额充值超过10分钟子账户加仍未成功");        
			alert(ERR_BUDAN_TOLONG, szErrMsg);
			if(ERR_TYPE_MSG)
			{
				throw; //差错补单一定时间未成功的发送告警后不再补单，避免死循环或发生雪崩,通过对账渠道进行补单
			}
		}

		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 子账户加失败，发差错，通过差错消息重试加子账户操作
		callErrorRpc(m_request, gPtrSysLog);

		throw;
	}

	return ;
}

void FundChargeAck::PreRecordChargeOrder()  throw (CException)
{
	strncpy(m_charge_data.Flistid,m_params["listid"],sizeof(m_charge_data.Flistid)-1);
	strncpy(m_charge_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_charge_data.Ftrade_id)-1);
	m_charge_data.Ftype = m_params.getInt("op_type");
	if (true == queryFundBalanceOrder(m_pFundCon, m_charge_data,  false))
	{
		CheckChargeOrder(false);
		return;
	}

	//微信和pc充值必然先调用请求接口，所以充值单必须存在
	if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_PAY)
	{
		throw CException(ERR_BAD_PARAM, "charge data not exists !", __FILE__, __LINE__);
	}

	//对于赎回和提现失败回余额的场景没有调用充值请求，所以创建充值单单独在一个事物中
	m_pFundCon->Begin();

	//记录充值请求单
	RecordChargeOrder();

	m_pFundCon->Commit();
}

void FundChargeAck::UpdateFundBalanceConfig()throw (CException)
{
	FundBalanceConfig fundBaCfg;
	memset(&fundBaCfg,0,sizeof(fundBaCfg));
	fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
	//加锁查询余额配置表
	if (false == queryFundBalanceConfig(m_pFundCon,fundBaCfg,true))
	{
		throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
	}
	strncpy(fundBaCfg.Fmodify_time,m_params["systime"],sizeof(fundBaCfg.Fmodify_time)-1);

	string tradeDate = getTradeDate(m_pFundCon,m_fund_trans_acc_time);
	if (tradeDate.empty())
	{
		throw CException(ERR_FUND_QUERY_TRADE_DATE, "getTradeDate fail", __FILE__, __LINE__);
	}

	TRACE_DEBUG("check balance config:Ftotal_available_balance=%ld,Ftotal_redem_balance_old=%ld,"
		"Ftotal_buy_balance_old=%ld,Ftotal_redem_balance=%ld,Ftotal_buy_balance=%ld,"
		"Ftotal_baker_fee=%ld,Ftotal_baker_fee_old=%ld,Ftotal_t1fetch_fee=%ld,Ftotal_t1fetch_fee_old=%ld,tradeDate=%s",
		fundBaCfg.Ftotal_available_balance,fundBaCfg.Ftotal_redem_balance_old,fundBaCfg.Ftotal_buy_balance_old,
		fundBaCfg.Ftotal_redem_balance,fundBaCfg.Ftotal_buy_balance,
		fundBaCfg.Ftotal_baker_fee,fundBaCfg.Ftotal_baker_fee_old,fundBaCfg.Ftotal_t1fetch_fee,fundBaCfg.Ftotal_t1fetch_fee_old,tradeDate.c_str());

	if (m_charge_data.Ftype == OP_TYPE_CHAEGE_PAY || m_charge_data.Ftype == OP_TYPE_CHAEGE_REDEM_T1
		|| m_charge_data.Ftype == OP_TYPE_CHAEGE_FETCH_FAIL|| m_charge_data.Ftype == OP_TYPE_BA_ACTION_CHARGE
		||m_charge_data.Ftype == OP_TYPE_BA_TRANSFER_REFUND) // 充值、普通赎回到余额，提现失败回余额都走直接充值
	{ 
		//更新余额总账户余额
		updateFundBalanceConfigForCharge(m_pFundCon,fundBaCfg,m_charge_data.Ftotal_fee,tradeDate);
		//记录余额总账户流水表
		recordTotalBalanceAccRoll(m_pFundCon,m_params["listid"],m_params.getInt("op_type")
			,m_charge_data.Ftotal_fee,m_params["systime"],fundBaCfg.Ftotal_available_balance,1);
	}
	else if (m_charge_data.Ftype == OP_TYPE_CHAEGE_REDEM_T0) // 快速赎回到余额
	{
		//更新赎回累加金额
		updateFundBalanceConfigForRedem(m_pFundCon,fundBaCfg,m_charge_data.Ftotal_fee,tradeDate);
	}

}

void FundChargeAck::checkSafePayCard()  throw (CException)
{
	//只有微信充值和pc充值才验证安全卡
	if (m_charge_data.Ftype != OP_TYPE_CHAEGE_PAY)
	{
		return;
	}

	FundPayCard fund_pay_card;
	memset(&fund_pay_card,0,sizeof(fund_pay_card));
	strncpy(fund_pay_card.Fqqid, m_fund_bind.Fqqid, sizeof(fund_pay_card.Fqqid) - 1);
	strncpy(fund_pay_card.Ftrade_id,  m_fund_bind.Ftrade_id, sizeof(fund_pay_card.Ftrade_id) - 1);
	fund_pay_card.Fuid = m_fund_bind.Fuid;
	strncpy(fund_pay_card.Fcreate_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fcreate_time) - 1);
	strncpy(fund_pay_card.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);

	//PC充值验证银行类型
	if (BA_CHARGE_CHANNEL_PC == m_charge_data.Fstandby1)
	{
		if (false == queryFundPayCard(m_pFundCon,fund_pay_card, false))
		{
			m_bNeedRefund = true;
			m_refundDesc = "安全卡不存在";
			TRACE_ERROR("checkSafePayCard fail,pay card not exist refund!");
		}
		else
		{
			if (m_params.getInt("bank_type") != fund_pay_card.Fbank_type)
			{
				m_bNeedRefund = true;
				m_refundDesc = "pc充值银行类型错误";
				TRACE_ERROR("checkSafePayCard fail,bank_type check fail  refund! input bank_type=%d,safe_bank_type=%d",m_params.getInt("bank_type"), fund_pay_card.Fbank_type);
			}
			m_params.setParam("bank_type", m_params.getInt("bank_type"));
		}
	}
	else  //微信和手q充值验证安全卡
	{
		TStr2StrMap bindCareInfo;
		if (m_params.getString("bind_serialno").empty())
		{
			m_bNeedRefund = true;
			m_refundDesc = "绑定序列号为空";
			TRACE_ERROR("checkSafePayCard fail refund! bind_serialno is empty");
		}
		else 
		{
			if(!queryBindCardInfo(fund_pay_card.Fqqid, m_params.getString("bind_serialno"), bindCareInfo))
    			{
       	 		bindCareInfo.clear();
    			}
    			else
    			{
        			m_params.setParam("bank_type", bindCareInfo["bank_type"]);
        			m_params.setParam("card_tail", bindCareInfo["card_tail"]);
    			}
			if (false == checkPayCard(m_pFundCon, fund_pay_card, m_params["bind_serialno"],bindCareInfo))
			{
				m_bNeedRefund = true;
				m_refundDesc = "校验安全卡失败";
				TRACE_ERROR("checkSafePayCard fail refund! check bind_serialno fail ");
			}
		}
	}
}

void FundChargeAck::queryFundTransAccTime()throw (CException)
{
	if (m_params.getInt("op_type") != OP_TYPE_CHAEGE_REDEM_T0)
	{
		return;
	}

	ST_TRADE_FUND fundTrans;
	memset(&fundTrans,0,sizeof(ST_TRADE_FUND));
	if (false == QueryTradeFund(m_pFundCon,m_charge_data.Flistid,PURTYPE_TRANSFER_REDEEM, &fundTrans, false))
	{
		throw CException(ERR_TRADE_INVALID, "redem trans record not exist! ", __FILE__, __LINE__);
	}

	if (fundTrans.Fstate != REDEM_SUC)
	{
		gPtrAppLog->error("fund redem state is invalid. listid[%s] ", m_charge_data.Flistid);
		throw CException(ERR_TRADE_INVALID, "fund redem state is invalid. ", __FILE__, __LINE__);
	}

	m_fund_trans_acc_time = fundTrans.Facc_time;
}

void FundChargeAck::UpdateChargeOrderNoRefundFlag()  throw (CException)
{
	//只有微信充值和pc充值才有退款情况，才需要更新这个不可退款状态
	if (m_charge_data.Ftype != OP_TYPE_CHAEGE_PAY)
	{
		return;
	}

	if (m_charge_data.Fstate == FUND_CHARGE_PAYED_NOREFUND)
	{
		return;
	}
	//启动事物
	m_pFundCon->Begin();

	//加锁充值单
	CheckChargeOrder(true);

	ST_BALANCE_ORDER data;
	memset(&data,0,sizeof(ST_BALANCE_ORDER));
	strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
	data.Ftype = m_params.getInt("op_type");
	strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);

	strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);

	data.Fstate = FUND_CHARGE_PAYED_NOREFUND;

	updateFundBalanceOrder(m_pFundCon,data);

	m_pFundCon->Commit();
}

void FundChargeAck::checkUserTotalAsset()throw (CException)
{
	//只有微信充值和pc充值才验证总资产
	if (m_charge_data.Ftype != OP_TYPE_CHAEGE_PAY)
	{
		return;
	}

	//如果是重入则不校验限额
	if (m_charge_data.Fstate == FUND_CHARGE_PAYED_NOREFUND)
	{
		return;
	}

	LONG currentTotalAsset = queryUserTotalAsset(m_fund_bind.Fuid,m_fund_bind.Ftrade_id);

	//如果用户当前级别为0，那么如果单笔充值大于指定配置值需要按照等级1验证总资产限额
	int asset_limit_level = m_fund_bind.Fasset_limit_lev;
	if (asset_limit_level == 0 && m_params.getLong("total_fee") >= gPtrConfig->m_AppCfg.assert_limit_level1_chargefee)
	{
		asset_limit_level = 1;
	}

	if (true == isUserAssetOverLimit(asset_limit_level,currentTotalAsset, m_params.getLong("total_fee")))
	{
		TRACE_ERROR("user total Asset Over Limit! charge refund!");
		m_bNeedRefund = true;
		m_refundDesc = "总资产限额超出";
	}

}

void FundChargeAck::UpdateUserAssetLevel() throw (CException)
{
	//pc充值单笔大于指定金额才调整限额
	if (m_charge_data.Ftype != OP_TYPE_CHAEGE_PAY
		|| (BA_CHARGE_CHANNEL_PC != m_charge_data.Fstandby1))
	{
		return;
	}

	//只有针对原资产级别为默认0，且单笔充值大于指定配置值时升级资产限制等级为1
	if (m_fund_bind.Fasset_limit_lev == 0 && m_params.getLong("total_fee")>=gPtrConfig->m_AppCfg.assert_limit_level1_chargefee)
	{
		//更新限额等级
		ST_FUND_BIND fundBind;
		memset(&fundBind, 0, sizeof(ST_FUND_BIND));
		strncpy(fundBind.Ftrade_id,m_fund_bind.Ftrade_id, sizeof(fundBind.Ftrade_id) - 1);
		fundBind.Fasset_limit_lev = 1;
		UpdateFundBind(m_pFundCon, fundBind, m_fund_bind, m_params.getString("systime"));

		//更新到ckv
		m_fund_bind.Fasset_limit_lev=1;
		setFundBindToKV(m_pFundCon,m_fund_bind,false);
	}
}

void FundChargeAck::excute()  throw (CException)
{
	//参数检查
	CheckParams();

	// 检查基金账户记录 
	CheckFundBind();

	try
	{
		//记录理财通余额充值单
		PreRecordChargeOrder();

		//微信充值的用户需要校验安全卡
		checkSafePayCard();

		//检查用户总资产是否超出，如果超出，充值要退款，此处不加锁，对于极端异常的并发会导致充值超过限额可以接受
		checkUserTotalAsset();

		//对于校验是否需要充值退款的情况，判断不需要退款后直接返回
		if  (m_bNeedRefund == false && 1 == m_params.getInt("check_refund_req"))
		{
			return;
		}
		//非校验退款流程不能出现退款，否则itg有转账风险
		if (m_bNeedRefund == true && 1 != m_params.getInt("check_refund_req"))
		{
			TRACE_ERROR("checkSafePayCard fail refund! but input para check_refund_req not equal 1! ");
			throw CException(ERR_BAD_PARAM, "checkSafePayCard fail refund! but input para check_refund_req not equal 1! ", __FILE__, __LINE__);
		}

		//查询赎回单的acc_time
		queryFundTransAccTime();

		if (m_bNeedRefund == false)
		{
			//退款逻辑如果已经判断为不需要退款，那么操作子账户之前需要把订单改为不可退款状态，避免资金风险。
			UpdateChargeOrderNoRefundFlag();
			//子账户虚拟充值,放到mysql事物之前，子账户接口支持重入所以可以放到事物之前
			doSubaccCharge();
		}

		//启动事物
		m_pFundCon->Begin();

		//加锁充值单
		CheckChargeOrder(true);

		//更新充值单状态为成功
		UpdateChargeOrder();

		if (m_bNeedRefund == false)
		{
			//PC充值更新用户总资产限额等级
			UpdateUserAssetLevel();

			//更新余额配置表累加总余额信息
			UpdateFundBalanceConfig();
		}

		m_pFundCon->Commit();
	}
	catch(CException &e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		m_pFundCon->Rollback();

		if (ERR_REPEAT_ENTRY != (unsigned)e.error())
		{
			throw;
		}
	}

}

void FundChargeAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
	CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_fund_trans_acc_time.c_str());
	if (m_bNeedRefund)
	{
		CUrlAnalyze::setParam(rqst->odata, "refund_flag", "1");
	}
	else
	{
		CUrlAnalyze::setParam(rqst->odata, "refund_flag", "0");
	}

	rqst->olen = strlen(rqst->odata);
	return;
}


