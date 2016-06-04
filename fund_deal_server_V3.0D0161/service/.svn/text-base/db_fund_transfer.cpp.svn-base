#include "db_fund_transfer.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

bool queryFundTransfer(CMySQL* pMysql, ST_TRANSFER_FUND& data,  bool lock) 
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fchange_id,Ftotal_fee,Ftrade_id,Fori_spid,Fnew_spid, "
                    " Fori_fund_code,Fnew_fund_code,Fbuy_id,Fredem_id,Fstate, "
                    " Fsubacc_state,Fspe_tag,Facc_time,Fbalance_fee,Fmemo,Fcreate_time, "
                    " Fmodify_time,Fchannel_id  "
                    " FROM fund_db_%02d.t_change_sp_record_%d "
                    " WHERE "
                    " Fchange_id='%s'  AND Flstate=1 " 
                    " %s ",
                    Sdb2(data.Fchange_id),
                    Stb2(data.Fchange_id),
                    pMysql->EscapeStr(data.Fchange_id).c_str(),
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
            strncpy(data.Fchange_id,row[0] ? row[0] : "", sizeof(data.Fchange_id) - 1);
            data.Ftotal_fee = row[1] ? atoll(row[1]) : 0;
            strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fori_spid,row[3] ? row[3] : "", sizeof(data.Fori_spid) - 1);
            strncpy(data.Fnew_spid,row[4] ? row[4] : "", sizeof(data.Fnew_spid) - 1);
            strncpy(data.Fori_fund_code,row[5] ? row[5] : "", sizeof(data.Fori_fund_code) - 1);
            strncpy(data.Fnew_fund_code,row[6] ? row[6] : "", sizeof(data.Fnew_fund_code) - 1);
            strncpy(data.Fbuy_id,row[7] ? row[7] : "", sizeof(data.Fbuy_id) - 1);
            strncpy(data.Fredem_id,row[8] ? row[8] : "", sizeof(data.Fredem_id) - 1);
            data.Fstate = row[9] ? atoi(row[9]) : 0;
            data.Fsubacc_state = row[10] ? atoi(row[10]) : 0;
            data.Fspe_tag= row[11] ? atoi(row[11]) : 0;
            strncpy(data.Facc_time,row[12] ? row[12] : "", sizeof(data.Facc_time) - 1);
            data.Fbalance_fee = row[13] ? atoll(row[13]) : 0;
            strncpy(data.Fmemo,row[14] ? row[14] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[15] ? row[15] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[16] ? row[16] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fchannel_id,row[17] ? row[17] : "", sizeof(data.Fchannel_id) - 1);

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



void insertFundTransfer(CMySQL* pMysql, ST_TRANSFER_FUND &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_change_sp_record_%d("
                    " Fchange_id,Ftotal_fee,Ftrade_id,Fori_spid,Fnew_spid, "
                    " Fori_fund_code,Fnew_fund_code,Fbuy_id,Fredem_id,Fstate, "
                    " Fsubacc_state,Fspe_tag,Facc_time,Fbalance_fee,Fmemo,Fcreate_time, "
                    " Fmodify_time,Fchannel_id,Flstate)"
                    " VALUES("
                    " '%s',%zd,'%s','%s','%s', "
                    " '%s','%s','%s','%s',%d, "
                    " %d,%d,'%s',%zd,'%s', "
                    " '%s','%s','%s',%d)",
                    Sdb2(data.Fchange_id),
                    Stb2(data.Fchange_id),
                    pMysql->EscapeStr(data.Fchange_id).c_str(),
                    data.Ftotal_fee,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fori_spid).c_str(),
                    pMysql->EscapeStr(data.Fnew_spid).c_str(),
                    pMysql->EscapeStr(data.Fori_fund_code).c_str(),
                    pMysql->EscapeStr(data.Fnew_fund_code).c_str(),
                    pMysql->EscapeStr(data.Fbuy_id).c_str(),
                    pMysql->EscapeStr(data.Fredem_id).c_str(),
                    data.Fstate,
                    data.Fsubacc_state,
                    data.Fspe_tag,
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    data.Fbalance_fee,
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
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
void updateFundTransfer(CMySQL* pMysql, ST_TRANSFER_FUND& data )
{
    stringstream ss_cond;
    map<string, string> kv_map;

    char tableName[256]={0};
    snprintf(tableName, sizeof(tableName),"fund_db_%02d.t_change_sp_record_%d",Sdb2(data.Fchange_id),Stb2(data.Fchange_id));


    // 设置需要更新的字段

    kv_map["Fspe_tag"] = toString(data.Fspe_tag);

    if(data.Fstate!= 0)
    {
        kv_map["Fstate"] = toString(data.Fstate);
    }

    if(data.Fsubacc_state!= 0)
    {
        kv_map["Fsubacc_state"] = toString(data.Fsubacc_state);
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
    
    ss_cond << "Fchange_id='" << escapeString(data.Fchange_id) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, tableName, ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }

}


bool checkIfExistTransferIngBill(CMySQL* pMysql, const string&tradeId)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ftrade_id  "
                    " FROM fund_db_%02d.t_change_sp_record_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND Flstate=1 AND Fstate=1 limit 1 " 
                    ,
                    Sdb2(tradeId.c_str()),
                    Stb2(tradeId.c_str()),
                    pMysql->EscapeStr(tradeId).c_str()
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

