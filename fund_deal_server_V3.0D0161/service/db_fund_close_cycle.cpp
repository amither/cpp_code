#include "db_fund_close_cycle.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
extern CMySQL* gPtrFundDB;




bool queryFundCloseCycle(CMySQL* pMysql, FundCloseCycle& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Fdate,Ffund_code,Ftrans_date,Ffirst_profit_date, "
                    " Fopen_date,Fbook_stop_date,Fstart_date,Fdue_date,Fprofit_end_date, "
                    " Flstate,Fmemo,Fcreate_time,Fmodify_time "
                    " FROM fund_db.t_fund_close_cycle "
                    " WHERE "
                    " Fdate='%s'  AND " 
                    " Ffund_code='%s' " 
                    " %s ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
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
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Fdate,row[1] ? row[1] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrans_date,row[3] ? row[3] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[4] ? row[4] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[5] ? row[5] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[6] ? row[6] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[7] ? row[7] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fdue_date,row[8] ? row[8] : "", sizeof(data.Fdue_date) - 1);
            strncpy(data.Fprofit_end_date,row[9] ? row[9] : "", sizeof(data.Fprofit_end_date) - 1);
            data.Flstate = row[10] ? atoi(row[10]) : 0;
            strncpy(data.Fmemo,row[11] ? row[11] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[12] ? row[12] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[13] ? row[13] : "", sizeof(data.Fmodify_time) - 1);

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


/*
*查询大于某一天的全部交易周期信息
*query返回多行数据函数
*/
bool queryFundCloseCycleList(CMySQL* pMysql,FundCloseCycle &where,vector< FundCloseCycle>& dataVec,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Fdate,Ffund_code,Ftrans_date,Ffirst_profit_date, "
                    " Fopen_date,Fbook_stop_date,Fstart_date,Fdue_date,Fprofit_end_date, "
                    " Flstate,Fmemo,Fcreate_time,Fmodify_time "
                    " FROM fund_db.t_fund_close_cycle "
                    " WHERE "
                    " Fdate >= '%s'" 

                    " %s ",
                    pMysql->EscapeStr(where.Fdate).c_str(),
                    lock ? "FOR UPDATE" : ""
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
            FundCloseCycle data;
            
            memset(&data, 0, sizeof(data));
            MYSQL_ROW row = mysql_fetch_row(pRes);
            
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Fdate,row[1] ? row[1] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrans_date,row[3] ? row[3] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[4] ? row[4] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[5] ? row[5] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[6] ? row[6] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[7] ? row[7] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fdue_date,row[8] ? row[8] : "", sizeof(data.Fdue_date) - 1);
            strncpy(data.Fprofit_end_date,row[9] ? row[9] : "", sizeof(data.Fprofit_end_date) - 1);
            data.Flstate = row[10] ? atoi(row[10]) : 0;
            strncpy(data.Fmemo,row[11] ? row[11] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[12] ? row[12] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[13] ? row[13] : "", sizeof(data.Fmodify_time) - 1);
                
            dataVec.push_back(data);
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

	return iRow >= 1;
}


bool setAllCloseFundCycleToKV()
{
	vector<FundCloseCycle> fundCloseCycleVec;
	FundCloseCycle queryFundCloseCycle;
	memset(&queryFundCloseCycle, 0, sizeof(FundCloseCycle));  
	strncpy(queryFundCloseCycle.Fdate , toString(GetDateToday()).c_str(), sizeof(queryFundCloseCycle.Fdate)-1);	
	
	if( !queryFundCloseCycleList(gPtrFundSlaveDB, queryFundCloseCycle, fundCloseCycleVec, false))
	{
		TRACE_DEBUG("no fund sp config");
		return true;
	}

    for(vector<FundCloseCycle>::iterator iter = fundCloseCycleVec.begin(); iter != fundCloseCycleVec.end(); ++iter)
    {
		FundCloseCycle& fundCloseCycle = *iter;

		string key = "close_fund_cycle_" + toString(fundCloseCycle.Ffund_code) + "_" + toString(fundCloseCycle.Fdate);
	    string szValue;

		CParams kvReqSet;
	    //设置要修改的数据szValue
		kvReqSet.setParam("Fdate",fundCloseCycle.Fdate);
		kvReqSet.setParam("Ffund_code",fundCloseCycle.Ffund_code);
		kvReqSet.setParam("Ftrans_date",fundCloseCycle.Ftrans_date);
		kvReqSet.setParam("Ffirst_profit_date",fundCloseCycle.Ffirst_profit_date);
		kvReqSet.setParam("Fopen_date",fundCloseCycle.Fopen_date);
		kvReqSet.setParam("Fbook_stop_date",fundCloseCycle.Fbook_stop_date);
		kvReqSet.setParam("Fstart_date",fundCloseCycle.Fstart_date);
		kvReqSet.setParam("Fdue_date",fundCloseCycle.Fdue_date);
		kvReqSet.setParam("Fprofit_end_date",fundCloseCycle.Fprofit_end_date);
		kvReqSet.setParam("Flstate",fundCloseCycle.Flstate);
		kvReqSet.setParam("Fmemo",fundCloseCycle.Fmemo);
	    szValue = kvReqSet.pack();

	    //将szValue写入ckv
		if(gCkvSvrOperator->set(CKV_KEY_CLOSE_FUND_CYCLE, key, szValue))
		{
			return false;
		}
		
    }

	return true;
		
}


string getCacheCloseMaxTradeDate(CMySQL* pMysql, const string& curDate, const string& fund_code)
{
	time_t tt = time(NULL);
	string key = curDate+fund_code;
	TradeDateCache *trade_date_cache = &gPtrConfig->m_closeMaxTradeDate[curDate];
	// 使用缓存日期
	if(trade_date_cache->timeout>tt){
		return trade_date_cache->tradeDate;
	}
    // 使用DB日期
	FundCloseCycle data;	 
	memset(&data, 0, sizeof(FundTransDate));
	strncpy(data.Fdate, curDate.c_str(), sizeof(data.Fdate)-1);
	strncpy(data.Ffund_code, fund_code.c_str(), sizeof(data.Ffund_code)-1);
	getCloseMaxTradeDate(pMysql,data);
	trade_date_cache->tradeDate=data.Ftrans_date;
	trade_date_cache->timeout=tt + 3600 * 12;// 本地缓存12小时;

	// 删除7日前日期配置
	string oldDate = addDays(curDate, -7);
	string oldKey = oldDate+fund_code;
	gPtrConfig->m_closeMaxTradeDate.erase(oldKey);
	return trade_date_cache->tradeDate;
}

int getCloseMaxTradeDate(CMySQL* pMysql, FundCloseCycle& data)
{

    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " b.Fid,b.Fdate,b.Ffund_code,b.Ftrans_date,b.Ffirst_profit_date, "
                    " b.Fopen_date,b.Fbook_stop_date,b.Fstart_date,b.Fdue_date,b.Fprofit_end_date, "
                    " b.Flstate,b.Fmemo,b.Fcreate_time,b.Fmodify_time "
                    " FROM fund_db.t_fund_close_cycle a, fund_db.t_fund_close_cycle b"
                    " WHERE "
                    " a.Fdate='%s'  AND " 
                    " a.Ffund_code='%s' AND " 
                    " a.Ffund_code=b.Ffund_code AND " 
                    " a.Fdue_date=b.Fdue_date " 
                    " order by b.Ftrans_date desc"
                    " limit 1 ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str()
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
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Fdate,row[1] ? row[1] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrans_date,row[3] ? row[3] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[4] ? row[4] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[5] ? row[5] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[6] ? row[6] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[7] ? row[7] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fdue_date,row[8] ? row[8] : "", sizeof(data.Fdue_date) - 1);
            strncpy(data.Fprofit_end_date,row[9] ? row[9] : "", sizeof(data.Fprofit_end_date) - 1);
            data.Flstate = row[10] ? atoi(row[10]) : 0;
            strncpy(data.Fmemo,row[11] ? row[11] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[12] ? row[12] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[13] ? row[13] : "", sizeof(data.Fmodify_time) - 1);
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

string getCacheCloseMinNatureDate(CMySQL* pMysql, const string& curDate, const string& fund_code)
{
	time_t tt = time(NULL);
	string key = curDate+fund_code;
	TradeDateCache *trade_date_cache = &gPtrConfig->m_closeMinNatureDate[key];
	// 使用缓存日期
	if(trade_date_cache->timeout>tt){
		return trade_date_cache->tradeDate;
	}
    // 使用DB日期
	FundCloseCycle data;	 
	memset(&data, 0, sizeof(FundTransDate));
	strncpy(data.Fdate, curDate.c_str(), sizeof(data.Fdate)-1);
	strncpy(data.Ffund_code, fund_code.c_str(), sizeof(data.Ffund_code)-1);
	getCloseMinNatureDate(pMysql,data);
	trade_date_cache->tradeDate=data.Fdate;
	trade_date_cache->timeout=tt + 3600 * 12;// 本地缓存12小时;
	// 删除7日前日期配置
	string oldDate = addDays(curDate, -7);
	string oldKey = oldDate+fund_code;
	gPtrConfig->m_closeMaxTradeDate.erase(oldKey);
	
	return trade_date_cache->tradeDate;
}


int getCloseMinNatureDate(CMySQL* pMysql, FundCloseCycle& data)
{

    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " b.Fid,b.Fdate,b.Ffund_code,b.Ftrans_date,b.Ffirst_profit_date, "
                    " b.Fopen_date,b.Fbook_stop_date,b.Fstart_date,b.Fdue_date,b.Fprofit_end_date, "
                    " b.Flstate,b.Fmemo,b.Fcreate_time,b.Fmodify_time "
                    " FROM fund_db.t_fund_close_cycle a, fund_db.t_fund_close_cycle b"
                    " WHERE "
                    " a.Fdate='%s'  AND " 
                    " a.Ffund_code='%s' AND " 
                    " a.Ffund_code=b.Ffund_code AND " 
                    " a.Fdue_date=b.Fdue_date " 
                    " order by b.Fdate"
                    " limit 1 ",
                    pMysql->EscapeStr(data.Fdate).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str()
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
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Fdate,row[1] ? row[1] : "", sizeof(data.Fdate) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrans_date,row[3] ? row[3] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[4] ? row[4] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[5] ? row[5] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[6] ? row[6] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[7] ? row[7] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fdue_date,row[8] ? row[8] : "", sizeof(data.Fdue_date) - 1);
            strncpy(data.Fprofit_end_date,row[9] ? row[9] : "", sizeof(data.Fprofit_end_date) - 1);
            data.Flstate = row[10] ? atoi(row[10]) : 0;
            strncpy(data.Fmemo,row[11] ? row[11] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[12] ? row[12] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[13] ? row[13] : "", sizeof(data.Fmodify_time) - 1);
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

