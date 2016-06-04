#include "db_fund_recon_log.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


bool queryFundReconLog(CMySQL* pMysql, FundReconLog& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Frecon_type,Fspid,Frecon_date,Frecon_state, "
                    " Fsuccess_count,Fsuccess_money,Fmemo,Fcreate_time, "
                    " Fmodify_time "
                    " FROM fund_db.t_fund_recon_log "
                    " WHERE "
                    " Frecon_type=%d  AND " 
                    " Frecon_date='%s' AND " 
                    " Frecon_state = %d "
                    " %s ",
                    data.Frecon_type,
                    pMysql->EscapeStr(data.Frecon_date).c_str(),
                    data.Frecon_state,
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
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            data.Frecon_type = row[1] ? atoi(row[1]) : 0;
            strncpy(data.Fspid,row[2] ? row[2] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Frecon_date,row[3] ? row[3] : "", sizeof(data.Frecon_date) - 1);
            data.Frecon_state = row[4] ? atoi(row[4]) : 0;
            data.Fsuccess_count = row[5] ? atoi(row[5]) : 0;
            data.Fsuccess_money = row[6] ? atoll(row[6]) : 0;
            strncpy(data.Fmemo,row[7] ? row[7] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);

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



void insertFundReconLog(CMySQL* pMysql, FundReconLog &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_recon_log("
                    " Frecon_type,Fspid,Frecon_date,Frecon_state,Fsuccess_count, "
                    " Fsuccess_money,Fmemo,Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " %d,'%s','%s',%d,%d, "
                    " %zd,'%s','%s','%s')",
                    data.Frecon_type,
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Frecon_date).c_str(),
                    data.Frecon_state,
                    data.Fsuccess_count,
                    data.Fsuccess_money,
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


  

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundReconLog(CMySQL* pMysql, FundReconLog& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_fund_recon_log SET "
                    " Frecon_state=%d,"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Fimt_id=%zd", 
                    data.Frecon_state,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    data.Fimt_id
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

bool queryFundSpReconLog(CMySQL* pMysql, FundReconLog& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Frecon_type,Fspid,Frecon_date,Frecon_state, "
                    " Fsuccess_count,Fsuccess_money,Fmemo,Fcreate_time, "
                    " Fmodify_time "
                    " FROM fund_db.t_fund_recon_log "
                    " WHERE "
                    " Frecon_type=%d  AND " 
                    " Frecon_date='%s' AND "
                    " Fspid='%s' AND "
                    " Frecon_state = 2 "
                    " %s ",
                    data.Frecon_type,
                    pMysql->EscapeStr(data.Frecon_date).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
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
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            data.Frecon_type = row[1] ? atoi(row[1]) : 0;
            strncpy(data.Fspid,row[2] ? row[2] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Frecon_date,row[3] ? row[3] : "", sizeof(data.Frecon_date) - 1);
            data.Frecon_state = row[4] ? atoi(row[4]) : 0;
            data.Fsuccess_count = row[5] ? atoi(row[5]) : 0;
            data.Fsuccess_money = row[6] ? atoll(row[6]) : 0;
            strncpy(data.Fmemo,row[7] ? row[7] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);

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
 * 查询是否所有对账都完成,存在不等于2状态的即未完成


bool queryFundReconLogFinishAll(CMySQL* pMysql, FundReconLog& data) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	int count=1;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT count(1) "
                    " FROM fund_db.t_fund_recon_log "
                    " WHERE "
                    " Frecon_type=%d  AND " 
                    " Frecon_date='%s' AND " 
                    " Frecon_state <> %d ",
                    data.Frecon_type,
                    pMysql->EscapeStr(data.Frecon_date).c_str(),
                    RECON_STATE_FINISH
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
		MYSQL_ROW row = mysql_fetch_row(pRes);
		count = row[0] ? atoll(row[0]) : 1;
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
    return count == 0;
}

*/



