#include "fund_commfunc.h"
#include "db_fund_balance_order.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

bool queryFundBalanceOrder(CMySQL* pMysql, ST_BALANCE_ORDER& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Fspid,Ftrade_id,Ftotal_fee, Fuin,"
                    " Fuid,Ftype,Fstate,Fcreate_time, Fmodify_time,"
                    " Facc_time,Ftotal_acc_trans_id,Fsubacc_trans_id,Fcontrol_id, Fcur_type,"
                    " Fchannel_id,Fsign,Ft1fetch_date,Fflag,Fstandby1,Fstandby2,Ffetch_arrival_time,Ffetch_result  "
                    " FROM fund_db_%02d.t_fund_balance_order_%d  "
                    " WHERE "
                    " Flistid='%s' AND Ftype=%d  AND Flstate=1 " 
                    " %s ",
                    Sdb2(data.Ftrade_id),Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    data.Ftype,
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
        for (int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
            data.Ftotal_fee = row[3] ? atoll(row[3]) : 0;
            strncpy(data.Fuin,row[4] ? row[4] : "", sizeof(data.Fuin) - 1);
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Ftype = row[6] ? atoi(row[6]) : 0;
            data.Fstate = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[10] ? row[10] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Ftotal_acc_trans_id,row[11] ? row[11] : "", sizeof(data.Ftotal_acc_trans_id) - 1);
            strncpy(data.Fsubacc_trans_id,row[12] ? row[12] : "", sizeof(data.Fsubacc_trans_id) - 1);
            strncpy(data.Fcontrol_id,row[13] ? row[13] : "", sizeof(data.Fcontrol_id) - 1);
            data.Fcur_type = row[14] ? atoi(row[14]) : 0;
            strncpy(data.Fchannel_id,row[15] ? row[15] : "", sizeof(data.Fchannel_id) - 1);
            strncpy(data.Fsign,row[16] ? row[16] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Ft1fetch_date,row[17] ? row[17] : "", sizeof(data.Ft1fetch_date) - 1);
            data.Fflag = row[18] ? atoi(row[18]) : 0;
            data.Fstandby1= row[19] ? atoi(row[19]) : 0;
	     data.Fstandby2 = row[20] ? atoi(row[20]) : 0;
            strncpy(data.Ffetch_arrival_time,row[21] ? row[21] : "", sizeof(data.Ffetch_arrival_time) - 1);
            data.Ffetch_result= row[22] ? atoi(row[22]) : 0;
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
  *  根据listid查询余额提现记录
  */
bool queryFundBalanceFetchByListid(CMySQL* pMysql, ST_BALANCE_ORDER& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Fspid,Ftrade_id,Ftotal_fee, Fuin,"
                    " Fuid,Ftype,Fstate,Fcreate_time, Fmodify_time,"
                    " Facc_time,Ftotal_acc_trans_id,Fsubacc_trans_id,Fcontrol_id, Fcur_type,"
                    " Fchannel_id,Fsign,Ft1fetch_date,Fflag,Fstandby1,Fstandby2,Ffetch_arrival_time,Ffetch_result  "
                    " FROM fund_db_%02d.t_fund_balance_order_%d  "
                    " WHERE "
                    " Flistid='%s' AND Flstate=1 AND Ftype in (%d,%d) " 
                    " %s ",
                    Sdb2(data.Ftrade_id),Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    OP_TYPE_BA_FETCH,
                    OP_TYPE_BA_FETCH_T1,
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
        for (int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
            data.Ftotal_fee = row[3] ? atoll(row[3]) : 0;
            strncpy(data.Fuin,row[4] ? row[4] : "", sizeof(data.Fuin) - 1);
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Ftype = row[6] ? atoi(row[6]) : 0;
            data.Fstate = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[10] ? row[10] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Ftotal_acc_trans_id,row[11] ? row[11] : "", sizeof(data.Ftotal_acc_trans_id) - 1);
            strncpy(data.Fsubacc_trans_id,row[12] ? row[12] : "", sizeof(data.Fsubacc_trans_id) - 1);
            strncpy(data.Fcontrol_id,row[13] ? row[13] : "", sizeof(data.Fcontrol_id) - 1);
            data.Fcur_type = row[14] ? atoi(row[14]) : 0;
            strncpy(data.Fchannel_id,row[15] ? row[15] : "", sizeof(data.Fchannel_id) - 1);
            strncpy(data.Fsign,row[16] ? row[16] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Ft1fetch_date,row[17] ? row[17] : "", sizeof(data.Ft1fetch_date) - 1);
            data.Fflag = row[18] ? atoi(row[18]) : 0;
            data.Fstandby1= row[19] ? atoi(row[19]) : 0;
	     data.Fstandby2 = row[20] ? atoi(row[20]) : 0;
            strncpy(data.Ffetch_arrival_time,row[21] ? row[21] : "", sizeof(data.Ffetch_arrival_time) - 1);
            data.Ffetch_result= row[22] ? atoi(row[22]) : 0;
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



void insertFundBalanceOrder(CMySQL* pMysql, ST_BALANCE_ORDER& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO  fund_db_%02d.t_fund_balance_order_%d("
                    " Flistid,Fspid,Ftrade_id,Ftotal_fee, Fuin,"
                    " Fuid,Ftype,Fstate,Fcreate_time, Fmodify_time,"
                    " Facc_time,Ftotal_acc_trans_id,Fsubacc_trans_id,Fcontrol_id, Fcur_type,"
                    " Fchannel_id,Fsign,Fmemo,Flstate,Fflag,Fstandby1,Fstandby2,Ffetch_result)"
                    " VALUES("
                    " '%s','%s','%s',%zd,'%s', "
                    " %d,%d,%d,'%s','%s', "
                    " '%s','%s','%s','%s',%d, "
                    " '%s','%s','%s',%d,%d,%d, %d, %d)",
                    Sdb2(data.Ftrade_id),Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Ftotal_fee,
                    pMysql->EscapeStr(data.Fuin).c_str(),
                    data.Fuid,
                    data.Ftype,
                    data.Fstate,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Ftotal_acc_trans_id).c_str(),
                    pMysql->EscapeStr(data.Fsubacc_trans_id).c_str(),
                    pMysql->EscapeStr(data.Fcontrol_id).c_str(),
                    data.Fcur_type,
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_balance_order", data)).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    1,
                    data.Fflag,
                    data.Fstandby1,
                    data.Fstandby2,
                    data.Ffetch_result
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

void updateFundBalanceOrder(CMySQL* pMysql, ST_BALANCE_ORDER& data )
{
    stringstream ss_cond;
    map<string, string> kv_map;

    // 设置需要更新的字段

    if(data.Fstate!= 0)
    {
        kv_map["Fstate"] = toString(data.Fstate);
    }

    if(data.Facc_time[0]!= 0)
    {
        kv_map["Facc_time"] = data.Facc_time;
    }
    if(data.Fmodify_time[0]!= 0)
    {
        kv_map["Fmodify_time"] = data.Fmodify_time;
    }

    if(data.Fmemo[0]!= 0)
    {
        kv_map["Fmemo"] = data.Fmemo;
    }
    if(data.Fflag != 0)
    {
        kv_map["Fflag"] = toString(data.Fflag);
    }

    if(data.Ft1fetch_date[0]!= 0)
    {
        kv_map["Ft1fetch_date"] = data.Ft1fetch_date;
    }

    if(data.Fstandby2!= 0)
    {
        kv_map["Fstandby2"] = toString(data.Fstandby2);
    }

    if(data.Fbank_type != 0)
    {
        kv_map["Fbank_type"] = toString(data.Fbank_type);
    }

    if(data.Fcard_tail[0] != 0)
    {
        kv_map["Fcard_tail"] = data.Fcard_tail;
    }

    if(data.Ffetch_result!= 0)
    {
        kv_map["Ffetch_result"] = toString(data.Ffetch_result);
    }

    if(data.Ffetch_arrival_time[0] != 0)
    {
        kv_map["Ffetch_arrival_time"] = data.Ffetch_arrival_time;
    }
	
    ss_cond << "Flistid='" << escapeString(data.Flistid) << "' AND Ftype= "<<data.Ftype;
    int affect_row =0;
    char tableName[128]={0};
    snprintf(tableName,sizeof(tableName),"fund_db_%02d.t_fund_balance_order_%d",Sdb2(data.Ftrade_id),Stb2(data.Ftrade_id));

    // 执行更新数据表操作
    affect_row = UpdateTable(pMysql, tableName, ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}



void insertOrderUserRelation(CMySQL* pMysql, ST_ORDER_USER_RELA& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO  fund_db_%02d.t_fund_order_to_user_%d("
                    " Flistid,Ftype,Ftrade_id,Fcreate_time, Fmodify_time)"
                    " VALUES("
                    " '%s',%d,'%s','%s','%s')",
                    Sdb2(data.Flistid),Stb2(data.Flistid),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    data.Ftype,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

bool queryOrderUserRelation(CMySQL* pMysql, ST_ORDER_USER_RELA& data )
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Ftype,Ftrade_id "
                    " FROM fund_db_%02d.t_fund_order_to_user_%d  "
                    " WHERE "
                    " Flistid='%s' AND Ftype=%d  limit 1",
                    Sdb2(data.Flistid),Stb2(data.Flistid),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    data.Ftype
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
        for (int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
            data.Ftype = row[1] ? atoi(row[1]) : 0;
            strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
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
 * 统计用户充值未完成的金额
 */
LONG getChargeRecordsFee(CMySQL* mysql,const string& trade_id, const string &cond)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    LONG total_fee=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_fund_balance_order_%d "
                    " WHERE "
                    " Ftrade_id='%s' AND %s ",
                    Sdb2(trade_id.c_str()),
                    Stb2(trade_id.c_str()),
                    escapeString(trade_id).c_str(),
                    cond.c_str());
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        total_fee = row[0] ? atoll(row[0]) : 0;

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
    return total_fee;
}


