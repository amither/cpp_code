/**
  * FileName: fund_redem_close_ack_service.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2014-9-6
  * Description: 基金交易服务 定期产品实时赎回确认 源文件
  */

#include "fund_commfunc.h"
#include "fund_redem_close_ack_service.h"

FundRedemCloseAck::FundRedemCloseAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

	memset(&m_stTradeRedem, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));
	memset(&m_close_trans, 0, sizeof(FundCloseTrans));

	m_draw_arrive_type = DRAW_ARRIVE_TYPE_T1; //默认t+1 提现
	m_loading_type = DRAW_NOT_USE_LOADING; //默认为不需要垫资
    m_stop_fetch = false;
	m_need_updateExauAuthLimit = false;
}

/**
  * service step 1: 解析输入参数
  */
void FundRedemCloseAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	TRACE_DEBUG("[fund_redem_close_ack_service] parseInputMsg start: ");

	// 要保留请求数据，抛差错使用
    m_request = rqst;

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_close_ack_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(szMsg, "fund_trans_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "spid", 10, 15);
	m_params.readStrParam(szMsg, "sp_billno", 0, 32);
    m_params.readStrParam(szMsg, "fund_name", 0, 64);
    m_params.readStrParam(szMsg, "fund_code", 0, 64);
	m_params.readIntParam(szMsg, "op_type", 1, 4);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_optype = m_params.getInt("op_type");

}

/*
 * 生成基金注册用token
 */
string FundRedemCloseAck::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|cft_bank_billno|spid| sp_billno | total_fee |key
    // 规则生成原串
    ss << m_params["uid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;
	TRACE_DEBUG("FundRedemCloseAck, sourceStr=[%s]", 
						ss.str().c_str());

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundRedemCloseAck::CheckToken() throw (CException)
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
void FundRedemCloseAck::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

	if(INF_REDEM_SP_ACK_SUC == m_optype)
	{
		CHECK_PARAM_EMPTY("sp_billno");   
	}

}

/**
  * 执行申购请求
  */
void FundRedemCloseAck::excute() throw (CException)
{
    try
    {
        CheckParams();

		CheckFundBind();

         /* 开启事务 */
        m_pFundCon->Begin();

        /* ckv操作放到事物之后真正提交 */
        gCkvSvrOperator->beginCkvtrans();

		/* 查询基金交易记录 */
        CheckFundTrade();

        /* 减账户余额，更新基金交易状态 */
        UpdateTradeState();

        /* 提交事务 */
        m_pFundCon->Commit();

        gCkvSvrOperator->commitCkvtrans();
		
		/* 发送MQ */
		sendFundBuy2MqMsg(m_stTradeRedem);
		
		/* 更新EXAU */
		updateExauAuthLimitNoExcp();

		/* 更新各类ckv ,放在事务之后是避免事务回滚却写入ckv的问题*/
		updateCkvs();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
         //回滚db前先回滚本地ckv
         gCkvSvrOperator->rollBackCkvtrans();
         
        m_pFundCon->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_CLOSE_REDEM_DRAW_SUCC != (unsigned)e.error()))
        {
            throw;
        }
    }
}


/*
 * 查询基金账户是否存在
 */
void FundRedemCloseAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}


/**
  * 检查基金交易记录是否已经生成
  */
void FundRedemCloseAck::CheckFundTrade() throw (CException)
{
	// 没有交易记录，报错
	if(!QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeRedem, true))
	{
		gPtrAppLog->error("buy record not exist, cft_bank_billno[%s]  ", m_params.getString("cft_bank_billno").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	// 物理状态无效，报错
	if(LSTATE_INVALID == m_stTradeRedem.Flstate)
	{
		gPtrAppLog->error("fund buy pay, lstate is invalid. listid[%s], uid[%d] ", m_stTradeRedem.Flistid, m_stTradeRedem.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
	}

	if(m_params.getInt("uid") != 0 && m_stTradeRedem.Fuid!=0 && m_params.getInt("uid") != m_stTradeRedem.Fuid)
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_stTradeRedem.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}

	if( (0 != strcmp(m_stTradeRedem.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeRedem.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}

	// 得到交易相关信息
	m_params.setParam("trade_id", m_stTradeRedem.Ftrade_id);
	m_params.setParam("close_id",m_stTradeRedem.Fclose_listid);
}


/**
  * 检查已扣减成功的定期交易记录
  */
void FundRedemCloseAck::CheckAckSucTrans() throw (CException)
{
	//获取定期记录( 重入检查不用锁记录)
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,false)){
	    TRACE_ERROR("unfound fund_close_trans close_id");
	    throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "unfound fund_close_trans close_id");
	}
	// 检查全部赎回的关联单号
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_ALL_REDEM
		&&0!=strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)){
	    TRACE_ERROR("all redem close_trans sellid diff");
	    throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem close_trans sellid diff");
	}
	// 检查部分赎回关联单号
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM
		&&0==strcmp(m_close_trans.Fsell_listid,m_stTradeRedem.Flistid)){
			TRACE_ERROR("all redem fund trade exists, stTradeRedem opt_type invalid");
			throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "all redem fund trade exists, stTradeRedem opt_type invalid");
	}
	// 部分赎回校验金额: 5状态使用total_fee检查
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM&&m_stTradeRedem.Ftotal_fee != m_params.getLong("total_fee"))
	{
		TRACE_ERROR("fund redem, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
}


/**
  * 减子账户前检查定期交易记录
  */
void FundRedemCloseAck::CheckForUpdateSuc() throw (CException)
{
	if( REDEM_ININ != m_stTradeRedem.Fstate){
		//不为赎回初始状态，不允许再次更新成功。
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	
	//获取定期记录
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,true)){
	    TRACE_ERROR("unfound fund_close_trans close_id");
	    throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "unfound fund_close_trans close_id");
	}
	// 检查total_fee应该相等
	if(m_stTradeRedem.Ftotal_fee != m_params.getLong("total_fee")){
		TRACE_ERROR("fund partial redem, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ",m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}

    // 检查时间    
	string dueDate=string(m_close_trans.Fdue_date);
	string dueTime=dueDate+"150000";	
	string bookStopDate=string(m_close_trans.Fbook_stop_date);
	string bookStopTime=bookStopDate+"150000"; 	
	if(m_params.getString("systime")>=changeDatetimeFormat(dueTime)||
		m_params.getString("systime")<changeDatetimeFormat(bookStopTime)){
		TRACE_ERROR("cannot redem close_trans right now");
		throw EXCEPTION(ERR_CLOSE_REDEM_TIME, "cannot redem close_trans right now");
	}
	// 已发生扫尾赎回,不能再发起其他形式赎回
	if(m_close_trans.Fsell_listid[0]!=0){
		TRACE_ERROR("close_trans sell_listid exists, cannot redem anymore");
		throw EXCEPTION(ERR_CLOSE_REDEM_HAS_ALL_REDEM, "close_trans sell_listid exists, cannot redem anymore");
	}
	// 预约部分赎回和全部顺延才可以发起赎回
	if(m_close_trans.Fuser_end_type!=CLOSE_FUND_END_TYPE_PATRIAL_REDEM
		&&m_close_trans.Fuser_end_type!=CLOSE_FUND_END_TYPE_ALL_EXTENSION){
		TRACE_ERROR("close_trans user_end_type cannot redem anymore");
		throw EXCEPTION(ERR_CLOSE_REDEM_USER_END, "close_trans user_end_type cannot redem anymore");
	}
	// 检查状态
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING&&m_close_trans.Fstate!=CLOSE_FUND_STATE_REDEM_SUC){
		TRACE_ERROR("close_trans status cannot redem");
		throw EXCEPTION(ERR_CLOSE_REDEM_STATE, "close_trans status cannot redem");
	}
	// 检查剩余金额
	LONG leftFee=m_close_trans.Fcurrent_total_fee;
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_PATRIAL_REDEM&&m_close_trans.Fstate==CLOSE_FUND_STATE_PENDING){
		leftFee=leftFee-m_close_trans.Fend_plan_amt;
	}
	if(leftFee<m_params.getLong("total_fee")){		
		TRACE_ERROR("close_trans status cannot redem");
		throw EXCEPTION(ERR_CLOSE_REDEM_NOT_ENOUGH_MONEY, "close_trans status cannot redem");
	}
	//扫尾赎回金额应该等于剩余全部金额
	if(leftFee!=m_params.getLong("total_fee")&&m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		TRACE_ERROR("close_trans all redem money[%ld][%ld] invalid",leftFee,m_params.getLong("total_fee"));
		throw EXCEPTION(ERR_CLOSE_REDEM_INVALID_MONEY, "close_trans all redem money invalid");
	}
	// 检查预约赎回截止日的收益入账是否已经完成
	string bookStopProfitTime=bookStopDate+gPtrConfig->m_AppCfg.close_redem_ack_stop_time;
	if(m_params.getString("systime")>=changeDatetimeFormat(bookStopProfitTime)){
		FundReconLog recon;
		memset(&recon,0,sizeof(FundReconLog));
		recon.Frecon_type=RECON_TYPE_PROFIT;
		strncpy(recon.Fspid,m_close_trans.Fspid,sizeof(recon.Fspid));
		strncpy(recon.Frecon_date,m_close_trans.Fbook_stop_date,sizeof(recon.Frecon_date));
		bool existRecon = queryFundSpReconLog(m_pFundCon,recon);
		if(!existRecon){
			TRACE_ERROR("profit is recording,cannot redem right now[%s][%s]",m_close_trans.Fspid,m_close_trans.Fbook_stop_date);
			throw EXCEPTION(ERR_CLOSE_REDEM_PROFIT_RECORDING, "profit is recording,cannot redem right now");
		}
	}
}


/**
*根据请求类型，做相应的处理 
*/
void FundRedemCloseAck::UpdateTradeState()
{
	switch (m_optype)
    {       
        case INF_REDEM_SP_ACK_SUC:
			UpdateRedemTradeForSuc();
            break;
			
		case INF_REDEM_SP_ACK_TIMEOUT:
            UpdateRedemTradeForTimeout();
            break;
			
		case INF_REDEM_SP_ACK_FAIL:
            UpdateRedemTradeForFail();
            break;

		case INF_REDEM_SP_ACK_FINISH:
            UpdateRedemTradeForFinish();
            break;
                    
        default:
            throw CException(ERR_BAD_PARAM, "op_type invalid", __FILE__, __LINE__);
            break;

    }
}

/**
*	到基金公司赎回成功处理
*	减基金账户余额
*/
void FundRedemCloseAck::UpdateRedemTradeForSuc() throw (CException)
{
	if(TRADE_RECORD_TIMEOUT == m_stTradeRedem.Fspe_tag){
		//超时补单的订单直接去补单
		UpdateRedemTradeForBudan();
		return;
	}
	
	if(REDEM_SUC == m_stTradeRedem.Fstate || REDEM_FINISH == m_stTradeRedem.Fstate){
		// 重入检查
		CheckAckSucTrans();
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}
	// 更新子账户前检查单状态
	CheckForUpdateSuc();

	//先减账户余额，减余额失败直接报错并回滚事务
	doDraw();

	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	if(INF_REDEM_SP_ACK_TIMEOUT == m_optype){
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
	}else{
		stRecord.Fspe_tag = 0;//超时补单成功需要将超时状态修改，否则导致不停补单
	}
	stRecord.Fstate = REDEM_SUC;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fuid = m_stTradeRedem.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	stRecord.Fpurpose = m_stTradeRedem.Fpurpose;
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);

	FundCloseTrans closeTrans;
	closeTrans.Fid = m_close_trans.Fid;
	strncpy(closeTrans.Ftrade_id, m_close_trans.Ftrade_id, sizeof(closeTrans.Ftrade_id)-1);
	strncpy(closeTrans.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(closeTrans.Fmodify_time)-1);
	// 扫尾赎回记录单号
	if(m_stTradeRedem.Fopt_type==FUND_TRADE_OPT_CLOSE_ALL_REDEM){
		strncpy(closeTrans.Fsell_listid, m_stTradeRedem.Flistid, sizeof(closeTrans.Fsell_listid)-1);
	}
	closeTrans.Fcurrent_total_fee=m_close_trans.Fcurrent_total_fee-m_stTradeRedem.Ftotal_fee;
	closeTrans.Fend_real_sell_amt=m_close_trans.Fend_real_sell_amt+m_stTradeRedem.Ftotal_fee;
	
	
	//交易记录发MQ,组装变化的参数
	m_stTradeRedem.Fstate= stRecord.Fstate;
	strncpy(m_stTradeRedem.Fcoding, stRecord.Fcoding, sizeof(m_stTradeRedem.Fcoding) - 1);
	strncpy(m_stTradeRedem.Fmemo, stRecord.Fmemo, sizeof(m_stTradeRedem.Fmemo) - 1);
	//减子账户成功即返回业务成功，更新赎回单失败发差错补单，必须做成功
	try
	{
		// 更新交易单
		UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));
		// 更新定期记录
		saveFundCloseTrans(closeTrans,m_close_trans,m_stTradeRedem.Flistid,PURTYPE_REDEEM);
		// 同步内存数据
		m_close_trans.Fcurrent_total_fee = closeTrans.Fcurrent_total_fee;
		m_close_trans.Fend_real_sell_amt = closeTrans.Fend_real_sell_amt;

		//减总账户，和更新单放在一个事务里，保证一个单只操作一次
		//recordFundTotalaccDraw();
		
		//最后减基金公司额度
		checkSpLoaning();
		
		//更新成功, 需要累加exau
		m_need_updateExauAuthLimit = true;
	}
	catch(CException& e)
	{
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);
		
		if(ERR_TYPE_MSG)
		{
			//来自差错补单时间和赎回单创建时间超过10分钟的告警
			int inteval = (int)(time(NULL) - toUnixTime(m_stTradeRedem.Facc_time));	
			gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
			//补单10分钟仍未成功告警,告警服务本身会做压制
			if(inteval >= 600 )
			{	
				char szErrMsg[256] = {0};
		        snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回差错补单超过%d分钟未成功" ,inteval);
		        
		        alert(ERR_BUDAN_TOLONG, szErrMsg);
			}	
			throw;//来自差错补单的直接抛出
		}else{
			// 非差错直接返回成功, 但回滚事务
			throw CException(ERR_CLOSE_REDEM_DRAW_SUCC, "draw success need retry! ", __FILE__, __LINE__);
		}
	}
	catch(...)
	{
		//不应该发生，发生了告警
		char szErrMsg[256] = {0};
        snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.update close redem trade for success unexception.");
        
        alert(ERR_DB_UNKNOW, szErrMsg);
	}
	
	//更新成功,调整用户操作排序, 只排序,非关键信息.更新失败不回滚
	//使用forupdate 锁用户，防止并发申购修改user_acc信息
	try{
		ST_FUND_BIND fund_bind; 
		QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &fund_bind, true);
		updateUserAcc(m_stTradeRedem);
	}catch(CException& e)
	{TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());}

}

void FundRedemCloseAck::checkSpLoaning() throw (CException)
{

    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, m_stTradeRedem.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stTradeRedem.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    if ((fundSpConfig.Fredem_valid&0x07) ==2) // 停止赎回
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }
	
	/** 计算金额**/
	/** 最多仅赎回全部本金,不赎回收益部分**/
	/** 算法: 先赎回收益,再赎回本金**/
	LONG redemScope;
	// 本次赎回完后,当前金额大于本金:只赎回收益, 不扣减余额
	if(m_close_trans.Fcurrent_total_fee>=m_close_trans.Fstart_total_fee){
		redemScope=0;
	// 本次赎回前,当前金额已经小于本金:之前收益已经赎回完,这次全部扣减
	}else if(m_params.getLong("total_fee")+m_close_trans.Fcurrent_total_fee<=m_close_trans.Fstart_total_fee){
		redemScope=m_params.getLong("total_fee");
	// 本次赎回包含部分收益,部分本金:只赎回本金减少部分
	}else{
		redemScope=m_close_trans.Fstart_total_fee-m_close_trans.Fcurrent_total_fee;
	}
	
    //更新赎回累计额度,t+1提现赎回不累计，其它包括普通赎回、消费、t+0赎回都要累计
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedom(m_pFundCon, fundSpConfig, m_stTradeRedem.Ftotal_fee,redemScope,m_stTradeRedem.Floading_type,m_stTradeRedem.Facc_time);
	
}


/**
*赎回超时处理
*/
void FundRedemCloseAck::UpdateRedemTradeForTimeout() throw (CException)
{
	//根据配置决定赎回超时当成功还是失败，如果当成功处理，需要开启补单批跑程序对超时赎回单进行补单
	if("true" == gPtrConfig->m_AppCfg.redem_timeout_conf)
	{
		UpdateRedemTradeForSuc();
	}
	else
	{
		UpdateRedemTradeForFail();
	}
}

/**
*	赎回失败处理
*/
void FundRedemCloseAck::UpdateRedemTradeForFail() throw (CException)
{
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	if(REDEM_FAIL == m_stTradeRedem.Fstate)
	{
		//重入错误
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_ININ != m_stTradeRedem.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
	}

	stRecord.Fstate = REDEM_FAIL;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fuid = m_stTradeRedem.Fuid;
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
	//保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);
    
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));
}

void FundRedemCloseAck::UpdateRedemTradeForFinish() throw (CException)
{
	// 校验关键参数:提现状态金额使用真实金额
	LONG checkFee=m_stTradeRedem.Freal_redem_amt>0?m_stTradeRedem.Freal_redem_amt:m_stTradeRedem.Ftotal_fee;
	if(checkFee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			checkFee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}	
	
	if(REDEM_FINISH == m_stTradeRedem.Fstate)
	{
		//重入
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_SUC != m_stTradeRedem.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	ST_TRADE_FUND  stRecord;
	
	// 赎回到余额直接更新到账时间和到账状态
	if(m_stTradeRedem.Fpurpose==PURPOSE_REDEM_TO_BA||m_stTradeRedem.Fpurpose==PURPOSE_REDEM_TO_BA_T1)
	{
		strncpy(stRecord.Ffetch_arrival_time,m_params.getString("systime").c_str(),sizeof(stRecord.Ffetch_arrival_time));
		stRecord.Ffetch_result=FETCH_RESULT_BALANCE_SUCCESS;
	}
	
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fstate = REDEM_FINISH;
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fspe_tag = m_stTradeRedem.Fspe_tag; //不能修改超时标记
	stRecord.Fuid = m_stTradeRedem.Fuid;
	//保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);
    
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));

}

void FundRedemCloseAck::UpdateRedemTradeForBudan() throw (CException)
{
	// 校验关键参数:商户补单使用total_fee校验金额
	if(m_stTradeRedem.Ftotal_fee!= m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeRedem.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	if(REDEM_SUC != m_stTradeRedem.Fstate && REDEM_FINISH != m_stTradeRedem.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(TRADE_RECORD_TIMEOUT != m_stTradeRedem.Fspe_tag || INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		//非超时单，当重入错误,或者超时再重入，直接返回
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}	

	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	stRecord.Fspe_tag = 0;
	strncpy(stRecord.Flistid, m_stTradeRedem.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeRedem.Fpur_type;
	stRecord.Fuid = m_stTradeRedem.Fuid;
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_stTradeRedem.Ftrade_id);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeRedem, m_params.getString("systime"));
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


/**
 * 子账户核心提现
 * 子账户失败了不能差错补单，用户可以主动再发起赎回，如果补单会多赎回或造成财付通损失
 * 写在这里的，赎回到底是以基金公司成功为准，还是以子账户为准，一直在争论，前面讨论定下来的方案，后面有人忘记了就再拿出来说事。。。
 * 赎回涉及多个系统 基金公司+余额增值系统+子账户+财付通 +及其它相关操作设计的系统
 * 一个大的原则是减钱成功加基金交易单成功即认为成功，其它的失败都通过补单来完成。
 * 减钱涉及到两个系统，基金公司系统减份额，子账户减钱，到底以哪个为准? 以谁为准都有问题:
 * 个人认为以子账户成功为准更为靠谱，理由: 1)子账户是财付通可控制的，成功与否不由基金公司决定；2)子账户异常了我们可以通过实时对账发现，并及时补单；
 * 存在的问题:子账户失败未扣钱，基金公司已经减份额，用户无法再次赎回，可以通过第二天与基金公司的对账把用户资金补上，后续可以优化，对这类单实时通知基金公司赎回失败；
 *
 */
void FundRedemCloseAck::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stTradeRedem.Fsub_trans_id);

	try
	{
	    SubaccDraw(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeRedem.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeRedem.Facc_time);
	}
	
	catch(CException& e)
	{

		//赎回要谨慎补单
		//如果赎回子账户减钱超过10分钟没成功的告警不在补单，无论是差错补单还是外部批跑补单，10分钟没成功的异常时都会触发告警
		if(payNotifyOvertime(m_stTradeRedem.Facc_time))	
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

void FundRedemCloseAck::updateCkvs()
{
	if(!m_need_updateExauAuthLimit)
	{
		return;
	}
	//更新定期记录CKV
	setFundCloseTransToKV(m_close_trans.Ftrade_id,m_close_trans.Ffund_code);
}

void FundRedemCloseAck::updateExauAuthLimitNoExcp()
{
	if(!m_need_updateExauAuthLimit)
	{
		return;
	}

	// T+0  垫资赎回才限额(定期T+1赎回不限额)
	if (m_stTradeRedem.Floading_type == 0)
	{
		return;
	}

	//累加exau 放在事务外，避免超时导致的事务回滚
	//赎回请求的时候检查exau限制，累加时失败不报错，避免多次累加，或赎回单无法处理成功的问题
	try
	{
		//累计用户赎回限额
		int redem_type = (m_stTradeRedem.Floading_type == 0?DRAW_ARRIVE_TYPE_T1:DRAW_ARRIVE_TYPE_T0);
		updateExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,redem_type);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}


void FundRedemCloseAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	// 更新金额成功才发MQ，避免多次发送
	if(!m_need_updateExauAuthLimit)
	{
		return;
	}
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


/**
  * 打包输出参数
  */
void FundRedemCloseAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "fetch_type",DRAW_ARRIVE_TYPE_T1);
    CUrlAnalyze::setParam(rqst->odata, "loading_type", DRAW_NOT_USE_LOADING);

    rqst->olen = strlen(rqst->odata);
    return;
}


