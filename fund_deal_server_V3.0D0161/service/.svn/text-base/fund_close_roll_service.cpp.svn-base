/**
  * FileName: fund_close_roll_service.cpp
  * Author: jessiegao
  * Version :1.0
  * Date: 2014-7-30
  * Description: 生成定期期末赎回记录
  */

#include "fund_commfunc.h"
#include "fund_close_roll_service.h"

FundCloseRoll::FundCloseRoll(CMySQL* mysql)
{
    m_pFundCon = mysql;
	memset(&m_close_trans, 0, sizeof(FundCloseTrans));
	memset(&m_cycle,0,sizeof(FundCloseCycle));
	memset(&m_next_close_trans, 0, sizeof(FundCloseTrans));
	m_roll_fee=0;
	m_seqno=0;
	m_hasRollTrans=false;
}

/**
  * service step 1: 解析输入参数
  */
void FundCloseRoll::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	m_request = rqst;
    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_close_roll_service] receives: %s", szMsg);

    // 读取参数
	m_params.readStrParam(szMsg, "trade_id", 1, 32);
	m_params.readLongParam(szMsg,"close_id", 0, MAX_LONG);
	m_params.readStrParam(szMsg, "fund_code", 0, 32);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
    
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};
	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

/*
 * 生成基金注册用token
 */
string FundCloseRoll::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|cft_bank_billno|spid|total_fee|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["close_id"] << "|" ;
    ss << m_params["fund_code"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundCloseRoll::CheckToken() throw (CException)
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
void FundCloseRoll::CheckParams() throw (CException)
{
	CHECK_PARAM_EMPTY("trade_id");
	CHECK_PARAM_EMPTY("close_id");
	CHECK_PARAM_EMPTY("fund_code");
    // 验证token
    CheckToken();
}
/**
  * 检查参数，获取内部参数
  */
void FundCloseRoll::CheckCloseTrans() throw (CException)
{
	//获取定期记录
	m_close_trans.Fid = m_params.getLong("close_id");
	strncpy(m_close_trans.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_close_trans.Ftrade_id)-1);
	char szErrMsg[128];
	if(!queryFundCloseTrans(m_pFundCon,m_close_trans,true)){
		snprintf(szErrMsg, sizeof(szErrMsg), "unfound fund_close_trans close_id=%ld,trade_id=%s", m_close_trans.Fid,m_close_trans.Ftrade_id);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_FUND_CLOSE_TRANS_NOT_EXIST, szErrMsg);    
	}
	//检查fund_code参数
	if(strcmp(m_close_trans.Ffund_code,m_params.getString("fund_code").c_str())!=0){
		snprintf(szErrMsg, sizeof(szErrMsg), "fund_code[%s] not equeal to DB[%s]", m_params.getString("fund_code").c_str(),m_close_trans.Ffund_code);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_FUND_CLOSE_END_BAD_PARA, szErrMsg);    
	}
	// 判断重入
	if(m_close_trans.Fstate==CLOSE_FUND_STATE_SUC||m_close_trans.Fstate==CLOSE_FUND_STATE_FAIL){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%s][%ld] state[%d] has finished",m_close_trans.Ftrade_id,m_close_trans.Fid,m_close_trans.Fstate);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_REPEAT_ENTRY, szErrMsg);
	}	
	// 判断定期记录状态
	if(m_close_trans.Fstate!=CLOSE_FUND_STATE_PROFIT_END){
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans state[%d] cannot be roll", m_close_trans.Fstate);
	    TRACE_ERROR("%s",szErrMsg);
	    throw EXCEPTION(ERR_FUND_CLOSE_END_BAD_STATE, szErrMsg);
	}

	/**
	  * 判断到期赎回策略
	  **/	  
	// 全额扫尾赎回: 不需要滚动
	if(m_close_trans.Fuser_end_type==CLOSE_FUND_END_TYPE_ALL_REDEM){
		if(m_close_trans.Fcurrent_total_fee==0){
			m_roll_fee = 0;
			return;
		}else{
			snprintf(szErrMsg, sizeof(szErrMsg), "[%ld][%ld]all redem money is invalid",m_close_trans.Fid,m_close_trans.Fcurrent_total_fee);
		    TRACE_ERROR("%s",szErrMsg);
		    throw EXCEPTION(ERR_FUND_CLOSE_END_BAD_REDEM, szErrMsg);
		}
	}
	m_roll_fee=m_close_trans.Fcurrent_total_fee;
}

/**
 * 检查日期
 */
void FundCloseRoll::checkNextCloseFundTransSeq() throw(CException){
	// 以上一期的到期日作为下一期的开始日
	strncpy(m_cycle.Fdate, m_close_trans.Fdue_date, sizeof(m_cycle.Fdate) - 1);
	strncpy(m_cycle.Ffund_code, m_close_trans.Ffund_code, sizeof(m_cycle.Ffund_code) - 1);
	bool hasCycle = queryFundCloseCycle(m_pFundCon, m_cycle, false);

	//无配置日期
	if(!hasCycle){
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "close_trans[%ld] next cycle[%s] unfound", m_close_trans.Fid,m_close_trans.Fdue_date);
		alert(ERR_FUND_CLOSE_UNFOUND_CYCLE,"rolling close_trans next cycle unfound");
		TRACE_ERROR("%s",szErrMsg);
		throw EXCEPTION(ERR_FUND_CLOSE_UNFOUND_CYCLE,szErrMsg);
	}

	// 检查生成单生产的时间在滚动日范围内
	bool rollNormal = true;
	string rollDate=addDays(string(m_close_trans.Fprofit_end_date),1);
	string rollTime=rollDate+"150000";// 设置默认值	
	string profitEndDate=string(m_close_trans.Fprofit_end_date);
	string profitEndTime=profitEndDate+"150000"; 
	
	if(m_params.getString("systime")>=changeDatetimeFormat(rollTime)||
		m_params.getString("systime")<changeDatetimeFormat(profitEndTime)){
		rollNormal=false;
	}
	
	//检查当前时间,超过滚动日15点不允许滚动
	if(gPtrConfig->m_AppCfg.check_end_redem_time==1&&!rollNormal){ 
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "close trans cannot be roll right now[%s][%s]", profitEndTime.c_str(),rollTime.c_str());
		alert(ERR_CLOSE_ROLL_OVERTIME,szErrMsg);
		throw CException(ERR_CLOSE_ROLL_OVERTIME, szErrMsg, __FILE__, __LINE__);
	}
	if(!rollNormal&&gPtrConfig->m_AppCfg.change_end_redem_time!=""){
		string accTimeChanged = rollDate+gPtrConfig->m_AppCfg.change_end_redem_time;
		string accTimeChangedFormat = changeDatetimeFormat(accTimeChanged);
		m_params.setParam("acctime",accTimeChangedFormat);
	}else{
		m_params.setParam("acctime",m_params.getString("systime"));
	}
	string alertTime=rollDate+"110000"; //11点开始告警
	if(m_params.getString("systime")>=changeDatetimeFormat(alertTime)){
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "close trans roll has not finished[%s][%ld]", m_close_trans.Ftrade_id,m_close_trans.Fid);
		alert(ERR_CLOSE_END_REDEM_OVERTIME,szErrMsg);
	}

	// 查询滚动后的定期交易,确认是否有追加购买的情况
	strncpy(m_next_close_trans.Ftrade_id, m_close_trans.Ftrade_id, sizeof(m_next_close_trans.Ftrade_id) - 1);
	strncpy(m_next_close_trans.Fspid, m_close_trans.Fspid, sizeof(m_next_close_trans.Fspid) - 1);
	strncpy(m_next_close_trans.Ffund_code, m_close_trans.Ffund_code, sizeof(m_next_close_trans.Ffund_code) - 1);
	strncpy(m_next_close_trans.Fend_date, m_cycle.Fdue_date, sizeof(m_next_close_trans.Fend_date) - 1);
	if(queryFundCloseTransByEndDate(gPtrFundDB,m_next_close_trans,true)){
		m_seqno = m_next_close_trans.Fseqno;
		m_hasRollTrans = true;
		return;
	}
	
	// 查询最近的定期交易
	FundCloseTrans data;
	memset(&data,0,sizeof(FundCloseTrans));
	strncpy(data.Ftrade_id, m_close_trans.Ftrade_id, sizeof(data.Ftrade_id) - 1);
	strncpy(data.Fspid, m_close_trans.Fspid, sizeof(data.Fspid) - 1);
	strncpy(data.Ffund_code, m_close_trans.Ffund_code, sizeof(data.Ffund_code) - 1);
	if(!queryLatestFundCloseTrans(gPtrFundDB, data, true))
	{
		//没有记录, 不应该存在这种情况，至少也应该存在待滚动期次的记录
		throw CException(ERR_FUND_CLOSE_TRANS_NOT_EXIST,"last close_trans info not found when roll");
	}
	m_seqno = data.Fseqno+1;
	m_hasRollTrans = false;
}


/**
  * 执行申购请求
  */
void FundCloseRoll::excute() throw (CException)
{
    try
    {
		CheckParams();
		 		
         /* 开启事务 */
        m_pFundCon->Begin();
		 
		/** 查询交易记录*/
		CheckCloseTrans();
		
		/** 检查滚动后的期次情况 */
		checkNextCloseFundTransSeq();
		
		closeRoll();
		
		updateCkvs();

		recordSpScope(); // 最后更新,减少对配置表 影响
        /* 提交事务 */
        m_pFundCon->Commit();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
}

void FundCloseRoll::updateCkvs(){
	
	//更新定期记录CKV
	setFundCloseTransToKV(m_close_trans.Ftrade_id,m_close_trans.Ffund_code);
	
	//使用forupdate 锁用户，防止并发申购修改user_acc信息
	ST_FUND_BIND fund_bind; 
	QueryFundBindByTradeid(m_pFundCon, m_params.getString("trade_id").c_str(), &fund_bind, true);
		
	//用户份额CKV: 删除老期次,增加新期次
	FundUserAcc oldUserAcc;
	vector<FundUserAcc> userAccVec;
	memset(&oldUserAcc,0,sizeof(FundUserAcc));
	oldUserAcc.Fcloseid = m_close_trans.Fid;
	strncpy(oldUserAcc.Ftrade_id, m_close_trans.Ftrade_id, sizeof(oldUserAcc.Ftrade_id) - 1);
	strncpy(oldUserAcc.Ffund_code, m_close_trans.Ffund_code, sizeof(oldUserAcc.Ffund_code) - 1);
	int i = getUserAcc(oldUserAcc,userAccVec);
	if(i>=0){
		userAccVec.erase(userAccVec.begin()+i);
	}
	
	// 没有要滚动的金额,不操作新期次
	if(m_roll_fee<=0){
		return;
	}
	FundUserAcc nextUserAcc;
	memset(&nextUserAcc,0,sizeof(FundUserAcc));
	nextUserAcc.Fcloseid = m_next_close_trans.Fid;
	strncpy(nextUserAcc.Ftrade_id, m_next_close_trans.Ftrade_id, sizeof(nextUserAcc.Ftrade_id) - 1);
	strncpy(nextUserAcc.Ffund_code, m_next_close_trans.Ffund_code, sizeof(nextUserAcc.Ffund_code) - 1);
	strncpy(nextUserAcc.Ftime, m_next_close_trans.Facc_time, sizeof(nextUserAcc.Ftime) - 1);
	nextUserAcc.Ftotal_fee= m_next_close_trans.Fcurrent_total_fee;
	nextUserAcc.Ftype = FUND_USER_ACC_TYPE_ROLL;
	nextUserAcc.Fidx=1;

	vector<FundUserAcc>::size_type j=userAccVec.size();
	for(vector<FundUserAcc>::size_type i= 0; i < userAccVec.size(); i++)
	{
		FundUserAcc& item=userAccVec[i];
		// 删除重复期次
		if(0==strcmp(item.Ffund_code,nextUserAcc.Ffund_code)&&item.Fcloseid==nextUserAcc.Fcloseid){
			j=i;
		}
	}
	if(j<userAccVec.size()){
		userAccVec.erase(userAccVec.begin()+j);
	}
	// 增加新期次
	userAccVec.insert(userAccVec.begin(),nextUserAcc);
	// 重新排序	
	for(vector<FundUserAcc>::size_type i= 0; i < userAccVec.size(); i++)
	{
		FundUserAcc& item=userAccVec[i];
		item.Fidx=i+1;
	}
	setUserAcc(userAccVec);
}

/**
*	到基金公司赎回成功处理
*	减基金账户余额
*/
void FundCloseRoll::closeRoll() throw (CException)
{	
	// 判断滚动金额大于0才需要更新新期次
	if(m_hasRollTrans&&m_roll_fee>0){		
		// 滚入已有新期次
		FundCloseTrans nextTrans;
		strncpy(nextTrans.Ftrade_id, m_next_close_trans.Ftrade_id, sizeof(nextTrans.Ftrade_id) - 1);
		nextTrans.Fid=m_next_close_trans.Fid;
		nextTrans.Fcurrent_total_fee=m_next_close_trans.Fcurrent_total_fee+m_roll_fee;
		nextTrans.Fstart_total_fee=m_next_close_trans.Fstart_total_fee+m_roll_fee;
		nextTrans.Fpay_type = m_next_close_trans.Fpay_type | CLOSE_FUND_PAY_TYPE_EXTENSION;
		nextTrans.Flastid=m_close_trans.Fid;
		strncpy(nextTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(nextTrans.Fmodify_time) - 1);
		snprintf(nextTrans.Flastids, sizeof(nextTrans.Flastids),"%s,%ld",m_next_close_trans.Flastids,m_close_trans.Fid);
		string lastid=toString(m_close_trans.Fid);
		saveFundCloseTrans(nextTrans,m_next_close_trans,lastid.c_str(),PURTYPE_ROLL_IN);
		
		// 更新全局变量新期次金额
		m_next_close_trans.Fcurrent_total_fee=nextTrans.Fcurrent_total_fee;
		m_next_close_trans.Fstart_total_fee=nextTrans.Fstart_total_fee;
		m_next_close_trans.Fpay_type = nextTrans.Fpay_type;
		m_next_close_trans.Flastid = nextTrans.Flastid;
	}else if(m_roll_fee>0){	
		// 生成新期次数据
		strncpy(m_next_close_trans.Ftrade_id, m_close_trans.Ftrade_id, sizeof(m_next_close_trans.Ftrade_id) - 1);
		strncpy(m_next_close_trans.Ffund_code, m_close_trans.Ffund_code, sizeof(m_next_close_trans.Ffund_code) - 1);
		strncpy(m_next_close_trans.Fspid, m_close_trans.Fspid, sizeof(m_next_close_trans.Fspid) - 1);
		m_next_close_trans.Fseqno = m_seqno;
		m_next_close_trans.Fuid =  m_close_trans.Fuid;
		m_next_close_trans.Fpay_type = CLOSE_FUND_PAY_TYPE_EXTENSION;
		m_next_close_trans.Fstart_total_fee = m_roll_fee;
		m_next_close_trans.Fcurrent_total_fee = m_roll_fee;
		//默认全部顺延TODO 根据产品规则决定
		m_next_close_trans.Fuser_end_type = CLOSE_FUND_END_TYPE_ALL_EXTENSION;
		// fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); 
		// fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		// strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
		// strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		strncpy(m_next_close_trans.Fcreate_time, m_params.getString("systime").c_str(), sizeof(m_next_close_trans.Fcreate_time) - 1);
		strncpy(m_next_close_trans.Facc_time, m_params.getString("acctime").c_str(), sizeof(m_next_close_trans.Facc_time) - 1);

		
		strncpy(m_next_close_trans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_next_close_trans.Fmodify_time) - 1);
		strncpy(m_next_close_trans.Ftrans_date, m_cycle.Ftrans_date, sizeof(m_next_close_trans.Ftrans_date) - 1);
		strncpy(m_next_close_trans.Ffirst_profit_date, m_cycle.Ffirst_profit_date, sizeof(m_next_close_trans.Ffirst_profit_date) - 1);
		strncpy(m_next_close_trans.Fopen_date, m_cycle.Fopen_date, sizeof(m_next_close_trans.Fopen_date) - 1);
		strncpy(m_next_close_trans.Fbook_stop_date, m_cycle.Fbook_stop_date, sizeof(m_next_close_trans.Fbook_stop_date) - 1);
		strncpy(m_next_close_trans.Fstart_date, m_cycle.Fstart_date, sizeof(m_next_close_trans.Fstart_date) - 1);
		strncpy(m_next_close_trans.Fdue_date, m_cycle.Fdue_date, sizeof(m_next_close_trans.Fdue_date) - 1);
		strncpy(m_next_close_trans.Fprofit_end_date, m_cycle.Fprofit_end_date, sizeof(m_next_close_trans.Fprofit_end_date) - 1);
		strncpy(m_next_close_trans.Fchannel_id, m_close_trans.Fchannel_id, sizeof(m_next_close_trans.Fchannel_id) - 1);
		m_next_close_trans.Flastid = m_close_trans.Fid;

		m_next_close_trans.Fstate= CLOSE_FUND_STATE_PENDING;
		m_next_close_trans.Flstate = LSTATE_VALID;
		strncpy(m_next_close_trans.Fend_date, m_cycle.Fdue_date, sizeof(m_next_close_trans.Fend_date) - 1);
		snprintf(m_next_close_trans.Flastids, sizeof(m_next_close_trans.Flastids),"%ld",m_close_trans.Fid);

		string lastid=toString(m_close_trans.Fid);
		createFundCloseTrans(m_next_close_trans,lastid.c_str(),PURTYPE_ROLL_IN);		
	}	
	
	// 更新老期次状态
	FundCloseTrans oldCloseTrans;
	strncpy(oldCloseTrans.Ftrade_id,m_close_trans.Ftrade_id,sizeof(oldCloseTrans.Ftrade_id)-1);
	oldCloseTrans.Fid=m_close_trans.Fid;
	oldCloseTrans.Fstate = CLOSE_FUND_STATE_SUC;
	oldCloseTrans.Fcurrent_total_fee = m_close_trans.Fcurrent_total_fee-m_roll_fee;
	strncpy(oldCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(oldCloseTrans.Fmodify_time) - 1);
	string nextId=toString(m_next_close_trans.Fid);
	saveFundCloseTrans(oldCloseTrans,m_close_trans,nextId.c_str(),PURTYPE_ROLL_OUT);
	
}

void FundCloseRoll::recordSpScope() throw (CException)
{
	// 配置不记录额度信息
	if(gPtrConfig->m_AppCfg.close_end_change_sp_config!=1){
		return;
	}
	/** 计算增加的基金规模**/
	/** 只增加新期次比老期次多的本金, 即最多只能增加老期次的所有收益**/
	/** 滚动金额小于初始本金,即没有新增基金规模**/
	if(m_roll_fee <= m_close_trans.Fstart_total_fee){
		return;
	}
	LONG rollScope = m_roll_fee - m_close_trans.Fstart_total_fee;
	
    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));
    strncpy(fundSpConfig.Fspid, m_close_trans.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_close_trans.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    // 可能不更新,尽量不锁表
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, false))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
	
	addFundSpScope(fundSpConfig,m_params.getString("systime"),rollScope);
}


/**
  * 打包输出参数
  */
void FundCloseRoll::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


