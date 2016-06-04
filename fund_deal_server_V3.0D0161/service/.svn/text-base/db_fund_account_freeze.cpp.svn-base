#include "db_fund_transfer.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

bool queryFundAccountFreeze(CMySQL* pMysql, ST_FUND_ACCOUNT_FREEZE_LOG& data,  bool lock) 
{
    //MYSQL_RES* pRes = NULL;
    //char szSql[MAX_SQL_LEN] = {0};
    //int iLen = 0, iRow = 0;
    //try
    //{
    //    iLen = snprintf(szSql, sizeof(szSql),
    //                " SELECT "
    //                " Fchange_id,Ftotal_fee,Ftrade_id,Fori_spid,Fnew_spid, "
    //                " Fori_fund_code,Fnew_fund_code,Fbuy_id,Fredem_id,Fstate, "
    //                " Fsubacc_state,Fspe_tag,Facc_time,Fbalance_fee,Fmemo,Fcreate_time, "
    //                " Fmodify_time,Fchannel_id  "
    //                " FROM fund_db_%02d.t_change_sp_record_%d "
    //                " WHERE "
    //                " Fchange_id='%s'  AND Flstate=1 " 
    //                " %s ",
    //                Sdb2(data.Fchange_id),
    //                Stb2(data.Fchange_id),
    //                pMysql->EscapeStr(data.Fchange_id).c_str(),
    //                lock ? "FOR UPDATE" : ""
    //                );
    //    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    //    // 执行查询
    //    pMysql->Query(szSql, iLen);
    //    // 取结果集
    //    pRes = pMysql->FetchResult();
    //    // 获取结果行
    //    iRow = mysql_num_rows(pRes);
    //    if(iRow <0 || iRow > 1)
    //    {
    //        throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
    //    }
    //    for(int i=0; i<iRow; i++) 
    //    {
    //        MYSQL_ROW row = mysql_fetch_row(pRes);
    //        strncpy(data.Fchange_id,row[0] ? row[0] : "", sizeof(data.Fchange_id) - 1);
    //        data.Ftotal_fee = row[1] ? atoll(row[1]) : 0;
    //        strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
    //        strncpy(data.Fori_spid,row[3] ? row[3] : "", sizeof(data.Fori_spid) - 1);
    //        strncpy(data.Fnew_spid,row[4] ? row[4] : "", sizeof(data.Fnew_spid) - 1);
    //        strncpy(data.Fori_fund_code,row[5] ? row[5] : "", sizeof(data.Fori_fund_code) - 1);
    //        strncpy(data.Fnew_fund_code,row[6] ? row[6] : "", sizeof(data.Fnew_fund_code) - 1);
    //        strncpy(data.Fbuy_id,row[7] ? row[7] : "", sizeof(data.Fbuy_id) - 1);
    //        strncpy(data.Fredem_id,row[8] ? row[8] : "", sizeof(data.Fredem_id) - 1);
    //        data.Fstate = row[9] ? atoi(row[9]) : 0;
    //        data.Fsubacc_state = row[10] ? atoi(row[10]) : 0;
    //        data.Fspe_tag= row[11] ? atoi(row[11]) : 0;
    //        strncpy(data.Facc_time,row[12] ? row[12] : "", sizeof(data.Facc_time) - 1);
    //        data.Fbalance_fee = row[13] ? atoll(row[13]) : 0;
    //        strncpy(data.Fmemo,row[14] ? row[14] : "", sizeof(data.Fmemo) - 1);
    //        strncpy(data.Fcreate_time,row[15] ? row[15] : "", sizeof(data.Fcreate_time) - 1);
    //        strncpy(data.Fmodify_time,row[16] ? row[16] : "", sizeof(data.Fmodify_time) - 1);
    //        strncpy(data.Fchannel_id,row[17] ? row[17] : "", sizeof(data.Fchannel_id) - 1);

    //    }
    //    mysql_free_result(pRes);
    //}
    //catch(CException& e)
    //{
    //    if(pRes)    mysql_free_result(pRes);
    //    throw;
    //}
    //catch( ... )
    //{
    //    if(pRes)    mysql_free_result(pRes);
    //    throw;
    //}
    //return iRow == 1;
    return true;
}



void insertFundAccountFreeze(CMySQL* pMysql, ST_FUND_ACCOUNT_FREEZE_LOG &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_account_freeze_%d("
                    " Ftrade_id,Fqqid,Fuid,Fop_type,Fcreate_time, "
                    " Fchannel_type,Fop_name)"
                    " VALUES("
                    " '%s','%s',%d,"
                    " %d,'%s',%d,'%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    data.Fuid,
                    data.Fop_type,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    data.Fchannel_type,
                    pMysql->EscapeStr(data.Fop_name).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}
