
#include "fund_commfunc.h"
#include "db_unfreeze_fund.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


bool queryFundUnFreezeByUnfreezeid(CMySQL* pMysql, ST_UNFREEZE_FUND& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ffreeze_id,Ftotal_fee,Fcur_type,Ftrade_id, Fuid,"
                    " Ffund_spid,Ffund_code,Funfreeze_type,Fspid, Fsub_acc_unfreeze_no,"
                    " Fcoding,Funfreeze_id,Fstate,Flstate, Fsub_acc_draw_no,"
                    " Fsub_acc_control_no,Fcreate_time, Fmodify_time,Facc_time,Fchannel_id,"
                    " Fsign,Fredem_id,Fpay_trans_id,Fcontrol_fee "
                    " FROM fund_db.t_unfreeze_fund  "
                    " WHERE "
                    " Funfreeze_id='%s'  AND Flstate=1 " 
                    " %s ",
                    pMysql->EscapeStr(data.Funfreeze_id).c_str(),
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
            strncpy(data.Ffreeze_id,row[0] ? row[0] : "", sizeof(data.Ffreeze_id) - 1);
            data.Ftotal_fee = row[1] ? atoll(row[1]) : 0;
            data.Fcur_type = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Ftrade_id,row[3] ? row[3] : "", sizeof(data.Ftrade_id) - 1);
            data.Fuid = row[4] ? atoi(row[4]) : 0;
            strncpy(data.Ffund_spid,row[5] ? row[5] : "", sizeof(data.Ffund_spid) - 1);
            strncpy(data.Ffund_code,row[6] ? row[6] : "", sizeof(data.Ffund_code) - 1);
            data.Funfreeze_type = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fspid,row[8] ? row[8] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsub_acc_unfreeze_no,row[9] ? row[9] : "", sizeof(data.Fsub_acc_unfreeze_no) - 1);
            strncpy(data.Fcoding,row[10] ? row[10] : "", sizeof(data.Fcoding) - 1);
            strncpy(data.Funfreeze_id,row[11] ? row[11] : "", sizeof(data.Funfreeze_id) - 1);
            data.Fstate = row[12] ? atoi(row[12]) : 0;
            data.Flstate = row[13] ? atoi(row[13]) : 0;
            strncpy(data.Fsub_acc_draw_no,row[14] ? row[14] : "", sizeof(data.Fsub_acc_draw_no) - 1);
            strncpy(data.Fsub_acc_control_no,row[15] ? row[15] : "", sizeof(data.Fsub_acc_control_no) - 1);
            strncpy(data.Fcreate_time,row[16] ? row[16] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[17] ? row[17] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[18] ? row[18] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fchannel_id,row[19] ? row[19] : "", sizeof(data.Fchannel_id) - 1);
            strncpy(data.Fsign,row[20] ? row[20] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fredem_id,row[21] ? row[21] : "", sizeof(data.Fredem_id) - 1);
            strncpy(data.Fpay_trans_id,row[22] ? row[22] : "", sizeof(data.Fpay_trans_id) - 1);
            data.Fcontrol_fee= row[23] ? atoll(row[23]) : 0;
            checkSign( "t_unfreeze_fund", data);
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

bool queryFundUnFreeze(CMySQL* pMysql, ST_UNFREEZE_FUND& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ffreeze_id,Ftotal_fee,Fcur_type,Ftrade_id, Fuid,"
                    " Ffund_spid,Ffund_code,Funfreeze_type,Fspid, Fsub_acc_unfreeze_no,"
                    " Fcoding,Funfreeze_id,Fstate,Flstate, Fsub_acc_draw_no,"
                    " Fsub_acc_control_no,Fcreate_time, Fmodify_time,Facc_time,Fchannel_id,"
                    " Fsign,Fredem_id,Fpay_trans_id,Fcontrol_fee "
                    " FROM fund_db.t_unfreeze_fund  "
                    " WHERE "
                    " Fspid='%s' AND Fcoding='%s'  AND Flstate=1 " 
                    " %s ",
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Fcoding).c_str(),
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
            strncpy(data.Ffreeze_id,row[0] ? row[0] : "", sizeof(data.Ffreeze_id) - 1);
            data.Ftotal_fee = row[1] ? atoll(row[1]) : 0;
            data.Fcur_type = row[2] ? atoi(row[2]) : 0;
            strncpy(data.Ftrade_id,row[3] ? row[3] : "", sizeof(data.Ftrade_id) - 1);
            data.Fuid = row[4] ? atoi(row[4]) : 0;
            strncpy(data.Ffund_spid,row[5] ? row[5] : "", sizeof(data.Ffund_spid) - 1);
            strncpy(data.Ffund_code,row[6] ? row[6] : "", sizeof(data.Ffund_code) - 1);
            data.Funfreeze_type = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fspid,row[8] ? row[8] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsub_acc_unfreeze_no,row[9] ? row[9] : "", sizeof(data.Fsub_acc_unfreeze_no) - 1);
            strncpy(data.Fcoding,row[10] ? row[10] : "", sizeof(data.Fcoding) - 1);
            strncpy(data.Funfreeze_id,row[11] ? row[11] : "", sizeof(data.Funfreeze_id) - 1);
            data.Fstate = row[12] ? atoi(row[12]) : 0;
            data.Flstate = row[13] ? atoi(row[13]) : 0;
            strncpy(data.Fsub_acc_draw_no,row[14] ? row[14] : "", sizeof(data.Fsub_acc_draw_no) - 1);
            strncpy(data.Fsub_acc_control_no,row[15] ? row[15] : "", sizeof(data.Fsub_acc_control_no) - 1);
            strncpy(data.Fcreate_time,row[16] ? row[16] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[17] ? row[17] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[18] ? row[18] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fchannel_id,row[19] ? row[19] : "", sizeof(data.Fchannel_id) - 1);
            strncpy(data.Fsign,row[20] ? row[20] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fredem_id,row[21] ? row[21] : "", sizeof(data.Fredem_id) - 1);
            strncpy(data.Fpay_trans_id,row[22] ? row[22] : "", sizeof(data.Fpay_trans_id) - 1);
            data.Fcontrol_fee= row[23] ? atoll(row[23]) : 0;
            checkSign( "t_unfreeze_fund", data);
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


void insertFundUnFreeze(CMySQL* pMysql, ST_UNFREEZE_FUND& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO  fund_db.t_unfreeze_fund("
                 " Ffreeze_id,Ftotal_fee,Fcur_type,Ftrade_id, Fuid,"
                    " Ffund_spid,Ffund_code,Funfreeze_type,Fspid, Fsub_acc_unfreeze_no,"
                    " Fcoding,Funfreeze_id,Fstate,Flstate, Fsub_acc_draw_no,"
                    " Fsub_acc_control_no,Fcreate_time, Fmodify_time,Facc_time,Fchannel_id,"
                    " Fsign,Fmemo,Fredem_id,Fpay_trans_id,Fcontrol_fee,"
                    " Fqqid,Fstandby3) "
                    " VALUES("
                    " '%s',%zd,%d,'%s',%d, "
                    " '%s','%s',%d,'%s','%s', "
                    " '%s','%s',%d,%d,'%s', "
                    " '%s','%s','%s','%s','%s', "
                    " '%s','%s','%s','%s',%zd,"
                    " '%s','%s')",
                    pMysql->EscapeStr(data.Ffreeze_id).c_str(),
                    data.Ftotal_fee,
                    data.Fcur_type,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fuid,
                    pMysql->EscapeStr(data.Ffund_spid).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    data.Funfreeze_type,
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Fsub_acc_unfreeze_no).c_str(),
                    pMysql->EscapeStr(data.Fcoding).c_str(),
                    pMysql->EscapeStr(data.Funfreeze_id).c_str(),
                    data.Fstate,
                    data.Flstate,
                    pMysql->EscapeStr(data.Fsub_acc_draw_no).c_str(),
                    pMysql->EscapeStr(data.Fsub_acc_control_no).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
                    pMysql->EscapeStr(genSign("t_unfreeze_fund", data)).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fredem_id).c_str(),
                    pMysql->EscapeStr(data.Fpay_trans_id).c_str(),
                    data.Fcontrol_fee,
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    pMysql->EscapeStr(data.Fstandby3).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

//参数说明: ST_UNFREEZE_FUND& dbData --- 更新前，当前DB的最新记录，且必须包含t_unfreeze_fund签名字段如下:
//              Funfreeze_id、Ftrade_id、Ftotal_fee、Fcontrol_fee、Fstate为最新DB值用于计算sign签名，
//              其他字段值可暂不关心
void updateFundUnFreeze(CMySQL* pMysql, ST_UNFREEZE_FUND& data, ST_UNFREEZE_FUND& dbData )
{
    stringstream ss_cond;
    map<string, string> kv_map;

    // 设置需要更新的字段
    bool bNeedSetDbSign = false;

    if(data.Fstate!= 0)
    {
        kv_map["Fstate"] = toString(data.Fstate);
        bNeedSetDbSign = true;
    }

    
    if(data.Fmodify_time[0]!= 0)
    {
        kv_map["Fmodify_time"] = data.Fmodify_time;
    }
    if(data.Facc_time[0]!= 0)
    {
        kv_map["Facc_time"] = data.Facc_time;
    }
    if(data.Fredem_id[0]!= 0)
    {
        kv_map["Fredem_id"] = data.Fredem_id;
    }
    if(data.Fpay_trans_id[0]!= 0)
    {
        kv_map["Fpay_trans_id"] = data.Fpay_trans_id;
    }
    
    if( bNeedSetDbSign ){
        kv_map["Fsign"] = genMergeSign("t_unfreeze_fund", data, dbData);
    }
    
    ss_cond << "Funfreeze_id='" << escapeString(data.Funfreeze_id) << "' AND Flstate=1 ";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, "fund_db.t_unfreeze_fund", ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }

}


