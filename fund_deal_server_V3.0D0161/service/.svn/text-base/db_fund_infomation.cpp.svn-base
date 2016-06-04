#include "db_fund_infomation.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


bool queryInfomation(CMySQL* pMysql, FundInfomation & data, bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT info_id,title,link,info_time,create_time,state,state_after_audit,published"
                    " FROM fund_db.t_infomation_log where info_id='%s' %s ",
                    data.info_id,
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
            strncpy(data.info_id ,row[0] ? row[0] : "", sizeof(data.info_id ) - 1);
            strncpy(data.title,row[1] ? row[1] : "", sizeof(data.title) - 1);
            strncpy(data.link,row[2] ? row[2] : "", sizeof(data.link) - 1);
            strncpy(data.info_time,row[3] ? row[3] : "", sizeof(data.info_time) - 1);
            strncpy(data.create_time,row[4] ? row[4] : "", sizeof(data.create_time) - 1);
            data.state = row[5] ? atoi(row[5]) : 0;
            data.state_after_audit = row[6] ? atoi(row[6]) : 0;
            data.published = row[7] ? atoi(row[7]) : 0;
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

void insertInfomation(CMySQL* pMysql, FundInfomation& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_infomation_log("
                    " info_id,title,link,info_time,source,html_content,create_time,state,state_after_audit)"
                    " VALUES('%s','%s','%s','%s','%s','%s','%s','0','0')",
                    pMysql->EscapeStr(data.info_id).c_str(),
                    pMysql->EscapeStr(data.title).c_str(),
                    pMysql->EscapeStr(data.link).c_str(),
                    pMysql->EscapeStr(data.info_time).c_str(),
                    pMysql->EscapeStr(data.source).c_str(),
                    pMysql->EscapeStr(data.html_content).c_str(),
                    pMysql->EscapeStr(data.create_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}
void updateInfomationState(CMySQL* pMysql, FundInfomation& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_infomation_log SET "
                    " state=%d,"
                    " state_after_audit=%d,"
                    " published=%d"
                    " WHERE"
                    " info_id='%s'", 
                    data.state,
                    data.state_after_audit,
                    data.published,
                    //--------where条件--------
                    pMysql->EscapeStr(data.info_id).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (!(pMysql->AffectedRows() == 1||pMysql->AffectedRows()==0))
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void updateInfomationHtmlContent(CMySQL* pMysql, FundInfomation& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_infomation_log SET "
                    " html_content='%s'"
                    " WHERE"
                    " info_id='%s'", 
                    pMysql->EscapeStr(data.html_content).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.info_id).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (!(pMysql->AffectedRows() == 1||pMysql->AffectedRows()==0))
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void AddToHtmlContent(CMySQL* pMysql, const string &info_id, const string & html_content)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_infomation_log SET "
                    " html_content= CONCAT(html_content,'%s')"
                    " WHERE"
                    " info_id='%s'", 
                    pMysql->EscapeStr(html_content).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(info_id).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (!(pMysql->AffectedRows() == 1))
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void updateInfomationTitle(CMySQL* pMysql, FundInfomation& data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_infomation_log SET "
                    " title='%s'"
                    " WHERE"
                    " info_id='%s'", 
                    pMysql->EscapeStr(data.title).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.info_id).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (!(pMysql->AffectedRows() == 1||pMysql->AffectedRows()==0))
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}