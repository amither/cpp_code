
#include "db_freeze_fund.h"
#include "fund_commfunc.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
 
bool queryFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    char szCond[MAX_SQL_LEN] = {0};
    if (data.Ffreeze_id[0] !=0)
    {
        snprintf(szCond,sizeof(szCond),"Ffreeze_id='%s' ", 
            pMysql->EscapeStr(data.Ffreeze_id).c_str());
    }
    else
    {
        snprintf(szCond,sizeof(szCond),"Fspid='%s' AND Fcoding='%s' ", 
            pMysql->EscapeStr(data.Fspid).c_str(),pMysql->EscapeStr(data.Fcoding).c_str());
    }
    
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ffreeze_id,Ftotal_fee,Fcur_type,Ftrade_id, Fuid,"
                    " Ffund_spid,Ffund_code,Ffreeze_type,Fspid, Fsub_acc_freeze_no,"
                    " Fcoding,Fbuy_id,Fstate,Flstate, Fsep_tag,"
                    " Ftotal_unfreeze_fee,Fcre_id,Ftrue_name,Fcreate_time, Fmodify_time,"
                    " Facc_time,Fchannel_id,Fsign,Fqqid,Fpay_type,Fpurpose,Fcre_type,Fpre_card_no,Fpre_card_partner "
                    " FROM fund_db.t_freeze_fund  "
                    " WHERE "
                    " %s  AND Flstate=1 " 
                    " %s ",
                    szCond,
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
            data.Ffreeze_type = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fspid,row[8] ? row[8] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Fsub_acc_freeze_no,row[9] ? row[9] : "", sizeof(data.Fsub_acc_freeze_no) - 1);
            strncpy(data.Fcoding,row[10] ? row[10] : "", sizeof(data.Fcoding) - 1);
            strncpy(data.Fbuy_id,row[11] ? row[11] : "", sizeof(data.Fbuy_id) - 1);
            data.Fstate = row[12] ? atoi(row[12]) : 0;
            data.Flstate = row[13] ? atoi(row[13]) : 0;
            data.Fsep_tag = row[14] ? atoi(row[14]) : 0;
            data.Ftotal_unfreeze_fee = row[15] ?atoll(row[15]) : 0;
            strncpy(data.Fcre_id,row[16] ? row[16] : "", sizeof(data.Fcre_id) - 1);
            strncpy(data.Ftrue_name,row[17] ? row[17] : "", sizeof(data.Ftrue_name) - 1);
            strncpy(data.Fcreate_time,row[18] ? row[18] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[19] ? row[19] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[20] ? row[20] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fchannel_id,row[21] ? row[21] : "", sizeof(data.Fchannel_id) - 1);
            strncpy(data.Fsign,row[22] ? row[22] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fqqid,row[23] ? row[23] : "", sizeof(data.Fqqid) - 1);
            data.Fpay_type= row[24] ? atoi(row[24]) : 0;
            strncpy(data.Fpurpose,row[25] ? row[25] : "", sizeof(data.Fpurpose) - 1);
            data.Fcre_type = row[26] ? atoi(row[26]) : 0;
            strncpy(data.Fpre_card_no,row[27] ? row[27] : "", sizeof(data.Fpre_card_no) - 1);
            strncpy(data.Fpre_card_partner,row[28] ? row[28] : "", sizeof(data.Fpre_card_partner) - 1);
            checkSign("t_freeze_fund", data);
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


void insertFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO  fund_db.t_freeze_fund("
                    " Ffreeze_id,Ftotal_fee,Fcur_type,Ftrade_id, Fuid,"
                    " Ffund_spid,Ffund_code,Ffreeze_type,Fspid, Fsub_acc_freeze_no,"
                    " Fcoding,Fbuy_id,Fstate,Flstate, Fsep_tag,"
                    " Ftotal_unfreeze_fee,Fcre_id,Ftrue_name,Fcreate_time, Fmodify_time,"
                    " Facc_time,Fchannel_id,Fsign,Fqqid,Fpay_type,Fpurpose,Fcre_type,Fpre_card_no,Fpre_card_partner )"
                    " VALUES("
                    " '%s',%zd,%d,'%s',%d, "
                    " '%s','%s',%d,'%s','%s', "
                    " '%s','%s',%d,%d,%d, "
                    " %zd,'%s','%s','%s','%s', "
                    " '%s','%s','%s','%s',%d,'%s',%d,'%s','%s')",
                    pMysql->EscapeStr(data.Ffreeze_id).c_str(),
                    data.Ftotal_fee,
                    data.Fcur_type,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fuid,
                    pMysql->EscapeStr(data.Ffund_spid).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    data.Ffreeze_type,
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Fsub_acc_freeze_no).c_str(),
                    pMysql->EscapeStr(data.Fcoding).c_str(),
                    pMysql->EscapeStr(data.Fbuy_id).c_str(),
                    data.Fstate,
                    data.Flstate,
                    data.Fsep_tag,
                    data.Ftotal_unfreeze_fee,
                    pMysql->EscapeStr(data.Fcre_id).c_str(),
                    pMysql->EscapeStr(data.Ftrue_name).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
                    pMysql->EscapeStr(genSign("t_freeze_fund", data)).c_str(),
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    data.Fpay_type,
                    pMysql->EscapeStr(data.Fpurpose).c_str(),
                    data.Fcre_type,
                    pMysql->EscapeStr(data.Fpre_card_no).c_str(),
                    pMysql->EscapeStr(data.Fpre_card_partner).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

//参数说明: ST_FREEZE_FUND& dbData --- 更新前，当前DB的最新记录，必须包含t_freeze_fund签名字段:
//              Ffreeze_id、Ftrade_id、Fuid、Ftotal_fee、Fstate、Ftotal_unfreeze_fee为最新DB值用于计算sign签名，
//              其他字段值可暂不关心
int updateFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data, ST_FREEZE_FUND& dbData)
{
    stringstream ss_cond;
    map<string, string> kv_map;

    bool bNeedSetDbSign = false;

    if(data.Fstate!= 0)
    {
        kv_map["Fstate"] = toString(data.Fstate);
        bNeedSetDbSign = true;
    }
    if(data.Fuid!= 0)
    {
        kv_map["Fuid"] = toString(data.Fuid);
        bNeedSetDbSign = true;
    }
    if(data.Ftrade_id[0]!= 0)
    {
        kv_map["Ftrade_id"] = data.Ftrade_id;
        bNeedSetDbSign = true;
    }
    if(data.Ftotal_unfreeze_fee!= 0)
    {
        kv_map["Ftotal_unfreeze_fee"] = toString(data.Ftotal_unfreeze_fee);
        bNeedSetDbSign = true;
    }
    
    if(data.Fsub_acc_freeze_no[0]!= 0)
    {
        kv_map["Fsub_acc_freeze_no"] = data.Fsub_acc_freeze_no;
    }
    if(data.Facc_time[0]!= 0)
    {
        kv_map["Facc_time"] = data.Facc_time;
    }
    if(data.Fmodify_time[0]!= 0)
    {
        kv_map["Fmodify_time"] = data.Fmodify_time;
    }

    if( bNeedSetDbSign ){
        kv_map["Fsign"] = genMergeSign("t_freeze_fund", data, dbData);
    }
    
    ss_cond << "Ffreeze_id='" << escapeString(data.Ffreeze_id) << "' AND Flstate=1 ";
    int affect_row =0;

    // 执行更新数据表操作
    affect_row = UpdateTable(pMysql, "fund_db.t_freeze_fund", ss_cond, kv_map);

    return affect_row;
}


