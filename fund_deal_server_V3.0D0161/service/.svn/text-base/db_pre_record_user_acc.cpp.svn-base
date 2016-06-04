
#include "db_pre_record_user_acc.h"
#include "fund_commfunc.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
 
bool queryPreUserAcc(CMySQL* pMysql, ST_PREUSER_ACC& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;

    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Facc_id,Facc_type,Fuin,Fchannel_id, Fbusiness_type "
                    " FROM fund_db.t_fund_pre_bind_acc  "
                    " WHERE "
                    " Facc_id='%s'  AND Facc_type=%d " 
                    " %s ",
                    pMysql->EscapeStr(data.Facc_id).c_str(),
                    data.Facc_type,
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
            strncpy(data.Facc_id,row[0] ? row[0] : "", sizeof(data.Facc_id) - 1);
            data.Facc_type = row[1] ? atoi(row[1]) : 0;
            strncpy(data.Fuin,row[2] ? row[2] : "", sizeof(data.Fuin) - 1);
            strncpy(data.Fchannel_id,row[3] ? row[3] : "", sizeof(data.Fchannel_id) - 1);
            strncpy(data.Fbusiness_type,row[4] ? row[4] : "", sizeof(data.Fbusiness_type) - 1);
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

void insertPreUserAcc(CMySQL* pMysql, ST_PREUSER_ACC& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_pre_bind_acc("
                    " Facc_id,Facc_type,Fuin,Fchannel_id, Fbusiness_type,"
                    " Fcreate_time, Fmodify_time )"
                    " VALUES("
                    " '%s',%d,'%s','%s','%s', "
                    " '%s','%s')",
                    pMysql->EscapeStr(data.Facc_id).c_str(),
                    data.Facc_type,
                    pMysql->EscapeStr(data.Fuin).c_str(),
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
                    pMysql->EscapeStr(data.Fbusiness_type).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

int updatePreUserAcc(CMySQL* pMysql, ST_PREUSER_ACC& data)
{
    stringstream ss_cond;
    map<string, string> kv_map;

    // 设置需要更新的字段

    if(data.Fuin[0]!= 0)
    {
        kv_map["Fuin"] = data.Fuin;
    }
    if(data.Fchannel_id[0]!= 0)
    {
        kv_map["Fchannel_id"] = data.Fchannel_id;
    }
    
    if(data.Fbusiness_type[0]!= 0)
    {
        kv_map["Fbusiness_type"] = data.Fbusiness_type;
    }

    if(data.Fmodify_time[0]!= 0)
    {
        kv_map["Fmodify_time"] = data.Fmodify_time;
    }
    
    ss_cond << "Facc_id='" << escapeString(data.Facc_id) << "' AND Facc_type= "<< data.Facc_type;
    int affect_row =0;

    // 执行更新数据表操作
    affect_row = UpdateTable(pMysql, "fund_db.t_fund_pre_bind_acc", ss_cond, kv_map);

    return affect_row;
}



