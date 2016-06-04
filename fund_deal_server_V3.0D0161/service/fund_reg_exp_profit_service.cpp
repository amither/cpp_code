/**
  * FileName: fund_reg_exp_profit_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-29
  * Description: 基金交易服务 基金收入登记 源文件
  */

#include "fund_commfunc.h"
#include "fund_reg_exp_profit_service.h"
#include <cstdlib>

FundRegExpProfit::FundRegExpProfit(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));    
	memset(&m_fund_profit, 0, sizeof(FundProfit));    

	m_fund_profit_exist = false;

}

/**
  * service step 1: 解析输入参数
  */
void FundRegExpProfit::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

	// 要保留请求数据，抛差错使用
    m_request = rqst;

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_reg_exp_profit_service] receives: %s", szMsg);
	
	m_params.readIntParam(szMsg, "op_type", 1, 2);  
	m_params.readStrParam(szMsg, "spid", 1, 15);
    m_params.readStrParam(szMsg, "trade_id", 1, 32);
	m_params.readStrParam(szMsg, "fund_trans_id", 0, 32);
    m_params.readStrParam(szMsg, "date", 1, 20);
	
	m_params.readLongParam(szMsg, "total_money", 0, MAX_LONG);   //总资产，单位分
	m_params.readLongParam(szMsg, "total_fund_units", 0, MAX_LONG);//总份额
	m_params.readLongParam(szMsg, "available_fund_units", 0, MAX_LONG); //可用份额
	m_params.readLongParam(szMsg, "freeze_fund_units", 0, MAX_LONG);//冻结份额
	m_params.readLongParam(szMsg, "admin_con_units", 0, MAX_LONG);//司法冻结
	m_params.readLongParam(szMsg, "day_profit_rate", 0, MAX_LONG);//净值
	m_params.readSignedLongParam(szMsg, "seven_day_profit_rate",MIN_LONG, MAX_LONG);//日涨跌幅
	m_params.readIntParam(szMsg, "deviation_profit", 0, MAX_INTEGER); //收益偏差(单位是分)
	m_params.readLongParam(szMsg, "t1_redem_units", 0, MAX_LONG); //当日普通赎回份额
	m_params.readLongParam(szMsg, "buy_money", 0, MAX_LONG);//当日申购金额
    m_params.readLongParam(szMsg, "valued_fund_units", 0, MAX_LONG);//产生收益的份额
    m_params.readLongParam(szMsg, "profit", 0, MAX_LONG);//收益值，单位分
    m_params.readIntParam(szMsg, "profit_sign", 1, 2); //收益正/负  1正收益；2-负收益
    
  
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 0, 32);   // 接口token


	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

	m_curtype = querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));

}

/*
 * 生成基金注册用token
 */
string FundRegExpProfit::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照trade_id|fund_trans_id|date|money|profit|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["fund_trans_id"] << "|" ;
    ss << m_params["date"] << "|" ;
    ss << m_params["total_money"] << "|" ;
    ss << m_params["profit"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

	TRACE_DEBUG("fund authen token  input=%s", ss.str().c_str());
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundRegExpProfit::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=[%s] not eq [%s]", 
	                m_params.getString("token").c_str(),StrUpper(token).c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}


/**
  * 检查参数，获取内部参数
  */
void FundRegExpProfit::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

}

/**
  * 执行收益入账
  */
void FundRegExpProfit::excute() throw (CException)
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
void FundRegExpProfit::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByTradeid(gPtrFundSlaveDB, m_params.getString("trade_id").c_str(), &m_fund_bind, false,false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, ("the fund bind[" + m_params.getString("trade_id") + "] record not exist! ").c_str(), __FILE__, __LINE__);
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

void FundRegExpProfit::AddProfit() throw (CException)
{

    m_isWriteLogTimeCost = false;
    srand((unsigned)time(0));
    int randValue = rand()%500000;
    //每入账5000条 打印1条日志
    if(randValue<100)
    {
        m_isWriteLogTimeCost = true;
    }

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
            TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=1 QueryFundProfit timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }

        m_tStart = m_tEnd;
		// 统计对账日所有的申购赎回金额,并判断金额是否一致
		CheckBalance(true);

        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=1 CheckBalance timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }

		/* 开启事务 */
		m_pFundCon->Begin();
		
		m_tStart = m_tEnd;
		// 更新CKV用户基金列表信息
		updateUserAccCKV();
        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=1 updateUserAccCKV timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }
		
        // 更新此次校验的总金额和总收益到t_fund_profit_x
        m_tStart = m_tEnd;
        UpdatePrecheckInfo();

        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=1 UpdatePrecheckInfo timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
        }
        
        m_tStart = m_tEnd;
        /* 提交事务 */
        m_pFundCon->Commit();
        if(m_isWriteLogTimeCost)
        {
            gettimeofday(&m_tEnd,NULL);
            TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=1 m_pFundCon->Commit() timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
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
                TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=2 QueryFundProfit timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }

			// 统计对账日所有的申购赎回金额,并判断金额是否一致
            m_tStart = m_tEnd;
			CheckBalance();
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=2 CheckBalance timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
			
			// 记录分红交易单
            m_tStart = m_tEnd;
			RecordFundProfit();	
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=2 RecordFundProfit timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
			// 累计收益，修改昨日余额，修改对账日期
            m_tStart = m_tEnd;
			UpdateProfitInfo();
			if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=2 UpdateProfitInfo timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
            }
	        /* 提交事务 */
            m_tStart = m_tEnd;
	        m_pFundCon->Commit();
            if(m_isWriteLogTimeCost)
            {
                gettimeofday(&m_tEnd,NULL);
                TRACE_NORMAL("[fund_reg_exp_profit_service] op_type=2 m_pFundCon->Commit() timeCost[%d]", 1000000*(m_tEnd.tv_sec-m_tStart.tv_sec)+(m_tEnd.tv_usec-m_tStart.tv_usec));
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
void FundRegExpProfit::QueryFundProfit(bool & isThroughPrecheck) throw(CException)
{
	strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id) - 1);
	m_fund_profit.Fcurtype = m_curtype;
	isThroughPrecheck = queryFundProfitAndCheck(m_fund_profit, m_fund_profit_exist, m_params["date"], calProfit());
}

/**
* 统计对账日所有的申购赎回(包括赠送申购)金额,并判断金额是否一致
*/
void FundRegExpProfit::CheckBalance(bool precheck)throw (CException)
{
    if(precheck == false)  //入账
    {
        if(m_isThroughPrecheck) //检查的时候已经记录的份额和收益
        {
            LONG profit = calProfit() ;
            if(m_params.getLong("total_money") == m_fund_profit.Fprecheck_money   //总资产
				 && profit == m_fund_profit.Fprecheck_profit  //收益
				 && m_params.getLong("total_fund_units")  == m_fund_profit.Fstandby7 //总份额 (也是可产生收益的份额)
				)
            {
                return;
            }
            else
            {
                char szErrMsg[1024];
		       snprintf(szErrMsg, sizeof(szErrMsg), "precheck balance is different! trade_id[%s], date[%s],"
					         "Fprecheck_money in db[%ld],Fprecheck_profit in db[%ld],total_fund_units money in db[%ld] "
					         "total_money  input[%ld],profit input[%ld],total_fund_units input[%ld]", 
			    m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(), 
			    m_fund_profit.Fprecheck_money, m_fund_profit.Fprecheck_profit, m_fund_profit.Fstandby7,
			    m_params.getLong("money"), profit,m_params.getLong("total_fund_units") );
		   
                TRACE_ERROR("%s",szErrMsg);
		        throw CException(ERR_DIFF_BALANCE,szErrMsg, __FILE__, __LINE__);
            }
        }
    }

	//核对总逻辑
	//总份额	= 昨日 +（入账日期确认份额 -赎回份额）
	//非交易日为0，交易日用下面的计算
	//----------	开始时间 <=入账时间的 获取三个T日:入账日期为交易日取第三个
	//-----------结束时间 <=入账时间的 获取三个T日:入账日期为交易日取第二个
	
	//冻结份额=司法冻结份额+赎回未确认的冻结份额
	//总资产 = 总份额* 净值+申购未确认总金额

	//总份额 = 产生收益的份额
	if(  m_params.getLong("total_fund_units") !=	m_params.getLong("valued_fund_units"))
	{

			  char szErrMsg[1024];
		       snprintf(szErrMsg, sizeof(szErrMsg), "total_fund_units is different! trade_id[%s], date[%s],"
						  "total_fund_units (input[%ld])!= valued_fund_units( input [%ld]))", 
			   m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
			   m_params.getLong("total_fund_units") , m_params.getLong("valued_fund_units"));
		   TRACE_ERROR("%s",szErrMsg);
		   throw CException(ERR_DIFF_BALANCE, szErrMsg, __FILE__, __LINE__);   
	 }
	
	ST_TRADE_FUND trade_fund;
	strncpy(trade_fund.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(trade_fund.Ftrade_id)-1);
	strncpy(trade_fund.Ftrade_date, m_params.getString("date").c_str(), sizeof(trade_fund.Ftrade_date)-1);
	trade_fund.Fuid = m_fund_bind.Fuid;
	trade_fund.Fcur_type = m_curtype;
	
	//获取当时的申购金额和赎回份额
	LONG curday_purchase_money =0; //申购
	LONG curday_redem_total_fee = 0; //赎回
	getTodayTran(trade_fund, curday_purchase_money,curday_redem_total_fee);
	// t1_redem_units		 当日普通赎回份额
	//buy_money  当日申购金额
	if( m_params.getLong("t1_redem_units") != curday_redem_total_fee )
	{
		char szErrMsg[1024];
		snprintf(szErrMsg, sizeof(szErrMsg), "t1_redem_units is different! trade_id[%s], date[%s],"
				"t1_redem_units(input[%ld])!= curday_redem_total_fee( in db [%ld])", 
				m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
				m_params.getLong("t1_redem_units")  , curday_redem_total_fee);
		TRACE_ERROR("%s",szErrMsg);
		throw CException(ERR_DIFF_BALANCE, szErrMsg, __FILE__, __LINE__);	  
	}
	if( m_params.getLong("buy_money") != curday_purchase_money )
	{
	     char szErrMsg[1024];
		       snprintf(szErrMsg, sizeof(szErrMsg), "buy_money is different! trade_id[%s], date[%s],"
						 "buy_money(input[%ld])!= curday_purchase_money( in db [%ld])", 
			  m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
			  m_params.getLong("buy_money")  , curday_purchase_money);
		  TRACE_ERROR("%s",szErrMsg);
		  throw CException(ERR_DIFF_BALANCE, szErrMsg, __FILE__, __LINE__);  
	 }

    //获取历史未确认的申购金额和赎回份额
    //申购未确认总金额 = (<=入账日期  第二个T日)15点到入账日期的15点
	//赎回未确认的冻结份额= 	 (<=入账日期  第二个T日)15点到入账日期的15点
	LONG listory_purchase_money =0; //申购
	LONG listory_redem_total_fee = 0; //赎回
	getHistoryNotAckTran(trade_fund, listory_purchase_money,listory_redem_total_fee);

    //冻结份额=司法冻结份额+赎回未确认的冻结份额
	if(  m_params.getLong("freeze_fund_units") !=  m_params.getLong("admin_con_units") + listory_redem_total_fee)
	{
		char szErrMsg[1024];
		snprintf(szErrMsg, sizeof(szErrMsg), "freeze_fund_units is different! trade_id[%s], date[%s],"
				"freeze_fund_units(input[%ld])!= admin_con_units( input [%ld])+listory_redem_total_fee (in db[%ld])", 
				m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
				m_params.getLong("freeze_fund_units")  , m_params.getLong("admin_con_units"), listory_redem_total_fee);
		TRACE_ERROR("%s",szErrMsg);
		throw CException(ERR_DIFF_BALANCE,szErrMsg, __FILE__, __LINE__);	 
	}
	
	LONG todayAck_purchase_money =0; //入账日期确认的申购份额
	LONG todayAck_redem_total_fee = 0; //入账日期赎回的份额
    getTodayAckTran(trade_fund, todayAck_purchase_money,todayAck_redem_total_fee);

   //总份额  = 昨日 +（入账日期确认份额 -赎回份额）
   LONG historyProfit = ((m_fund_profit_exist) ? m_fund_profit.Fvalid_money: 0 );
   if(  m_params.getLong("total_fund_units") != historyProfit + todayAck_purchase_money - todayAck_redem_total_fee)
   {
         char szErrMsg[1024];
		       snprintf(szErrMsg, sizeof(szErrMsg), "total_fund_units is different! trade_id[%s], date[%s],"
			           "total_fund_units (input[%ld])!= historytotal( in db [%ld])+todayAck_purchase_money (in db[%ld])-todayAck_redem_total_fee (in db[%ld])", 
			m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
			m_params.getLong("total_fund_units") , historyProfit, todayAck_purchase_money,todayAck_redem_total_fee);
        TRACE_ERROR("%s",szErrMsg);
		throw CException(ERR_DIFF_BALANCE, szErrMsg, __FILE__, __LINE__);	
   }
  
	//总资产=总份额*净值+申购未确认总金额
     LONG ackMoney =	m_params.getLong("total_money") - listory_purchase_money ;
	 LONG tempMoney = (LONG) ( (m_params.getLong("total_fund_units")  * m_params.getLong("day_profit_rate"))/10000);
	 LONG diffMoney = tempMoney - ackMoney;  
	 if( diffMoney >	m_params.getInt("deviation_profit") ||	 diffMoney < (0-  m_params.getInt("deviation_profit") ) )
	{
		   char szErrMsg[1024];
		   snprintf(szErrMsg, sizeof(szErrMsg),"total_money is different! trade_id[%s], date[%s],"
			           "total_money (input[%ld])!= total_fund_units( in put [%ld])*day_profit_rate([%.4f])+listory_purchase_money (in db[%ld]),diffMoney(%ld),deviation_profit(%d)", 
			m_params.getString("trade_id").c_str(), m_params.getString("date").c_str(),
			m_params.getLong("total_money") , m_params.getLong("total_fund_units"),m_params.getLong("day_profit_rate")/10000.0f, listory_purchase_money,diffMoney,m_params.getInt("deviation_profit"));
		   
		   TRACE_ERROR("%s",szErrMsg);
		throw CException(ERR_DIFF_BALANCE, szErrMsg, __FILE__, __LINE__);	
	}
}

/**
  * 生成基金分红记录
  */
void FundRegExpProfit::RecordFundProfit()
{
	//if(m_params.getLong("profit") <= 0 && m_params.getLong("total_fund_units")<=0)
	if(m_params.getLong("profit") <= 0 )
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
	stRecord.Fvalid_money =m_params.getLong("valued_fund_units");  //产生收益的份额
	stRecord.Fstop_money = 0; //未产生收益的份额(总份额=产生收益的份额)
	stRecord.Ftotal_profit =((m_fund_profit_exist) ? m_fund_profit.Ftotal_profit : 0) + calProfit();
	stRecord.Fprofit = calProfit(); //收益是带有正负号的
	strncpy(stRecord.Fday, m_params.getString("date").c_str(), sizeof(stRecord.Fday)-1);
    stRecord.Fprofit_type = PROFIT_TYPE_SHARE;
	stRecord.F1day_profit_rate= m_params.getLong("day_profit_rate"); //净值
	stRecord.F7day_profit_rate= m_params.getLong("seven_day_profit_rate");// 日涨跌幅
	strncpy(stRecord.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(stRecord.Flogin_ip)-1);
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Ftplus_redem_money = m_params.getLong("t1_redem_units"); //当日赎回份额
    stRecord.Frecon_balance = m_params.getLong("total_money");
    stRecord.Fstandby1 = 1;//这样收益和子账户对账就不用添加指数基金的条件
    stRecord.Fend_tail_fee = 0; // 活期理财不存在扫尾收益
    
    insertFundProfitRecord(m_pFundCon, stRecord);

    if (1 == gPtrConfig->m_AppCfg.update_profit_ckv_switch) 
	{
        //将查数据最新21条收益写入ckv改为从ckv查询现有数据,直接将数据追加到ckv
        //如果cvk中已有21条则淘汰Fday最老的一条
    	bool ret = addFundProfitRecordToCache(stRecord);
        //如果调用失败则使用查询DB数据更新到CKV方式再调用一次
        if (true != ret) 
		{
            TRACE_ERROR("call addFundProfitRecordToCache fail. retry to call setFundProfitRecordToKV");
            
            setFundProfitRecordToKV(m_pFundCon,stRecord);            
        }
    } else 
    {
        setFundProfitRecordToKV(m_pFundCon,stRecord);        
    }
	
}

/**
* 更新账户收益记录
*/
void FundRegExpProfit::UpdateProfitInfo()
{
    strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id)-1);
	m_fund_profit.Fcurtype = m_curtype;
	strncpy(m_fund_profit.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_profit.Fspid)-1);
	m_fund_profit.Frecon_balance = m_params.getLong("total_money");
	strncpy(m_fund_profit.Frecon_day, m_params.getString("date").c_str(), sizeof(m_fund_profit.Frecon_day)-1);
	m_fund_profit.Fprofit = calProfit();
	m_fund_profit.Freward_profit = 0;//昨日赠送收益要置为0
	m_fund_profit.Ftotal_profit =((m_fund_profit_exist) ? m_fund_profit.Ftotal_profit : 0) + calProfit();
	// 增加记录tplus_redem_money  
	m_fund_profit.Ftplus_redem_money = m_params.getLong("t1_redem_units");
	m_fund_profit.Fvalid_money= m_params.getLong("valued_fund_units"); 
	strncpy(m_fund_profit.Flogin_ip, m_params.getString("client_ip").c_str(), sizeof(m_fund_profit.Flogin_ip)-1);
    strncpy(m_fund_profit.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_fund_profit.Fmodify_time)-1);
	//m_fund_profit.Ffinancial_days ++ ; 目前的入账没有增加该字段，指数基金也不增加
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

void FundRegExpProfit::UpdatePrecheckInfo()
{
    if(!m_fund_profit_exist)
    {
        return;
    }
	//存在就记录今天入账的份额和收益到 账户收益表
    strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id)-1);
	m_fund_profit.Fcurtype = m_curtype;
	m_fund_profit.Frecon_balance = m_params.getLong("total_money");  //总资产
	strncpy(m_fund_profit.Frecon_day, m_params.getString("date").c_str(), sizeof(m_fund_profit.Frecon_day)-1);
	m_fund_profit.Fprofit = calProfit();  //收益
	m_fund_profit.Fstandby7 = m_params.getLong("total_fund_units")  ;
    strncpy(m_fund_profit.Fmodify_time, m_params.getString("systime").c_str(), sizeof(m_fund_profit.Fmodify_time)-1);

    updateTotalFeeAndProfit(m_pFundCon, m_fund_profit);
}

void FundRegExpProfit::updateCache(FundProfit& fund_profit)
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


/**
* inteval 超时间隔,单位为秒
*/
bool FundRegExpProfit::payNotifyOvertime(string pay_suc_time, int inteval)
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
* 更新用户基金CKV信息
*/
void FundRegExpProfit::updateUserAccCKV()
	{
	    //可产生收益的份额  或者总资产大于 0，不删除我的资产
		if(m_params.getLong("total_fund_units")>0||m_params.getLong("total_money")>0)
		{ //总份额 = 可产生收益的份额    ，检查了可产生收益的份额，就不用检查收益了
			return;
		}
		
		SpConfigCache sp_config;
		querySpConfigCache(m_pFundCon, m_params.getString("spid"),sp_config);
		
		int subacc_curtype = sp_config.curtype;
	
		//账户不存在返回账户余额为0，而不抛出异常，避免开户时子账户失败，或者同步TTC失败，导致用户查询一致报错
		LONG con=0;
		LONG balance = querySubaccBalance(m_fund_bind.Fuid, subacc_curtype, false,&con);
		bool hasUnconfirm = false;
		
		//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
		if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
		{
			hasUnconfirm = queryFundUnconfirmExists(m_pFundCon,m_params.getString("spid"),m_params.getString("trade_id"));
		}else{
			hasUnconfirm = queryUnfinishTransExistsBySp(m_pFundCon,m_params.getString("spid"),m_params.getString("trade_id"));
		}
		
		if(balance + con == 0&&!hasUnconfirm) //退出用户在我的资产中删除
		{
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
/*
*获取当天的申购金额，赎回份额
*/
void  FundRegExpProfit::getTodayTran(ST_TRADE_FUND &trade_fund, LONG &purchase_total_fee,LONG &redem_total_fee)
{   
	string purTypes = "(Fpur_type=" + toString(PURTYPE_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_PROFIT) 
		+ " or Fpur_type=" + toString(PURTYPE_TRANSFER_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_SHARE) + ")";
	StatTransNotAckFee(gPtrFundSlaveDB, trade_fund, purchase_total_fee,purTypes);


	purTypes = "(Fpur_type=" + toString(PURTYPE_REDEEM) + " or Fpur_type=" + toString(PURTYPE_TRANSFER_REDEEM) +  ")";
	StatTransNotAckFee(gPtrFundSlaveDB, trade_fund, redem_total_fee,purTypes);
}

/*
*获取历史未确认的申购金额，赎回份额
*/
void  FundRegExpProfit::getHistoryNotAckTran(ST_TRADE_FUND &trade_fund, LONG &purchase_total_fee,LONG &redem_total_fee)
{   
	string purTypes = "(Fpur_type=" + toString(PURTYPE_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_PROFIT) 
		+ " or Fpur_type=" + toString(PURTYPE_TRANSFER_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_SHARE) + ")";
	StatHisotryNotAckFee(gPtrFundSlaveDB, trade_fund, purchase_total_fee,purTypes);


	purTypes = "(Fpur_type=" + toString(PURTYPE_REDEEM) + " or Fpur_type=" + toString(PURTYPE_TRANSFER_REDEEM) +  ")";
	StatHisotryNotAckFee(gPtrFundSlaveDB, trade_fund, redem_total_fee,purTypes);
}
/*
*获取今日确认的申购份额和赎回份额
*/
void  FundRegExpProfit::getTodayAckTran(ST_TRADE_FUND &trade_fund, LONG &purchase_total_fee,LONG &redem_total_fee)
{   
	string purTypes = "(Fpur_type=" + toString(PURTYPE_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_PROFIT) 
		+ " or Fpur_type=" + toString(PURTYPE_TRANSFER_PURCHASE) + " or Fpur_type=" + toString(PURTYPE_REWARD_SHARE) + ")";
	StatTodayAckPurchaseFee(gPtrFundSlaveDB, trade_fund, purchase_total_fee,purTypes);


	purTypes = "(Fpur_type=" + toString(PURTYPE_REDEEM) + " or Fpur_type=" + toString(PURTYPE_TRANSFER_REDEEM) +  ")";
	StatTodayAckRedemFee(gPtrFundSlaveDB, trade_fund, redem_total_fee,purTypes);
}

/**
* 计算收益
*/
LONG FundRegExpProfit::calProfit()
{
	LONG profit = m_params.getLong("profit");
	if( m_params.getLong("profit_sign") == 2  ) //负数
	{
		profit = (-1) * profit ;
	}
    return profit;
}

/**
  * 打包输出参数
  */
void FundRegExpProfit::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
  
    rqst->olen = strlen(rqst->odata);
    return;
}




