#include "db_trade_fund.h"
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 
extern CMySQL* gPtrFundDB;



/**
 * 按用户统计某日的某交易类型的交易总额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool statCloseRedemTransFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, const string& start_day, const string& end_day)
{
	MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	string start_time,end_time;
	
	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(addDays(start_day,-1)) + " 15:00:00";
		end_time = changeDateFormat(end_day) + " 14:59:59";	
	}
	else
	{
		start_time = start_day;
		end_time = changeDateFormat(end_day) + " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " Ffund_code= '%s' AND"
                    " Fspid= '%s' AND"
                    " Fstate in (5,10) AND Flstate=1 AND "
                    " Fpur_type in (4,12) AND "
                    " Fclose_listid = '%ld' ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(end_time).c_str(),
                    pstRecord.Fcur_type,
                    mysql->EscapeStr(pstRecord.Ffund_code).c_str(),
                    mysql->EscapeStr(pstRecord.Fspid).c_str(),
                    pstRecord.Fclose_listid
                    );
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
		pstRecord.Ftotal_fee = row[0] ? atoll(row[0]) : 0;
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
 * 按用户统计某日的某交易类型的交易总额
 * 返回: 查询到记录返回true, 没有查询到记录返回false
 */
bool statCloseBuyTransFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, const string& start_day, const string& end_day)
{
	MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	string start_time,end_time;
	
	if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
	{
		start_time = changeDateFormat(addDays(start_day,-1)) + " 15:00:00";
		end_time = changeDateFormat(end_day) + " 14:59:59";	
	}
	else
	{
		start_time = start_day;
		end_time = changeDateFormat(end_day) + " 23:59:59";
	}
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Ftotal_fee) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time <= '%s' AND "
                    " Fcur_type=%d AND "
                    " Ffund_code= '%s' AND"
                    " Fspid= '%s' AND"
                    " Fstate in (2,3) AND Flstate=1 AND "
                    " Fpur_type in (1,9,10,11) AND "
                    " Fclose_listid = '%ld' AND "
                    " Fuid = '%d' ",
                    Sdb1(pstRecord.Fuid),
                    Stb1(pstRecord.Fuid),
                    mysql->EscapeStr(pstRecord.Ftrade_id).c_str(),
                    mysql->EscapeStr(start_time).c_str(),
                    mysql->EscapeStr(end_time).c_str(),
                    pstRecord.Fcur_type,
                    mysql->EscapeStr(pstRecord.Ffund_code).c_str(),
                    mysql->EscapeStr(pstRecord.Fspid).c_str(),
                    pstRecord.Fclose_listid,
                    pstRecord.Fuid
                    );
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
		pstRecord.Ftotal_fee = row[0] ? atoll(row[0]) : 0;
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
 * 新增定期预约赎回单的基金交易流水，基金交易(订单)表
 * 相比主行数要新增SPE_TAG
 */
void InsertCloseBookTradeFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    // 转义单号
    string sEscapelistId = escapeString(pstRecord->Flistid);

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "insert into fund_db_%02d.t_trade_fund_%d "
        "(Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee,"
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fmodify_time, Fstandby1, Fcft_trans_id, "
        "Fcft_charge_ctrl_id,Fsp_fetch_id,Fcft_bank_billno, Fsub_trans_id, Fcur_type, Facc_time, Fpurpose,Fchannel_id, " 
        "Floading_type,Fstandby4,Fend_date,Freal_redem_amt, Fopt_type, Fclose_listid, Fspe_tag, Fsign) values "
        "('%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %zd, %d, '%s', %d, %d,'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', " 
        " %d, '%s', '%s', %d, '%s', '%s', '%s', '%s', '%s',%d, '%s',%d, '%s',%d, '%s', '%s', %zd, %d, %zd, %d, '%s')",
        Sdb2(sEscapelistId.c_str()), Stb2(sEscapelistId.c_str()), sEscapelistId.c_str(), 
        escapeString(pstRecord->Fspid).c_str(), escapeString(pstRecord->Fcoding).c_str(), escapeString(pstRecord->Ftrade_id).c_str(), pstRecord->Fuid, 
        escapeString(pstRecord->Ffund_name).c_str(), escapeString(pstRecord->Ffund_code).c_str(), 
        pstRecord->Fpur_type, pstRecord->Ftotal_fee, pstRecord->Fbank_type, escapeString(pstRecord->Fcard_no).c_str(), 
        pstRecord->Fstate, pstRecord->Flstate, escapeString(pstRecord->Ftrade_date).c_str(), escapeString(pstRecord->Ffund_value).c_str(), escapeString(pstRecord->Ffund_vdate).c_str(), 
        escapeString(pstRecord->Ffund_type).c_str(), escapeString(pstRecord->Fnotify_url).c_str(), 
        escapeString(pstRecord->Frela_listid).c_str(), escapeString(pstRecord->Fdrawid).c_str(), escapeString(pstRecord->Ffetchid).c_str(), pstRecord->Fcft_timestamp, 
        escapeString(pstRecord->Fcreate_time).c_str(), escapeString(pstRecord->Fmodify_time).c_str(), pstRecord->Fstandby1, escapeString(pstRecord->Fcft_trans_id).c_str(), 
        escapeString(pstRecord->Fcft_charge_ctrl_id).c_str(),escapeString(pstRecord->Fsp_fetch_id).c_str(),escapeString(pstRecord->Fcft_bank_billno).c_str(), escapeString(pstRecord->Fsub_trans_id).c_str(), 
        pstRecord->Fcur_type, pstRecord->Facc_time, pstRecord->Fpurpose,escapeString(pstRecord->Fchannel_id).c_str(),pstRecord->Floading_type,escapeString(pstRecord->Fcoupon_id).c_str(),
        escapeString(pstRecord->Fend_date).c_str(),pstRecord->Freal_redem_amt, pstRecord->Fopt_type, pstRecord->Fclose_listid, pstRecord->Fspe_tag,
        escapeString(genSign("t_trade_fund", *pstRecord)).c_str());

    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    if(1 != mysql->AffectedRows())
    {
        gPtrAppLog->error("insert tradefund affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert tradefund affected rows error! ", __FILE__, __LINE__);
    }
}


/**
 * 新增定期预约赎回单的基金交易流水，基金交易(用户)表
 * 相比主行数要新增SPE_TAG
 */
void InsertCloseBookTradeUserFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord)
{
    char    szSqlStr[MAX_SQL_LEN*2+1] = {0};

    int iLen = snprintf(szSqlStr, sizeof(szSqlStr), 
        "insert into fund_db_%02d.t_trade_user_fund_%d "
        "(Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, Fpur_type, Ftotal_fee,"
        "Fbank_type, Fcard_no, Fstate, Flstate, Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, Fnotify_url, "
        "Frela_listid, Fdrawid, Ffetchid, Fcft_timestamp, Fcreate_time, Fmodify_time,Fcft_trans_id,"
        "Fcft_charge_ctrl_id,Fsp_fetch_id,Fcft_bank_billno, Fsub_trans_id, Fcur_type, Facc_time, Fpurpose,Fchannel_id,"
        "Floading_type,Fopt_type,Fclose_listid,Freal_redem_amt, Fend_date, Fspe_tag, Fsign) values "
        "('%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %zd, %d, '%s', %d, %d,'%s', '%s', '%s', '%s', '%s', '%s',"
        "'%s', '%s', %d, '%s', '%s', '%s', '%s', '%s','%s','%s',%d, '%s',%d, '%s',%d,%d, %zd,%zd,  '%s',%d, '%s')", 
        Sdb1(pstRecord->Fuid), Stb1(pstRecord->Fuid), escapeString(pstRecord->Flistid).c_str(), 
        escapeString(pstRecord->Fspid).c_str(), escapeString(pstRecord->Fcoding).c_str(), escapeString(pstRecord->Ftrade_id).c_str(), pstRecord->Fuid, 
        escapeString(pstRecord->Ffund_name).c_str(), escapeString(pstRecord->Ffund_code).c_str(), 
        pstRecord->Fpur_type, pstRecord->Ftotal_fee, pstRecord->Fbank_type, escapeString(pstRecord->Fcard_no).c_str(), 
        pstRecord->Fstate, pstRecord->Flstate, escapeString(pstRecord->Ftrade_date).c_str(), escapeString(pstRecord->Ffund_value).c_str(), escapeString(pstRecord->Ffund_vdate).c_str(), 
        escapeString(pstRecord->Ffund_type).c_str(), escapeString(pstRecord->Fnotify_url).c_str(), 
        escapeString(pstRecord->Frela_listid).c_str(), escapeString(pstRecord->Fdrawid).c_str(), escapeString(pstRecord->Ffetchid).c_str(), pstRecord->Fcft_timestamp, 
        escapeString(pstRecord->Fcreate_time).c_str(), escapeString(pstRecord->Fmodify_time).c_str(), escapeString(pstRecord->Fcft_trans_id).c_str(), 
        escapeString(pstRecord->Fcft_charge_ctrl_id).c_str(),escapeString(pstRecord->Fsp_fetch_id).c_str(),escapeString(pstRecord->Fcft_bank_billno).c_str(), escapeString(pstRecord->Fsub_trans_id).c_str(), 
        pstRecord->Fcur_type, pstRecord->Facc_time, pstRecord->Fpurpose,escapeString(pstRecord->Fchannel_id).c_str(),pstRecord->Floading_type,pstRecord->Fopt_type,pstRecord->Fclose_listid,
        pstRecord->Freal_redem_amt, escapeString(pstRecord->Fend_date).c_str(), pstRecord->Fspe_tag, 
        escapeString(genSign("t_trade_user_fund", *pstRecord)).c_str());

    mysql->Query(szSqlStr, iLen);

    // 判断影响行数
    if(1 != mysql->AffectedRows())
    {
        gPtrAppLog->error("insert tradeuserfund affected rows[%d] error, sql[%s]", mysql->AffectedRows(), szSqlStr); 
        throw CException(ERR_DB_AFFECT_MULTIROWS, "insert tradeuserfund affected rows error! ", __FILE__, __LINE__);
    }
}

/**
* 检查是否存在未完成的定期赎回记录(不包含预约赎回记录)
*/
bool isCloseInitRedemExists(CMySQL* mysql, const LONG closeId, const int uid, const char* fundCode, const char* startTime, const char* endTime)
{
	
	MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Fuid='%d'  AND " 
                    " Facc_time >= '%s' AND "
                    " Facc_time < '%s' AND "
                    " Ffund_code= '%s' AND"
                    " Fstate =4 AND Flstate=1 AND "
                    " Fpur_type in (4,12) AND "
                    " Fspe_tag <> 3 AND "
                    " Fclose_listid = '%ld' limit 1",
                    Sdb1(uid),
                    Stb1(uid),
                    uid,
                    mysql->EscapeStr(startTime).c_str(),
                    mysql->EscapeStr(endTime).c_str(),
                    mysql->EscapeStr(fundCode).c_str(),
                    closeId
                    );
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
    return iRow > 0;
}

/**
 * 按用户统计活期某日存在的T+1赎回状态, 时间为 (T-2, ...) 
 * 从统计日期的T-2+1D日起统计
 * 返回: fund_code, 最后一笔未提现的T+1时间, 最后一笔已提现的T+1时间
 */
bool statOpenRedeemModifyTime(CMySQL* mysql, const string& curDay,const int uid,map<string,string>& redeemTimeMap)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    string Tminus2Date;
    string TminusDate;
    bool isCurTDay = false;
    getTminus2TransDate(mysql,curDay,Tminus2Date,TminusDate,isCurTDay);
	if (TminusDate.empty() ||Tminus2Date.empty())
    {
        throw CException(ERR_DB_UNKNOW, "get T-2 transday fail from db!", __FILE__, __LINE__);
    }
	string startTime=Tminus2Date+"150000";
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ffund_code,Fstate,max(Fmodify_time) "
                    " FROM fund_db_%02d.t_trade_user_fund_%d "
                    " WHERE "
                    " Fuid='%d'  AND " 
                    " Facc_time >= '%s' AND "
                    " Flstate=1 AND Fclose_listid=0 AND "
                    " Fpur_type in (4,12) AND "
                    " Fstate in (5,10) "
                    " group by Ffund_code,Fstate",
                    Sdb1(uid),
                    Stb1(uid),
                    uid,
                    mysql->EscapeStr(startTime).c_str()
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        mysql->Query(szSql, iLen);
        // 取结果集
        pRes = mysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0;i<iRow;i++){
        	MYSQL_ROW row = mysql_fetch_row(pRes);
			string fundCode=row[0] ? string(row[0]) : "";
			string state = row[1] ? string(row[1]) : "";
			string key=fundCode+"_"+state;
			string time=row[2] ? string(row[2]) : "";
			redeemTimeMap[key]=time;
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
    return redeemTimeMap.size()>0;
}




