/**
  * FileName: fund_refund_service.h
  * Version :1.0
  * Date: 2015-3-3
  * Description: 基金交易退款接口
  */


#include "fund_commfunc.h"
#include "fund_refund_service.h"

extern CftLog* gPtrSysLog;

FundRefund::FundRefund(CMySQL* mysql)
{
	m_pFundCon = mysql;

	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fundSpConfig, 0, sizeof(FundSpConfig));

}

/**
* service step 1: 解析输入参数
*/
void FundRefund::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szMsg[MAX_MSG_LEN] = {0};
	char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
	m_request = rqst;

	// 解密原始消息
	getDecodeMsg(rqst, szMsg, szSpId);
	m_spid = szSpId;

	TRACE_DEBUG("[fund_refund_service] receives: %s", szMsg);


	// 读取参数
	m_params.readStrParam(szMsg, "trade_id", 0, 32);
	m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readIntParam(szMsg,"op_type", 1, 2); // 1发起退款 2退款完成
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "refund_type", 1, 3);//退款类型1 退款到银行卡;2 退款到理财通余额;3 退款到财付通
	m_params.readStrParam(szMsg, "desc", 0, 128);
	m_params.readStrParam(szMsg, "watch_word", 0, 32);
	m_params.readStrParam(szMsg, "client_ip", 0, 16);
	m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
	m_params.readIntParam(szMsg, "refund_reason", 1, 22);
	m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);

	GetTimeNow(szTimeNow);
	m_params.setParam("systime", szTimeNow);


}

/*
* 生成基金注册用token
*/
string FundRefund::GenFundToken()
{
	stringstream ss;
	char buff[128] = {0};

	// 按照uid|fund_trans_id|spid|op_type|sp_billno|total_fee|key
	// 规则生成原串
	ss << m_params["trade_id"] << "|" ;
	ss << m_params["spid"] << "|" ;
	ss << m_params["op_type"] << "|";
	ss << m_params["fund_trans_id"] << "|" ;
	ss << m_params["total_fee"] << "|" ;
	ss << gPtrConfig->m_AppCfg.refund_service_key;

	getMd5(ss.str().c_str(), ss.str().size(), buff);

	return buff;
}

/*
* 检验token
*/
void FundRefund::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

	if (StrUpper(m_params.getString("token")) != StrUpper(token))
	{   
		TRACE_DEBUG("fund authen token check failed, input=%s,real=%s", 
			m_params.getString("token").c_str(),token.c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
	}   
}


/**
* 检查参数，获取内部参数
*/
void FundRefund::CheckParams() throw (CException)
{
	// 验证token
	CheckToken();
}


/**
* 执行申购请求
*/
void FundRefund::excute() throw (CException)
{
	try
	{
		CheckParams();

		/* 开启事务 */
		m_pFundCon->Begin();
		
		/* ckv操作放到事务之后真正提交*/
		gCkvSvrOperator->beginCkvtrans();

		/* 检查基金账户记录*/
		CheckFundBind();

		/* 查询基金交易记录 */
		CheckFundTrade();

		/* 查询基金公司配置*/
		CheckSpConfig();

		/* 退款逻辑*/
		UpdateTradeRefund();

		/* 提交事务 */
		m_pFundCon->Commit();

		updateCkvs();
		
		/* 更新各类ckv ,放在事务之后是避免事务回滚却写入ckv的问题*/
        gCkvSvrOperator->commitCkvtrans();
	}
	catch (CException& e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		//回滚db前先回滚本地ckv
		gCkvSvrOperator->rollBackCkvtrans();

		m_pFundCon->Rollback();

		if ((ERR_REPEAT_ENTRY != (unsigned)e.error()))
		{
			throw;
		}
	}
}
/**
  * 更新CKV
  */
void FundRefund::updateCkvs()
{
	//指数型基金:记录未确认份额
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
		setFundUnconfirm(m_pFundCon, m_params.getString("trade_id"));
	}

	setFundUnfinishTransCKV(m_pFundCon, m_params.getString("trade_id"));
}


/*
 * 查询基金账户是否存在
 */
void FundRefund::CheckFundBind() throw (CException)
{
	if(m_params.getInt("refund_reason") == FUND_REFUND_REASON_20 || 
		m_params.getInt("refund_reason") == FUND_REFUND_REASON_7 ||
		m_params.getInt("refund_reason") == FUND_REFUND_REASON_6)
	{
		return;	
	}
	
	bool bind_exist;
	//对于新用户首次申购鉴权成功，支付失败(额度不足，或其它问题)，用户基本信息表中没有uid
	bind_exist = QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &m_fund_bind, false);
	
	if(!bind_exist)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }

	// 记录存在，读出记录中的uin
    m_params.setParam("uin", m_fund_bind.Fqqid);
    m_params.setParam("uid", m_fund_bind.Fuid);

}

/**
* 检查基金交易记录是否已经生成,检查订单前置状态
*/
void FundRefund::CheckFundTrade() throw (CException)
{
	// 没有购买记录，报错
	if(!QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), PURTYPE_BUY, &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, listid[%s]  ", m_params.getString("fund_trans_id").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
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

	//理财通余额购买的单必须检查退到理财通余额
	if (m_stTradeBuy.Fpurpose == PURPOSE_BALANCE_BUY  )
	{
		if (REFUND_BALANCE != m_params.getInt("refund_type"))
		{
			gPtrAppLog->error("fund purpose=%d. invalid for refundtype [%d]"
                                        , m_stTradeBuy.Fpurpose,m_params.getInt("refund_type"));
               throw CException(ERR_REFUND_TYPE, " refund type is invaid. ", __FILE__, __LINE__);      
		}
		
	}
	
       // 校验关键参数
       if(m_params.getInt("refund_reason") != FUND_REFUND_REASON_20 &&
	   	m_params.getInt("refund_reason") != FUND_REFUND_REASON_7 &&
	   	m_params.getInt("refund_reason") != FUND_REFUND_REASON_6)
       {
		if(m_stTradeBuy.Ftrade_id!= m_params.getString("trade_id"))
		{
			gPtrAppLog->error("fund buy, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
				m_stTradeBuy.Ftrade_id, m_params.getString("trade_id").c_str());
			throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, trade_id is different!", __FILE__, __LINE__);
		}
	
		// 校验关键参数
		if(m_stTradeBuy.Fuid!= m_params.getInt("uid"))
		{
			gPtrAppLog->error("fund buy, uid is different! uid in db[%d], uid input[%d] ", 
				m_stTradeBuy.Fuid, m_params.getInt("uid"));
			throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, uid is different!", __FILE__, __LINE__);
		}
       }
}

void FundRefund::CheckSpConfig()
{
    strncpy(m_fundSpConfig.Fspid,m_params.getString("spid").c_str(), sizeof(m_fundSpConfig.Fspid) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fundSpConfig,false);
}

void FundRefund::UpdateTradeRefund()
{
    switch (m_params.getInt("op_type"))
    {
    	case OP_TYPE_REFUND_REQ:
		UpdateTradeRefundForReq();
		break;

	case OP_TYPE_REFUND_ACK:
		UpdateTradeRefundForAck();
		break;

	default:
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
		break;
		
    }
}


void FundRefund::UpdateTradeRefundForReq()
{
    //校验交易单状态
    CheckTradeStateForReq();
	
    //校验用户未确认份额
    CheckFundUnconfirm();
	
    //更新用户未确认份额
    UpdateFundUnconfirm();
	
    //更新交易单状态
    UpdateTradeStateForReq();
}

void FundRefund::UpdateTradeRefundForAck()
{
   //校验交易单状态
    CheckTradeStateForAck();
   
   //更新交易单状态	
    UpdateTradeStateForAck();
}

void FundRefund::CheckTradeStateForReq()
{
    if (m_stTradeBuy.Fstate == PURCHASE_APPLY_REFUND ||m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC) 
	{
		 gPtrAppLog->error("fund buy state=%d. repeat entry for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
                throw CException(ERR_REPEAT_ENTRY, "buy list is already refund. ", __FILE__, __LINE__);

	}
	//非8状态调退款接口，退款原因要传20以上
	if(m_params.getInt("refund_reason") < FUND_REFUND_REASON_20)
	{
		throw CException(ERR_REFUND_REASON, "refund reason invalid. ", __FILE__, __LINE__);
	}
	else if(m_params.getInt("refund_reason") == FUND_REFUND_REASON_20)
	{
		CHECK_PARAM_EMPTY("cft_trans_id");
    		if ((m_stTradeBuy.Fstate != CREATE_INIT && m_stTradeBuy.Fstate != PAY_INIT) || m_params.getInt("refund_type") != REFUND_CARD) 
		{
			 gPtrAppLog->error("fund buy state=%d,type=%d invalid for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_params.getInt("refund_type"),m_stTradeBuy.Flistid);
               	throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);
		}
	}
	else
	{
		if (m_stTradeBuy.Fstate != PAY_OK && m_stTradeBuy.Fstate !=PAY_ACK_SUC ) 
		{
			 gPtrAppLog->error("fund buy state=%d. invalid for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
               	throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);      

		}
	}
}

void FundRefund::CheckTradeStateForAck()
{
    if (m_stTradeBuy.Fstate == PURCHASE_REFUND_SUC ) 
	{
		 gPtrAppLog->error("fund buy state=%d. repeat entry for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
                throw CException(ERR_REPEAT_ENTRY, "buy list is already refund. ", __FILE__, __LINE__);

	}
    if (m_stTradeBuy.Fstate != PURCHASE_APPLY_REFUND ) 
	{
		 gPtrAppLog->error("fund buy state=%d. invalid for processRefund listid[%s]"
                                        , m_stTradeBuy.Fstate,m_stTradeBuy.Flistid);
               throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);      

	}
}
void FundRefund::UpdateTradeStateForReq()
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fstate = PURCHASE_APPLY_REFUND;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	stRecord.Frefund_type=m_params.getInt("refund_type");
	stRecord.Frefund_reason=m_params.getInt("refund_reason");

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
* 更新交易单状态
*/
void FundRefund::UpdateTradeStateForAck()
{
       ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fstate = PURCHASE_REFUND_SUC;
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
* 查询用户是否有未确认份额
*/
void FundRefund::CheckFundUnconfirm()
{
	if(m_fundSpConfig.Fbuy_confirm_type == 0)
	{
		return;
	}
	
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
	    strncpy(m_fundUnconfirm.Ftrade_id,m_stTradeBuy.Ftrade_id, sizeof(m_fundUnconfirm.Ftrade_id) - 1);
	    strncpy(m_fundUnconfirm.Fspid,m_stTradeBuy.Fspid, sizeof(m_fundUnconfirm.Fspid) - 1);
	    strncpy(m_fundUnconfirm.Ftrade_date,m_stTradeBuy.Ftrade_date, sizeof(m_fundUnconfirm.Ftrade_date) - 1);
	    if(!queryFundUnconfirm(m_pFundCon,m_fundUnconfirm,true))
	    {
	        gPtrAppLog->error("unconfirm fund not exist.");
	        throw CException(ERR_REFUND_REQUEST, "unconfirm fund not exist. ", __FILE__, __LINE__); //add errorcode     
	    }

	    if(m_fundUnconfirm.Flstate== UNCONFIRM_FUND_INVALID)
		{
			gPtrAppLog->error("unconfirm flstate invalid.");
			throw CException(ERR_UNCONFIM_LSTATE, "unconfirm flstate invalid.", __FILE__, __LINE__);
		}
		//退款金额不能大于未确认份额
	    if(m_fundUnconfirm.Ftotal_fee < m_params.getLong("total_fee"))
		{
	        gPtrAppLog->error("not enough unconfirm money.");
	        throw CException(ERR_REFUND_REQUEST, "not enough unconfirm money.", __FILE__, __LINE__); //add errorcode     
			
	    }
	}
	
	// 检查指数基金未完成数据
	strncpy(m_fundIndexTrans.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fundIndexTrans.Ftrade_id)-1);
	strncpy(m_fundIndexTrans.Flistid,m_stTradeBuy.Flistid,sizeof(m_fundIndexTrans.Flistid)-1);
	hasTransProcess = queryFundTransProcess(m_pFundCon,m_fundIndexTrans,true);

	//TODO:fundIndexTrans增加逻辑:第二步增加检查新表数据
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
	{
		char szMsg[128]={0};
		if(hasTransProcess)
		{
			if(m_fundIndexTrans.Flstate==PROCESS_TRANS_LSTATE_INVALID)
			{
				snprintf(szMsg, sizeof(szMsg), "指数型基金退款,指数关联数据非法[%s][%s]",m_stTradeBuy.Flistid,m_fund_bind.Ftrade_id);
				throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
			}
			if(m_fundIndexTrans.Fstate==PROCESS_TRANS_STATE_BUY_CONFIRM||m_fundIndexTrans.Fstate==PROCESS_TRANS_STATE_BUY_USABLE)
			{
				snprintf(szMsg, sizeof(szMsg), "指数型基金退款,指数关联数据状态不允许退款[%s][%s][%d]",m_stTradeBuy.Flistid,m_fund_bind.Ftrade_id,m_fundIndexTrans.Fstate);
				throw CException(ERR_INDEX_REFUND_TRANS, szMsg, __FILE__, __LINE__);
			}
		}
	}
}

/**
* 更新未确认份额
*/

void FundRefund::UpdateFundUnconfirm()
{
	if(m_fundSpConfig.Fbuy_confirm_type == 0)
	{
		return;
	}

	if(!hasTransProcess)
	{
		return;	
	}
	
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
	    m_fundUnconfirm.Ftotal_fee=m_fundUnconfirm.Ftotal_fee-m_params.getLong("total_fee");
	    strncpy(m_fundUnconfirm.Fmodify_time,m_params.getString("systime").c_str(),sizeof(m_fundUnconfirm.Fmodify_time)-1);
	    updateFundUnconfirmById(m_pFundCon,m_fundUnconfirm);
	}

	//  更新指数基金关联表
	FundTransProcess fundIndexTrans;	
	fundIndexTrans.Fid = m_fundIndexTrans.Fid;
	strncpy(fundIndexTrans.Ftrade_id,m_fundIndexTrans.Ftrade_id,sizeof(fundIndexTrans.Ftrade_id)-1);
	fundIndexTrans.Fstate = PROCESS_TRANS_STATE_BUY_CONFIRM_FAIL;
	strncpy(fundIndexTrans.Ffinish_time,m_params.getString("systime").c_str(),sizeof(fundIndexTrans.Ffinish_time)-1);
	strncpy(fundIndexTrans.Fmodify_time,m_params.getString("systime").c_str(),sizeof(fundIndexTrans.Fmodify_time)-1);
	
	//updateFundTransProcess(m_pFundCon,fundIndexTrans);
	updateFundTransProcessWithSign(m_pFundCon,fundIndexTrans,m_fundIndexTrans);

   
}

/**
* 打包输出参数
*/
void FundRefund::packReturnMsg(TRPC_SVCINFO* rqst)
{
	CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
	CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "uin", m_params.getString("uin").c_str());
	rqst->olen = strlen(rqst->odata);
	return;
}


