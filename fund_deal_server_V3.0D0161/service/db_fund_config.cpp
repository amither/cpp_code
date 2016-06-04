#include "db_fund_config.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 






bool queryFundConfig(CMySQL* pMysql, FundConfig& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fkeyword,Fvalue,FcreateTime,FmodifyTime,Fmemo "
                    " FROM fund_db.t_fund_config "
                    " WHERE "
                    " Fkeyword='%s' " 
                    " %s ",
                    pMysql->EscapeStr(data.Fkeyword).c_str(),
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
            strncpy(data.Fkeyword,row[0] ? row[0] : "", sizeof(data.Fkeyword) - 1);
            strncpy(data.Fvalue,row[1] ? row[1] : "", sizeof(data.Fvalue) - 1);
            strncpy(data.FcreateTime,row[2] ? row[2] : "", sizeof(data.FcreateTime) - 1);
            strncpy(data.FmodifyTime,row[3] ? row[3] : "", sizeof(data.FmodifyTime) - 1);
            strncpy(data.Fmemo,row[4] ? row[4] : "", sizeof(data.Fmemo) - 1);

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



void insertFundConfig(CMySQL* pMysql, FundConfig & data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " REPLACE INTO fund_db.t_fund_config("
                    " Fkeyword,Fvalue,FcreateTime,FmodifyTime,Fmemo )"
                    " VALUES("
                    " '%s','%s','%s','%s','%s' )",
                    pMysql->EscapeStr(data.Fkeyword).c_str(),
                    pMysql->EscapeStr(data.Fvalue).c_str(),
                    pMysql->EscapeStr(data.FcreateTime).c_str(),
                    pMysql->EscapeStr(data.FmodifyTime).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

  


