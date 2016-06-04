#include "db_fund_fetch.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 




bool queryFundFetch(CMySQL* pMysql, FundFetch& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ffetchid,Ffund_trans_id,Fcreate_time,Fmodify_time "
                    " FROM fund_db_%02d.t_fund_fetch_%d "
                    " WHERE "
                    " Ffetchid='%s' " 
                    " %s ",
                    Sdb2(data.Ffetchid),
                    Stb2(data.Ffetchid),
                    pMysql->EscapeStr(data.Ffetchid).c_str(),
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
            strncpy(data.Ffetchid,row[0] ? row[0] : "", sizeof(data.Ffetchid) - 1);
            strncpy(data.Ffund_trans_id,row[1] ? row[1] : "", sizeof(data.Ffund_trans_id) - 1);
            strncpy(data.Fcreate_time,row[2] ? row[2] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[3] ? row[3] : "", sizeof(data.Fmodify_time) - 1);

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



void insertFundFetch(CMySQL* pMysql, FundFetch &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_fetch_%d("
                    " Ffetchid,Ffund_trans_id,Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " '%s','%s','%s','%s')",
                    Sdb2(data.Ffetchid),
                    Stb2(data.Ffetchid),
                    pMysql->EscapeStr(data.Ffetchid).c_str(),
                    pMysql->EscapeStr(data.Ffund_trans_id).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


  


