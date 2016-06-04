#include "db_fund_close_trans.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig;
extern CMySQL* gPtrFundDB;

/*
*query返回多行数据函数
*/
bool queryFundCloseTransForRegProfit(CMySQL* pMysql,const string& trade_id, const string& fund_code,const string& date, vector< FundCloseTrans>& dataVec,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	int offset=0;
	int limit=gPtrConfig->m_AppCfg.close_buy_date_upper_limit;

	string acc_end_time;
	if(gPtrConfig->m_AppCfg.trans_recon_type == 1){
		acc_end_time = changeDateFormat(date) + " 14:59:59";	
	}else{
		acc_end_time = changeDateFormat(date) + " 23:59:59";
	}
	string max_trade_date=getCacheCloseMaxTradeDate(gPtrFundSlaveDB, date, fund_code);
	string last_recon_date=addDays(date,-1);
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " date_format(Fprofit_recon_date,'%%Y%%m%%d'),Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Ffund_code='%s' AND " 
                    " Flstate <> %d AND " 
                    " Fprofit_end_date>='%s' AND " 
                    " Ftrans_date<='%s'  AND "
                    " Facc_time<='%s' AND " 
                    " Fprofit_recon_date in ('00000000','%s') " 
                    " LIMIT %d, %d "
                    " %s ",
                    Sdb2(trade_id.c_str()),
                    Stb2(trade_id.c_str()),
                    pMysql->EscapeStr(trade_id).c_str(),
                    pMysql->EscapeStr(fund_code).c_str(),
                    LSTATE_INVALID,
                    pMysql->EscapeStr(date).c_str(),
                    pMysql->EscapeStr(max_trade_date).c_str(),
                    pMysql->EscapeStr(acc_end_time).c_str(),
                    pMysql->EscapeStr(last_recon_date).c_str(),
                    offset,limit,
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        for(int i=0; i<iRow; i++) 
        {
            FundCloseTrans data;
            
            memset(&data, 0, sizeof(data));
            MYSQL_ROW row = mysql_fetch_row(pRes);
            
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Fspid,row[3] ? row[3] : "", sizeof(data.Fspid) - 1);
            data.Fseqno = row[4] ? atoi(row[4]) : 0;
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Fpay_type = row[6] ? atoi(row[6]) : 0;
            data.Flastid = row[7] ? atoll(row[7]) : 0;
            data.Fstart_total_fee = row[8] ? atoll(row[8]) : 0;
            data.Fcurrent_total_fee = row[9] ? atoll(row[9]) : 0;
            data.Fend_tail_fee = row[10] ? atoll(row[10]) : 0;
            data.Fuser_end_type = row[11] ? atoi(row[11]) : 0;
            data.Fend_sell_type = row[12] ? atoi(row[12]) : 0;
            data.Fend_plan_amt = row[13] ? atoll(row[13]) : 0;
            data.Fend_real_buy_amt = row[14] ? atoll(row[14]) : 0;
            data.Fend_real_sell_amt = row[15] ? atoll(row[15]) : 0;
            strncpy(data.Fpresell_listid,row[16] ? row[16] : "", sizeof(data.Fpresell_listid) - 1);
            strncpy(data.Fsell_listid,row[17] ? row[17] : "", sizeof(data.Fsell_listid) - 1);
            strncpy(data.Fend_listid_1,row[18] ? row[18] : "", sizeof(data.Fend_listid_1) - 1);
            strncpy(data.Fend_listid_2,row[19] ? row[19] : "", sizeof(data.Fend_listid_2) - 1);
            strncpy(data.Fend_transfer_fundcode,row[20] ? row[20] : "", sizeof(data.Fend_transfer_fundcode) - 1);
            strncpy(data.Fend_transfer_spid,row[21] ? row[21] : "", sizeof(data.Fend_transfer_spid) - 1);
            strncpy(data.Ftrans_date,row[22] ? row[22] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[23] ? row[23] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[24] ? row[24] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[25] ? row[25] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[26] ? row[26] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[27] ? row[27] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fprofit_end_date,row[28] ? row[28] : "", sizeof(data.Fprofit_end_date) - 1);
            strncpy(data.Fchannel_id,row[29] ? row[29] : "", sizeof(data.Fchannel_id) - 1);
            data.Fstate = row[30] ? atoi(row[30]) : 0;
            data.Flstate = row[31] ? atoi(row[31]) : 0;
            strncpy(data.Fcreate_time,row[32] ? row[32] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[33] ? row[33] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[34] ? row[34] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[35] ? row[35] : "", sizeof(data.Fexplain) - 1);
            strncpy(data.Fsign,row[36] ? row[36] : "", sizeof(data.Fsign) - 1);
			strncpy(data.Facc_time,row[37] ? row[37] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fprofit_recon_date,row[38] ? row[38] : "", sizeof(data.Fprofit_recon_date) - 1);
			data.Flast_profit = row[39]?atoll(row[39]):0;
            strncpy(data.Fdue_date,row[40] ? row[40] : "", sizeof(data.Fdue_date) - 1);
			data.Ftotal_profit = row[41]?atoll(row[41]):0;
            strncpy(data.Flastids,row[42] ? row[42] : "", sizeof(data.Flastids) - 1);
            checkSign( "t_fund_close_trans", data);
            dataVec.push_back(data);
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

	return iRow >= 1;
}

/*
*query返回有效的定期交易个数
*/
int queryValidFundCloseTransCount(CMySQL* pMysql, const FundCloseTrans &where, const string &trade_date)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	int count = 0;
    try
    {
        // 计算个数的时候，对于处于开放日的期次不计算在内
        // 计算个数的时候，对于完成周期的期次不计算在内
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT count(1)"
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Ffund_code='%s' AND " 
                    " Flstate <> %d AND " 
                    " Fprofit_end_date>='%s'  AND"  
                    " Fopen_date<>'%s'  AND"
                    " Fstate<>5 AND Fstate<>6" , 
                    Sdb2(where.Ftrade_id),
                    Stb2(where.Ftrade_id),
                    pMysql->EscapeStr(where.Ftrade_id).c_str(),
                    pMysql->EscapeStr(where.Ffund_code).c_str(),
                    LSTATE_INVALID,
                    pMysql->EscapeStr(trade_date).c_str(),
                    pMysql->EscapeStr(trade_date).c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        MYSQL_ROW row = mysql_fetch_row(pRes);            
        count = row[0] ? atoi(row[0]) : 0;
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

	return count;
}


/*
* query返回多行数据函数
*
* 此接口只能定期更新CKV调用
*/
bool queryFundCloseTransWithProfitEndDate(CMySQL* pMysql,int offset,int limit,FundCloseTrans &where,vector< FundCloseTrans>& dataVec,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;

    string last_date = addDays(where.Fprofit_end_date,-1);
    
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " date_format(Fprofit_recon_date,'%%Y%%m%%d'),Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Ffund_code='%s' AND " 
                    " Flstate <> %d AND " 
                    " Fprofit_end_date>='%s'  " 
                    " LIMIT %d, %d "
                    " %s ",
                    Sdb2(where.Ftrade_id),
                    Stb2(where.Ftrade_id),
                    pMysql->EscapeStr(where.Ftrade_id).c_str(),
                    pMysql->EscapeStr(where.Ffund_code).c_str(),
                    LSTATE_INVALID,
                    pMysql->EscapeStr(last_date).c_str(),
                    offset,limit,
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        for(int i=0; i<iRow; i++) 
        {
            FundCloseTrans data;
            
            memset(&data, 0, sizeof(data));
            MYSQL_ROW row = mysql_fetch_row(pRes);
            
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Fspid,row[3] ? row[3] : "", sizeof(data.Fspid) - 1);
            data.Fseqno = row[4] ? atoi(row[4]) : 0;
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Fpay_type = row[6] ? atoi(row[6]) : 0;
            data.Flastid = row[7] ? atoll(row[7]) : 0;
            data.Fstart_total_fee = row[8] ? atoll(row[8]) : 0;
            data.Fcurrent_total_fee = row[9] ? atoll(row[9]) : 0;
            data.Fend_tail_fee = row[10] ? atoll(row[10]) : 0;
            data.Fuser_end_type = row[11] ? atoi(row[11]) : 0;
            data.Fend_sell_type = row[12] ? atoi(row[12]) : 0;
            data.Fend_plan_amt = row[13] ? atoll(row[13]) : 0;
            data.Fend_real_buy_amt = row[14] ? atoll(row[14]) : 0;
            data.Fend_real_sell_amt = row[15] ? atoll(row[15]) : 0;
            strncpy(data.Fpresell_listid,row[16] ? row[16] : "", sizeof(data.Fpresell_listid) - 1);
            strncpy(data.Fsell_listid,row[17] ? row[17] : "", sizeof(data.Fsell_listid) - 1);
            strncpy(data.Fend_listid_1,row[18] ? row[18] : "", sizeof(data.Fend_listid_1) - 1);
            strncpy(data.Fend_listid_2,row[19] ? row[19] : "", sizeof(data.Fend_listid_2) - 1);
            strncpy(data.Fend_transfer_fundcode,row[20] ? row[20] : "", sizeof(data.Fend_transfer_fundcode) - 1);
            strncpy(data.Fend_transfer_spid,row[21] ? row[21] : "", sizeof(data.Fend_transfer_spid) - 1);
            strncpy(data.Ftrans_date,row[22] ? row[22] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[23] ? row[23] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[24] ? row[24] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[25] ? row[25] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[26] ? row[26] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[27] ? row[27] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fprofit_end_date,row[28] ? row[28] : "", sizeof(data.Fprofit_end_date) - 1);
            strncpy(data.Fchannel_id,row[29] ? row[29] : "", sizeof(data.Fchannel_id) - 1);
            data.Fstate = row[30] ? atoi(row[30]) : 0;
            data.Flstate = row[31] ? atoi(row[31]) : 0;
            strncpy(data.Fcreate_time,row[32] ? row[32] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[33] ? row[33] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[34] ? row[34] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[35] ? row[35] : "", sizeof(data.Fexplain) - 1);
            strncpy(data.Fsign,row[36] ? row[36] : "", sizeof(data.Fsign) - 1);
			strncpy(data.Facc_time,row[37] ? row[37] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fprofit_recon_date,row[38] ? row[38] : "", sizeof(data.Fprofit_recon_date) - 1);
			data.Flast_profit = row[39]?atoll(row[39]):0;
            strncpy(data.Fdue_date,row[40] ? row[40] : "", sizeof(data.Fdue_date) - 1);
			data.Ftotal_profit = row[41]?atoll(row[41]):0;
            strncpy(data.Flastids,row[42] ? row[42] : "", sizeof(data.Flastids) - 1);
            checkSign( "t_fund_close_trans", data);
            dataVec.push_back(data);
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

	return iRow >= 1;
}

/**
* 查询指定基金的定期记录
*/
bool queryFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " date_format(Fprofit_recon_date,'%%Y%%m%%d'),Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Fid= %ld " 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Fid,
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
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Fspid,row[3] ? row[3] : "", sizeof(data.Fspid) - 1);
            data.Fseqno = row[4] ? atoi(row[4]) : 0;
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Fpay_type = row[6] ? atoi(row[6]) : 0;
            data.Flastid = row[7] ? atoll(row[7]) : 0;
            data.Fstart_total_fee = row[8] ? atoll(row[8]) : 0;
            data.Fcurrent_total_fee = row[9] ? atoll(row[9]) : 0;
            data.Fend_tail_fee = row[10] ? atoll(row[10]) : 0;
            data.Fuser_end_type = row[11] ? atoi(row[11]) : 0;
            data.Fend_sell_type = row[12] ? atoi(row[12]) : 0;
            data.Fend_plan_amt = row[13] ? atoll(row[13]) : 0;
            data.Fend_real_buy_amt = row[14] ? atoll(row[14]) : 0;
            data.Fend_real_sell_amt = row[15] ? atoll(row[15]) : 0;
            strncpy(data.Fpresell_listid,row[16] ? row[16] : "", sizeof(data.Fpresell_listid) - 1);
            strncpy(data.Fsell_listid,row[17] ? row[17] : "", sizeof(data.Fsell_listid) - 1);
            strncpy(data.Fend_listid_1,row[18] ? row[18] : "", sizeof(data.Fend_listid_1) - 1);
            strncpy(data.Fend_listid_2,row[19] ? row[19] : "", sizeof(data.Fend_listid_2) - 1);
            strncpy(data.Fend_transfer_fundcode,row[20] ? row[20] : "", sizeof(data.Fend_transfer_fundcode) - 1);
            strncpy(data.Fend_transfer_spid,row[21] ? row[21] : "", sizeof(data.Fend_transfer_spid) - 1);
            strncpy(data.Ftrans_date,row[22] ? row[22] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[23] ? row[23] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[24] ? row[24] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[25] ? row[25] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[26] ? row[26] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[27] ? row[27] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fprofit_end_date,row[28] ? row[28] : "", sizeof(data.Fprofit_end_date) - 1);
            strncpy(data.Fchannel_id,row[29] ? row[29] : "", sizeof(data.Fchannel_id) - 1);
            data.Fstate = row[30] ? atoi(row[30]) : 0;
            data.Flstate = row[31] ? atoi(row[31]) : 0;
            strncpy(data.Fcreate_time,row[32] ? row[32] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[33] ? row[33] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[34] ? row[34] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[35] ? row[35] : "", sizeof(data.Fexplain) - 1);
            strncpy(data.Fsign,row[36] ? row[36] : "", sizeof(data.Fsign) - 1);
			strncpy(data.Facc_time,row[37] ? row[37] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fprofit_recon_date,row[38] ? row[38] : "", sizeof(data.Fprofit_recon_date) - 1);
			data.Flast_profit = row[39]?atoll(row[39]):0;
            strncpy(data.Fdue_date,row[40] ? row[40] : "", sizeof(data.Fdue_date) - 1);
			data.Ftotal_profit = row[41]?atoll(row[41]):0;
            strncpy(data.Flastids,row[42] ? row[42] : "", sizeof(data.Flastids) - 1);
            checkSign( "t_fund_close_trans", data);
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
* 查询指定基金的最新一条记录
*/
bool queryLatestFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " date_format(Fprofit_recon_date,'%%Y%%m%%d'),Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Ffund_code='%s' " 
                    " order by Fseqno desc limit 1"
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
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
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Fspid,row[3] ? row[3] : "", sizeof(data.Fspid) - 1);
            data.Fseqno = row[4] ? atoi(row[4]) : 0;
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Fpay_type = row[6] ? atoi(row[6]) : 0;
            data.Flastid = row[7] ? atoll(row[7]) : 0;
            data.Fstart_total_fee = row[8] ? atoll(row[8]) : 0;
            data.Fcurrent_total_fee = row[9] ? atoll(row[9]) : 0;
            data.Fend_tail_fee = row[10] ? atoll(row[10]) : 0;
            data.Fuser_end_type = row[11] ? atoi(row[11]) : 0;
            data.Fend_sell_type = row[12] ? atoi(row[12]) : 0;
            data.Fend_plan_amt = row[13] ? atoll(row[13]) : 0;
            data.Fend_real_buy_amt = row[14] ? atoll(row[14]) : 0;
            data.Fend_real_sell_amt = row[15] ? atoll(row[15]) : 0;
            strncpy(data.Fpresell_listid,row[16] ? row[16] : "", sizeof(data.Fpresell_listid) - 1);
            strncpy(data.Fsell_listid,row[17] ? row[17] : "", sizeof(data.Fsell_listid) - 1);
            strncpy(data.Fend_listid_1,row[18] ? row[18] : "", sizeof(data.Fend_listid_1) - 1);
            strncpy(data.Fend_listid_2,row[19] ? row[19] : "", sizeof(data.Fend_listid_2) - 1);
            strncpy(data.Fend_transfer_fundcode,row[20] ? row[20] : "", sizeof(data.Fend_transfer_fundcode) - 1);
            strncpy(data.Fend_transfer_spid,row[21] ? row[21] : "", sizeof(data.Fend_transfer_spid) - 1);
            strncpy(data.Ftrans_date,row[22] ? row[22] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[23] ? row[23] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[24] ? row[24] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[25] ? row[25] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[26] ? row[26] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[27] ? row[27] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fprofit_end_date,row[28] ? row[28] : "", sizeof(data.Fprofit_end_date) - 1);
            strncpy(data.Fchannel_id,row[29] ? row[29] : "", sizeof(data.Fchannel_id) - 1);
            data.Fstate = row[30] ? atoi(row[30]) : 0;
            data.Flstate = row[31] ? atoi(row[31]) : 0;
            strncpy(data.Fcreate_time,row[32] ? row[32] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[33] ? row[33] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[34] ? row[34] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[35] ? row[35] : "", sizeof(data.Fexplain) - 1);
            strncpy(data.Fsign,row[36] ? row[36] : "", sizeof(data.Fsign) - 1);
			strncpy(data.Facc_time,row[37] ? row[37] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fprofit_recon_date,row[38] ? row[38] : "", sizeof(data.Fprofit_recon_date) - 1);
			data.Flast_profit = row[39]?atoll(row[39]):0;
            strncpy(data.Fdue_date,row[40] ? row[40] : "", sizeof(data.Fdue_date) - 1);
			data.Ftotal_profit = row[41]?atoll(row[41]):0;
            strncpy(data.Flastids,row[42] ? row[42] : "", sizeof(data.Flastids) - 1);
            checkSign( "t_fund_close_trans", data);
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



void insertFundCloseTrans(CMySQL* pMysql, FundCloseTrans &data, unsigned long long&  mysqlInsertId )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_close_trans_%d("
                    " Ftrade_id,Ffund_code,Fspid,Fseqno,Fuid, "
                    " Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee,Fend_tail_fee, "
                    " Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt,Fend_real_sell_amt, "
                    " Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2,Fend_transfer_fundcode, "
                    " Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date,Fbook_stop_date, "
                    " Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id,Fstate, "
                    " Flstate,Fcreate_time,Fmodify_time,Fmemo,Fexplain, "
                    " Fsign,Facc_time,Fprofit_recon_date,Flast_profit,Fdue_date,Ftotal_profit,Flastids)"
                    " VALUES("
                    " '%s','%s','%s',%d,%d, "
                    " %d,%ld,%ld,%ld,%ld, "
                    " %d,%d,%ld,%ld,%ld, "
                    " '%s','%s','%s','%s','%s', "
                    " '%s','%s','%s','%s','%s', "
                    " '%s','%s','%s','%s',%d, "
                    " %d,'%s','%s','%s','%s', "
                    " '%s','%s' ,'%s',%ld,'%s',%ld,'%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Fseqno,
                    data.Fuid,
                    data.Fpay_type,
                    data.Flastid,
                    data.Fstart_total_fee,
                    data.Fcurrent_total_fee,
                    data.Fend_tail_fee,
                    data.Fuser_end_type,
                    data.Fend_sell_type,
                    data.Fend_plan_amt,
                    data.Fend_real_buy_amt,
                    data.Fend_real_sell_amt,
                    pMysql->EscapeStr(data.Fpresell_listid).c_str(),
                    pMysql->EscapeStr(data.Fsell_listid).c_str(),
                    pMysql->EscapeStr(data.Fend_listid_1).c_str(),
                    pMysql->EscapeStr(data.Fend_listid_2).c_str(),
                    pMysql->EscapeStr(data.Fend_transfer_fundcode).c_str(),
                    pMysql->EscapeStr(data.Fend_transfer_spid).c_str(),
                    pMysql->EscapeStr(data.Ftrans_date).c_str(),
                    pMysql->EscapeStr(data.Ffirst_profit_date).c_str(),
                    pMysql->EscapeStr(data.Fopen_date).c_str(),
                    pMysql->EscapeStr(data.Fbook_stop_date).c_str(),
                    pMysql->EscapeStr(data.Fstart_date).c_str(),
                    pMysql->EscapeStr(data.Fend_date).c_str(),
                    pMysql->EscapeStr(data.Fprofit_end_date).c_str(),
                    pMysql->EscapeStr(data.Fchannel_id).c_str(),
                    data.Fstate,
                    data.Flstate,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fexplain).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_close_trans", data)).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Fprofit_recon_date).c_str(),
                    data.Flast_profit,
                    pMysql->EscapeStr(data.Fdue_date).c_str(),
                    data.Ftotal_profit,
                    pMysql->EscapeStr(data.Flastids).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);

	mysqlInsertId = pMysql->InsertID();
}

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常

void updateFundCloseTrans(CMySQL* pMysql, FundCloseTrans& data )
{
	stringstream tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    string trade_id = escapeString(data.Ftrade_id);
    
    tb_name << "fund_db_" << trade_id.substr(trade_id.size() - 2);
    tb_name << ".t_fund_close_trans_" << trade_id.substr(trade_id.size() - 3, 1);

    // 设置需要更新的字段
    kv_map["Fmodify_time"] = data.Fmodify_time;
	
	if(data.Fpay_type!= MIN_INTEGER&&data.Fpay_type!=0)
	{
		kv_map["Fpay_type"] = toString(data.Fpay_type);
	}
	if(data.Fstart_total_fee!= MIN_INTEGER&&data.Fstart_total_fee!=0)
	{
		kv_map["Fstart_total_fee"] = toString(data.Fstart_total_fee);
	}
	if(data.Fcurrent_total_fee!= MIN_INTEGER&&data.Fcurrent_total_fee!=0)
	{
		kv_map["Fcurrent_total_fee"] = toString(data.Fcurrent_total_fee);
    	genSign(data);
		kv_map["Fsign"] = data.Fsign;
	}
	if(data.Fuser_end_type!= MIN_INTEGER&&data.Fuser_end_type!=0)
	{
		kv_map["Fuser_end_type"] = toString(data.Fuser_end_type);
	}
	if(data.Fend_sell_type!= MIN_INTEGER&&data.Fend_sell_type!=0)
	{
		kv_map["Fend_sell_type"] = toString(data.Fend_sell_type);
	}
	if(data.Fend_plan_amt!= MIN_INTEGER&&data.Fend_plan_amt!=0)
	{
		kv_map["Fend_plan_amt"] = toString(data.Fend_plan_amt);
	}
	if(data.Flastid!= MIN_INTEGER&&data.Flastid!=0)
	{
		kv_map["Flastid"] = toString(data.Flastid);
	}
	if(!(0 == strcmp("", data.Fend_transfer_spid)))
	{
		kv_map["Fend_transfer_spid"] = data.Fend_transfer_spid;
	}
	if(!(0 == strcmp("", data.Fend_transfer_fundcode)))
	{
		kv_map["Fend_transfer_fundcode"] = data.Fend_transfer_fundcode;
	}
	if(!(0 == strcmp("", data.Fsign)))
	{
		kv_map["Fsign"] = data.Fsign;
	}
	if(!(0 == strcmp("", data.Fpresell_listid)))
	{
		kv_map["Fpresell_listid"] = data.Fpresell_listid;
	}
	if(!(0 == strcmp("", data.Fprofit_recon_date)))
	{
		kv_map["Fprofit_recon_date"] = data.Fprofit_recon_date;
		kv_map["Flast_profit"] = toString(data.Flast_profit);
		kv_map["Ftotal_profit"] = toString(data.Ftotal_profit);
	}
	if(!(0 == strcmp("", data.Fmemo)))
	{
		kv_map["Fmemo"] = data.Fmemo;
	}
    
    ss_cond << "Ftrade_id='" << trade_id <<"' ";
	ss_cond << " AND Ffund_code='" << toString(data.Ffund_code)<<"' ";
	ss_cond << " AND Fseqno=" << toString(data.Fseqno);

    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, tb_name.str(), ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
	
}
*/  

/**
*  获取用户最新一期seqno
*/
int checkPermissionBuyCloseFund(FundCloseTrans& data, const FundSpConfig& fundSpConfig, string trans_date, string close_due_date, bool lock)
{
	// 检查是否存在相同期次
	strncpy(data.Fend_date,close_due_date.c_str(), sizeof(data.Fend_date)-1);	
	if(queryFundCloseTransByEndDate(gPtrFundDB,data,lock))
	{
		if(data.Flstate != LSTATE_VALID){
			// 所在期次已经有无效或冻结记录，不可以购买
			throw CException(ERR_NO_PERMISSION_PURCHASE_CLOSE_FUND, "close_trans is invalid.", __FILE__, __LINE__);
		}
		return data.Fseqno;
	}
	
	// 查询最近一期
	if(!queryLatestFundCloseTrans(gPtrFundDB, data, lock))
	{
		//没有记录
		int default_seqno = 1;//默认开始序列
		return default_seqno;
	}
	
	if(data.Flstate == LSTATE_FREEZE){
		//上期记录被冻结,不可以购买
		throw CException(ERR_NO_PERMISSION_PURCHASE_CLOSE_FUND, "last close_trans is freezed.", __FILE__, __LINE__);
	}else if(data.Flstate == LSTATE_INVALID){
		// 上期记录无效,以新期次购买
		return data.Fseqno+1;
	}

	//上个期次的开放日，或者已超过结束期的，可以再次购买
	if(trans_date == data.Fopen_date || trans_date > data.Fend_date)
	{
		return data.Fseqno +1;
	}
	
	if(!(fundSpConfig.Frestrict_mode & RESTRICT_MODE_BUY) || data.Fseqno < fundSpConfig.Frestrict_num)
	{
		//不限制购买次数，或者未达到次数限制
		return data.Fseqno + 1;
	}

	if(fundSpConfig.Frestrict_num <=1)
	{
		//超过购买次数限制
		throw CException(ERR_NO_PERMISSION_PURCHASE_CLOSE_FUND, "over buy limit.", __FILE__, __LINE__);
	}

	// 检查用户购买次数
	int validTransCount = queryValidFundCloseTransCount(gPtrFundDB, data, trans_date);
	if(validTransCount >= fundSpConfig.Frestrict_num)
	{
		throw CException(ERR_NO_PERMISSION_PURCHASE_CLOSE_FUND, "over buy limit.", __FILE__, __LINE__);
	}
	else
	{
		return data.Fseqno +1;
	}

}


void checkPermissionBuyCloseFund(const string trade_id, const FundSpConfig& fundSpConfig, const string systime, bool lock)
{
	//非封闭产品不检查
	if(fundSpConfig.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}

	// 各类日期计算
	FundCloseCycle fundCloseCycle;
	memset(&fundCloseCycle, 0, sizeof(FundCloseCycle));

	strncpy(fundCloseCycle.Fdate, calculateFundDate(systime).c_str(), sizeof(fundCloseCycle.Fdate) - 1);
	strncpy(fundCloseCycle.Ffund_code, fundSpConfig.Ffund_code, sizeof(fundCloseCycle.Ffund_code) - 1);
	bool hasCycle = queryFundCloseCycle(gPtrFundDB, fundCloseCycle, false);
	if(!hasCycle){
		//日期配置不正确
		alert(ERR_UNFOUND_TRADE_DATE,"cannot find close cycle:"+string(fundSpConfig.Ffund_code));
		throw CException(ERR_UNFOUND_TRADE_DATE, "cannot find close cycle", __FILE__, __LINE__);
	}
	
	FundCloseTrans fundCloseTrans;
	memset(&fundCloseTrans, 0, sizeof(FundCloseTrans));
	
	strncpy(fundCloseTrans.Ftrade_id, trade_id.c_str(), sizeof(fundCloseTrans.Ftrade_id) - 1);
	strncpy(fundCloseTrans.Ffund_code, fundSpConfig.Ffund_code, sizeof(fundCloseTrans.Ffund_code) - 1);
	
	checkPermissionBuyCloseFund(fundCloseTrans, fundSpConfig, fundCloseCycle.Ftrans_date,fundCloseCycle.Fdue_date, lock);
}



/**
*设置cache
*/
bool setFundCloseTransToKV(const string &trade_id, const string &fund_code)
{
	FundCloseTrans fundCloseTrans;
	memset(&fundCloseTrans, 0, sizeof(FundCloseTrans));
	
	strncpy(fundCloseTrans.Ftrade_id, trade_id.c_str(), sizeof(fundCloseTrans.Ftrade_id) - 1);
	strncpy(fundCloseTrans.Ffund_code, fund_code.c_str(), sizeof(fundCloseTrans.Ffund_code) - 1);
	//用当前时间作为查询条件，过了0点的才无效
	strncpy(fundCloseTrans.Fprofit_end_date, toString(GetDateToday()).c_str(), sizeof(fundCloseTrans.Fprofit_end_date) - 1);

    vector<FundCloseTrans> fundCloseTransVec;
	if(!queryFundCloseTransWithProfitEndDate(gPtrFundDB, 0, CACHE_CLOSE_TRANS_MAX_NUM, fundCloseTrans, fundCloseTransVec, false))
	{
		TRACE_DEBUG("no close fund trans.");

        return true;        
	}
    
	setFundCloseTransToKV(fundCloseTransVec);
	return true;
}

/**
 * 根据数据组装定期记录ckv的值
 * @param list 记录列表
 * @param value ckv中存储的值
 * @return 0-成功，其他-失败
 */
int packFundCloseTransCkvValue(const vector< FundCloseTrans> &list, string &value)
{
    CParams kvReqSet;
	
    //设置要修改的数据szValue
    vector<FundCloseTrans>::size_type i= 0;
    for(i= 0; i != list.size() && i < CACHE_CLOSE_TRANS_MAX_NUM; ++i)
    {
		const FundCloseTrans &data= list[i];
		
		kvReqSet.setParam(string("Fid_"+toString(i)).c_str(), data.Fid);
		kvReqSet.setParam(string("Ftrade_id_"+toString(i)).c_str(), data.Ftrade_id);
		kvReqSet.setParam(string("Ffund_code_"+toString(i)).c_str(), data.Ffund_code);
		kvReqSet.setParam(string("Fspid_"+toString(i)).c_str(), data.Fspid);
		kvReqSet.setParam(string("Fseqno_"+toString(i)).c_str(), data.Fseqno);
		kvReqSet.setParam(string("Fuid_"+toString(i)).c_str(), data.Fuid);
		kvReqSet.setParam(string("Fpay_type_"+toString(i)).c_str(), data.Fpay_type);
		kvReqSet.setParam(string("Flastid_"+toString(i)).c_str(), data.Flastid);
		kvReqSet.setParam(string("Fstart_total_fee_"+toString(i)).c_str(), data.Fstart_total_fee);
		kvReqSet.setParam(string("Fcurrent_total_fee_"+toString(i)).c_str(), data.Fcurrent_total_fee);
		kvReqSet.setParam(string("Fend_tail_fee_"+toString(i)).c_str(), data.Fend_tail_fee);
		kvReqSet.setParam(string("Fuser_end_type_"+toString(i)).c_str(), data.Fuser_end_type);
		kvReqSet.setParam(string("Fend_sell_type_"+toString(i)).c_str(), data.Fend_sell_type);
		kvReqSet.setParam(string("Fend_plan_amt_"+toString(i)).c_str(), data.Fend_plan_amt);
		kvReqSet.setParam(string("Fend_real_buy_amt_"+toString(i)).c_str(), data.Fend_real_buy_amt);
		kvReqSet.setParam(string("Fend_real_sell_amt_"+toString(i)).c_str(), data.Fend_real_sell_amt);
		kvReqSet.setParam(string("Fpresell_listid_"+toString(i)).c_str(), data.Fpresell_listid);
		kvReqSet.setParam(string("Fsell_listid_"+toString(i)).c_str(), data.Fsell_listid);
		kvReqSet.setParam(string("Fend_listid_1_"+toString(i)).c_str(), data.Fend_listid_1);
		kvReqSet.setParam(string("Fend_listid_2_"+toString(i)).c_str(), data.Fend_listid_2);
		kvReqSet.setParam(string("Fend_transfer_fundcode_"+toString(i)).c_str(), data.Fend_transfer_fundcode);
		kvReqSet.setParam(string("Fend_transfer_spid_"+toString(i)).c_str(), data.Fend_transfer_spid);
		kvReqSet.setParam(string("Facc_time_"+toString(i)).c_str(), data.Facc_time);
		kvReqSet.setParam(string("Ftrans_date_"+toString(i)).c_str(), data.Ftrans_date);
		kvReqSet.setParam(string("Ffirst_profit_date_"+toString(i)).c_str(), data.Ffirst_profit_date);
		kvReqSet.setParam(string("Fopen_date_"+toString(i)).c_str(), data.Fopen_date);
		kvReqSet.setParam(string("Fbook_stop_date_"+toString(i)).c_str(), data.Fbook_stop_date);
		kvReqSet.setParam(string("Fstart_date_"+toString(i)).c_str(), data.Fstart_date);
		kvReqSet.setParam(string("Fend_date_"+toString(i)).c_str(), data.Fend_date);
		kvReqSet.setParam(string("Fprofit_end_date_"+toString(i)).c_str(), data.Fprofit_end_date);
		kvReqSet.setParam(string("Fchannel_id_"+toString(i)).c_str(), data.Fchannel_id);
		kvReqSet.setParam(string("Fstate_"+toString(i)).c_str(), data.Fstate);
		kvReqSet.setParam(string("Flstate_"+toString(i)).c_str(), data.Flstate);
		kvReqSet.setParam(string("Fcreate_time_"+toString(i)).c_str(), data.Fcreate_time);
		kvReqSet.setParam(string("Fmodify_time_"+toString(i)).c_str(), data.Fmodify_time);
		kvReqSet.setParam(string("Fmemo_"+toString(i)).c_str(), data.Fmemo);
		kvReqSet.setParam(string("Fprofit_recon_date_"+toString(i)).c_str(), data.Fprofit_recon_date);
		kvReqSet.setParam(string("Flast_profit_"+toString(i)).c_str(), data.Flast_profit);
		kvReqSet.setParam(string("Fdue_date_"+toString(i)).c_str(), data.Fdue_date);
		kvReqSet.setParam(string("Ftotal_profit_"+toString(i)).c_str(), data.Ftotal_profit);
		kvReqSet.setParam(string("Flastids_"+toString(i)).c_str(), data.Flastids);

    }


	kvReqSet.setParam("total_num",(int)(i));
	if(list.size() >= CACHE_CLOSE_TRANS_MAX_NUM)
	{
		kvReqSet.setParam("isMore", "1"); //标识有更多数据在数据库中
	}
	else
	{
		kvReqSet.setParam("isMore", "0");
	}
    
    value = kvReqSet.pack();

    return 0;
}

/**
 * 解析CKV中用户定期交易记录
 * @param value ckv中了字串
 * @param list 解析后的定期交易记录
 * @return 0-成功，其他-失败
 */
int parseFundCloseTransCkvValue(const string &value, vector< FundCloseTrans> &list)
{   
    if (value.empty())
        return -1;
    
    CParams kvRspGet;
    kvRspGet.parse(value);
    //如果total_num字段不存在表示数据格式有问题
    if (!kvRspGet.isExists("total_num"))
        return -2;
    
    int total_num = atoi(kvRspGet.getString("total_num").c_str());
	    
    for(int i = 0; i < total_num; i++)
	{
		FundCloseTrans data;
        //已有构造函数,无需调用memset
		//memset(&data,0,sizeof(data));
		
		data.Fid = kvRspGet.getLong(string("Fid_"+toString(i)).c_str());
		strncpy(data.Ftrade_id, kvRspGet.getString(string("Ftrade_id_" + toString(i)).c_str()).c_str(), sizeof(data.Ftrade_id) - 1);
		strncpy(data.Ffund_code, kvRspGet.getString(string("Ffund_code_" + toString(i)).c_str()).c_str(), sizeof(data.Ffund_code) - 1);
		strncpy(data.Fspid, kvRspGet.getString(string("Fspid_" + toString(i)).c_str()).c_str(), sizeof(data.Fspid) - 1);
		data.Fseqno = kvRspGet.getInt(string("Fseqno_"+toString(i)).c_str());
		data.Fuid = kvRspGet.getInt(string("Fuid_"+toString(i)).c_str());
		data.Fpay_type = kvRspGet.getInt(string("Fpay_type_"+toString(i)).c_str());
		data.Flastid = kvRspGet.getLong(string("Flastid_"+toString(i)).c_str());
		data.Fstart_total_fee = kvRspGet.getLong(string("Fstart_total_fee_"+toString(i)).c_str());
		data.Fcurrent_total_fee = kvRspGet.getLong(string("Fcurrent_total_fee_"+toString(i)).c_str());
		data.Fend_tail_fee = kvRspGet.getLong(string("Fend_tail_fee_"+toString(i)).c_str());
		data.Fuser_end_type = kvRspGet.getInt(string("Fuser_end_type_"+toString(i)).c_str());
		data.Fend_sell_type = kvRspGet.getInt(string("Fend_sell_type_"+toString(i)).c_str());
		data.Fend_plan_amt = kvRspGet.getLong(string("Fend_plan_amt_"+toString(i)).c_str());
		data.Fend_real_buy_amt = kvRspGet.getLong(string("Fend_real_buy_amt_"+toString(i)).c_str());
		data.Fend_real_sell_amt = kvRspGet.getLong(string("Fend_real_sell_amt_"+toString(i)).c_str());
		strncpy(data.Fpresell_listid,kvRspGet.getString(string("Fpresell_listid_"+toString(i)).c_str()).c_str(), sizeof(data.Fpresell_listid)-1);
		strncpy(data.Fsell_listid,kvRspGet.getString(string("Fsell_listid_"+toString(i)).c_str()).c_str(), sizeof(data.Fsell_listid)-1);
		strncpy(data.Fend_listid_1,kvRspGet.getString(string("Fend_listid_1_"+toString(i)).c_str()).c_str(), sizeof(data.Fend_listid_1)-1);
		strncpy(data.Fend_listid_2,kvRspGet.getString(string("Fend_listid_2_"+toString(i)).c_str()).c_str(), sizeof(data.Fend_listid_2)-1);
		strncpy(data.Fend_transfer_fundcode,kvRspGet.getString(string("Fend_transfer_fundcode_"+toString(i)).c_str()).c_str(),sizeof(data.Fend_transfer_fundcode)-1);
		strncpy(data.Fend_transfer_spid,kvRspGet.getString(string("Fend_transfer_spid_"+toString(i)).c_str()).c_str(),sizeof(data.Fend_transfer_spid)-1);
		strncpy(data.Facc_time,kvRspGet.getString(string("Facc_time_"+toString(i)).c_str()).c_str(),sizeof(data.Facc_time)-1);
		strncpy(data.Ftrans_date,kvRspGet.getString(string("Ftrans_date_"+toString(i)).c_str()).c_str(), sizeof(data.Ftrans_date)-1);
		strncpy(data.Ffirst_profit_date,kvRspGet.getString(string("Ffirst_profit_date_"+toString(i)).c_str()).c_str(), sizeof(data.Ffirst_profit_date)-1);
		strncpy(data.Fopen_date,kvRspGet.getString(string("Fopen_date_"+toString(i)).c_str()).c_str(), sizeof(data.Fopen_date)-1);
		strncpy(data.Fbook_stop_date,kvRspGet.getString(string("Fbook_stop_date_"+toString(i)).c_str()).c_str(), sizeof(data.Fbook_stop_date)-1);
		strncpy(data.Fstart_date,kvRspGet.getString(string("Fstart_date_"+toString(i)).c_str()).c_str(), sizeof(data.Fstart_date)-1);
		strncpy(data.Fend_date,kvRspGet.getString(string("Fend_date_"+toString(i)).c_str()).c_str(), sizeof(data.Fend_date)-1);
		strncpy(data.Fprofit_end_date,kvRspGet.getString(string("Fprofit_end_date_"+toString(i)).c_str()).c_str(),sizeof(data.Fprofit_end_date)-1);
		strncpy(data.Fchannel_id,kvRspGet.getString(string("Fchannel_id_"+toString(i)).c_str()).c_str(), sizeof(data.Fchannel_id)-1);
		data.Fstate = kvRspGet.getInt(string("Fstate_"+toString(i)).c_str());
		data.Flstate = kvRspGet.getInt(string("Flstate_"+toString(i)).c_str());
		strncpy(data.Fcreate_time,kvRspGet.getString(string("Fcreate_time_"+toString(i)).c_str()).c_str(), sizeof(data.Fcreate_time)-1);
		strncpy(data.Fmodify_time,kvRspGet.getString(string("Fmodify_time_"+toString(i)).c_str()).c_str(), sizeof(data.Fmodify_time)-1);
		strncpy(data.Fmemo,kvRspGet.getString(string("Fmemo_"+toString(i)).c_str()).c_str(), sizeof(data.Fmemo)-1);
		strncpy(data.Fprofit_recon_date,kvRspGet.getString(string("Fprofit_recon_date_"+toString(i)).c_str()).c_str(), sizeof(data.Fprofit_recon_date)-1);
		data.Flast_profit = kvRspGet.getLong(string("Flast_profit_"+toString(i)).c_str());
		strncpy(data.Fdue_date,kvRspGet.getString(string("Fdue_date_"+toString(i)).c_str()).c_str(), sizeof(data.Fdue_date)-1);
		data.Ftotal_profit = kvRspGet.getLong(string("Ftotal_profit_"+toString(i)).c_str());
		strncpy(data.Flastids,kvRspGet.getString(string("Flastids_"+toString(i)).c_str()).c_str(), sizeof(data.Flastids)-1);
		list.push_back(data);
	}

    return 0;
}

/**
*设置cache
*/
bool setFundCloseTransToKV(vector< FundCloseTrans>& fundCloseTransVec)
{
	string key;
    if(fundCloseTransVec.size()<=0){
		TRACE_DEBUG("fundCloseTransVec no close fund trans.");
        return true;
    }else{
    	stringstream ss;
    	ss<<"fund_close_trans_";
		ss<<fundCloseTransVec[0].Ftrade_id;
		ss<<"_";
		ss<<fundCloseTransVec[0].Ffund_code;
	 	key =ss.str();
    }

    string szValue;
    packFundCloseTransCkvValue(fundCloseTransVec, szValue);
	
    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_FUND_CLOSE_TRANS, key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool getFundCloseTransKV(string trade_id, string fund_code,vector<FundCloseTrans> & fundCloseTransVec)
{
	stringstream ss;
	ss<<"fund_close_trans_";
	ss<<trade_id;
	ss<<"_";
	ss<<fund_code;
 	string key =ss.str();

    //取kv数据
    string ckv_value;
    int ret = gCkvSvrOperator->get(key, ckv_value);
    if (ERR_KEY_NOT_EXIST == ret)
        return true;
    
	// 查询失败
	if(ret!=0&&ret!=-13200){
		gPtrAppLog->error("getFundCloseTransKV error! key=%s, ret=%d,\n", key.c_str(),ret);
		return false;
	}

    //解析CKV内容
    ret = parseFundCloseTransCkvValue(ckv_value, fundCloseTransVec);
    	
    return (0 == ret);
}

/**
*设置cache，此接口只能定期收益入账调用
*/
bool addFundCloseTransToKV(vector< FundCloseTrans>& fundCloseTransVec)
{
	// 无记录不更新CKV
	if(fundCloseTransVec.size()<=0){
		TRACE_DEBUG("fundCloseTransVec no close fund trans.");
		return true;
    }
	// 查询CKV数据
	string key;
	string trade_id=fundCloseTransVec[0].Ftrade_id;
	string fund_code=fundCloseTransVec[0].Ffund_code;
	stringstream ss;
	ss<<"fund_close_trans_";
	ss<<trade_id;
	ss<<"_";
	ss<<fund_code;
 	key =ss.str();
	vector< FundCloseTrans> kvVec;
	bool ret = getFundCloseTransKV(trade_id,fund_code,kvVec);
	// 获取CKV失败, 查询DB更新CKV
	if(!ret){
		return setFundCloseTransToKV(trade_id,fund_code);
	}

    string cur_day = toString(GetDateToday());
    string last_day = addDays(cur_day, -1);
    
	// 将CKV数据与传入数据合并
	for(size_t i=0;i<kvVec.size();i++){
		FundCloseTrans& kvData= kvVec[i];

        TRACE_DEBUG("close trans(Fid=%ld,Fprofit_end_date=%s),last_day=%s", 
            kvData.Fid, kvData.Fprofit_end_date, last_day.c_str());
        //如果收益截至日小于前一天,则表明该期要不是到期赎回完成，
        //要不就是到期滚动完成，此记录不在保存在ckv中
        if (kvData.Fprofit_end_date < last_day) {
            TRACE_DEBUG("close trans(Fid=%ld) remove from ckv", kvData.Fid)
            continue;
        }
        
		bool exist = false;
		for(size_t j=0;j<fundCloseTransVec.size();j++){
			if(kvData.Fid==fundCloseTransVec[j].Fid){
				exist = true;
				break;
			}            
		}
		if(!exist)
			fundCloseTransVec.push_back(kvData);		
	}
	// 将数据写入CKV
	return setFundCloseTransToKV(fundCloseTransVec);
}


/*
*查询单只定期基金
*/
bool queryFundCloseTransByEndDate(CMySQL* pMysql, FundCloseTrans& data, bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " date_format(Fprofit_recon_date,'%%Y%%m%%d'),Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Ftrade_id= '%s' "
                    " AND Ffund_code= '%s' "
                    " AND Fend_date= '%s' "
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    pMysql->EscapeStr(data.Fend_date).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // ????
        pMysql->Query(szSql, iLen);
        // ????
        pRes = pMysql->FetchResult();
        // ?????
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++)
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Fspid,row[3] ? row[3] : "", sizeof(data.Fspid) - 1);
            data.Fseqno = row[4] ? atoi(row[4]) : 0;
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Fpay_type = row[6] ? atoi(row[6]) : 0;
            data.Flastid = row[7] ? atoll(row[7]) : 0;
            data.Fstart_total_fee = row[8] ? atoll(row[8]) : 0;
            data.Fcurrent_total_fee = row[9] ? atoll(row[9]) : 0;
            data.Fend_tail_fee = row[10] ? atoll(row[10]) : 0;
            data.Fuser_end_type = row[11] ? atoi(row[11]) : 0;
            data.Fend_sell_type = row[12] ? atoi(row[12]) : 0;
            data.Fend_plan_amt = row[13] ? atoll(row[13]) : 0;
            data.Fend_real_buy_amt = row[14] ? atoll(row[14]) : 0;
            data.Fend_real_sell_amt = row[15] ? atoll(row[15]) : 0;
            strncpy(data.Fpresell_listid,row[16] ? row[16] : "", sizeof(data.Fpresell_listid) - 1);
            strncpy(data.Fsell_listid,row[17] ? row[17] : "", sizeof(data.Fsell_listid) - 1);
            strncpy(data.Fend_listid_1,row[18] ? row[18] : "", sizeof(data.Fend_listid_1) - 1);
            strncpy(data.Fend_listid_2,row[19] ? row[19] : "", sizeof(data.Fend_listid_2) - 1);
            strncpy(data.Fend_transfer_fundcode,row[20] ? row[20] : "", sizeof(data.Fend_transfer_fundcode) - 1);
            strncpy(data.Fend_transfer_spid,row[21] ? row[21] : "", sizeof(data.Fend_transfer_spid) - 1);
            strncpy(data.Ftrans_date,row[22] ? row[22] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[23] ? row[23] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[24] ? row[24] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[25] ? row[25] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[26] ? row[26] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[27] ? row[27] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fprofit_end_date,row[28] ? row[28] : "", sizeof(data.Fprofit_end_date) - 1);
            strncpy(data.Fchannel_id,row[29] ? row[29] : "", sizeof(data.Fchannel_id) - 1);
            data.Fstate = row[30] ? atoi(row[30]) : 0;
            data.Flstate = row[31] ? atoi(row[31]) : 0;
            strncpy(data.Fcreate_time,row[32] ? row[32] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[33] ? row[33] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[34] ? row[34] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[35] ? row[35] : "", sizeof(data.Fexplain) - 1);
            strncpy(data.Fsign,row[36] ? row[36] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Facc_time,row[37] ? row[37] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fprofit_recon_date,row[38] ? row[38] : "", sizeof(data.Fprofit_recon_date) - 1);
			data.Flast_profit = row[39]?atoll(row[39]):0;
            strncpy(data.Fdue_date,row[40] ? row[40] : "", sizeof(data.Fdue_date) - 1);
			data.Ftotal_profit = row[41]?atoll(row[41]):0;
            strncpy(data.Flastids,row[42] ? row[42] : "", sizeof(data.Flastids) - 1);
            checkSign( "t_fund_close_trans", data);
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

/*
*update一条记录
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundCloseTransById(CMySQL* pMysql, FundCloseTrans& data )
{
	TRACE_DEBUG("updateFundCloseTransById");
	stringstream tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    string trade_id = escapeString(data.Ftrade_id);
    
    tb_name << "fund_db_" << trade_id.substr(trade_id.size() - 2);
    tb_name << ".t_fund_close_trans_" << trade_id.substr(trade_id.size() - 3, 1);

    // 设置需要更新的字段
    kv_map["Fmodify_time"] = toString(data.Fmodify_time);

	TRACE_DEBUG("[Fmodify_time:%s]", kv_map["Fmodify_time"].c_str());
	
	if(data.Fpay_type!= MIN_INTEGER)
	{
		kv_map["Fpay_type"] = toString(data.Fpay_type);
	}
	if(data.Fstate!= MIN_INTEGER)
	{
		kv_map["Fstate"] = toString(data.Fstate);
	}
	if(data.Fstart_total_fee!= MIN_INTEGER)
	{
		kv_map["Fstart_total_fee"] = toString(data.Fstart_total_fee);
	}
	if(data.Fcurrent_total_fee!= MIN_INTEGER)
	{
		kv_map["Fcurrent_total_fee"] = toString(data.Fcurrent_total_fee);
		kv_map["Fsign"] = genSign("t_fund_close_trans", data);
	}
	if(data.Fuser_end_type!= MIN_INTEGER)
	{
		kv_map["Fuser_end_type"] = toString(data.Fuser_end_type);
	}
	if(data.Fend_sell_type!= MIN_INTEGER)
	{
		kv_map["Fend_sell_type"] = toString(data.Fend_sell_type);
	}
	if(data.Fend_plan_amt!= MIN_INTEGER)
	{
		kv_map["Fend_plan_amt"] = toString(data.Fend_plan_amt);
	}
	if(data.Fend_real_sell_amt!= MIN_INTEGER)
	{
		kv_map["Fend_real_sell_amt"] = toString(data.Fend_real_sell_amt);
	}
	if(data.Fend_tail_fee!= MIN_INTEGER&&data.Fend_tail_fee!=0)
	{
		kv_map["Fend_tail_fee"] = toString(data.Fend_tail_fee);
	}
	if(data.Fend_real_buy_amt!= MIN_INTEGER)
	{
		kv_map["Fend_real_buy_amt"] = toString(data.Fend_real_buy_amt);
	}
	if(!(0 == strcmp("", data.Fend_transfer_spid)))
	{
		kv_map["Fend_transfer_spid"] = data.Fend_transfer_spid;
	}
	if(!(0 == strcmp("", data.Fend_transfer_fundcode)))
	{
		kv_map["Fend_transfer_fundcode"] = data.Fend_transfer_fundcode;
	}
#if 0    
	if(!(0 == strcmp("", data.Fsign)))
	{
		kv_map["Fsign"] = data.Fsign;
	}
#endif    
	if(!(0 == strcmp("", data.Fprofit_recon_date)))
	{
		kv_map["Fprofit_recon_date"] = data.Fprofit_recon_date;
		kv_map["Flast_profit"] = toString(data.Flast_profit);
		kv_map["Ftotal_profit"] = toString(data.Ftotal_profit);
	}
	if(!(0 == strcmp("", data.Fsell_listid)))
	{
		kv_map["Fsell_listid"] = data.Fsell_listid;
	}
	if(!(0 == strcmp("", data.Fpresell_listid)))
	{
		kv_map["Fpresell_listid"] = data.Fpresell_listid;
	}
	if(!(0 == strcmp("", data.Fmemo)))              
	{                                               
		kv_map["Fmemo"] = data.Fmemo;                 
	}                                               
	if(data.Flastid!= MIN_INTEGER&&data.Flastid!=0) 
	{                                               
		kv_map["Flastid"] = toString(data.Flastid);   
	}                                            
	if(data.Fbkid!= MIN_INTEGER) 
	{                                               
		kv_map["Fbkid"] = toString(data.Fbkid);   
	}
	if(!(0 == strcmp("", data.Flastids)))
	{
		kv_map["Flastids"] = data.Flastids;
	}

    ss_cond << "Fid='" <<  toString(data.Fid) <<"' ";

    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, tb_name.str(), ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
	
}

void saveFundCloseTrans(FundCloseTrans& data,FundCloseTrans& dbData,const char* listid,const int subject){
	// 赋值需要记录流水表的不变字段
	FundCloseBalanceRolllist fundCloseBalanceRolllist;
	memset(&fundCloseBalanceRolllist, 0, sizeof(FundCloseBalanceRolllist));	
	fundCloseBalanceRolllist.Fclose_id=data.Fid;
	strncpy(fundCloseBalanceRolllist.Ftrade_id, data.Ftrade_id, sizeof(fundCloseBalanceRolllist.Ftrade_id)-1);
	strncpy(fundCloseBalanceRolllist.Ffund_code, dbData.Ffund_code, sizeof(fundCloseBalanceRolllist.Ffund_code)-1);
	strncpy(fundCloseBalanceRolllist.Fspid, dbData.Fspid, sizeof(fundCloseBalanceRolllist.Fspid)-1);
	strncpy(fundCloseBalanceRolllist.Flistid, listid, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Facc_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Fcreate_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Fmodify_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	fundCloseBalanceRolllist.Fsubject = subject;
	// 可能不会修改的金额,使用DB数据记录流水
	if(data.Fstart_total_fee==MIN_INTEGER){
		fundCloseBalanceRolllist.Fstart_balance=dbData.Fstart_total_fee;
	}else{
		fundCloseBalanceRolllist.Fstart_balance=data.Fstart_total_fee;
	}
	if(data.Fend_tail_fee==MIN_INTEGER){
		fundCloseBalanceRolllist.Ftail_balance = dbData.Fend_tail_fee;
	}else{
		fundCloseBalanceRolllist.Ftail_balance = data.Fend_tail_fee;
	}
	if(data.Fcurrent_total_fee==MIN_INTEGER){
		fundCloseBalanceRolllist.Fbalance = dbData.Fcurrent_total_fee;
	}else{
		fundCloseBalanceRolllist.Fbalance = data.Fcurrent_total_fee;
	}
	
	// 计算变更金额
	LONG totalFee,tailFee;
	int type;
	// 加金额类型
	if(data.Fcurrent_total_fee>=dbData.Fcurrent_total_fee){
		totalFee=fundCloseBalanceRolllist.Fbalance-dbData.Fcurrent_total_fee;
		tailFee=fundCloseBalanceRolllist.Ftail_balance-dbData.Fend_tail_fee;
		type=CLOSE_BALANCE_ROLLLIST_TYPE_SAVE;
	// 减金额
	}else{
		totalFee=dbData.Fcurrent_total_fee-fundCloseBalanceRolllist.Fbalance;
		tailFee=dbData.Fend_tail_fee-fundCloseBalanceRolllist.Ftail_balance;
		type=CLOSE_BALANCE_ROLLLIST_TYPE_DRAW;
	}
	// 滚动类型:定期交易金额没有变化，只将状态改成5
	// 流水记录实际金额变化
	/**将定期交易金额也变成0,删除这里的特殊判断
	if(subject==PURTYPE_ROLL_OUT){
		fundCloseBalanceRolllist.Fbalance=0;
		totalFee=dbData.Fcurrent_total_fee;
		type=CLOSE_BALANCE_ROLLLIST_TYPE_DRAW;
	}**/
	fundCloseBalanceRolllist.Ftype= type;
	fundCloseBalanceRolllist.Fbiz_fee = totalFee;
	fundCloseBalanceRolllist.Fbiz_tail_fee = tailFee;
	
	// 更新流水,获取流水号
	unsigned long long bkid;
	insertFundCloseBalanceRolllist(gPtrFundDB,fundCloseBalanceRolllist,bkid);
	
	//更新定期交易数据
	data.Fbkid = (LONG)bkid;
	updateFundCloseTransById(gPtrFundDB,data);
}

void createFundCloseTrans(FundCloseTrans& data,const char* listid,const int subject){
	// 创建定期交易	
	unsigned long long	closeId;
	insertFundCloseTrans(gPtrFundDB,data,closeId);
	data.Fid = (LONG)closeId;
	
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
	fundCloseBalanceRolllist.Ftype= CLOSE_BALANCE_ROLLLIST_TYPE_SAVE;
	fundCloseBalanceRolllist.Fbiz_fee = data.Fcurrent_total_fee;
	fundCloseBalanceRolllist.Fbiz_tail_fee = data.Fend_tail_fee;
	fundCloseBalanceRolllist.Fsubject = subject;	
	strncpy(fundCloseBalanceRolllist.Flistid, listid, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Facc_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Fcreate_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	strncpy(fundCloseBalanceRolllist.Fmodify_time, data.Fmodify_time, sizeof(fundCloseBalanceRolllist.Flistid)-1);
	// 更新流水,获取流水号
	unsigned long long bkid;
	insertFundCloseBalanceRolllist(gPtrFundDB, fundCloseBalanceRolllist, bkid);	
	data.Fbkid = (LONG)bkid;
	
	//更新流水号数据
	FundCloseTrans closeTrans;
	strncpy(closeTrans.Ftrade_id,data.Ftrade_id,sizeof(closeTrans.Ftrade_id)-1);
	closeTrans.Fid=data.Fid;
	closeTrans.Fbkid=data.Fbkid;
	strncpy(closeTrans.Fmodify_time, data.Fmodify_time, sizeof(closeTrans.Fmodify_time)-1);
	updateFundCloseTransById(gPtrFundDB,closeTrans);//无验签字段更新，Fsign不用更新
}


/*
*query返回多行数据函数
*/
bool queryFundCloseTransAllByProfitEnd(CMySQL* pMysql,const string& trade_id,int date, vector< FundCloseTrans>& dataVec,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	int offset=0;
	int limit=gPtrConfig->m_AppCfg.close_buy_date_upper_limit;

	string last_recon_date=addDays(toString(date),-1);
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " date_format(Fprofit_recon_date,'%%Y%%m%%d'),Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Flstate <> %d AND " 
                    " Fprofit_end_date>='%s' AND " 
                    " Fprofit_recon_date in ('00000000','%s') " 
                    " LIMIT %d, %d "
                    " %s ",
                    Sdb2(trade_id.c_str()),
                    Stb2(trade_id.c_str()),
                    pMysql->EscapeStr(trade_id).c_str(),
                    LSTATE_INVALID,
                    pMysql->EscapeStr(last_recon_date).c_str(),
                    pMysql->EscapeStr(last_recon_date).c_str(),
                    offset,limit,
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        for(int i=0; i<iRow; i++) 
        {
            FundCloseTrans data;
            
            memset(&data, 0, sizeof(data));
            MYSQL_ROW row = mysql_fetch_row(pRes);
            
            data.Fid = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Ffund_code,row[2] ? row[2] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Fspid,row[3] ? row[3] : "", sizeof(data.Fspid) - 1);
            data.Fseqno = row[4] ? atoi(row[4]) : 0;
            data.Fuid = row[5] ? atoi(row[5]) : 0;
            data.Fpay_type = row[6] ? atoi(row[6]) : 0;
            data.Flastid = row[7] ? atoll(row[7]) : 0;
            data.Fstart_total_fee = row[8] ? atoll(row[8]) : 0;
            data.Fcurrent_total_fee = row[9] ? atoll(row[9]) : 0;
            data.Fend_tail_fee = row[10] ? atoll(row[10]) : 0;
            data.Fuser_end_type = row[11] ? atoi(row[11]) : 0;
            data.Fend_sell_type = row[12] ? atoi(row[12]) : 0;
            data.Fend_plan_amt = row[13] ? atoll(row[13]) : 0;
            data.Fend_real_buy_amt = row[14] ? atoll(row[14]) : 0;
            data.Fend_real_sell_amt = row[15] ? atoll(row[15]) : 0;
            strncpy(data.Fpresell_listid,row[16] ? row[16] : "", sizeof(data.Fpresell_listid) - 1);
            strncpy(data.Fsell_listid,row[17] ? row[17] : "", sizeof(data.Fsell_listid) - 1);
            strncpy(data.Fend_listid_1,row[18] ? row[18] : "", sizeof(data.Fend_listid_1) - 1);
            strncpy(data.Fend_listid_2,row[19] ? row[19] : "", sizeof(data.Fend_listid_2) - 1);
            strncpy(data.Fend_transfer_fundcode,row[20] ? row[20] : "", sizeof(data.Fend_transfer_fundcode) - 1);
            strncpy(data.Fend_transfer_spid,row[21] ? row[21] : "", sizeof(data.Fend_transfer_spid) - 1);
            strncpy(data.Ftrans_date,row[22] ? row[22] : "", sizeof(data.Ftrans_date) - 1);
            strncpy(data.Ffirst_profit_date,row[23] ? row[23] : "", sizeof(data.Ffirst_profit_date) - 1);
            strncpy(data.Fopen_date,row[24] ? row[24] : "", sizeof(data.Fopen_date) - 1);
            strncpy(data.Fbook_stop_date,row[25] ? row[25] : "", sizeof(data.Fbook_stop_date) - 1);
            strncpy(data.Fstart_date,row[26] ? row[26] : "", sizeof(data.Fstart_date) - 1);
            strncpy(data.Fend_date,row[27] ? row[27] : "", sizeof(data.Fend_date) - 1);
            strncpy(data.Fprofit_end_date,row[28] ? row[28] : "", sizeof(data.Fprofit_end_date) - 1);
            strncpy(data.Fchannel_id,row[29] ? row[29] : "", sizeof(data.Fchannel_id) - 1);
            data.Fstate = row[30] ? atoi(row[30]) : 0;
            data.Flstate = row[31] ? atoi(row[31]) : 0;
            strncpy(data.Fcreate_time,row[32] ? row[32] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[33] ? row[33] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[34] ? row[34] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[35] ? row[35] : "", sizeof(data.Fexplain) - 1);
            strncpy(data.Fsign,row[36] ? row[36] : "", sizeof(data.Fsign) - 1);
			strncpy(data.Facc_time,row[37] ? row[37] : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fprofit_recon_date,row[38] ? row[38] : "", sizeof(data.Fprofit_recon_date) - 1);
			data.Flast_profit = row[39]?atoll(row[39]):0;
            strncpy(data.Fdue_date,row[40] ? row[40] : "", sizeof(data.Fdue_date) - 1);
			data.Ftotal_profit = row[41]?atoll(row[41]):0;
            strncpy(data.Flastids,row[42] ? row[42] : "", sizeof(data.Flastids) - 1);
            checkSign( "t_fund_close_trans", data);
            dataVec.push_back(data);
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

	return iRow >= 1;
}


