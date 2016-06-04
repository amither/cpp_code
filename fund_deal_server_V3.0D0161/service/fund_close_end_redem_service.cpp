/**
  * FileName: fund_close_end_redem_service.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2014-7-30
  * Description: 生成定期期末赎回记录
  */

#include "fund_commfunc.h"
#include "fund_close_end_redem_service.h"

FundCloseEndRedem::FundCloseEndRedem(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_close_trans, 0, sizeof(FundCloseTrans));
	memset(&m_stRecord, 0, sizeof(ST_TRADE_FUND));
	memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));
	m_RedemTradeExist = false;
	m_optype=0;
}

/**
  * service step 1: 解析输入参数
  */
void FundCloseEndRedem::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	m_request = rqst;
    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_close_end_redem_service] receives: %s", szMsg);

    // 读取参数
	m_params.readStrParam(szMsg, "uin", 1, 64);
	m_params.readLongParam(szMsg,"close_id", 0, MAX_LONG);
	m_params.readStrParam(szMsg, "spid", 0, 32);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_charge_ctrl_id", 0, 32);
	m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
	m_params.readStrParam(szMsg, "client_ip", 0, 32);
	m_params.readIntParam(szMsg, "op_type", 1, 4);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

/*
 * 生成基金注册用token
 */
string FundCloseEndRedem::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|cft_bank_billno|spid|total_fee|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["close_id"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundCloseEndRedem::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * 检查参数，获取内部参数
  */
void FundCloseEndRedem::CheckParams() throw (CException)
{
	CHECK_PARAM_EMPTY("uin");
	CHECK_PARAM_EMPTY("close_id");
	CHECK_PARAM_EMPTY("spid");
	CHECK_PARAM_EMPTY("op_type");
	CHECK_PARAM_EMPTY("cft_bank_billno");
	CHECK_PARAM_EMPTY("cft_trans_id");
	//CHECK_PARAM_EMPTY("cft_fetch_id");
	CHECK_PARAM_EMPTY("cft_charge_ctrl_id");
	CHECK_PARAM_EMPTY("bank_type");
    // 验证token
    CheckToken();
	// 检查参数
	m_optype = m_params.getInt("op_type");
	if(m_optype!=OP_TYPE_END_REDEM_REQ&&m_optype!=OP_TYPE_END_REDEM_ACK){
        throw EXCEPTION(ERR_BAD_PARAM, "op_type is invalid");
	}
}


/**
  * 检查参数，获取内部参数
  */
void FundCloseEndRedem::CheckCloseTrans() throw (CException)
{
	char szErrMsg[128];
	//获取定期记录
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,true)){
		snprintf(szErrMsg, sizeof(szErrMsg), "unfound fund_close_trans close_id=%ld,trade_id=%s", m_close_trans.Fid,m_close_trans.Ftrade_id);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);    
	}
	if(strcmp(m_params.getString("trade_id").c_str(),m_close_trans.Ftrade_id)!=0)
	{	
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld]trade_id[%s][%s] is different", m_close_trans.Fid,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
	
	// 判断商户号参数
	if(0!=strcmp(m_close_trans.Fspid,m_params.getString("spid").c_str())){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] spid[%s] is not equal to input[%s]", m_close_trans.Fid,m_close_trans.Fspid,m_params.getString("spid").c_str());
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
	
	// 只有待执行状态才可以发起
	// 赎回成功状态判断重入参数
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_REDEM_SUC&&m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] status[%d] cannot be redem", m_close_trans.Fid,m_close_trans.Fstate);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}	
	
	//判断到期赎回策略
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_ALL_REDEM){
		m_params.setParam("opt_type",FUND_TRADE_OPT_CLOSE_ALL_REDEM);
		m_params.setParam("total_fee",m_close_trans.Fcurrent_total_fee);
		m_params.setParam("next_close_state",CLOSE_FUND_STATE_REDEM_SUC); // 标记为预约赎回成功
	}else if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_PATRIAL_REDEM){
		m_params.setParam("opt_type",FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM);
		m_params.setParam("total_fee",m_close_trans.Fend_plan_amt);
		m_params.setParam("next_close_state",CLOSE_FUND_STATE_REDEM_SUC); //标记为预约赎回成功
	}else{
		// 不需要发起赎回
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] user_end_type[%d] cannot be redem", m_close_trans.Fid,m_close_trans.Fuser_end_type);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);		
	}
	
	/**检查关联赎回单号**/
	string presell_listid=string(m_close_trans.Fpresell_listid);
	string default_presellId=toString(m_close_trans.Fid);//兼容旧逻辑,原来的预约赎回单号是close_id
	if(presell_listid.empty()||presell_listid==default_presellId){
	    m_RedemTradeExist = QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
									m_params.getInt("bank_type"), &m_stRecord, false);
		// 关联单号不存在,但是单号存在，不应该的情况
		if(m_RedemTradeExist){
			snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] status[%d] cannot be redem by bankbillno[%s] when op[%d]", m_close_trans.Fid,m_close_trans.Fstate,m_params.getString("cft_bank_billno").c_str(),m_optype);
		    TRACE_ERROR("%s",szErrMsg);
		    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
		}
	}else{		
		/**关联单号存在**/
		m_RedemTradeExist = QueryTradeFund(m_pFundCon, m_close_trans.Fpresell_listid, PURTYPE_REDEEM, &m_stRecord, true);
		if(!m_RedemTradeExist){
			// 记录异常,不该出现的情况:有记录关联单号,但交易记录没有
			char szErrMsg[128];
			snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] relate sellid[%s] unfound", m_close_trans.Fid,m_close_trans.Fpresell_listid);
		    TRACE_ERROR("%s",szErrMsg);
		    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
		}
	}

	// 已经赎回成功重入检查
	if(m_close_trans.Fstate==CLOSE_FUND_STATE_REDEM_SUC){
		checkAckRepeat();
	
	// 赎回请求重入检查
	}else if(m_RedemTradeExist&&m_optype==OP_TYPE_END_REDEM_REQ){
		checkReqRepeat();
	
	// 赎回确认
	}else if(m_optype==OP_TYPE_END_REDEM_ACK){
		checkAck();
	
	// 赎回请求
	}else if(m_optype==OP_TYPE_END_REDEM_REQ){
		checkReq();
		
	}else{
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] and related sell[%s] status unexcepted[%d][%d]", m_close_trans.Fid,m_stRecord.Flistid,m_close_trans.Fstate,m_stRecord.Fstate);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
	
}
void FundCloseEndRedem::checkReq() throw (CException){	
	char szErrMsg[128];
	memset(&szErrMsg,0,sizeof(szErrMsg));
	// 检查定期交易状态
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] redem req state[%d] is invalid",m_close_trans.Fid,m_close_trans.Fstate);
	
	// 检查金额一致性
	}else if(m_params.getLong("total_fee")>m_close_trans.Fcurrent_total_fee){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] redem req total_fee[%ld] larger than current[%ld]",  m_close_trans.Fid,m_params.getLong("total_fee"),m_close_trans.Fcurrent_total_fee);	
	}
	
	if(szErrMsg[0]!=0){
		TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}

	memset(&m_stRecord, 0, sizeof(ST_TRADE_FUND));
}

void FundCloseEndRedem::checkAck() throw (CException){	
	char szErrMsg[128];
	memset(&szErrMsg,0,sizeof(szErrMsg));
	// 检查赎回单是否是预约赎回单
	if(m_stRecord.Fspe_tag!=TRADE_SPETAG_BOOK_REDEM||m_stRecord.Fclose_listid!=m_close_trans.Fid){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redem ack stTradeRedem[%s] related invalid[%d][%ld]", m_stRecord.Flistid,m_stRecord.Fspe_tag,m_stRecord.Fclose_listid);
	
	// 检查赎回单用户
	}else if(strcmp(m_stRecord.Ftrade_id,m_params.getString("trade_id").c_str())!=0||strcmp(m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redem ack stTradeRedem[%s] trade_id[%s][%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Ftrade_id,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());
	// 检查定期交易状态:未赎回完成
	}else if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING){
		snprintf(szErrMsg, sizeof(szErrMsg), "stTradeRedem[%s] redem ack close_trans state[%d] is invalid", m_stRecord.Flistid,m_close_trans.Fstate);
	
	// 检查赎回单金额
	}else if(m_stRecord.Ftotal_fee!=m_params.getLong("total_fee")){
		snprintf(szErrMsg, sizeof(szErrMsg), "stTradeRedem[%s] redem ack total_fee[%ld] is not equal[%ld]", m_stRecord.Flistid,m_stRecord.Ftotal_fee,m_close_trans.Fcurrent_total_fee);
	
	// 检查金额一致性
	}else if(m_params.getLong("total_fee")>m_close_trans.Fcurrent_total_fee){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] redem ack end_plan_amt[%ld] larger than current[%ld]",  m_close_trans.Fid,m_params.getLong("total_fee"),m_close_trans.Fcurrent_total_fee);	
	
	// 检查赎回单单号一致性
	}else if(strcmp(m_stRecord.Fcft_bank_billno,m_params.getString("cft_bank_billno").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redem ack stTradeRedem[%s] cft_bank_billno[%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Fcft_bank_billno,m_params.getString("cft_bank_billno").c_str());	
	
	// 检查赎回单提现单号一致性
	}else if(strcmp(m_stRecord.Ffetchid,m_params.getString("cft_fetch_id").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redem ack stTradeRedem[%s] cft_bank_billno[%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Ffetchid,m_params.getString("cft_fetch_id").c_str());	
	}
	
	if(szErrMsg[0]!=0){
		TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
}
void FundCloseEndRedem::checkReqRepeat() throw (CException){	
	char szErrMsg[128];
	memset(&szErrMsg,0,sizeof(szErrMsg));
	// 检查赎回单是否是预约赎回单
	if(m_stRecord.Fspe_tag!=TRADE_SPETAG_BOOK_REDEM||m_stRecord.Fclose_listid!=m_close_trans.Fid){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans created but stTradeRedem[%s] related invalid[%d][%ld]", m_stRecord.Flistid,m_stRecord.Fspe_tag,m_stRecord.Fclose_listid);
	
	// 检查赎回单用户
	}else if(strcmp(m_stRecord.Ftrade_id,m_params.getString("trade_id").c_str())!=0||strcmp(m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans created but stTradeRedem[%s] trade_id[%s][%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Ftrade_id,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());
	// 检查定期交易状态
	}else if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PENDING){
		snprintf(szErrMsg, sizeof(szErrMsg), "stTradeRedem[%s] created but close_trans state[%d] is invalid", m_stRecord.Flistid,m_close_trans.Fstate);
	
	// 检查赎回单金额
	}else if(m_stRecord.Ftotal_fee!=m_params.getLong("total_fee")){
		snprintf(szErrMsg, sizeof(szErrMsg), "stTradeRedem[%s] created but total_fee[%ld] is not equal[%ld]", m_stRecord.Flistid,m_stRecord.Ftotal_fee,m_close_trans.Fcurrent_total_fee);
	
	// 检查金额一致性
	}else if(m_params.getLong("total_fee")>m_close_trans.Fcurrent_total_fee){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] created but end_plan_amt[%ld] larger than current[%ld]",  m_close_trans.Fid,m_params.getLong("total_fee"),m_close_trans.Fcurrent_total_fee);	
	}
	
	if(szErrMsg[0]!=0){
		TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
	
	/**
	* 有赎回记录
	* 记录真实单号(避免重入单号重新生成)
	*/	
	m_params.setParam("cft_bank_billno", m_stRecord.Fcft_bank_billno);
	m_params.setParam("cft_trans_id", m_stRecord.Fcft_trans_id);
	m_params.setParam("cft_fetch_id", m_stRecord.Ffetchid);
	m_params.setParam("cft_charge_ctrl_id", m_stRecord.Fcft_charge_ctrl_id);
	m_params.setParam("fund_trans_id", m_stRecord.Flistid);
	
	throw EXCEPTION(ERR_REPEAT_ENTRY,"stTradeRedem has exists");
}

void FundCloseEndRedem::checkAckRepeat() throw (CException){		
	char szErrMsg[128];
	memset(&szErrMsg,0,sizeof(szErrMsg));
	// 检查赎回单是否是预约赎回单
	if(m_stRecord.Fspe_tag!=TRADE_SPETAG_BOOK_REDEM||m_stRecord.Fclose_listid!=m_close_trans.Fid){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redeemed but stTradeRedem[%s] related invalid[%d][%ld]", m_stRecord.Flistid,m_stRecord.Fspe_tag,m_stRecord.Fclose_listid);
	
	// 检查赎回单用户
	}else if(strcmp(m_stRecord.Ftrade_id,m_params.getString("trade_id").c_str())!=0||strcmp(m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redeemed but stTradeRedem[%s] trade_id[%s][%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Ftrade_id,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());
		
	// 检查赎回单状态
	}else if(m_stRecord.Fstate!=REDEM_SUC&&m_stRecord.Fstate!=REDEM_FINISH){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redeemed but stTradeRedem[%s] status [%s][%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Ftrade_id,m_close_trans.Ftrade_id,m_params.getString("trade_id").c_str());	
	
	// 检查赎回单单号一致性
	}else if(strcmp(m_stRecord.Fcft_bank_billno,m_params.getString("cft_bank_billno").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redeemed but stTradeRedem[%s] cft_bank_billno[%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Fcft_bank_billno,m_params.getString("cft_bank_billno").c_str());	
	
	// 检查赎回单提现单号一致性
	}else if(strcmp(m_stRecord.Ffetchid,m_params.getString("cft_fetch_id").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans redeemed but stTradeRedem[%s] cft_bank_billno[%s] is not equal[%s]", m_stRecord.Flistid,m_stRecord.Ffetchid,m_params.getString("cft_fetch_id").c_str());	
	}
	
	if(szErrMsg[0]!=0){
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
	throw EXCEPTION(ERR_REPEAT_ENTRY,"stTradeRedem has redeemed");
}


/**
  * 执行申购请求
  */
void FundCloseEndRedem::excute() throw (CException)
{
    try
    {
		CheckParams();
		 
		 /* 检查基金账户绑定基金公司交易账户记录 */
		CheckFundBindSpAcc();
		
         /* 开启事务 */
        m_pFundCon->Begin();

		/** 查询交易记录*/
		CheckCloseTrans();
		 
		 /* 检查交易时间 */
		checkAccTime();

		CheckFundBalance();

		// 记录赎回单
		RecordFundTrade();
		
		// 更新子账户,更新赎回成功
		RecordRedemTradeForSuc();
		
		m_pFundCon->Commit();		

		//交易记录发MQ,组装变化的参数	
		sendFundBuy2MqMsg(m_stRecord);
		
		updateCkvs();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 更新失败子账户不回滚事务
        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
}

void FundCloseEndRedem::updateCkvs(){
	
	// 赎回确认才更新CKV
	if(m_optype!=OP_TYPE_END_REDEM_ACK){
		return;
	}

	//更新定期记录CKV
	setFundCloseTransToKV(m_close_trans.Ftrade_id,m_close_trans.Ffund_code);
	//用户份额CKV不更新,不修改排序
	
	//更新交易记录CKV
	//setTradeRecordsToKV(m_pFundCon,m_stRecord);
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

void FundCloseEndRedem::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stRecord.Fsub_trans_id);

	try
	{
		TRACE_DEBUG("[SubaccDraw][spid:%s][Fqqid:%s][client_ip:%s][Fsub_trans_id:%s][total_fee:%ld][acc_time:%s]",
				m_stRecord.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip").c_str(),
				m_stRecord.Fsub_trans_id, m_stRecord.Ftotal_fee, m_params.getString("systime").c_str());
	    SubaccDraw(gPtrSubaccRpc, m_stRecord.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
		        m_stRecord.Fsub_trans_id, m_stRecord.Ftotal_fee,m_params.getString("systime"));
	}
	
	catch(CException& e)
	{

		//赎回要谨慎补单
		//如果赎回子账户减钱超过10分钟没成功的告警不在补单
		//无论是差错补单还是外部批跑补单，10分钟没成功的异常时都会触发告警
		if(payNotifyOvertime(m_stRecord.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回补单超过10分钟子账户仍未成功");		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		
		//直接抛出异常，阻止后面继续执行，不能把赎回单做成功
		throw;
		
	}
	
}


/**
*	到基金公司赎回成功处理
*	减基金账户余额
*/
void FundCloseEndRedem::RecordRedemTradeForSuc() throw (CException)
{
	// 赎回确认才需要操作
	if(m_optype!=OP_TYPE_END_REDEM_ACK){
		return;
	}
	
	//减子账户失败回滚事务,以保证记录调用子账户单号一致
	//同时发差错, 以便补单成功
	try
	{
		TRACE_DEBUG("[记录赎回交易记录]");
		
		//减账户余额
		doDraw();
		
		/* 记录赎回交易记录 */
        updateToRedemSuccess();
		
		TRACE_DEBUG("[减总账户]");
		//减总账户，和更新单放在一个事务里，保证一个单只操作一次
		//recordFundTotalaccDraw();
		
		//减基金总份额fund_scope, 只减本金
		//放在事务的最后调用,减少锁公共记录时间
		recordSpLoaning();
	}
	catch(CException& e)
	{
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);
		
		if(ERR_TYPE_MSG)
		{
			//来自差错补单时间和赎回单创建时间超过10分钟的告警
			int inteval = (int)(time(NULL) - toUnixTime(m_stRecord.Facc_time));	
			gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
			//补单10分钟仍未成功告警,告警服务本身会做压制
			if(inteval >= 600 )
			{	
				char szErrMsg[256] = {0};
		        snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回差错补单超过%d分钟未成功" ,inteval);
		        
		        alert(ERR_BUDAN_TOLONG, szErrMsg);
			}
		}
		// 异常抛出,回滚
		throw;
		
	}

}

void FundCloseEndRedem::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	// 赎回确认才需要操作
	if(m_optype!=OP_TYPE_END_REDEM_ACK){
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

/*
*检查是否绑定基金公司帐号
*/
void FundCloseEndRedem::CheckFundBindSpAcc() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin").c_str(), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
	// 记录存在，读出记录中的trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
	
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);
	queryValidFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);

}

/**
* 查询余额通账户余额
* 查询定期基金当前金额是否够赎回
*/
void FundCloseEndRedem::CheckFundBalance() throw (CException)
{
    if ( m_optype==OP_TYPE_END_REDEM_ACK)
    {
        //确认接口不校验余额
        return;
    }
   
	LONG balance = querySubaccBalance(m_close_trans.Fuid,querySubaccCurtype(m_pFundCon, m_close_trans.Fspid));

	if(balance < m_close_trans.Fcurrent_total_fee)
	{
		throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
	}
}

/**
  * 检查记录更新时间
  */
void FundCloseEndRedem::checkAccTime() throw (CException)
{
	if(m_optype!=OP_TYPE_END_REDEM_REQ){
		return;
	}
	bool endRedemNormal=true;
	// 检查赎回单生产的时间在到期日范围内
	string dueDate=string(m_close_trans.Fdue_date);
	string dueTime=dueDate+"150000";// 设置默认值	
	string bookStopDate=string(m_close_trans.Fbook_stop_date);
	string bookStopTime=bookStopDate+"150000"; 
	
	if(m_params.getString("systime")>=changeDatetimeFormat(dueTime)||
		m_params.getString("systime")<changeDatetimeFormat(bookStopTime)){
		endRedemNormal=false;
	}
	// 检查不通过,不允许赎回
	if(gPtrConfig->m_AppCfg.check_end_redem_time==1&&!endRedemNormal){ 
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "close end redem cannot be redem right now[%s][%s][%s][%s]",dueDate.c_str(),bookStopDate.c_str(), bookStopTime.c_str(),dueTime.c_str());
		alert(ERR_CLOSE_END_REDEM_OVERTIME,szErrMsg);
        throw CException(ERR_CLOSE_END_REDEM_OVERTIME, szErrMsg, __FILE__, __LINE__);
	}
	if(!endRedemNormal&&gPtrConfig->m_AppCfg.change_end_redem_time!=""){
		string accTimeChanged = dueDate+gPtrConfig->m_AppCfg.change_end_redem_time;
		m_params.setParam("acc_time",accTimeChanged);
	}else{
		m_params.setParam("acc_time",m_params.getString("systime"));
	}
	string alertTime=dueDate+"110000"; //11点开始告警
	if(m_params.getString("systime")>=changeDatetimeFormat(alertTime)){
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "close end redem has not finished[%s][%s]", m_close_trans.Ftrade_id,m_close_trans.Fpresell_listid);
		alert(ERR_CLOSE_END_REDEM_OVERTIME,szErrMsg);
	}
}


void FundCloseEndRedem::GenerFundTrade() throw (CException)
{
	memset(&m_stRecord, 0, sizeof(ST_TRADE_FUND));
	string drawid = genSubaccDrawid();
	string cft_bank_billno = m_params.getString("cft_bank_billno");
	//赎回单号内部生成  10商户号+8位日期+10序列号+cft_bank_billno后3位，保证cft_bank_billno和listid的分库分表规则一致，不用在拆分表
	string listid =  m_params.getString("spid") + drawid + cft_bank_billno.substr(cft_bank_billno.size()-3);
	m_params.setParam("fund_trans_id", listid);
    strncpy(m_stRecord.Flistid, listid.c_str(), sizeof(m_stRecord.Flistid)-1);
    strncpy(m_stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(m_stRecord.Fspid)-1);
	// 到期赎回的sp_billno并不知道，设置为空,收到期末反馈文件的时候可以获得
    // strncpy(m_stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(m_stRecord.Fcoding)-1);
    strncpy(m_stRecord.Ftrade_id, m_close_trans.Ftrade_id, sizeof(m_stRecord.Ftrade_id)-1);
    m_stRecord.Fuid = m_close_trans.Fuid;
	
	//查询fund_name
    FundSpConfig fund_sp_config;
	memset(&fund_sp_config, 0, sizeof(FundSpConfig));
	strncpy(fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(fund_sp_config.Fspid) - 1);
	strncpy(fund_sp_config.Ffund_code, m_close_trans.Ffund_code, sizeof(fund_sp_config.Ffund_code) - 1);
	checkFundSpAndFundcode(m_pFundCon,fund_sp_config, false);
	
    strncpy(m_stRecord.Ffund_name, fund_sp_config.Ffund_name, sizeof(m_stRecord.Ffund_name)-1);
    strncpy(m_stRecord.Ffund_code, m_close_trans.Ffund_code, sizeof(m_stRecord.Ffund_code)-1);
    m_stRecord.Fpur_type = PURTYPE_REDEEM;
	m_stRecord.Ftotal_fee = m_params.getLong("total_fee");
    m_stRecord.Fbank_type = m_params.getInt("bank_type");	
    // 卡号为空strncpy(m_stRecord.Fcard_no, m_params.getString("card_no").c_str(), sizeof(m_stRecord.Fcard_no)-1);
    // m_stRecord.Fstate = REDEM_SUC; 
    m_stRecord.Fstate = REDEM_ININ;
    m_stRecord.Flstate = LSTATE_VALID;
    // 到期日 必须是交易日    
    strncpy(m_stRecord.Ftrade_date, m_close_trans.Fdue_date, sizeof(m_stRecord.Ftrade_date)-1);
    // Ffund_vdate T+1到账日期: 赎回到账日为收益到期日+1日
	string vDate = addDays(m_close_trans.Fprofit_end_date,1);
    strncpy(m_stRecord.Ffund_vdate, vDate.c_str(), sizeof(m_stRecord.Ffund_vdate)-1);
	// Ffund_value,Ffund_type,Fnotify_url,Frela_listid,Fdrawid 无需填值
    //财付通核心是否支持重入? m_stRecord.Fstandby1 = 1; // 锁定记录    
    strncpy(m_stRecord.Ffetchid, m_params.getString("cft_fetch_id").c_str(), sizeof(m_stRecord.Ffetchid)-1);
    m_stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(m_stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Fcreate_time)-1);
    strncpy(m_stRecord.Facc_time, m_params.getString("acc_time").c_str(), sizeof(m_stRecord.Facc_time)-1);
    strncpy(m_stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_stRecord.Fmodify_time)-1);
	strncpy(m_stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(m_stRecord.Fcft_trans_id)-1);
	strncpy(m_stRecord.Fcft_charge_ctrl_id, m_params.getString("cft_charge_ctrl_id").c_str(), sizeof(m_stRecord.Fcft_charge_ctrl_id)-1);
    //	T+1赎回,没有sp_fetch_id。
    //strncpy(m_stRecord.Fsp_fetch_id, m_params.getString("sp_fetch_id").c_str(), sizeof(m_stRecord.Fsp_fetch_id)-1);
	m_stRecord.Floading_type = DRAW_NOT_USE_LOADING;
	strncpy(m_stRecord.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(m_stRecord.Fcft_bank_billno)-1);
	strncpy(m_stRecord.Fsub_trans_id, drawid.c_str(), sizeof(m_stRecord.Fsub_trans_id)-1);	
    m_stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));; // 币种类型
	// charge_type 赎回不涉及
	m_stRecord.Fpurpose = (m_close_trans.Fend_sell_type==CLOSE_FUND_SELL_TYPE_BALANCE?PURPOSE_REDEM_TO_BA_T1:PURPOSE_DRAW_T1);
	m_stRecord.Fspe_tag = TRADE_SPETAG_BOOK_REDEM;

	strncpy(m_stRecord.Fchannel_id, m_close_trans.Fchannel_id, sizeof(m_stRecord.Fchannel_id)-1);
	//m_stRecord.Fsettlement=0;
	m_stRecord.Fclose_listid = m_close_trans.Fid;
	m_stRecord.Fopt_type = m_params.getInt("opt_type");
	//Freal_redem_amt当前还不知道,设置total，入账的时候再加上扫尾
	m_stRecord.Freal_redem_amt =m_params.getLong("total_fee");
	strncpy(m_stRecord.Fend_date,m_close_trans.Fend_date, sizeof(m_stRecord.Fend_date) - 1);
	
}

/**
  * 生成基金赎回记录，状态: 初始赎回状态
  */
void FundCloseEndRedem::RecordFundTrade() throw (CException)
{
	// 存在记录则不需要创建
	if(m_RedemTradeExist){
		return;
	}
	GenerFundTrade();
	
	InsertCloseBookTradeFund(m_pFundCon, &m_stRecord);
	InsertCloseBookTradeUserFund(m_pFundCon, &m_stRecord);

	//更新定期产品关联赎回单
	FundCloseTrans fundCloseTrans;
	strncpy(fundCloseTrans.Ftrade_id,m_close_trans.Ftrade_id,sizeof(fundCloseTrans.Ftrade_id)-1);
	fundCloseTrans.Fid=m_close_trans.Fid;
	strncpy(fundCloseTrans.Fpresell_listid, m_stRecord.Flistid, sizeof(fundCloseTrans.Fpresell_listid)-1);
	strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time)-1);
	updateFundCloseTransById(m_pFundCon, fundCloseTrans);
	//同步内存数据
	strncpy(m_close_trans.Fpresell_listid, fundCloseTrans.Fpresell_listid, sizeof(m_close_trans.Fpresell_listid)-1);
	strncpy(m_close_trans.Fmodify_time,fundCloseTrans.Fmodify_time, sizeof(m_close_trans.Fmodify_time)-1);
}


/**
  * 更新状态为赎回成功
  */
void FundCloseEndRedem::updateToRedemSuccess() throw (CException)
{
	m_stRecord.Fstate=REDEM_SUC;
	m_stRecord.Fspe_tag=TRADE_SPETAG_BOOK_REDEM;
	m_stRecord.Fpur_type=PURTYPE_REDEEM;
	// strncpy(m_stRecord.Facc_time, m_params.getString("acc_time").c_str(), sizeof(m_stRecord.Facc_time)-1);
	
	UpdateFundTrade(m_pFundCon, m_stRecord, m_stRecord, m_params.getString("systime"));
	
	//T+1赎回,不记录垫资帐号提现单和基金交易单对应关系
	/*
	if(!m_params.getString("sp_fetch_id").empty())
	{
		FundFetch fundFetch;
		memset(&fundFetch, 0, sizeof(FundFetch));
		
		strncpy(fundFetch.Ffetchid, m_params.getString("sp_fetch_id").c_str(), sizeof(fundFetch.Ffetchid)-1);
		strncpy(fundFetch.Ffund_trans_id, m_stRecord.Flistid, sizeof(fundFetch.Ffund_trans_id)-1);
		strncpy(fundFetch.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fcreate_time)-1);
    		strncpy(fundFetch.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fmodify_time)-1);
		

		insertFundFetch(m_pFundCon, fundFetch);
	}*/
	//更新定期产品状态
	FundCloseTrans fundCloseTrans;
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_ALL_REDEM){
		strncpy(m_close_trans.Fsell_listid,m_stRecord.Flistid, sizeof(m_close_trans.Fsell_listid)-1);
		strncpy(fundCloseTrans.Fsell_listid,m_stRecord.Flistid, sizeof(fundCloseTrans.Fsell_listid)-1);
	}
	strncpy(fundCloseTrans.Ftrade_id,m_close_trans.Ftrade_id,sizeof(fundCloseTrans.Ftrade_id)-1);
	fundCloseTrans.Fid=m_close_trans.Fid;
	fundCloseTrans.Fcurrent_total_fee = m_close_trans.Fcurrent_total_fee-m_params.getLong("total_fee");
	fundCloseTrans.Fstate = m_params.getInt("next_close_state");
	strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time)-1);
	fundCloseTrans.Fend_real_sell_amt= m_close_trans.Fend_real_sell_amt+m_params.getLong("total_fee");
	fundCloseTrans.Fend_plan_amt = m_params.getLong("total_fee");
	saveFundCloseTrans(fundCloseTrans,m_close_trans,m_stRecord.Flistid,PURTYPE_REDEEM);
	
	// 同步到内存数据
	m_close_trans.Fcurrent_total_fee = fundCloseTrans.Fcurrent_total_fee;
	m_close_trans.Fstate = fundCloseTrans.Fstate;
	strncpy(m_close_trans.Fmodify_time, fundCloseTrans.Fmodify_time, sizeof(m_close_trans.Fmodify_time)-1);
	m_close_trans.Fend_real_sell_amt= fundCloseTrans.Fend_real_sell_amt;
	m_close_trans.Fend_plan_amt = fundCloseTrans.Fend_plan_amt;
}


void FundCloseEndRedem::recordSpLoaning() throw (CException)
{
	// 配置不记录赎回额度信息
	if(gPtrConfig->m_AppCfg.close_end_change_sp_config!=1){
		return;
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
	
    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));
    strncpy(fundSpConfig.Fspid, m_stRecord.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stRecord.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
	// 停止赎回,仅告警, 期末应该允许赎回
    if ((fundSpConfig.Fredem_valid&0x07) ==2) 
    {
    	alert(ERR_REDEM_DRAW_REFUSE,"close end redem is called to stop, check sp config");
        // throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }

    //更新赎回累计额度
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedom(m_pFundCon, fundSpConfig,m_stRecord.Ftotal_fee,redemScope,m_stRecord.Floading_type,m_stRecord.Facc_time);
}

/**
  * 打包输出参数
  */
void FundCloseEndRedem::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "cft_bank_billno", m_stRecord.Fcft_bank_billno);
    CUrlAnalyze::setParam(rqst->odata, "cft_trans_id", m_stRecord.Fcft_trans_id);
    CUrlAnalyze::setParam(rqst->odata, "cft_fetch_id", m_stRecord.Ffetchid);
    CUrlAnalyze::setParam(rqst->odata, "cft_charge_ctrl_id", m_stRecord.Fcft_charge_ctrl_id);
    CUrlAnalyze::setParam(rqst->odata, "fund_trans_id",  m_stRecord.Flistid);
    CUrlAnalyze::setParam(rqst->odata, "acc_time", m_stRecord.Facc_time);
    CUrlAnalyze::setParam(rqst->odata, "fund_vdate",  m_stRecord.Ffund_vdate);
    
	
    rqst->olen = strlen(rqst->odata);
    return;
}


