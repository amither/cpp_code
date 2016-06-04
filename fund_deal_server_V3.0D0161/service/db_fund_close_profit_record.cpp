#include "db_fund_close_profit_record.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

bool queryFundCloseProfitRecord(CMySQL* pMysql, FundCloseProfitRecord& data,  bool lock) //查询一条记录
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
					" Flistid,Fclose_id,Ftrade_id,Fend_date,Fday, "
                    " Ffund_code,Ftotal_fee,Fprofit,Fend_tail_profit,Fprofit_type, "
                    " F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign, "
                    " Fcreate_time,Fmodify_time"
                    " FROM fund_db_%02d.t_fund_close_profit_record_%d "
                    " WHERE "
                    " Fclose_id='%ld' and Fday='%s'" 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Fclose_id,
                    pMysql->EscapeStr(data.Fday).c_str(),
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
            int j=0;
            strncpy(data.Flistid,row[j] ? row[j] : "", sizeof(data.Flistid) - 1);
            data.Fclose_id = row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ftrade_id,row[++j] ? row[j] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fend_date,row[++j] ? row[j] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fday,row[++j] ? row[j] : "", sizeof(data.Fday) - 1);
            strncpy(data.Ffund_code,row[++j] ? row[j] : "", sizeof(data.Ffund_code) - 1);
            data.Ftotal_fee = row[++j] ? atoll(row[j]) : 0;
            data.Fprofit = row[++j] ? atoll(row[j]) : 0;
            data.Fend_tail_profit = row[++j] ? atoll(row[j]) : 0;
            data.Fprofit_type = row[++j] ? atoi(row[j]) : 0;
            data.F1day_profit_rate = row[++j] ? atoll(row[j]) : 0;
            data.F7day_profit_rate = row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Flogin_ip,row[++j] ? row[j] : "", sizeof(data.Flogin_ip) - 1);
            strncpy(data.Fsign,row[++j] ? row[j] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fcreate_time,row[++j] ? row[j] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[++j] ? row[j] : "", sizeof(data.Fmodify_time) - 1);

            checkSign( "t_fund_close_profit_record", data);
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

//查询昨日对账余额
LONG statFundCloseReconBalance(CMySQL* pMysql, const string& date, const char* trade_id, const LONG closeId, const LONG lastCloseId)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	char closeIdSql[128];
	LONG sum;
	if(lastCloseId>0){
		snprintf(closeIdSql,sizeof(closeIdSql),"(%ld,%ld)",closeId,lastCloseId);
	}else{
		snprintf(closeIdSql,sizeof(closeIdSql),"(%ld)",closeId);
	}
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT sum(Ftotal_fee)"
                    " FROM fund_db_%02d.t_fund_close_profit_record_%d "
                    " WHERE "
                    " Fclose_id in %s and Fday='%s'",
                    Sdb2(trade_id),
                    Stb2(trade_id),
                    closeIdSql,
                    pMysql->EscapeStr(date).c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <=0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        MYSQL_ROW row = mysql_fetch_row(pRes);
		sum=row[0]?atoll(row[0]):0;
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
    return sum;
}


void insertFundCloseProfitRecord(CMySQL* pMysql, FundCloseProfitRecord &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " REPLACE INTO fund_db_%02d.t_fund_close_profit_record_%d("
                    " Flistid,Fclose_id,Ftrade_id,Fend_date,Fday, "
                    " Ffund_code,Ftotal_fee,Fprofit,Fend_tail_profit,Fprofit_type, "
                    " F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign, "
                    " Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " '%s',%ld,'%s','%s','%s', "
                    " '%s',%ld,%ld,%ld,%d, "
                    " %ld,%ld,'%s','%s', "
                    " '%s','%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    data.Fclose_id,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fend_date).c_str(),
                    pMysql->EscapeStr(data.Fday).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    data.Ftotal_fee,
                    data.Fprofit,
                    data.Fend_tail_profit,
                    data.Fprofit_type,
                    data.F1day_profit_rate,
                    data.F7day_profit_rate,
                    pMysql->EscapeStr(data.Flogin_ip).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_close_profit_record", data)).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}



