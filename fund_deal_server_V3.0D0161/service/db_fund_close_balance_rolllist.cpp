#include "db_fund_close_balance_rolllist.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


void insertFundCloseBalanceRolllist(CMySQL* pMysql, FundCloseBalanceRolllist &data, unsigned long long&  mysqlInsertId)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_close_balance_rolllist_%d("
                    " Fclose_id,Ftrade_id,Ffund_code,Fspid,Fstart_balance,Fbalance, "
                    " Ftail_balance,Ftype,Fbiz_fee,Fbiz_tail_fee,Fsubject, "
                    " Flistid,Facc_time,Fcreate_time,Fmodify_time, "
                    " Fmemo,Fexplain,Fsign)"
                    " VALUES("
                    " %ld,'%s','%s','%s',%ld,%ld, "
                    " %ld,%d,%ld,%ld,%d, "
                    " '%s','%s','%s','%s', "
                    " '%s','%s','%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Fclose_id,
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Fstart_balance,
                    data.Fbalance,
                    data.Ftail_balance,
                    data.Ftype,
                    data.Fbiz_fee,
                    data.Fbiz_tail_fee,
                    data.Fsubject,
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fexplain).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_close_balance_rolllist", data)).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);

	mysqlInsertId = pMysql->InsertID();

}

void recordFundCloseBalanceRolllist(FundCloseTrans& data,char* listid, LONG& totalFee, LONG& tailFee, int type,int subject, unsigned long long&  mysqlInsertId)
{
	//记录流水
	FundCloseBalanceRolllist fundCloseBalanceRolllist;
	memset(&fundCloseBalanceRolllist, 0, sizeof(FundCloseBalanceRolllist));
	
	fundCloseBalanceRolllist.Fclose_id=data.Fid;
	strncpy(fundCloseBalanceRolllist.Ftrade_id, data.Ftrade_id, sizeof(fundCloseBalanceRolllist.Ftrade_id)-1);
	strncpy(fundCloseBalanceRolllist.Ffund_code, data.Ffund_code, sizeof(fundCloseBalanceRolllist.Ffund_code)-1);
	strncpy(fundCloseBalanceRolllist.Fspid, data.Fspid, sizeof(fundCloseBalanceRolllist.Fspid)-1);
	fundCloseBalanceRolllist.Fstart_balance = data.Fstart_total_fee;
	fundCloseBalanceRolllist.Fbalance = data.Fcurrent_total_fee;
	fundCloseBalanceRolllist.Ftail_balance = data.Fend_tail_fee;
	fundCloseBalanceRolllist.Ftype= type;
	fundCloseBalanceRolllist.Fbiz_fee = totalFee;
	fundCloseBalanceRolllist.Fbiz_tail_fee = tailFee;
	fundCloseBalanceRolllist.Fsubject = subject;	
	strncpy(fundCloseBalanceRolllist.Flistid, listid, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Facc_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Fcreate_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Fmodify_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	insertFundCloseBalanceRolllist(gPtrFundDB, fundCloseBalanceRolllist, mysqlInsertId);
}



