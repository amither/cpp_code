/**
  * FileName: fund_reg_daily_profit_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-29
  * Description: 基金交易服务 基金收入登记 源文件
  */

#include "fund_commfunc.h"
#include "fund_reg_daily_profit_service.h"
#include <cstdlib>

FundRegProfit::FundRegProfit(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));    
	memset(&m_fund_profit, 0, sizeof(FundProfit));    
	memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));

	m_fund_profit_exist = false;

}

/**
  * service step 1: 解析输入参数
  */
void FundRegProfit::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
    m_request = rqst;

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_reg_daily_profit_service] receives: %s", szMsg);

    m_params.readStrParam(szMsg, "spid", 1, 15);
    m_params.readStrParam(szMsg, "trade_id", 1, 32);
	m_params.readStrParam(szMsg, "fund_trans_id", 0, 32);
    m_params.readStrParam(szMsg, "date", 1, 20);
	m_params.readLongParam(szMsg, "money", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "stop_money", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "profit", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "op_type", 1, 2);
	m_params.readLongParam(szMsg, "seven_day_profit_rate", 0, MAX_LONG);
	m_params.readLongParam(szMsg, "day_profit_rate", 0, MAX_LONG);
    m_params.readLongParam(szMsg, "tplus_redem_money", 0, MAX_LONG);//T+1 赎回份额
    m_params.readLongParam(szMsg, "valued_money", 0, MAX_LONG);//产生收益本金
    m_params.readStrParam(szMsg, "close_detail", 0, 1024); 

    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 0, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_curtype = querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));

}

/*
 * 生成基金注册用token
 */
string FundRegProfit::GenFundToken()
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
void FundRegProfit::CheckToken() throw (CException)
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
void FundRegProfit::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

	if(!m_params.getString("close_detail").empty())
	{
		//应测试要求，根据参数限制定期产品使用非定期产品进行入账，避免前置机调用错接口
		throw EXCEPTION(ERR_BAD_PARAM, "can't reg close_fund's profit."); 
	}

}

/**
  * 执行收益入账
  */
void FundRegProfit::excute() throw (CException)
{
    try
    {
        CheckParams();

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
void FundRegProfit::CheckFundBind() throw (CException)
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

void FundRegProfit::AddProfit() throw (CException)
{

    m_isWriteLogTimeCost = false;
    srand((unsigned)time(0));
    int randValue = rand()%500000;
    //每入账5000条 打印1条日志
    if(randValue<100)
        m_isWriteLogTimeCost = true;

	//账户金额核对

   
	gettimeofday(&m_tStart,NULL);

	if(1 == m_params.getInt("op_type"))
	{
		// 查询用户收益信息
        m_isThroughPrecheck = false;
		QueryFundProfit(m_isThroughPrecheck);
        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=1 QueryFundProfit timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }

        m_tStart = m_tEnd;
		// 统计对账日所有的申购赎回金额,并判断金额是否一致
		CheckBalance(true);

        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=1 CheckBalance timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }

		/* 开启事务 */
		m_pFundCon->Begin();

        m_tStart = m_tEnd;
		// 更新CKV用户基金列表信息
		updateUserAccCKV();

        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=1 updateUserAccCKV timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }
		
        // 更新此次校验的总金额和总收益到t_fund_profit_x
        m_tStart = m_tEnd;
        UpdatePrecheckInfo();

        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=1 UpdatePrecheckInfo timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }
        
        m_tStart = m_tEnd;
        /* 提交事务 */
        m_pFundCon->Commit();
        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=1 m_pFundCon->Commit() timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }
	}
	else if(2 == m_params.getInt("op_type"))
	{
		if(!ERR_TYPE_MSG)
		{
			/* 开启事务 */
	        m_pFundCon->Begin();

			// 查询用户收益信息
            m_isThroughPrecheck = false;
			QueryFundProfit(m_isThroughPrecheck);
			if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=2 QueryFundProfit timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }

			// 统计对账日所有的申购赎回金额,并判断金额是否一致
            m_tStart = m_tEnd;
			CheckBalance();
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=2 CheckBalance timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
            
            //锁住用户，防止并发写入收益流水到CKV
            QueryFundBindByTradeid(gPtrFundDB, m_params.getString("trade_id").c_str(), &m_fund_bind, true,false);

			// 记录分红交易单
            m_tStart = m_tEnd;
			RecordFundProfit();	
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=2 RecordFundProfit timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
			// 累计收益，修改昨日余额，修改对账日期
            m_tStart = m_tEnd;
			UpdateProfitInfo();
			if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=2 UpdateProfitInfo timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
	        /* 提交事务 */
            m_tStart = m_tEnd;
	        m_pFundCon->Commit();
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=2 m_pFundCon->Commit() timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
			
		}

		//增加用户账户余额
		if(m_params.getLong("profit") > 0)
		{
			//调用子账户事物管理器增加基金账户余额
            m_tStart = m_tEnd;
			doSave();
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_daily_profit_service] op_type=2 doSave() timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
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
void FundRegProfit::QueryFundProfit(bool & isThroughPrecheck) throw(CException)
{
	strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id) - 1);
	m_fund_profit.Fcurtype = m_curtype;

	isThroughPrecheck = queryFundProfitAndCheck(m_fund_profit, m_fund_profit_exist, m_params["date"], m_params.getLong("profit"));
}

/**
* 统计对账日所有的申购赎回(包括赠送申购)金额,并判断金额是否一致
*/
void FundRegProfit::CheckBalance(bool precheck)throw (CException)
{
    if(precheck == false)
    {
        if(m_isThroughPrecheck)
        {
            if(m_params.getLong("money") == m_fund_profit.Fprecheck_money && m_params.getLong("profit")== m_fund_profit.Fprecheck_profit)
            {
                return;
            }
            else
            {
                TRACE_ERROR("precheck balance is different! trade_id[%s], date[%s], precheck balance in db[%lld], precheck profit in db[%lld], money input[%lld],  profit input[%lld]", 
			    m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(), m_fund_profit.Fprecheck_money, m_fund_profit.Fprecheck_profit, m_params.getLong("money"), m_params.getLong("profit"));
		        throw CException(ERR_DIFF_BALANCE, "the precheck  balance is different!", __FILE__, __LINE__);
            }
        }
    }
	LONG purchase_total_fee =0;
	//LONG reward_total_fee =0;//赠送申购
	LONG redem_total_fee = 0;
	ST_TRADE_FUND trade_fund;
	
	strncpy(trade_fund.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(trade_fund.Ftrade_id)-1);
	strncpy(trade_fund.Ftrade_date, m_params.getString("date").c_str(), sizeof(trade_fund.Ftrade_date)-1);
	trade_fund.Fuid = m_fund_bind.Fuid;
	//trade_fund.Fpur_type = PURTYPE_PURCHASE;
	string purTypeConf = "(Fpur_type=" + toString(PURTYPE_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_PROFIT) 
		+ " or Fpur_type=" + toString(PURTYPE_TRANSFER_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_SHARE) + ")";
	trade_fund.Fcur_type = m_curtype;
	StatTransFee(gPtrFundSlaveDB, trade_fund, purchase_total_fee,purTypeConf);


	//trade_fund.Fpur_type = PURTYPE_REDEEM;
	purTypeConf = "(Fpur_type=" + toString(PURTYPE_REDEEM) + " or Fpur_type=" + toString(PURTYPE_TRANSFER_REDEEM) +  ")";
	StatTransFee(gPtrFundSlaveDB, trade_fund, redem_total_fee,purTypeConf);

	LONG local_total_fee = ((m_fund_profit_exist) ? m_fund_profit.Frecon_balance : 0) + purchase_total_fee  - redem_total_fee;
	
	if(m_params.getLong("money") -m_params.getLong("profit") != local_total_fee)
	{
		TRACE_ERROR("balance is different! trade_id[%s], date[%s], balance in db[%lld], money input[%lld],  profit input[%lld]", 
			m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(), local_total_fee, m_params.getLong("money"), m_params.getLong("profit"));
		throw CException(ERR_DIFF_BALANCE, "the balance is different!", __FILE__, __LINE__);
	}

    if (precheck == true)
    {
        //校验T+1赎回金额
        purTypeConf = "(Fpur_type=" + toString(PURTYPE_REDEEM) + ") AND Floading_type=0 ";
        LONG tplus_redem_total_fee = 0;
        StatTransFee(gPtrFundSlaveDB, trade_fund, tplus_redem_total_fee,purTypeConf);
        if (tplus_redem_total_fee != m_params.getLong("tplus_redem_money"))
        {
            TRACE_ERROR("tplus_redem_money is different! trade_id[%s], date[%s], tplus_redem_money in db[%lld], tplus_redem_money input[%lld] ", 
    			m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(), tplus_redem_total_fee, m_params.getLong("tplus_redem_money"));

          
                throw CException(ERR_DIFF_BALANCE, "the tplus_redem_money is different!", __FILE__, __LINE__);   
                      
        }

        //校验产生收益份额
        //产生收益份额+不产生收益份儿=总份额+总T+1赎回份额
        LONG stathistoryTplusRedem = StatTplusRedemFee(gPtrFundSlaveDB,m_params["date"],m_params["trade_id"],m_curtype); 
        
        if (m_params.getLong("valued_money")+m_params.getLong("stop_money")
             != m_params.getLong("money")+stathistoryTplusRedem+m_params.getLong("tplus_redem_money"))
        {
            TRACE_ERROR("check valued_money fail! trade_id[%s], date[%s], input:valued_money=%s,stop_money=%s,profit=%s,money=%s,tplus_redem_money=%s,history_tplus_redem_money=%lld ", 
    			m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
    			m_params["valued_money"],m_params["stop_money"],
    			m_params["profit"],m_params["money"],
    			m_params["tplus_redem_money"],stathistoryTplusRedem);

            if (gPtrConfig->m_AppCfg.check_tpulsbalance_for_reg_profit == 1)
            {
                throw CException(ERR_DIFF_BALANCE, "valued_money+stop_money != money+tplus_redem_money+history_tplus_redem_money", __FILE__, __LINE__);   
            }
            else
            {
                alert(ERR_DIFF_BALANCE, (m_params.getString("trade_id")+" reg daily profit check fail: valued_money+stop_money != money+tplus_redem_money+history_tplus_redem_money").c_str());
            }
        }
    }
   
}


/**
  * 生成基金分红记录
  */
void FundRegProfit::RecordFundProfit()
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
	stRecord.Fprofit = m_params.getLong("profit");
	strncpy(stRecord.Fday, m_params.getString("date").c_str(), sizeof(stRecord.Fday)-1);
    stRecord.Fprofit_type = PROFIT_TYPE_SHARE;
	stRecord.F1day_profit_rate= m_params.getLong("day_profit_rate");
	stRecord.F7day_profit_rate= m_params.getLong("seven_day_profit_rate");
	strncpy(stRecord.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(stRecord.Flogin_ip)-1);
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Ftplus_redem_money = m_params.getLong("tplus_redem_money");
    stRecord.Frecon_balance = m_params.getLong("money");
    stRecord.Fstandby1 = ((m_params.getLong("profit") <= 0 && m_params.getLong("tplus_redem_money")>0)?1:0);
    stRecord.Fend_tail_fee = 0; // 活期理财不存在扫尾收益

    //微信预付卡合作项目临时方案--below
    ST_FUND_CONTROL_INFO controlInfo;
    memset(&controlInfo,0,sizeof(controlInfo));
    strncpy(controlInfo.Ftrade_id,m_params["trade_id"],sizeof(controlInfo.Ftrade_id));
    bool isWxWfjUser = isWxPrePayCardBusinessUser(m_pFundCon,m_params["spid"],controlInfo);
    if (isWxWfjUser && controlInfo.Ftotal_fee >0 && m_params.getLong("profit")>0)
    {
        addFundControlProfit(m_pFundCon,m_params.getLong("profit"),m_params.getString("date"),m_params["systime"],m_params["trade_id"]);
        strncpy(stRecord.Fstandby3, gPtrConfig->m_AppCfg.wx_wfj_spid.c_str(), sizeof(stRecord.Fstandby3)-1);
    }
    //微信预付卡合作项目临时方案--above
    
    insertFundProfitRecord(m_pFundCon, stRecord);

	// 无收益不增加CKV
	if(m_params.getLong("profit") <= 0){
		return;
	}
    if (1 == gPtrConfig->m_AppCfg.update_profit_ckv_switch) {
        //将查数据最新21条收益写入ckv改为从ckv查询现有数据,直接将数据追加到ckv
        //如果cvk中已有21条则淘汰Fday最老的一条
    	bool ret = addFundProfitRecordToCache(stRecord);
        //如果调用失败则使用查询DB数据更新到CKV方式再调用一次
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
void FundRegProfit::UpdateProfitInfo()
{
    strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id)-1);
	m_fund_profit.Fcurtype = m_curtype;
	strncpy(m_fund_profit.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_profit.Fspid)-1);
	m_fund_profit.Frecon_balance = m_params.getLong("money");
	strncpy(m_fund_profit.Frecon_day, m_params.getString("date").c_str(), sizeof(m_fund_profit.Frecon_day)-1);
	m_fund_profit.Fprofit = m_params.getLong("profit");
	m_fund_profit.Freward_profit = 0;//昨日赠送收益要置为0
	m_fund_profit.Ftotal_profit =((m_fund_profit_exist) ? m_fund_profit.Ftotal_profit : 0) + m_params.getLong("profit");
	// 增加记录tplus_redem_money,收益检查余额的时候可以查出来
	m_fund_profit.Ftplus_redem_money = m_params.getLong("valued_money")+m_params.getLong("stop_money")-m_params.getLong("money");
    m_fund_profit.Fvalid_money= m_params.getLong("valued_money");
	strncpy(m_fund_profit.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(m_fund_profit.Flogin_ip)-1);
	
    strncpy(m_fund_profit.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_fund_profit.Fmodify_time)-1);
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

void FundRegProfit::UpdatePrecheckInfo()
{
    if(!m_fund_profit_exist)
        return;

    strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id)-1);
	m_fund_profit.Fcurtype = m_curtype;
	m_fund_profit.Frecon_balance = m_params.getLong("money");
	strncpy(m_fund_profit.Frecon_day, m_params.getString("date").c_str(), sizeof(m_fund_profit.Frecon_day)-1);
	m_fund_profit.Fprofit = m_params.getLong("profit");
    strncpy(m_fund_profit.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_fund_profit.Fmodify_time)-1);

    updateTotalFeeAndProfit(m_pFundCon, m_fund_profit);
}

void FundRegProfit::updateCache(FundProfit& fund_profit)
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

void FundRegProfit::CheckProfitRecord() throw (CException)
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
bool FundRegProfit::payNotifyOvertime(string pay_suc_time, int inteval)
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

/**
 * 核心充值
 */
void FundRegProfit::doSave() throw(CException)
{
	gPtrAppLog->normal("doSave, listid[%s]  ", m_params.getString("fund_trans_id").c_str());

	try
	{
		//收益入账，子账户不能报错，报错直接抛出，有入账程序决定如何处理
		//补单用系统当前时间传给子账户，否则子账户按时间核对余额的会有问题
		int iResult = SubaccSave(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_params.getString("fund_trans_id"),m_params.getLong("profit"),"基金收益", m_params.getString("systime"), 2);

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
void FundRegProfit::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
  
    rqst->olen = strlen(rqst->odata);
    return;
}

/**
* 更新用户基金CKV信息
*/
void FundRegProfit::updateUserAccCKV()
{
	if(m_params.getLong("valued_money")>0||m_params.getLong("stop_money")>0){
		return;
	}
	
	SpConfigCache sp_config;
	querySpConfigCache(m_pFundCon, m_params.getString("spid"),sp_config);
	
	int subacc_curtype = sp_config.curtype;

	//账户不存在返回账户余额为0，而不抛出异常，避免开户时子账户失败，或者同步TTC失败，导致用户查询一致报错
	LONG con=0;
	LONG balance = querySubaccBalance(m_fund_bind.Fuid, subacc_curtype, false,&con);
	if(balance==0){
		// 更新useracc锁用户表防止并发
		ST_FUND_BIND pstRecord;
		memset(&pstRecord,0,sizeof(ST_FUND_BIND));
		strncpy(pstRecord.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(pstRecord.Ftrade_id)-1);
		QueryFundBindByTradeid(m_pFundCon,m_fund_bind.Ftrade_id,&pstRecord,true);
		
		FundUserAcc userAcc;
		memset(&userAcc,0,sizeof(userAcc));
		strncpy(userAcc.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(userAcc.Ftrade_id)-1);
		strncpy(userAcc.Ffund_code,sp_config.fund_code.c_str(),sizeof(userAcc.Ffund_code)-1);		
		removeUserAcc(userAcc);		
	}
}


