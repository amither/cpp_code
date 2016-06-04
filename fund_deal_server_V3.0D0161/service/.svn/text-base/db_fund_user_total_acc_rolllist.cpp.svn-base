#include "db_fund_user_total_acc_rolllist.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
// 连接基金数据库句柄
extern CMySQL* gPtrFundDB;



void insertFundUserTotalAccRolllist(CMySQL* pMysql, FundUserTotalAccRolllist &data, unsigned long long&  mysqlInsertId)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_user_total_acc_rolllist_%d("
                    " Flistid,Fspid,Ftrade_id,Fqqid,Fbusiness_type,Ftype,Fbank_type, "
                    " Fbalance,Ffreeze,Fpaynum,Ffreezenum,Fsubject, "
                    " Fcurtype,Flogin_ip,Fsign,Fcreate_time,Fmodify_time, "
                    " Fmemo,Fexplain)"
                    " VALUES("
                    " '%s','%s','%s','%s',%d,%d,%d, "
                    " %zd,%zd,%zd,%zd,%d, "
                    " %d,'%s','%s','%s','%s', "
                    " '%s','%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    data.Fbusiness_type,
                    data.Ftype,
                    data.Fbank_type,
                    data.Fbalance,
                    data.Ffreeze,
                    data.Fpaynum,
                    data.Ffreezenum,
                    data.Fsubject,
                    data.Fcurtype,
                    pMysql->EscapeStr(data.Flogin_ip).c_str(),
                    pMysql->EscapeStr(data.Fsign).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fexplain).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);

	mysqlInsertId = pMysql->InsertID();

}


  


