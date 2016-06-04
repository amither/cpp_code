#include "db_fund_trans_date.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 






bool queryFundTransDate(CMySQL* pMysql, FundTransDate& data, bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fdate,Fspid,Fstate,Fmemo,Fcreate_time, "
                    " Fmodify_time,Fstandby1,Fstandby3 "
                    " FROM fund_db.t_fund_trans_date "
                    " WHERE "
                    " Fdate >= '%s' " 
                    " and Fstate = 1 "
                    " order by Fdate "
                    " LIMIT 1 "
                    " %s ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Fdate,row[0] ? row[0] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            data.Fstate = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Fmemo,row[3] ? row[3] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[4] ? row[4] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[5] ? row[5] : "", sizeof(data.Fmodify_time) - 1);
		data.Fstandby1= row[6] ? atoi(row[6]) : -1;
		strncpy(data.Fstandby3,row[7] ? row[7] : "", sizeof(data.Fstandby3) - 1);
                   }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}


bool queryFundTplusNDates(CMySQL* pMysql, vector<string>& dateVec, string curTime,int n) //查询t+n-1日期
{
	string dateStr = calculateFundDate(curTime);

    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fdate "
                    " FROM fund_db.t_fund_trans_date "
                    " WHERE "
                    " Fdate >= '%s' " 
                    " and Fstate = 1 "
                    " order by Fdate "
                    " LIMIT %d ",
                    pMysql->EscapeStr(dateStr).c_str(),
                    n
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
			string tDate = row[0] ? string(row[0]) : "";
			dateVec.push_back(tDate);
        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == n;
}



bool queryFundProfitDate(CMySQL* pMysql,  FundTransDate& data, bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fdate,Fspid,Fstate,Fmemo,Fcreate_time, "
                    " Fmodify_time  "
                    " FROM fund_db.t_fund_trans_date "
                    " WHERE "
                    " Fdate >= '%s' " 
                    " and Fstate = 1 "
                    " order by Fdate "
                    " LIMIT 1, 1 "
                    " %s ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Fdate,row[0] ? row[0] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            data.Fstate = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Fmemo,row[3] ? row[3] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[4] ? row[4] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[5] ? row[5] : "", sizeof(data.Fmodify_time) - 1);
            
        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

 /**
 * 根据输入时间计算基金日期
 * 基金日志是上一日的15:00:00至本日的14:59:59
 * @input		curTime 	YYYY-MM-DD HH:MM:SS
 * @output 基金日期 YYYYMMDD
 */
  string calculateFundDate(const string& curTime)
  {
	 string dateStr = changeDateFormat(curTime.substr(0, 10), 2); 
	 //15:00:00 之后则为下一交易日
	 if(curTime.substr(11) >= "15:00:00")
	 {
		 dateStr = addDays(dateStr, 1);    
	 }
 
	 return dateStr;
  }


string getCacheTradeDate(CMySQL* pMysql, const string& curTime)
{
   	time_t tt = time(NULL);
   	TradeDateCache *trade_date_cache = &gPtrConfig->m_fundTradeDate[curTime];
   	// 使用缓存日期
   	if(trade_date_cache->timeout>tt){
   		return trade_date_cache->tradeDate;
   	}
   	// 使用DB日期
   	vector<string> dateVec;
   	if(!queryFundTplusNDates(pMysql,dateVec,curTime,2))
   	{
   		alert(ERR_UNFOUND_TRADE_DATE,"T+1日期未配置:"+curTime);
   		trade_date_cache->tradeDate=getTradeDate(pMysql,curTime);
   		trade_date_cache->t1Date="";
   	}else{
   		trade_date_cache->tradeDate=dateVec[0];
   		trade_date_cache->t1Date=dateVec[1];
   	}
   	trade_date_cache->timeout=tt + 3600 * 12;// 本地缓存12小时;
   	return trade_date_cache->tradeDate;
}

string getCacheT1Date(CMySQL* pMysql, const string& curTime)
{
	time_t tt = time(NULL);
	TradeDateCache *trade_date_cache = &gPtrConfig->m_fundTradeDate[curTime];
	// 使用缓存日期
	if(trade_date_cache->timeout>tt){
	 return trade_date_cache->t1Date;
	}
	// 使用DB日期
	vector<string> dateVec;
	if(!queryFundTplusNDates(pMysql,dateVec,curTime,2))
	{
		alert(ERR_UNFOUND_TRADE_DATE,"T+1日期未配置:"+curTime);
		trade_date_cache->tradeDate=getTradeDate(pMysql,curTime);
		trade_date_cache->t1Date="";
	}else{
		trade_date_cache->tradeDate=dateVec[0];
		trade_date_cache->t1Date=dateVec[1];
	}
	trade_date_cache->timeout=tt + 3600 * 12;// 本地缓存12小时;
	return trade_date_cache->t1Date;
}
 

 /**
 * 计算交易日
 * input curTime
 * output trade_date
 */
 string getTradeDate(CMySQL* pMysql, const string& curTime)
 {
	 string dateStr = calculateFundDate(curTime);
 
	 FundTransDate data;
	 
	 memset(&data, 0, sizeof(FundTransDate));
	 strncpy(data.Fdate, dateStr.c_str(), sizeof(data.Fdate)-1);
 
	 if(queryFundTransDate(pMysql, data, false))
	 {
		return data.Fdate;
	 }

	 return "";
 
 }

  /**
  * 计算交易日及首次产生收益日
  * input curTime
  * output trade_date, fund_vdate
  */
  void getTradeDate(CMySQL* pMysql, const string& curTime , string& trade_date, string& fund_vdate)
  {
	  string dateStr = calculateFundDate(curTime);

	  FundTransDate data;
	  
	  memset(&data, 0, sizeof(FundTransDate));
	  strncpy(data.Fdate, dateStr.c_str(), sizeof(data.Fdate)-1);

	  if(queryFundTransDate(pMysql, data, false))
	  {
	  	 trade_date = data.Fdate;
	  }

	  memset(&data, 0, sizeof(FundTransDate));
	  strncpy(data.Fdate, dateStr.c_str(), sizeof(data.Fdate)-1);

	  if(queryFundProfitDate(pMysql, data, false))
	  {
	  	 fund_vdate = data.Fdate;
	  }
  }


    /**
  * 计算 T+1赎回的到账日
  * input curTime
  * output fund_fetch_date
  */
  void getTplusFetchDate(CMySQL* pMysql, const string& curTime , string& fund_fetch_date)
  {
    string dateStr = calculateFundDate(curTime);

    FundTransDate data;

    memset(&data, 0, sizeof(FundTransDate));
    strncpy(data.Fdate, dateStr.c_str(), sizeof(data.Fdate)-1);

    if(queryFundProfitDate(pMysql, data, false))
    {
        fund_fetch_date = data.Fdate;
    }
    else
    {
        throw CException(ERR_DB_UNKNOW, "getTplusFetchDate fail!", __FILE__, __LINE__);
    }
  }


    /**
  * 计算 T-2日期
  * input curTime 格式YYYYMMDD
  * output 
  */
void getTminus2TransDate(CMySQL* pMysql, const string& curData , string& Tminus2Date,string &TminusDate,bool &isCurTDay)
{
    static string s_curDate="";
    static string s_Tminus2Date;
    static string s_TminusDate;
    static bool   s_isCurDateTday;
    
    if (s_curDate == curData)
    {
        Tminus2Date = s_Tminus2Date;
        TminusDate = s_TminusDate;
        isCurTDay = s_isCurDateTday;
        return;
    }

    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fdate "
                    " FROM fund_db.t_fund_trans_date "
                    " WHERE "
                    " Fdate <= '%s' " 
                    " and Fstate = 1 "
                    " order by Fdate DESC "
                    " LIMIT 3 ",
                    pMysql->EscapeStr(curData).c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow != 3)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        string transDate[3];
        
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            transDate[i] = row[0] ? row[0] : "";
        }
        s_curDate = curData;
        if (transDate[0] == curData)
        {
            s_isCurDateTday = true;
            s_TminusDate = transDate[1];
            s_Tminus2Date = transDate[2];
        }
        else
        {
            s_isCurDateTday = false;
            s_TminusDate = transDate[0];
            s_Tminus2Date = transDate[1];
        }
        Tminus2Date = s_Tminus2Date;
        TminusDate = s_TminusDate;
        isCurTDay = s_isCurDateTday;
        
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return;
}


/**
 * 查询大于等于curDate的基金交易日信息
 * @param pMysql
 * @param curDate 
 * @param list 
 * @return 
 */
void queryLastAndNextDate(CMySQL* pMysql, const string &curDate, vector<FundLastAndNextTransDate>& list, bool limit1)
{
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    iLen = snprintf(szSql, sizeof(szSql),
                    " select a.Fdate date, a.Fspid Fspid, a.Fmemo memo,a.Fstandby3 standby3, "
                    " (select Fdate from fund_db.t_fund_trans_date where Fdate>a.Fdate and Fstate=1 limit 1) next_date,"
                    " (select Fdate from fund_db.t_fund_trans_date where Fdate<a.Fdate and Fstate=1 order by Fdate desc limit 1) last_date "
                    " from fund_db.t_fund_trans_date a "
                    " where Fdate>='%s' "
                    " %s ",
                    escapeString(curDate).c_str(),
                    limit1 ? "limit 1" : "");

    if (iLen <= 0 || iLen >= MAX_SQL_LEN) {
        throw CException(ERR_DB_SQL_NOVALUE, "construct sql string failed!", __FILE__, __LINE__);
    }
        
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行查询
    pMysql->Query(szSql, iLen);
    // 取结果集
    scope_mysql_res mysql_res(pMysql->FetchResult());
   
    // 获取结果行
    iRow = mysql_num_rows(mysql_res.handle());
    
    if(iRow <=0) {
        return;
    }
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(mysql_res.handle())) != NULL) {
        FundLastAndNextTransDate data;
        memset(&data, 0, sizeof(data));
        SCPY(data.Fdate, row[0]);
        SCPY(data.Fspid, row[1]);
	 SCPY(data.Fmemo, row[2]);
	 SCPY(data.Fstandby3, row[3]);
        SCPY(data.Fnext_date, row[4]);
        SCPY(data.Flast_date, row[5]);

        list.push_back(data);
    } 
}


/**
 * 将基金交易日信息写入ckv
 * @param pMysql 
 * @param cur_day 
 * @return 
 */
bool setFundTransDay2Ckv(CMySQL* pMysql, const string &cur_day)
{
    vector<FundLastAndNextTransDate> list;
    queryLastAndNextDate(pMysql, cur_day, list);

    TRACE_DEBUG("list size=%zd", list.size());
    
    vector<FundLastAndNextTransDate>::const_iterator cite;
    for (cite = list.begin(); cite != list.end(); ++cite) {
        if (0 == strlen(cite->Fdate)
            || 0 == strlen(cite->Flast_date)
            || 0 == strlen(cite->Fnext_date)) {
            TRACE_DEBUG("Fdate=%s last or next date is null", cite->Fdate);
            
            continue;
        }
        
        CParams kvReqSet;
        kvReqSet.setParam("Flast_trans_date", cite->Flast_date);
        kvReqSet.setParam("Fnext_trans_date", cite->Fnext_date);

        string key = "fund_trans_date_" + string(cite->Fdate);
        string value = kvReqSet.pack();
        if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_FUND_TRANS_DATE), key, value))
    	{
    		return false;
    	}    	
    }

    return true;
}

void insertTransDate(CMySQL* pMysql, FundTransDate& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_trans_date("
                    " Fdate,Fspid,Fstate,Fmemo,Fcreate_time,Fmodify_time,Fstandby1,Fstandby4)"
                    " VALUES('%s','%s',%d,'%s','%s','%s',%d,'%s')",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Fstate,
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    data.Fstandby1,
                    pMysql->EscapeStr(data.Fstandby4).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

void updateTransDate(CMySQL* pMysql, FundTransDate &data)
{
      stringstream ss_cond;
      map<string, string> kv_map;
    
      TRACE_DEBUG("updateTransDate:[%s]",data.Fdate);

      kv_map["Fmodify_time"] = toString(data.Fmodify_time);
	
	//if(!(0 == strcmp("", data.Fmemo)))
	{
		kv_map["Fmemo"] = data.Fmemo;
	}
	if(!(0 == strcmp("", data.Fstandby4)))
	{
		kv_map["Fstandby4"] = data.Fstandby4;
	}  
	kv_map["Fstandby1"] = toString(data.Fstandby1);   
	
      ss_cond << "Fdate='" <<  escapeString(data.Fdate) <<"' ";

      // 执行更新数据表操作
      int affect_row = UpdateTable(pMysql, "fund_db.t_fund_trans_date", ss_cond, kv_map);

      if (affect_row != 1)
      {
          throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
      }
}

void queryHKLastAndNextDate(CMySQL* pMysql, const string &curDate, vector<FundLastAndNextTransDate>& list, bool limit1)
{
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    iLen = snprintf(szSql, sizeof(szSql),
                    " select a.Fdate date, a.Fspid Fspid,a.Fstandby1 Fstandby1,a.Fstandby3 Fstandby3,a.Fmemo Fmemo,"
                    " (select Fdate from fund_db.t_fund_trans_date where Fdate>a.Fdate and Fstate=1 and Fstandby1 in (0,1) limit 1) next_date,"
                    " (select Fdate from fund_db.t_fund_trans_date where Fdate<a.Fdate and Fstate=1 and Fstandby1 in (0,1) order by Fdate desc limit 1) last_date "
                    " from fund_db.t_fund_trans_date a "
                    " where Fdate>='%s'  and Fstate = 1 and Fstandby1 in (0,1)"
                    " %s ",
                    escapeString(curDate).c_str(),
                    limit1 ? "limit 1" : "");

    if (iLen <= 0 || iLen >= MAX_SQL_LEN) {
        throw CException(ERR_DB_SQL_NOVALUE, "construct sql string failed!", __FILE__, __LINE__);
    }
        
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行查询
    pMysql->Query(szSql, iLen);
    // 取结果集
    scope_mysql_res mysql_res(pMysql->FetchResult());
   
    // 获取结果行
    iRow = mysql_num_rows(mysql_res.handle());
    
    if(iRow <=0) {
        return;
    }
    
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(mysql_res.handle())) != NULL) {
        FundLastAndNextTransDate data;
        memset(&data, 0, sizeof(data));
        SCPY(data.Fdate, row[0]);
        SCPY(data.Fspid, row[1]);
	 data.Fstandby1 = row[2] ? atoi(row[2]) : -1;
	 SCPY(data.Fstandby3, row[3]);
	 SCPY(data.Fmemo, row[4]);
        SCPY(data.Fnext_date, row[5]);
        SCPY(data.Flast_date, row[6]);

        list.push_back(data);
    } 
}

bool queryExactTransDate(CMySQL* pMysql, FundTransDate& data, bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fdate,Fspid,Fstate,Fmemo,Fcreate_time, "
                    " Fmodify_time,Fstandby1,Fstandby3 "
                    " FROM fund_db.t_fund_trans_date "
                    " WHERE "
                    " Fdate='%s' " 
                    " LIMIT 1 "
                    " %s ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Fdate,row[0] ? row[0] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            data.Fstate = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Fmemo,row[3] ? row[3] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[4] ? row[4] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[5] ? row[5] : "", sizeof(data.Fmodify_time) - 1);
		data.Fstandby1= row[6] ? atoi(row[6]) : -1;
		strncpy(data.Fstandby3,row[7] ? row[7] : "", sizeof(data.Fstandby3) - 1);
                   }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

bool queryHKTransDate(CMySQL* pMysql, FundTransDate& data, bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fdate,Fspid,Fstate,Fmemo,Fcreate_time, "
                    " Fmodify_time,Fstandby1,Fstandby3 "
                    " FROM fund_db.t_fund_trans_date "
                    " WHERE "
                    " Fdate >= '%s' " 
                    " and Fstate = 1 "
                    " and Fstandby1 in (0,1) "
                    " order by Fdate "
                    " LIMIT 1 "
                    " %s ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Fdate,row[0] ? row[0] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            data.Fstate = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Fmemo,row[3] ? row[3] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[4] ? row[4] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[5] ? row[5] : "", sizeof(data.Fmodify_time) - 1);
		data.Fstandby1= row[6] ? atoi(row[6]) : -1;
		strncpy(data.Fstandby3,row[7] ? row[7] : "", sizeof(data.Fstandby3) - 1);
                   }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

void queryFundTDateforCache(CMySQL* mysql, string sDay, string &sData, string &sHKData) throw (CException)
{
	time_t cache_time = gPtrConfig->m_fTradeDate[sDay].timeout;
	time_t tt = time(NULL);
	if(cache_time > tt)
	{
		sData = gPtrConfig->m_fTradeDate[sDay].sTData;
		sHKData = gPtrConfig->m_fTradeDate[sDay].sHKData;
		return;
	}

	FundTransDate stTDate; 
	memset(&stTDate, 0, sizeof(FundTransDate));
       strncpy(stTDate.Fdate, sDay.c_str(), sizeof(stTDate.Fdate) - 1);
	if(!queryFundTransDate(mysql, stTDate, false))
	{
		throw CException(ERR_DB_NULL_RESULT, "fund trade date not exist. ", __FILE__, __LINE__);
	}

	FundTransDate stHKDate; 
	memset(&stHKDate, 0, sizeof(FundTransDate));
       strncpy(stHKDate.Fdate, sDay.c_str(), sizeof(stHKDate.Fdate) - 1);
	if(!queryHKTransDate(mysql, stHKDate, false))
	{
		throw CException(ERR_DB_NULL_RESULT, "fund trade date not exist. ", __FILE__, __LINE__);
	}
	
	gPtrConfig->m_fTradeDate[sDay].sTData = sData = stTDate.Fdate;
	gPtrConfig->m_fTradeDate[sDay].sHKData = sHKData = stHKDate.Fdate;
	gPtrConfig->m_fTradeDate[sDay].timeout = tt + 60;
	return;
}


