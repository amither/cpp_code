/**
  * FileName: fund_redem_sp_req_service.cpp
  * Author: rajeszhou
  * Version :1.0
  * Date: 2014-8-19
  * Description: 基金交易服务 基金客服强制赎回请求 源文件
  */

#include "fund_commfunc.h"
#include "fund_kf_redem_sp_req_service.h"

FundKfRedemSpReq::FundKfRedemSpReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_close, 0, sizeof(FundCloseTrans));

	m_bBuyTradeExist = false;

}

/**
  * service step 1: 解析输入参数
  */
void FundKfRedemSpReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	m_request = rqst;
    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_kf_sp_req_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_charge_ctrl_id", 0, 32);
	m_params.readStrParam(szMsg, "sp_fetch_id", 0, 32);
	//m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
     m_params.readIntParam(szMsg, "fetch_type", 0,DRAW_ARRIVE_TYPE_T1);
	//m_params.readStrParam(szMsg, "transfer_id", 1, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
    //m_params.readStrParam(szMsg, "fund_name", 0, 64);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "purpose", 0,4);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "end_date", 1, 16);
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	if (!IsTimeLimt(SetTodayTime(gPtrConfig->m_AppCfg.kf_start_time), SetTodayTime(gPtrConfig->m_AppCfg.kf_end_time)))
	{
		throw EXCEPTION(ERR_KF_ONLY_TINE, "客服強赎只允许在固定时间内发起"); 
	}

	if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T0)
	{
	   throw CException(ERR_KF_ONLY_T0_REDEM, "客服強赎只能T+0.", __FILE__, __LINE__);
	}


}

/*
 * 生成基金注册用token
 */
string FundKfRedemSpReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|cft_bank_billno|spid|total_fee|key
    // 规则生成原串
    ss << m_params["uid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundKfRedemSpReq::CheckToken() throw (CException)
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
void FundKfRedemSpReq::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

	//检查spid 及fund_code 是否有效
	strncpy(m_fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_sp_config.Fspid) - 1);
	strncpy(m_fund_sp_config.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_sp_config.Ffund_code) - 1);
	checkFundSpAndFundcode(m_pFundCon,m_fund_sp_config, false);//不强制必须是有效基金公司，用户已开通了该基金公司，可使用带限制的基金公司

	//检查是否支持赎回到财付通余额
	if(m_params.getInt("purpose") == PURPOSE_DEFAULT && !(gPtrConfig->m_AppCfg.support_redem_to_cft == 1))
	{
		throw CException(ERR_CANNOT_REDEM_TO_CFT, "not support redeem to cft balance.", __FILE__, __LINE__);
	}

    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1)
    {
        m_params.setParam("fetch_type", DRAW_ARRIVE_TYPE_T0);
    }
    /*if (m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T0 && 0 == m_params.getInt("bank_type"))
    {
        throw CException(ERR_BAD_PARAM, "缺少参数 bank_type.", __FILE__, __LINE__);
    }*/

	/*if(m_fund_sp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
	{	
		//定期产品暂时限制赎回
		throw EXCEPTION(ERR_BAD_PARAM, "Do not support the redemption of the Fund's");    
	}*/
}


/**
  * 执行申购请求
  */
void FundKfRedemSpReq::excute() throw (CException)
{
	/* 事务1: 处理请求，生成赎回单 */
	if ( CreatTradeInfo() )
	{
		/* 事务2:  真正赎回操作*/
		ProcessTradeInfo();
	}
}

bool FundKfRedemSpReq::CreatTradeInfo()
{
	try
    {
        CheckParams();
		
        /* 检查基金绑定记录 */
        CheckFundBind();

		/* 检查基金账户绑定基金公司交易账户记录 */
		CheckFundBindSpAcc();

         /* 开启事务 */
        m_pFundCon->Begin();

		/** 查询赎回交易记录 
		* 为了支持赎回补单重入，必须先查询单是否存在
		*/
        CheckFundTrade();

		/* 赎回但不存在，创建赎回请求单 */
		if (!m_bBuyTradeExist)
		{
			//由cgi 在入口时检查，本处检查也无法完全避免余额不足
			//无份额基金公司会赎回失败，即时基金公司发生错误，在赎回确认时减子账户余额也会失败
			CheckFundBalance();

			/* 检查赎回垫资账户额度，不更新垫资额度(因为还没开始真正赎回) */
			checkSpLoaning();

			/*构造赎回单内容  */
			GenerFundTrade();		

			/* 记录赎回交易记录 */
			RecordFundTrade();

			UpdateFundCloseListid();
		}

        /* 提交事务 */
        m_pFundCon->Commit();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }

		/* 重入的时候不用执行事务2 */
		return false;
    }

	return true;
}


void FundKfRedemSpReq::ProcessTradeInfo()
{
	try
    {        
        /* 检查基金绑定记录 */
        CheckFundBind();

         /* 开启事务 */
        m_pFundCon->Begin();

		/* 检查重入 */
        CheckFundTrade();

		if(!m_bBuyTradeExist)
	{
		throw CException(ERR_SYSTEM_UNKNOW_ERROR, "fund redem record not exist,error! ", __FILE__, __LINE__);
	}

		QueryFundCloseId();

		/* 检查赎回额度，并更新赎回额度 */
		checkSpLoaningPlus();

		/* 减子账户，并更新状态 */
		RecordRedemTradeForSuc();

        /* 提交事务 */
        m_pFundCon->Commit();

		/* 更新ckv */
		updateCkvs();

		/* 更新赎回金额上限 */
		updateExauAuthLimitNoExcp();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }
}

void FundKfRedemSpReq::updateCkvs()
{
	//更新定期交易记录到ckv
	setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_params.getString("fund_code"));
	//更新交易记录到ckv
	// 此次ckv更新被放在在UpdateFundTrade中
	//setTradeRecordsToKV(m_pFundCon, m_stRecord);

}

void FundKfRedemSpReq::updateExauAuthLimitNoExcp()
{
	//累加exau 放在事务外，避免超时导致的事务回滚
	//赎回请求的时候检查exau限制，累加时失败不报错，避免多次累加，或赎回单无法处理成功的问题
	try
	{
		//累计用户赎回限额
		int redem_type = DRAW_ARRIVE_TYPE_T0;
		updateExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_real_fee,m_fund_bind.Fcre_id,redem_type);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
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

void FundKfRedemSpReq::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stTradeBuy.Fsub_trans_id);

	try
	{
		TRACE_DEBUG("[SubaccDraw][spid:%s][Fqqid:%s][client_ip:%s][Fsub_trans_id:%s][total_fee:%ld][acc_time:%s]",
				m_params.getString("spid").c_str(), m_fund_bind.Fqqid, m_params.getString("client_ip").c_str(),
				m_stTradeBuy.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeBuy.Facc_time);
		//TODO: 记得去掉注释Not enough balance on account!
	    SubaccDraw(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
		m_stTradeBuy.Fsub_trans_id, m_total_fee, m_stTradeBuy.Facc_time);
	}
	
	catch(CException& e)
	{

		//赎回要谨慎补单
		//如果赎回子账户减钱超过10分钟没成功的告警不在补单，无论是差错补单还是外部批跑补单，10分钟没成功的异常时都会触发告警
		if(payNotifyOvertime(m_stTradeBuy.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回补单超过10分钟子账户仍未成功");		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);

		throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功
		
	}
	
}


/**
 * 更新赎回状态
 * @return 
 */
void FundKfRedemSpReq::UpdateState()
{
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	stRecord.Fstate = REDEM_SUC;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
    stRecord.Fpurpose = m_stTradeBuy.Fpurpose;
    stRecord.Floading_type = m_stTradeBuy.Floading_type;
    SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
	UpdateFundCloseState();
}

/**
*	到基金公司赎回成功处理
*	减基金账户余额
*/
void FundKfRedemSpReq::RecordRedemTradeForSuc() throw (CException)
{
	UpdateState();
	
	doDraw();

	//交易记录发MQ,组装变化的参数	
	sendFundBuy2MqMsg(m_stTradeBuy);
}

void FundKfRedemSpReq::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	char szMsg[MAX_MSG_LEN + 1] = {0};

    // 组装关键参数
    CUrlAnalyze::setParam(szMsg, "Flistid", fundTradeBuy.Flistid, true);
    CUrlAnalyze::setParam(szMsg, "Fspid", fundTradeBuy.Fspid);
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


/*
 * 查询基金账户是否存在，以及验证参数的一致性
 */
void FundKfRedemSpReq::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }

	// 记录存在，读出记录中的trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
}

/*
*检查是否绑定基金公司帐号
*/
void FundKfRedemSpReq::CheckFundBindSpAcc() throw (CException)
{
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	//赎回不再限制必须是主交易帐号，只限制申购
	//queryValidMasterSpAcc(m_pFundCon, m_fund_bind_sp_acc,m_params.getString("spid"), false);
	queryValidFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);

}

/**
  * 检查基金交易记录是否已经生成
  */
void FundKfRedemSpReq::CheckFundTrade() throw (CException)
{
    // 没有赎回记录，继续下一步
    m_bBuyTradeExist = QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		0, &m_stTradeBuy, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", m_bBuyTradeExist);

	if(!m_bBuyTradeExist)
	{
		return;
	}

	m_params.setParam("fund_trans_id", m_stTradeBuy.Flistid);
	
	// 检查关键参数
	if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}


	if(0 != strcmp(m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str()))
	{
		gPtrAppLog->error("fund trade exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fund_code is different!", __FILE__, __LINE__);
	}

	//请求的total_fee就是赎回的本金
	if(m_stTradeBuy.Freal_redem_amt!= m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, total_fee is different!", __FILE__, __LINE__);
	}

	if(0 != strcmp(m_stTradeBuy.Fcft_trans_id, m_params.getString("cft_trans_id").c_str()))
	{
		gPtrAppLog->error("fund trade exists, cft_trans_id is different! cft_trans_id in db[%s], cft_trans_id input[%s] ", 
			m_stTradeBuy.Fcft_trans_id, m_params.getString("cft_trans_id").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, cft_trans_id is different!", __FILE__, __LINE__);
	}
    
    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T0 || m_stTradeBuy.Floading_type != DRAW_USE_LOADING)
    {
            gPtrAppLog->error("fund trade exists, Floading_type is confict! Floading_type in db[%d], fetch_type input[%s] ", 
    			m_stTradeBuy.Floading_type, m_params.getString("fetch_type").c_str());
            throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fetch_type is confict!", __FILE__, __LINE__);
    }


	if(m_params.getInt("uid") != 0 && m_stTradeBuy.Fuid!=0 && m_params.getInt("uid") != m_stTradeBuy.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_stTradeBuy.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}
    
	// 记录存在，物理状态无效，报错
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeBuy.Flistid, m_stTradeBuy.Ftrade_id, m_stTradeBuy.Fpur_type);
		throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
	}

    if(m_stTradeBuy.Fstate == REDEM_ININ )
    {
		//补单重入，不抛异常，继续执行
		//可能存在扣子账户失败
		return;
    }
	else if(m_stTradeBuy.Fstate == REDEM_SUC)
	{
		//重入
		throw CException(ERR_REPEAT_ENTRY, "fund redem from sp success. ", __FILE__, __LINE__);
	}
	else
    {
		//其它状态的赎回不可以重入
		throw CException(ERR_REDEM_REPEAT_ENTRY, "fund redem trade exist. ", __FILE__, __LINE__);
    }
}

void FundKfRedemSpReq::QueryFundCloseId()
{
	strncpy(m_fund_close.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_close.Ftrade_id) - 1);
	strncpy(m_fund_close.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fund_close.Ftrade_id) - 1);
	strncpy(m_fund_close.Fend_date, m_params.getString("end_date").c_str(), sizeof(m_fund_close.Fend_date) - 1);

	bool ret = queryFundCloseTransByEndDate(m_pFundCon, m_fund_close, true);

	if (ret) 
	{
		//检查bank_billno
		if(strlen(m_fund_close.Fsell_listid) != 0 && 
			strncmp(m_stTradeBuy.Flistid, m_fund_close.Fsell_listid, sizeof(m_fund_close.Fsell_listid)) != 0)
		{
			throw CException(ERR_FUND_CLOSE_REQ_FAIL, "sell_listid already exist.", __FILE__, __LINE__);		
		}
		
		m_real_fee = m_fund_close.Fstart_total_fee;
		m_total_fee = m_fund_close.Fcurrent_total_fee;
		m_params.setParam("fund_close_fid", m_fund_close.Fid);
	}
	else 
	{
		throw CException(ERR_FUND_CLOSE_REQ_FAIL, "fund close query error.", __FILE__, __LINE__);
	}

	if(m_real_fee != m_params.getLong("total_fee"))
	{
		TRACE_DEBUG("[定期本金:%ld][強赎金额:%ld]",m_real_fee, m_params.getLong("total_fee"))
		throw CException(ERR_FUND_CLOSE_BALANCE_NOT_EQ, "強赎金额不等于本金", __FILE__, __LINE__);
	}
}

/**
* 查询余额通账户余额
* 查询定期基金本金是否和total fee一致
*/
void FundKfRedemSpReq::CheckFundBalance()
{
	//TODO 冻结金额是哪个字段,子账户只用来记账，暂时不存在冻结部分
	//TODO 有冻结资金不能做份额转换	
	strncpy(m_fund_close.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_close.Ffund_code) - 1);
	strncpy(m_fund_close.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fund_close.Ftrade_id) - 1);
	strncpy(m_fund_close.Fend_date, m_params.getString("end_date").c_str(), sizeof(m_fund_close.Fend_date) - 1);

	bool ret = queryFundCloseTransByEndDate(m_pFundCon, m_fund_close, true);

	if (ret)
	{
		//检查bank_billno
		if(strlen(m_fund_close.Fsell_listid) != 0)//原来有生成赎回单，需要判断是重入还是重新发起的,如果是重新发起的就拒绝
		{
			ST_TRADE_FUND m_stTrade;
			if(!QueryTradeFund(m_pFundCon, m_fund_close.Fsell_listid, PURTYPE_REDEEM, &m_stTrade, true))//之前记录的赎回单号有误
			{
				gPtrAppLog->error("buy record not exist, listid[%s]  ", m_fund_close.Fsell_listid);
				throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
			}

			if(strncmp(m_stTrade.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(m_stTrade.Fcft_bank_billno)) != 0)
			{
				throw CException(ERR_FUND_CLOSE_REQ_FAIL, "sell_listid already exist.", __FILE__, __LINE__);		
			}
		}
		m_real_fee = m_fund_close.Fstart_total_fee;
		m_total_fee = m_fund_close.Fcurrent_total_fee;

		m_params.setParam("fund_close_fid", m_fund_close.Fid);
	}
	else 
	{
		throw CException(ERR_FUND_CLOSE_REQ_FAIL, "fund close query error.", __FILE__, __LINE__);
	}
	if (m_fund_close.Fstate != CLOSE_FUND_STATE_PENDING)
	{
		throw CException(ERR_FUND_CLOSE_REDEM_SUC, "该基金状态不能被违约赎回", __FILE__, __LINE__);
	}
	
	if(m_real_fee != m_params.getLong("total_fee"))
	{
		TRACE_DEBUG("[定期本金:%ld][強赎金额:%ld]",m_real_fee, m_params.getLong("total_fee"))
		throw CException(ERR_FUND_CLOSE_BALANCE_NOT_EQ, "強赎金额不等于本金", __FILE__, __LINE__);
	}

	string max_trade_date=getCacheCloseMaxTradeDate(gPtrFundSlaveDB, string(m_fund_close.Ftrans_date), m_params.getString("fund_code"));
	
	if (atoi(max_trade_date.c_str())>=GetDateToday())
	{
		throw CException(ERR_FUND_CLOSE_FOCRE_STATE, "该期基金份额未确认完整,不能强制赎回", __FILE__, __LINE__);
	}
	
	if (atoi(m_fund_close.Fbook_stop_date)<=GetDateToday())
	{
		throw CException(ERR_FUND_CLOSE_FOCRE_TOO_LATE, "该期基金马上结束,不能强制赎回", __FILE__, __LINE__);
	}	

}

/**
* 检查赎回垫资账户额度
*/
void FundKfRedemSpReq::checkSpLoaning() throw (CException)
{
	//赎回类型为提现才累计赎回额度

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));
	
	strncpy(fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}


    if ((fundSpConfig.Fredem_valid&0x07) ==2) // 停止赎回
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
    
    //垫资账户额度超限的处理
    checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),true);

}

/**
* 检查赎回垫资账户额度,并累加额度
*/
void FundKfRedemSpReq::checkSpLoaningPlus() throw (CException)
{
	//赎回类型为提现才累计赎回额度

	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));
	
	strncpy(fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}


    if ((fundSpConfig.Fredem_valid&0x07) ==2) // 停止赎回
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
    
    //垫资账户额度超限的处理
    checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),true);

	//更新赎回累计额度,t+1提现赎回不累计，其它包括普通赎回、消费、t+0赎回都要累计
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    LONG tmp_total_fee = m_params.getLong("total_fee");
    updateFundSpRedomTotal(m_pFundCon, fundSpConfig, tmp_total_fee,m_stTradeBuy.Floading_type,m_stTradeBuy.Facc_time);
	
}

void FundKfRedemSpReq::GenerFundTrade()
{
	memset(&m_stRecord, 0, sizeof(ST_TRADE_FUND));

	string drawid = genSubaccDrawid();
	string cft_bank_billno = m_params.getString("cft_bank_billno");
	//赎回单号内部生成  10商户号+8位日期+10序列号+cft_bank_billno后3位，保证cft_bank_billno和listid的分库分表规则一致，不用在拆分表
	string listid =  m_params.getString("spid") + drawid + cft_bank_billno.substr(cft_bank_billno.size()-3);
	m_params.setParam("fund_trans_id", listid);

    strncpy(m_stRecord.Flistid, listid.c_str(), sizeof(m_stRecord.Flistid)-1);
    strncpy(m_stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(m_stRecord.Fspid)-1);
    strncpy(m_stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(m_stRecord.Fcoding)-1);
    strncpy(m_stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_stRecord.Ftrade_id)-1);
    m_stRecord.Fuid = m_params.getInt("uid");
    strncpy(m_stRecord.Ffund_name, m_fund_sp_config.Ffund_name, sizeof(m_stRecord.Ffund_name)-1);
    strncpy(m_stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_stRecord.Ffund_code)-1);
    m_stRecord.Fbank_type = 0;
    strncpy(m_stRecord.Fcard_no, m_params.getString("card_no").c_str(), sizeof(m_stRecord.Fcard_no)-1);
    m_stRecord.Fpur_type = PURTYPE_REDEEM;
	//total fee = 取整个期次的本金+收益
	//Freal_redem_amt = 取整个期次的本金
    m_stRecord.Ftotal_fee = m_total_fee;
	m_stRecord.Freal_redem_amt = m_real_fee;
	m_stRecord.Fspe_tag = 0;//超时补单成功需要将超时状态修改，否则导致不停补单
    m_stRecord.Fstate = REDEM_ININ;
    m_stRecord.Flstate = LSTATE_VALID;
	m_stRecord.Fopt_type = KF_REDEM;
	m_stRecord.Fclose_listid = m_params.getLong("fund_close_fid");
	strncpy(m_stRecord.Fend_date, m_params.getString("end_date").c_str(), sizeof(m_stRecord.Fend_date) - 1);
	
    strncpy(m_stRecord.Ffetchid, m_params.getString("cft_fetch_id").c_str(), sizeof(m_stRecord.Ffetchid)-1);
    m_stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(m_stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Fcreate_time)-1);
    strncpy(m_stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Fmodify_time)-1);
    //财付通核心是否支持重入? m_stRecord.Fstandby1 = 1; // 锁定记录
    m_stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));; // 币种类型
    strncpy(m_stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Facc_time)-1);
	strncpy(m_stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(m_stRecord.Fcft_trans_id)-1);
	strncpy(m_stRecord.Fcft_charge_ctrl_id, m_params.getString("cft_charge_ctrl_id").c_str(), sizeof(m_stRecord.Fcft_charge_ctrl_id)-1);
	strncpy(m_stRecord.Fsp_fetch_id, m_params.getString("sp_fetch_id").c_str(), sizeof(m_stRecord.Fsp_fetch_id)-1);
	strncpy(m_stRecord.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(m_stRecord.Fcft_bank_billno)-1);
	strncpy(m_stRecord.Fsub_trans_id, drawid.c_str(), sizeof(m_stRecord.Fsub_trans_id)-1);
	m_stRecord.Fpurpose = m_params.getInt("purpose");
	strncpy(m_stRecord.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(m_stRecord.Fchannel_id)-1);

    m_stRecord.Floading_type = DRAW_USE_LOADING;
}

/**
  * 生成基金赎回记录，状态: 初始赎回状态
  */
void FundKfRedemSpReq::RecordFundTrade()
{
    InsertTradeFund(m_pFundCon, &m_stRecord);
    InsertTradeUserFund(m_pFundCon, &m_stRecord);

	//记录垫资帐号提现单和基金交易单对应关系
	if(!m_params.getString("sp_fetch_id").empty())
	{
		FundFetch fundFetch;
		memset(&fundFetch, 0, sizeof(FundFetch));
		
		strncpy(fundFetch.Ffetchid, m_params.getString("sp_fetch_id").c_str(), sizeof(fundFetch.Ffetchid)-1);
		strncpy(fundFetch.Ffund_trans_id, m_stRecord.Flistid, sizeof(fundFetch.Ffund_trans_id)-1);
		strncpy(fundFetch.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fcreate_time)-1);
    	strncpy(fundFetch.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fmodify_time)-1);
		

		insertFundFetch(m_pFundCon, fundFetch);
	}

	
}


/**
 * 更新定期状态为成功
 * @return 
 */
void FundKfRedemSpReq::UpdateFundCloseState()
{
	//更新定期产品状态
	FundCloseTrans fund_close_set;
	fund_close_set.Fid = m_params.getLong("fund_close_fid");
	fund_close_set.Fcurrent_total_fee = 0;
	fund_close_set.Fstate = CLOSE_FUND_STATE_SUC;
	fund_close_set.Fuser_end_type = KF_REDEM;

	//strncpy(fund_close_set.Fsell_listid, m_params.getString("fund_trans_id").c_str(), sizeof(fund_close_set.Fsell_listid)-1);
	strncpy(fund_close_set.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fund_close_set.Fmodify_time)-1);
	strncpy(fund_close_set.Ftrade_id, m_stTradeBuy.Ftrade_id, sizeof(fund_close_set.Ftrade_id)-1);
	
	saveFundCloseTrans(fund_close_set,m_fund_close,m_params.getString("fund_trans_id").c_str(),PURTYPE_REDEEM);
}

void FundKfRedemSpReq::UpdateFundCloseListid()
{
	FundCloseTrans closeTrans;

	closeTrans.Fid=m_params.getLong("fund_close_fid");
	strncpy(closeTrans.Fsell_listid, m_params.getString("fund_trans_id").c_str(), sizeof(closeTrans.Fsell_listid)-1);
	strncpy(closeTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(closeTrans.Fmodify_time)-1);
	strncpy(closeTrans.Ftrade_id, m_stRecord.Ftrade_id, sizeof(closeTrans.Ftrade_id)-1);
	
	updateFundCloseTransById(gPtrFundDB,closeTrans);
}


/**
  * 打包输出参数
  */
void FundKfRedemSpReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_bBuyTradeExist ? m_stTradeBuy.Facc_time : m_params.getString("systime").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
	CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_trans_id", m_params.getString("fund_trans_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "spid", m_params.getString("spid").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_name", m_fund_sp_config.Fsp_name);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_params.getString("fund_code").c_str());
	CUrlAnalyze::setParam(rqst->odata, "fund_name", m_fund_sp_config.Ffund_name);
	CUrlAnalyze::setParam(rqst->odata, "cur_total_fee", m_total_fee);

    rqst->olen = strlen(rqst->odata);
    return;
}
