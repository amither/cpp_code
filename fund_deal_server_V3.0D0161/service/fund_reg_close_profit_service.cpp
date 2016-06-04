/**
  * FileName: fund_reg_close_profit_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2014-05-27
  * Description: 基金交易服务 定期基金收入登记 源文件
  */

#include "fund_commfunc.h"
#include "fund_reg_close_profit_service.h"

FundRegCloseProfit::FundRegCloseProfit(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));    
	memset(&m_fund_profit, 0, sizeof(FundProfit));    
	memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));

	m_tail_profit = 0;
	m_fund_profit_exist = false;
	m_is_final_profit = true;
}

/**
  * service step 1: 解析输入参数
  */
void FundRegCloseProfit::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
    m_request = rqst;

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_reg_close_profit_service] receives: %s", szMsg);

    m_params.readStrParam(szMsg, "spid", 1, 15);
	m_params.readStrParam(szMsg, "fund_code", 1, 64);
    m_params.readStrParam(szMsg, "trade_id", 1, 32);
	m_params.readStrParam(szMsg, "fund_trans_id", 0, 32);
    m_params.readStrParam(szMsg, "date", 1, 20);
	m_params.readLongParam(szMsg, "money", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "stop_money", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "profit", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "end_tail_profit", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "op_type", 1, 2);
	m_params.readLongParam(szMsg, "seven_day_profit_rate", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "day_profit_rate", 0, MAX_LONG);
    m_params.readLongParam(szMsg, "valued_money", 0, MAX_LONG);//产生收益本金
    m_params.readStrParam(szMsg, "close_detail", 1, 10240); // 多条收益明细,close_start1:close_end1:total_fee1:profit1#close_start2:close_end2:total_fee2:profit2
    

    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_curtype = querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));

}

/*
 * 生成基金注册用token
 */
string FundRegCloseProfit::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照trade_id|fund_trans_id|date|money|profit|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["fund_trans_id"] << "|" ;
    ss << m_params["date"] << "|" ;
    ss << m_params["money"] << "|" ;
    ss << m_params["profit"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

	TRACE_DEBUG("fund authen token  input=%s", 
	                ss.str().c_str());

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundRegCloseProfit::CheckToken() throw (CException)
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
void FundRegCloseProfit::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

	
}

/**
  * 执行收益入账
  */
void FundRegCloseProfit::excute() throw (CException)
{
    try
    {
        CheckParams();

		/* 解析输入，并检查总分是否匹配 */
		parseCloseFundDetail();

		/* 检查基金绑定记录 */
        CheckFundBind();


        /* ckv操作对备ckv的操作延迟到mysql事物提交之后*/
        gCkvSvrOperator->beginCkvtrans(false);
        
        AddProfit();
        
        gCkvSvrOperator->commitCkvtrans();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

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
 * 查询基金账户是否存在
 */
void FundRegCloseProfit::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByTradeid(gPtrFundSlaveDB, m_params.getString("trade_id").c_str(), &m_fund_bind, false,false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }

    // 下面这个逻辑正常情况走不进去
    if (m_fund_bind.Flstate == LSTATE_INVALID)
    {
        //注销后qqid需要从注销表中获取
        ST_FUND_UNBIND unbindInfo;
        memset(&unbindInfo,0,sizeof(ST_FUND_UNBIND));
        strncpy(unbindInfo.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(unbindInfo.Ftrade_id)-1);
        if (getUnbindInfoByTradeid(m_pFundCon, unbindInfo))
        {
            strncpy(m_fund_bind.Fqqid,unbindInfo.Fqqid,sizeof(m_fund_bind.Fqqid)-1);
        }
    }
}

/*
* 解析输入，并检查总分是否匹配
*/
void FundRegCloseProfit::parseCloseFundDetail() throw (CException)
{
	//解析close_detail
	unsigned int trans_profit_item_length = 4;
	vector<string> close_detail_line = split(m_params.getString("close_detail"),"#");
	if(close_detail_line.empty())
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input close_detail error"); 
	}

	LONG input_total_fee = 0;
	LONG input_total_profit = 0;
//	m_fundCloseTransProfitMap.clear();
	m_fundCloseProfitMap.clear();
	for(vector<string>::iterator iter = close_detail_line.begin(); iter != close_detail_line.end(); ++iter)
	{
		string trans_profit = *iter;
		vector<string> trans_profit_detail = split(trans_profit, ":");
		if(trans_profit_detail.size() != trans_profit_item_length)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "input close_detail error"); 
		}

		//close_start1:close_end1:total_fee1:profit1#close_start2:close_end2:total_fee2:profit2
/**		FundCloseTransProfit fundCloseTransProfit;
		strncpy(fundCloseTransProfit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(fundCloseTransProfit.Ftrade_id) - 1);
		strncpy(fundCloseTransProfit.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundCloseTransProfit.Ffund_code) - 1);
		strncpy(fundCloseTransProfit.Fspid, m_params.getString("spid").c_str(), sizeof(fundCloseTransProfit.Fspid) - 1);
		strncpy(fundCloseTransProfit.Fstart_date, trans_profit_detail[0].c_str(), sizeof(fundCloseTransProfit.Fstart_date) - 1);
		strncpy(fundCloseTransProfit.Fend_date, trans_profit_detail[1].c_str(), sizeof(fundCloseTransProfit.Fend_date) - 1);
		fundCloseTransProfit.Ftotal_fee = atoll(trans_profit_detail[2].c_str());
		fundCloseTransProfit.Fprofit = atoll(trans_profit_detail[3].c_str());
		
		m_fundCloseTransProfitMap[trans_profit_detail[1]] = fundCloseTransProfit; //封闭结束时间为key
*/
		FundCloseProfitRecord record;
		memset(&record,0,sizeof(record));
		strncpy(record.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(record.Ftrade_id) - 1);
		strncpy(record.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(record.Ffund_code) - 1);
//		strncpy(record.Fspid, m_params.getString("spid").c_str(), sizeof(record.Fspid) - 1);
		strncpy(record.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(record.Flistid) - 1);
//		strncpy(record.Fstart_date, trans_profit_detail[0].c_str(), sizeof(record.Fstart_date) - 1);
		strncpy(record.Fday, m_params.getString("date").c_str(), sizeof(record.Fday) - 1);
		record.Fprofit_type = PROFIT_TYPE_SHARE;
		record.F1day_profit_rate=m_params.getLong("day_profit_rate");
		record.F7day_profit_rate=m_params.getLong("seven_day_profit_rate");
		strncpy(record.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(record.Flogin_ip)-1);
		strncpy(record.Fend_date, trans_profit_detail[1].c_str(), sizeof(record.Fend_date) - 1);
		record.Ftotal_fee = atoll(trans_profit_detail[2].c_str());
		record.Fprofit = atoll(trans_profit_detail[3].c_str());
		m_fundCloseProfitMap[trans_profit_detail[1]] = record;

		input_total_fee += record.Ftotal_fee;
		input_total_profit += record.Fprofit;
		
	}

	if(input_total_fee != m_params.getLong("money") || input_total_profit != m_params.getLong("profit"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input money or profit and details does not match."); 
	}
}

void FundRegCloseProfit::AddProfit() throw (CException)
{
	//账户金额核对
	if(1 == m_params.getInt("op_type"))
	{
		// 查询用户收益信息
		QueryFundProfit();
		 
		//份额核对
		CheckBalance();			

		/* 开启事务 */
		m_pFundCon->Begin();

		// 添加记录
		saveCloseProfitRecord();
			
        /* 提交事务 */
        m_pFundCon->Commit();
	}
	else if(2 == m_params.getInt("op_type"))
	{
		if(!ERR_TYPE_MSG)
		{
			/* 开启事务 */
	        m_pFundCon->Begin();

			// 查询用户收益信息
			QueryFundProfit();
			 
			// 份额检查，需要加锁定期记录，加锁避免收益流水并发写ckv的问题
			CheckBalance();

            QueryFundBindByTradeid(gPtrFundDB, m_params.getString("trade_id").c_str(), &m_fund_bind, true,false);

			// 记录分红交易单
			RecordFundProfit();	

			// 累计收益，修改昨日余额，修改对账日期
			UpdateProfitInfo();

			// 更新定期记录对应收益
			updateFundCloseProfit();
			
	        /* 提交事务 */
	        m_pFundCon->Commit();
			
		}
		else{
			// 差错数据: 调用子账户失败,查询本地扫尾收益		
			
			FundProfitRecord  profitRecord;
			memset(&profitRecord, 0, sizeof(FundProfitRecord));							
			strncpy(profitRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(profitRecord.Flistid)-1);
			strncpy(profitRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(profitRecord.Ftrade_id)-1);
			
			if(!queryFundProfitRecord(m_pFundCon, profitRecord, false))
			{
				char msg[64]={0};
				snprintf(msg, sizeof(msg), "From ErrorMsg:profit record[%s][%s] does not exist!", m_params.getString("fund_trans_id").c_str(),m_params.getString("trade_id").c_str());
				throw CException(ERR_PROFIT_NOT_EXIST, msg, __FILE__, __LINE__);
			}
			
			m_tail_profit = profitRecord.Fend_tail_fee;
		}

		//增加用户账户余额
		gPtrAppLog->debug("add user balance[%ld][%ld][%ld]",m_params.getLong("profit"),m_tail_profit,m_params.getLong("profit") - m_tail_profit);
		if(m_params.getLong("profit") - m_tail_profit > 0)
		{
			//调用子账户事物管理器增加基金账户余额
			doSave();
		}
	}
	else
	{
		throw EXCEPTION(ERR_BAD_PARAM, "op_type invalid");
	}
}


/**
* 查询用户收益信息,检查对账日期
*/
void FundRegCloseProfit::QueryFundProfit() throw(CException)
{
	strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id) - 1);
	m_fund_profit.Fcurtype = m_curtype;

	queryFundProfitAndCheck(m_fund_profit, m_fund_profit_exist, m_params["date"], m_params.getLong("profit"));
}

/**
* 份额对账
*/
void FundRegCloseProfit::CheckBalance()throw (CException)
{
	string max_trade_date=getCacheCloseMaxTradeDate(gPtrFundSlaveDB, m_params.getString("date"), m_params.getString("fund_code"));
	string min_nature_date=getCacheCloseMinNatureDate(gPtrFundSlaveDB,m_params.getString("date"),m_params.getString("fund_code"));

	m_fundCloseTransVec.clear();

	//未过收益截止日的产品都必须独立提供明细
	//不查出已经对账过的余额
	if( !queryFundCloseTransForRegProfit(m_pFundCon, m_params.getString("trade_id"),m_params.getString("fund_code") ,m_params.getString("date"), m_fundCloseTransVec, true))
	{
		TRACE_DEBUG("no close fund trans.");
		throw EXCEPTION(ERR_DIFF_BALANCE, "local balance is empty.");
	}

	//明细记录和入账提供的不一致，报异常
	if(m_fundCloseTransVec.size() != m_fundCloseProfitMap.size())
	{
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "not enough closed_end trans count[%s][%zd][%zd]",m_params.getString("trade_id").c_str(),m_fundCloseProfitMap.size(),m_fundCloseTransVec.size());
		throw EXCEPTION(ERR_BAD_PARAM, szErrMsg);
	}
	// 多期的总扫尾收益, 从到期日当天开始计算
	// m_tail_fee是不需要计入子账户的收益,可能比local_end_tail_fee多预约赎回截止日当天收益
	LONG local_end_tail_fee = 0; 
	char recon_date[16+1];
	strncpy(recon_date,m_params.getString("date").c_str(),sizeof(recon_date)-1);
	// 检查每期明细份额
	m_is_final_profit=true;
	for(vector<FundCloseTrans>::iterator iter = m_fundCloseTransVec.begin(); iter != m_fundCloseTransVec.end(); ++iter)
	{
		FundCloseTrans& itemFundCloseTrans = *iter;
		FundCloseProfitRecord& record = m_fundCloseProfitMap[itemFundCloseTrans.Fend_date];
		
		if(0 != strcmp(itemFundCloseTrans.Fend_date, record.Fend_date))
		{
			throw EXCEPTION(ERR_BASE_INFO_DIFF, "end_date diff.");
		}
		if(0 == strcmp(itemFundCloseTrans.Fprofit_recon_date, record.Fday))
		{
			char szErrMsg[128];
			snprintf(szErrMsg, sizeof(szErrMsg), "the issue have recon[%s][%s][%s]",itemFundCloseTrans.Ftrade_id,record.Fend_date, record.Fday);
			throw EXCEPTION(ERR_HAVE_RECON, szErrMsg);
		}
		record.Fclose_id=itemFundCloseTrans.Fid;
		strncpy(record.Fprofit_end_date, itemFundCloseTrans.Fprofit_end_date, sizeof(record.Fprofit_end_date) - 1);
		
		LONG local_total_fee = itemFundCloseTrans.Fcurrent_total_fee;
		LONG sp_total_fee = record.Ftotal_fee-record.Fprofit;
		
		// 客服强制赎回应该只在15点前发起，余额与流水一致
		// 检查客服强赎应该0 收益0赎回情况
		if(itemFundCloseTrans.Fuser_end_type==CLOSE_FUND_END_TYPE_ALL_KEFU&&itemFundCloseTrans.Fstate==CLOSE_FUND_STATE_SUC){
			local_total_fee=0;
			if(itemFundCloseTrans.Fcurrent_total_fee!=0||record.Fprofit!=0||record.Ftotal_fee!=0){
				char szErrMsg[128];
				snprintf(szErrMsg, sizeof(szErrMsg), "force redem balance, unexcept status[%s][%s][%ld][%ld]",itemFundCloseTrans.Ftrade_id,record.Fend_date, local_total_fee,record.Ftotal_fee - record.Fprofit);
				throw EXCEPTION(ERR_BASE_INFO_DIFF, szErrMsg);
			}
		}
		// 对于当前仍然处于交易期间的期次，根据申购和收益流水来对账份额
		else if(strcmp(min_nature_date.c_str(),itemFundCloseTrans.Ftrans_date)<=0
			&&strcmp(max_trade_date.c_str(),itemFundCloseTrans.Ftrans_date)>=0){
			// 获取昨天15点至今天15点前的交易记录
			local_total_fee = calBalanceFeeInBuy(itemFundCloseTrans);
		}
		// 处于预约赎回截止日的期次,不可以发生交易,但15:00后可以发起赎回
		// 1,根据昨日收益流水来对账份额
		// 2,对于已发起实时扫尾赎回期次,收益不计入子账户
		else if(strcmp(recon_date,itemFundCloseTrans.Fbook_stop_date)>=0
			 && strcmp(recon_date,itemFundCloseTrans.Fend_date)<0){
			// 获取昨天的对账份额
		    // 兼容上一期滚动日等于预约赎回截止日的情况
			local_total_fee=statFundCloseReconBalance(gPtrFundSlaveDB,addDays(m_params.getString("date"),-1),
			itemFundCloseTrans.Ftrade_id,itemFundCloseTrans.Fid,itemFundCloseTrans.Flastid);
			
			// 扫尾赎回已记录完成, 收益记录扫尾不入账
			// 商户余额以已入账收益比对
			if(!string(itemFundCloseTrans.Fsell_listid).empty()){
				m_tail_profit += record.Fprofit; // 收益不增加子账户
				record.Fend_tail_profit = record.Fprofit; //该期的扫尾收益以实际金额记录
			}
		}
		//如果是到期日之后,收益截至日之前,全部赎回的收益为扫尾收益
		//商户余额也以未入账收益比对,应该加上扫尾收益
		else if(strcmp(recon_date,itemFundCloseTrans.Fend_date)>=0
			&& strcmp(recon_date,itemFundCloseTrans.Fprofit_end_date)<=0
			&&!string(itemFundCloseTrans.Fsell_listid).empty()){
			m_tail_profit += record.Fprofit; // 收益不增加子账户
			record.Fend_tail_profit = record.Fprofit; //该期的扫尾收益以实际金额记录
			sp_total_fee += record.Fprofit; // 商户比对份额不应该扣除已确认扫尾收益
			local_end_tail_fee += record.Fprofit; // 需要比对的扫尾收益应该包含已确认部分
		}
		// 扫尾收益不进行比对
		if(sp_total_fee!= local_total_fee)
		{
			char szErrMsg[128];
			snprintf(szErrMsg, sizeof(szErrMsg), "balance diff[%s][%s][%ld][%ld]",itemFundCloseTrans.Ftrade_id,record.Fend_date, local_total_fee,sp_total_fee);
			throw EXCEPTION(ERR_BASE_INFO_DIFF, szErrMsg);
		}
		// 收益截止日判断最后收益
		// 只要有一个期次不在收益截止日,则认为不是最后的收益
 		if(strcmp(recon_date,itemFundCloseTrans.Fprofit_end_date)!= 0 ||record.Ftotal_fee>0){
			m_is_final_profit=false;
		}
	}
	// 比较提供的当日扫尾收益与实际的扫尾收益
	if(m_params.getLong("end_tail_profit") != local_end_tail_fee)
	{
		char szErrMsg[128];
		snprintf(szErrMsg, sizeof(szErrMsg), "end_tail_profit diff[%s][%ld][%ld]",m_params.getString("trade_id").c_str(),m_params.getLong("end_tail_profit"),local_end_tail_fee);
		throw EXCEPTION(ERR_BASE_INFO_DIFF, szErrMsg);
	}
	
}


/**
  * 生成基金分红记录
  */
void FundRegCloseProfit::RecordFundProfit()
{
	if(m_params.getLong("profit") <= 0 && m_params.getLong("tplus_redem_money")<=0)
	{
		return;//无收益不增加记录
	}
	FundProfitRecord  stRecord;
    memset(&stRecord, 0, sizeof(FundProfitRecord));
                    
    strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid)-1);
	strncpy(stRecord.Fsub_trans_id, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Fsub_trans_id)-1);
	strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
	stRecord.Fcurtype= m_curtype; // 币种类型
    strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid)-1);
	stRecord.Fvalid_money =m_params.getLong("valued_money");
	stRecord.Fstop_money = m_params.getLong("stop_money");
	stRecord.Ftotal_profit =((m_fund_profit_exist) ? m_fund_profit.Ftotal_profit : 0) + m_params.getLong("profit");
	stRecord.Fprofit = m_params.getLong("profit") ;
	strncpy(stRecord.Fday, m_params.getString("date").c_str(), sizeof(stRecord.Fday)-1);
    stRecord.Fprofit_type = PROFIT_TYPE_SHARE;
	stRecord.F1day_profit_rate= m_params.getLong("day_profit_rate");
	stRecord.F7day_profit_rate= m_params.getLong("seven_day_profit_rate");
	strncpy(stRecord.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(stRecord.Flogin_ip)-1);
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Ftplus_redem_money = m_params.getLong("tplus_redem_money");
    stRecord.Frecon_balance = m_params.getLong("money");
    stRecord.Fstandby1 = ((m_params.getLong("profit") && m_params.getLong("tplus_redem_money")>0)?1:0);
	stRecord.Fend_tail_fee = m_tail_profit;
    insertFundProfitRecord(m_pFundCon, stRecord);

    if (1 == gPtrConfig->m_AppCfg.update_profit_ckv_switch) {
        //将查数据最新21条收益写入ckv改为从ckv查询现有数据,直接将数据追加到ckv
        //如果cvk中已有21条则淘汰Fday最老的一条
    	bool ret = addFundProfitRecordToCache(stRecord);
        if (true != ret) {
            TRACE_ERROR("call addFundProfitRecordToCache fail. retry to call setFundProfitRecordToKV");
            
            setFundProfitRecordToKV(m_pFundCon,stRecord);            
        }
    } else {
        setFundProfitRecordToKV(m_pFundCon,stRecord);        
    }	
}

/**
* 更新账户收益记录
*/
void FundRegCloseProfit::UpdateProfitInfo()
{
    strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id)-1);
	m_fund_profit.Fcurtype = m_curtype;
	strncpy(m_fund_profit.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_profit.Fspid)-1);
	m_fund_profit.Frecon_balance = m_params.getLong("money");
	strncpy(m_fund_profit.Frecon_day, m_params.getString("date").c_str(), sizeof(m_fund_profit.Frecon_day)-1);
	m_fund_profit.Fprofit = m_params.getLong("profit");
	m_fund_profit.Freward_profit = 0;//昨日赠送收益要置为0
	m_fund_profit.Ftotal_profit =((m_fund_profit_exist) ? m_fund_profit.Ftotal_profit : 0) + m_params.getLong("profit");
    m_fund_profit.Fvalid_money= m_params.getLong("valued_money");
	m_fund_profit.Ftplus_redem_money = m_params.getLong("valued_money")+m_params.getLong("stop_money")-m_params.getLong("money");
	strncpy(m_fund_profit.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(m_fund_profit.Flogin_ip)-1);
	
    strncpy(m_fund_profit.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_fund_profit.Fmodify_time)-1);
	m_fund_profit.Fstandby1=m_is_final_profit?1:0;
	//TODO Fsign

	if(m_fund_profit_exist)
	{
		updateFundProfit(m_pFundCon, m_fund_profit);
	}
	else
	{
		strncpy(m_fund_profit.Fcreate_time, m_params.getString("systime").c_str(), sizeof(m_fund_profit.Fcreate_time)-1);
		insertFundProfit(m_pFundCon, m_fund_profit);
	}

	//更新成功后写cache,写cache失败不能抛出任何异常
	updateCache(m_fund_profit);
}

void FundRegCloseProfit::updateCache(FundProfit& fund_profit)
{
	try
	{
        if (1 == gPtrConfig->m_AppCfg.update_profit_ckv_switch) {
            bool ret = addTotalProfit2Cache(fund_profit, m_fund_bind.Fuid, DEF_USR_TOTAL_PROFIT_CKV_TIMEOUT);
            //使用新方式更新CKV失败时，按原有查询DB数据更新CKV方式重试一次
            if (true != ret) {
                TRACE_ERROR("call addTotalProfit2Cache fail. retry to call setTotalProfit");
                
                vector<FundProfit> fundProfitVec;
                setTotalProfit(fund_profit,m_fund_bind.Fuid, DEF_USR_TOTAL_PROFIT_CKV_TIMEOUT, fundProfitVec);               
            }
        } else {
            vector<FundProfit> fundProfitVec;
            //实时数据更新，可以设置超时时间长点
            setTotalProfit(fund_profit,m_fund_bind.Fuid, DEF_USR_TOTAL_PROFIT_CKV_TIMEOUT, fundProfitVec);
        }
	}
	catch(...)
	{
		TRACE_ERROR("addTotalProfit2CKV to kv cache error.");
	}
	
}

/* 期末赎回反馈文件入账时不更新,扫尾收益在此处需要更新*/
void FundRegCloseProfit::addEndProfitToTrans(const char* tailRedemId, LONG tailProfit)
{
	if(tailProfit <=0)
	{
		return;
	}
	string listId=string(tailRedemId);
	if(listId.empty())
	{
		return;
	}
	// 没有购买记录，报错
	ST_TRADE_FUND stTradeBuy;
	memset(&stTradeBuy, 0, sizeof(ST_TRADE_FUND));
	if(!QueryTradeFund(m_pFundCon, tailRedemId, PURTYPE_REDEEM, &stTradeBuy, true))
	{
		gPtrAppLog->error("redem record not exist, listid[%s]  ", tailRedemId);
		throw CException(ERR_BUYPAY_NOLIST, "redem record not exist! ", __FILE__, __LINE__);
	}
	
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	strncpy(stRecord.Flistid, stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = PURTYPE_REDEEM;
	stRecord.Fspe_tag = stTradeBuy.Fspe_tag;
	stRecord.Freal_redem_amt = stTradeBuy.Freal_redem_amt + tailProfit;
	stRecord.Fstate= stTradeBuy.Fstate; // 记录状态,否则不更新CKV
    stRecord.Fuid = m_fund_bind.Fuid;    
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_fund_bind.Ftrade_id);

    UpdateFundTrade(m_pFundCon, stRecord, stTradeBuy, m_params.getString("systime"));
	
}


void FundRegCloseProfit::updateFundCloseProfit()
{
    vector<FundCloseTrans> fundCloseTransVec;
	for(vector<FundCloseTrans>::iterator iter = m_fundCloseTransVec.begin(); iter != m_fundCloseTransVec.end(); ++iter)
	{
		FundCloseTrans& itemFundCloseTrans = *iter;
		FundCloseProfitRecord& record = m_fundCloseProfitMap[itemFundCloseTrans.Fend_date];

		// 已到最终状态，不记录交易表
		if(itemFundCloseTrans.Fstate==CLOSE_FUND_STATE_SUC||itemFundCloseTrans.Fstate==CLOSE_FUND_STATE_FAIL)
		{
			continue;
		}
		FundCloseTrans fundCloseTrans;
		strncpy(fundCloseTrans.Ftrade_id,itemFundCloseTrans.Ftrade_id,sizeof(fundCloseTrans.Ftrade_id)-1);
		fundCloseTrans.Fid=itemFundCloseTrans.Fid;
		fundCloseTrans.Fcurrent_total_fee = itemFundCloseTrans.Fcurrent_total_fee + record.Fprofit-record.Fend_tail_profit;
		fundCloseTrans.Flast_profit= record.Fprofit;
		fundCloseTrans.Ftotal_profit = itemFundCloseTrans.Ftotal_profit+record.Fprofit;
		strncpy(fundCloseTrans.Fprofit_recon_date,record.Fday,sizeof(fundCloseTrans.Fprofit_recon_date)-1);
		// record.Fday>=itemFundCloseTrans.Fbook_stop_date, 进入扫尾期
		if(0<=strcmp(record.Fday,itemFundCloseTrans.Fbook_stop_date )){
			fundCloseTrans.Fend_tail_fee = itemFundCloseTrans.Fend_tail_fee + record.Fend_tail_profit;
			fundCloseTrans.Fend_real_sell_amt = itemFundCloseTrans.Fend_real_sell_amt+record.Fend_tail_profit;
		}
		// 收益到期日
		if(0==strcmp(record.Fday,itemFundCloseTrans.Fprofit_end_date)){
			if(fundCloseTrans.Fcurrent_total_fee>0){ // 金额未清完,需要滚动
				fundCloseTrans.Fstate = CLOSE_FUND_STATE_PROFIT_END;
				fundCloseTrans.Fend_real_buy_amt = itemFundCloseTrans.Fcurrent_total_fee;
			}else{ // 金额已经清理完,不需要滚动
				fundCloseTrans.Fstate = CLOSE_FUND_STATE_SUC;
			}
			// 全赎:将扫尾收益记录到赎回单中
			if(strcmp(itemFundCloseTrans.Fsell_listid,"")!=0&&fundCloseTrans.Fend_tail_fee>0){
				addEndProfitToTrans(itemFundCloseTrans.Fsell_listid,fundCloseTrans.Fend_tail_fee);
			}
		}
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);
		// 更新DB数据
		saveFundCloseTrans(fundCloseTrans,itemFundCloseTrans,m_params.getString("fund_trans_id").c_str(),PURTYPE_SHARE);

		// 更新内存数据
		itemFundCloseTrans.Fcurrent_total_fee = fundCloseTrans.Fcurrent_total_fee;
		itemFundCloseTrans.Flast_profit=fundCloseTrans.Flast_profit;
		itemFundCloseTrans.Ftotal_profit = fundCloseTrans.Ftotal_profit;
		strncpy(itemFundCloseTrans.Fprofit_recon_date,fundCloseTrans.Fprofit_recon_date,sizeof(itemFundCloseTrans.Fprofit_recon_date)-1);
		if(fundCloseTrans.Fend_tail_fee!=MIN_INTEGER){
			itemFundCloseTrans.Fend_tail_fee = fundCloseTrans.Fend_tail_fee;
			itemFundCloseTrans.Fend_real_sell_amt = fundCloseTrans.Fend_real_sell_amt;
		}
		if(fundCloseTrans.Fend_real_buy_amt!=MIN_INTEGER){
			itemFundCloseTrans.Fend_real_buy_amt = fundCloseTrans.Fend_real_buy_amt;
		}
		if(fundCloseTrans.Fstate!=MIN_INTEGER){
			itemFundCloseTrans.Fstate = fundCloseTrans.Fstate;
		}
		strncpy(itemFundCloseTrans.Fmodify_time, fundCloseTrans.Fmodify_time, sizeof(itemFundCloseTrans.Fmodify_time) - 1);		
        fundCloseTransVec.push_back(itemFundCloseTrans);
		
	}
	
	//集中更新ckv
	addFundCloseTransToKV(fundCloseTransVec);

}



void FundRegCloseProfit::CheckProfitRecord() throw (CException)
{
	FundProfitRecord  stRecord;
    memset(&stRecord, 0, sizeof(FundProfitRecord));
                    
    strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid)-1);
	strncpy(stRecord.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(stRecord.Ftrade_id)-1);

	if(!queryFundProfitRecord(m_pFundCon, stRecord, false))
	{
		throw CException(ERR_PROFIT_NOT_EXIST, "profit record not exist! ", __FILE__, __LINE__);
	}

	// 得到账户相关信息
	m_params.setParam("create_time", stRecord.Fcreate_time);//用于补单控制

}

/**
* inteval 超时间隔,单位为秒
*/
bool FundRegCloseProfit::payNotifyOvertime(string pay_suc_time, int inteval)
{
	if(pay_suc_time.size() == 14)
	{
		//YYYYMMDDHHMMSS 转YYYY-MM-DD HH:MM:SS
		pay_suc_time = changeDatetimeFormat(pay_suc_time);
	}
	int pay_time = toUnixTime(pay_suc_time.c_str());
	if(pay_time + inteval < (int)(time(NULL)) )
	{
		return true;	
	}

	return false;
}

LONG FundRegCloseProfit::calBalanceFeeInBuy(const FundCloseTrans &closeTrans)
{
	// 获取昨天15点至今天15点前的交易记录
	ST_TRADE_FUND trade_fund;			
	memset(&trade_fund, 0, sizeof(trade_fund));
	strncpy(trade_fund.Ftrade_id, closeTrans.Ftrade_id, sizeof(trade_fund.Ftrade_id)-1);
	strncpy(trade_fund.Fspid, closeTrans.Fspid, sizeof(trade_fund.Fspid)-1);
	strncpy(trade_fund.Ffund_code, closeTrans.Ffund_code, sizeof(trade_fund.Ffund_code) - 1);
	trade_fund.Fuid = m_fund_bind.Fuid;
	trade_fund.Fcur_type = m_curtype;
	trade_fund.Fclose_listid = closeTrans.Fid;
	statCloseBuyTransFee(gPtrFundSlaveDB, trade_fund,m_params.getString("date"),m_params.getString("date"));
	// 获取昨天的对账份额
    // 兼容滚动期次处理,应查询出上个期次的份额
	LONG lastBalance=statFundCloseReconBalance(gPtrFundSlaveDB,addDays(m_params.getString("date"),-1),closeTrans.Ftrade_id,closeTrans.Fid,closeTrans.Flastid);
	return trade_fund.Ftotal_fee+lastBalance;
}

LONG FundRegCloseProfit::calBalanceFeeLastProfit(const FundCloseTrans &closeTrans)
{	
	// 获取昨天的对账份额
	FundCloseProfitRecord lastProfit;
	memset(&lastProfit, 0, sizeof(lastProfit));
	strncpy(lastProfit.Fday,addDays(m_params.getString("date"),-1).c_str(),sizeof(lastProfit.Fday)-1);
	strncpy(lastProfit.Ftrade_id,closeTrans.Ftrade_id,sizeof(lastProfit.Ftrade_id)-1);
	lastProfit.Fclose_id = closeTrans.Fid;
	queryFundCloseProfitRecord(gPtrFundSlaveDB,lastProfit,false);
	return lastProfit.Ftotal_fee;	
}



/**
 * 核心充值
 */
void FundRegCloseProfit::doSave() throw(CException)
{
	gPtrAppLog->normal("doSave, listid[%s]  ", m_params.getString("fund_trans_id").c_str());

	try
	{
		//收益入账，子账户不能报错，报错直接抛出，有入账程序决定如何处理
		//补单用系统当前时间传给子账户，否则子账户按时间核对余额的会有问题
		int iResult = SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("fund_trans_id"),m_params.getLong("profit") - m_tail_profit,"基金收益", m_params.getString("systime"), 2);

		if(0 != iResult)
		{
			throw CException(iResult, "doSave subacc exception.", __FILE__, __LINE__);
		}
	}
	catch(CException& e)
	{
		//子账户不应该失败，所有加钱失败的都发给差错
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		
		if(ERR_TYPE_MSG)
		{
			//差错补单
			
			//查询收益单记录，如果收益记录时间超过2分钟的不再补单，避免雪崩
			CheckProfitRecord();

			if(payNotifyOvertime(m_params.getString("create_time"), 2 * 60))	
			{	
				char szErrMsg[256] = {0};
	        	snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.收益入账超过2分钟子账户仍未补单成功");        
	        	alert(ERR_BUDAN_TOLONG, szErrMsg);
				return; //补单一定时间未成功的不再补单，避免死循环或发生雪崩，由对账程序进行告警
			}
			
		}

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);

		//throw ; //把错误抛给批跑进行统计，差错补单抛错不受影响
	
	}
}


/**
  * 打包输出参数
  */
void FundRegCloseProfit::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
  
    rqst->olen = strlen(rqst->odata);
    return;
}

/**
* 保存定期收益记录
*/
void FundRegCloseProfit::saveCloseProfitRecord() throw(CException)
{
    for(map<string,FundCloseProfitRecord>::iterator it = m_fundCloseProfitMap.begin(); it != m_fundCloseProfitMap.end();it++ )
    {
	    FundCloseProfitRecord& record = it->second;
	    strncpy(record.Fcreate_time, m_params.getString("systime").c_str(), sizeof(record.Fcreate_time)-1);
	    strncpy(record.Fmodify_time, m_params.getString("systime").c_str(), sizeof(record.Fmodify_time)-1);
		insertFundCloseProfitRecord(m_pFundCon,record);

		/** 改成批跑统一同步用户CKV

		// 客服强赎终止,删除有份额期次列表,判断金额都为0
		// 到期日全部赎回,删除有份额期次列表
		if(record.Ftotal_fee==0&&(record.Fprofit==0||strcmp(record.Fprofit_end_date,m_params.getString("date").c_str())==0)){
			FundUserAcc userAcc;
			memset(&userAcc, 0, sizeof(FundUserAcc));
			userAcc.Fcloseid = record.Fclose_id;
			strncpy(userAcc.Ffund_code,record.Ffund_code,sizeof(userAcc.Ffund_code)-1);
			strncpy(userAcc.Ftrade_id,record.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
			userAcc.Ftype = FUND_USER_ACC_TYPE_PROFIT;
			// 更新useracc锁用户表防止并发
			ST_FUND_BIND pstRecord;
			memset(&pstRecord,0,sizeof(ST_FUND_BIND));
			strncpy(pstRecord.Ftrade_id,record.Ftrade_id,sizeof(pstRecord.Ftrade_id)-1);
			QueryFundBindByTradeid(m_pFundCon,record.Ftrade_id,&pstRecord,true);
			removeUserAcc(userAcc);
		} **/
    }
}


