#include "db_fund_dynamic.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


bool queryFundDynamic(CMySQL* pMysql, ST_FUND_DYNAMIC& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ftrade_id,Fredem_day	,Fredem_times_day,Fdyn_status_mask "
                    " FROM fund_db.t_fund_dynamic "
                    " WHERE "
                    " Ftrade_id='%s'  AND Flstate=1 " 
                    " %s ",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
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
            strncpy(data.Ftrade_id,row[0] ? row[0] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fredem_day,row[1] ? row[1] : "", sizeof(data.Fredem_day) - 1);
            data.Fredem_times_day = row[2] ? atoi(row[2]) : 0;
            data.Fdyn_status_mask = row[3] ? atoi(row[3]) : 0;
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


void insertFundDynamic(CMySQL* pMysql, ST_FUND_DYNAMIC& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO  fund_db.t_fund_dynamic("
                    " Ftrade_id,Fredem_day	,Fredem_times_day,Fdyn_status_mask,Fcreate_time,"
                    " Fmodify_time,Flstate )"
                    " VALUES("
                    " '%s','%s',%d,%d,'%s', "
                    " '%s',%d)",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fredem_day).c_str(),
                    data.Fredem_times_day,
                    data.Fdyn_status_mask,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    data.Flstate
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundDynamic(CMySQL* pMysql, ST_FUND_DYNAMIC& data )
{
    stringstream ss_cond;
    map<string, string> kv_map;

    // 设置需要更新的字段

    if(data.Fredem_times_day!= 0)
    {
        kv_map["Fredem_times_day"] = toString(data.Fredem_times_day);
    }

    if(data.Fredem_day[0]!= 0)
    {
        kv_map["Fredem_day"] = data.Fredem_day;
    }
    
    if(data.Fmodify_time[0]!= 0)
    {
        kv_map["Fmodify_time"] = data.Fmodify_time;
    }
    kv_map["Fdyn_status_mask"] = toString(data.Fdyn_status_mask);
    
    ss_cond << "Ftrade_id='" << escapeString(data.Ftrade_id) << "' AND Flstate=1 ";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, "fund_db.t_fund_dynamic", ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }

}

bool setUserDynamicInfoToCkv(CMySQL* pMysql,ST_FUND_DYNAMIC& data,bool queryDb)
{
    string key = string("user_dynamic_info_")+ data.Ftrade_id;
    if (queryDb)
    {
        if (false ==queryFundDynamic(pMysql,data,false))
        {
            return false;
        }
    }

    string szValue;

    CParams kvReqSet;
    //设置要修改的数据szValue
    kvReqSet.setParam("Ftrade_id",data.Ftrade_id);
    kvReqSet.setParam("Fredem_day",data.Fredem_day);
    kvReqSet.setParam("Fredem_times_day",data.Fredem_times_day);
    kvReqSet.setParam("Fdyn_status_mask",data.Fdyn_status_mask);
    szValue = kvReqSet.pack();

    //将szValue写入ckv
    if(gCkvSvrOperator->set(CKV_KEY_USER_DYNAMIC_INFO, key, szValue))
    {
        return false;
    }
    return true;

}

