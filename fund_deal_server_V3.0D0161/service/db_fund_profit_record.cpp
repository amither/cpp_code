#include <algorithm>
#include "db_fund_profit_record.h" 
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

using std::sort;

static void addFundProfitRecord2List(const FundProfitRecord& fundProfitRecord, vector<FundProfitRecord> &list);
static bool fundprofit_rec_cmp(const FundProfitRecord& rec1, const FundProfitRecord& rec2);

/*
*query返回多行数据函数
*/
bool queryFundProfitRecord(CMySQL* pMysql,int offset,int limit,FundProfitRecord &where,vector<FundProfitRecord>& datavec,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Fsub_trans_id,Ftrade_id,Fcurtype,Fspid, "
                    " Fvalid_money,Fstop_money,Fprofit,Fday,Fprofit_type, "
                    " F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign,Fcreate_time, "
                    " Fmodify_time,Fmemo,Ftotal_profit,Ftplus_redem_money,Fend_tail_fee "
                    " FROM fund_db_%02d.t_fund_profit_record_%d "
                    " WHERE "
                    " Ftrade_id='%s' " 
                    " AND Fprofit>0 "
                    " order by Fday desc" 
                    " LIMIT %d, %d "
                    " %s ",
                    Sdb2(where.Ftrade_id),
                    Stb2(where.Ftrade_id),
                    pMysql->EscapeStr(where.Ftrade_id).c_str(),
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
            FundProfitRecord data;
            
            memset(&data, 0, sizeof(data));
            MYSQL_ROW row = mysql_fetch_row(pRes);
            
            strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Fsub_trans_id,row[1] ? row[1] : "", sizeof(data.Fsub_trans_id) - 1);
            strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
            data.Fcurtype = row[3] ? atoi(row[3]) : 0;
            strncpy(data.Fspid,row[4] ? row[4] : "", sizeof(data.Fspid) - 1);
            data.Fvalid_money = row[5] ? atoll(row[5]) : 0;
            data.Fstop_money = row[6] ? atoll(row[6]) : 0;
            data.Fprofit = row[7] ? atoll(row[7]) : 0;
            strncpy(data.Fday,row[8] ? row[8] : "", sizeof(data.Fday) - 1);
            data.Fprofit_type = row[9] ? atoi(row[9]) : 0;
            data.F1day_profit_rate = row[10] ? atoll(row[10]) : 0;
            data.F7day_profit_rate = row[11] ? atoll(row[11]) : 0;
            strncpy(data.Flogin_ip,row[12] ? row[12] : "", sizeof(data.Flogin_ip) - 1);
            strncpy(data.Fsign,row[13] ? row[13] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fcreate_time,row[14] ? row[14] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[15] ? row[15] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[16] ? row[16] : "", sizeof(data.Fmemo) - 1);
            data.Ftotal_profit = row[17] ? atoll(row[17]) : 0;
            data.Ftplus_redem_money= row[18] ? atoll(row[18]) : 0;
            data.Fend_tail_fee=row[19]? atoll(row[19]):0;
            
            checkSign( "t_fund_profit_record", data);
            datavec.push_back(data);
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


bool queryFundProfitRecord(CMySQL* pMysql, FundProfitRecord& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Fsub_trans_id,Ftrade_id,Fcurtype,Fspid, "
                    " Fvalid_money,Fstop_money,Ftotal_profit,Fprofit,Fday, "
                    " Fprofit_type,F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign, "
                    " Fcreate_time,Fmodify_time,Fmemo,Fend_tail_fee "
                    " FROM fund_db_%02d.t_fund_profit_record_%d "
                    " WHERE "
                    " Flistid='%s' " 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
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
            strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Fsub_trans_id,row[1] ? row[1] : "", sizeof(data.Fsub_trans_id) - 1);
            strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
            data.Fcurtype = row[3] ? atoi(row[3]) : 0;
            strncpy(data.Fspid,row[4] ? row[4] : "", sizeof(data.Fspid) - 1);
            data.Fvalid_money = row[5] ? atoll(row[5]) : 0;
            data.Fstop_money = row[6] ? atoll(row[6]) : 0;
            data.Ftotal_profit = row[7] ? atoll(row[7]) : 0;
            data.Fprofit = row[8] ? atoll(row[8]) : 0;
            strncpy(data.Fday,row[9] ? row[9] : "", sizeof(data.Fday) - 1);
            data.Fprofit_type = row[10] ? atoi(row[10]) : 0;
            data.F1day_profit_rate = row[11] ? atoll(row[11]) : 0;
            data.F7day_profit_rate = row[12] ? atoll(row[12]) : 0;
            strncpy(data.Flogin_ip,row[13] ? row[13] : "", sizeof(data.Flogin_ip) - 1);
            strncpy(data.Fsign,row[14] ? row[14] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fcreate_time,row[15] ? row[15] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[16] ? row[16] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[17] ? row[17] : "", sizeof(data.Fmemo) - 1);
		   data.Fend_tail_fee=row[18]? atoll(row[18]):0;
            checkSign( "t_fund_profit_record", data);
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
*query返回多行数据函数
*/
bool queryFundProfitRecordByTime(CMySQL* pMysql, const string &trade_id, const string &start_time, 
    const string &end_time, vector<FundProfitRecord>& datavec)
{
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;

    iLen = snprintf(szSql, sizeof(szSql),
                " SELECT "
                " Flistid,Fsub_trans_id,Ftrade_id,Fcurtype,Fspid, "
                " Fvalid_money,Fstop_money,Fprofit,Fday,Fprofit_type, "
                " F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign,Fcreate_time, "
                " Fmodify_time,Fmemo,Ftotal_profit,Ftplus_redem_money,Fend_tail_fee "
                " FROM fund_db_%02d.t_fund_profit_record_%d "
                " WHERE "
                " Ftrade_id='%s' " 
                " AND Fprofit>0 "
                " AND Fcreate_time>='%s' AND Fcreate_time<'%s' ",
                Sdb2(trade_id.c_str()),
                Stb2(trade_id.c_str()),
                pMysql->EscapeStr(trade_id).c_str(),
                pMysql->EscapeStr(start_time).c_str(),
                pMysql->EscapeStr(end_time).c_str());

    if (iLen <= 0 || iLen >= MAX_SQL_LEN) {
        throw CException(ERR_DB_SQL_NOVALUE, "construct sql string failed!", __FILE__, __LINE__);
    }
    
    gPtrAppLog->debug("[%s][%d]%s", __FILE__, __LINE__, szSql);
    
    // 执行查询
    pMysql->Query(szSql, iLen);
    // 取结果集
    scope_mysql_res myqlres = pMysql->FetchResult();
  
    // 获取结果行
    iRow = mysql_num_rows(myqlres.handle());
    for(int i=0; i<iRow; i++) 
    {
        FundProfitRecord data;        
        memset(&data, 0, sizeof(data));
        
        MYSQL_ROW row = mysql_fetch_row(myqlres.handle());
        
        strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
        strncpy(data.Fsub_trans_id,row[1] ? row[1] : "", sizeof(data.Fsub_trans_id) - 1);
        strncpy(data.Ftrade_id,row[2] ? row[2] : "", sizeof(data.Ftrade_id) - 1);
        data.Fcurtype = row[3] ? atoi(row[3]) : 0;
        strncpy(data.Fspid,row[4] ? row[4] : "", sizeof(data.Fspid) - 1);
        data.Fvalid_money = row[5] ? atoll(row[5]) : 0;
        data.Fstop_money = row[6] ? atoll(row[6]) : 0;
        data.Fprofit = row[7] ? atoll(row[7]) : 0;
        strncpy(data.Fday,row[8] ? row[8] : "", sizeof(data.Fday) - 1);
        data.Fprofit_type = row[9] ? atoi(row[9]) : 0;
        data.F1day_profit_rate = row[10] ? atoll(row[10]) : 0;
        data.F7day_profit_rate = row[11] ? atoll(row[11]) : 0;
        strncpy(data.Flogin_ip,row[12] ? row[12] : "", sizeof(data.Flogin_ip) - 1);
        strncpy(data.Fsign,row[13] ? row[13] : "", sizeof(data.Fsign) - 1);
        strncpy(data.Fcreate_time,row[14] ? row[14] : "", sizeof(data.Fcreate_time) - 1);
        strncpy(data.Fmodify_time,row[15] ? row[15] : "", sizeof(data.Fmodify_time) - 1);
        strncpy(data.Fmemo,row[16] ? row[16] : "", sizeof(data.Fmemo) - 1);
        data.Ftotal_profit = row[17] ? atoll(row[17]) : 0;
        data.Ftplus_redem_money= row[18] ? atoll(row[18]) : 0;
        data.Fend_tail_fee=row[19]? atoll(row[19]):0;
        checkSign( "t_fund_profit_record", data);
        datavec.push_back(data);
    }

	return iRow >= 1;
}


void insertFundProfitRecord(CMySQL* pMysql, FundProfitRecord &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_profit_record_%d("
                    " Flistid,Fsub_trans_id,Ftrade_id,Fcurtype,Fspid, "
                    " Fvalid_money,Fstop_money,Fprofit,Fday,Fprofit_type, "
                    " F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign,Fcreate_time, "
                    " Fmodify_time,Fmemo,Ftotal_profit,Ftplus_redem_money,Frecon_balance,Fstandby1,Fend_tail_fee,Fstandby3)"
                    " VALUES("
                    " '%s','%s','%s',%d,'%s', "
                    " %zd,%zd,%zd,'%s',%d, "
                    " %zd,%zd,'%s','%s','%s', "
                    " '%s','%s',%zd,%zd,%zd,%d,%zd,'%s')",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    pMysql->EscapeStr(data.Fsub_trans_id).c_str(),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype,
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Fvalid_money,
                    data.Fstop_money,
                    data.Fprofit,
                    pMysql->EscapeStr(data.Fday).c_str(),
                    data.Fprofit_type,
                    data.F1day_profit_rate,
                    data.F7day_profit_rate,
                    pMysql->EscapeStr(data.Flogin_ip).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_profit_record", data)).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    data.Ftotal_profit,
                    data.Ftplus_redem_money,
                    data.Frecon_balance,
                    data.Fstandby1,
                    data.Fend_tail_fee,
                    pMysql->EscapeStr(data.Fstandby3).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

/**
 * 解析ckv中收益流水数据
 * @param value ckv中的收益流水列表值
 * @param list 解析后的列表
 * @return 0-成功，其它-失败
 */
int parseProfitRecordCkvValue(const string &value, vector<FundProfitRecord> &list)
{
    CParams kvRecords;
    kvRecords.parse(value);

    int total_num = kvRecords.getInt("total_num");
    for (int i = 0; i < total_num; ++i) {
        FundProfitRecord rec;
        char szParaName[128] = {0};
        
	    snprintf(szParaName, sizeof(szParaName), "Flistid_%d", i);
        SCPY(rec.Flistid, kvRecords.getString(szParaName).c_str());
       
		memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Ftrade_id_%d", i);
        SCPY(rec.Ftrade_id, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fcurtype_%d", i);
        ICPY(rec.Fcurtype, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fspid_%d", i);
        SCPY(rec.Fspid, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fvalid_money_%d", i);
        LCPY(rec.Fvalid_money, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fstop_money_%d", i);
        LCPY(rec.Fstop_money, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fprofit_%d", i);
        LCPY(rec.Fprofit, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Ftotal_profit_%d", i);
        LCPY(rec.Ftotal_profit, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fday_%d", i);
        SCPY(rec.Fday, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fprofit_type_%d", i);
        ICPY(rec.Fprofit_type, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "F1day_profit_rate_%d", i);
        LCPY(rec.F1day_profit_rate, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "F7day_profit_rate_%d", i);
        LCPY(rec.F7day_profit_rate, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fcreate_time_%d", i);
        SCPY(rec.Fcreate_time, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Fmemo_%d", i);
        SCPY(rec.Fmemo, kvRecords.getString(szParaName).c_str());

        memset(szParaName, 0, sizeof(szParaName));
        snprintf(szParaName, sizeof(szParaName), "Ftplus_redem_money_%d", i);
        LCPY(rec.Ftplus_redem_money, kvRecords.getString(szParaName).c_str());

        list.push_back(rec);
    }  

    return 0;
}

/**
 * 将收益列表拼接成ckv中的保存的value值
 * @param list 用户的收益列表
 * @param value 组装好的ckv值
 * @return 0-成功，其他-失败
 */
int packProfitRecordsCkvValue(const vector<FundProfitRecord> &list, string &value)
{
	CParams kvReqSet;
	char szParaName[64] = {0};

    size_t rec_size = std::min<size_t>(list.size(), CACHE_USR_PROFIT_REC_MAX_NUM);
    
    //设置要修改的数据szValue
    for(vector<FundProfitRecord>::size_type i= 0; i < rec_size; ++i)
    {
        const FundProfitRecord &tradeFund = list[i];
			
		snprintf(szParaName, sizeof(szParaName), "Flistid_%zd", i);
		kvReqSet.setParam(szParaName, tradeFund.Flistid);
		
		snprintf(szParaName, sizeof(szParaName), "Ftrade_id_%zd", i);
		kvReqSet.setParam(szParaName, tradeFund.Ftrade_id);

		snprintf(szParaName, sizeof(szParaName), "Fcurtype_%zd", i);
		kvReqSet.setParam(szParaName, tradeFund.Fcurtype);
		
		snprintf(szParaName, sizeof(szParaName), "Fspid_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fspid);
		
		snprintf(szParaName, sizeof(szParaName), "Fvalid_money_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fvalid_money);
		
		snprintf(szParaName, sizeof(szParaName), "Fstop_money_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fstop_money);
		
		snprintf(szParaName, sizeof(szParaName), "Fprofit_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fprofit);

		snprintf(szParaName, sizeof(szParaName), "Ftotal_profit_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Ftotal_profit);
		
		snprintf(szParaName, sizeof(szParaName), "Fday_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fday);
		
		snprintf(szParaName, sizeof(szParaName), "Fprofit_type_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fprofit_type);
		
		snprintf(szParaName, sizeof(szParaName), "F1day_profit_rate_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.F1day_profit_rate);
		
		snprintf(szParaName, sizeof(szParaName), "F7day_profit_rate_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.F7day_profit_rate);
		
		snprintf(szParaName, sizeof(szParaName), "Fcreate_time_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fcreate_time);
		
		snprintf(szParaName, sizeof(szParaName), "Fmemo_%zd", i);
		kvReqSet.setParam(szParaName,tradeFund.Fmemo);
        
        snprintf(szParaName, sizeof(szParaName), "Ftplus_redem_money_%zd", i);
        kvReqSet.setParam(szParaName,tradeFund.Ftplus_redem_money);
    }


	kvReqSet.setParam("total_num", (int)rec_size);
	if(rec_size >= CACHE_USR_PROFIT_REC_MAX_NUM)
	{
		kvReqSet.setParam("isMore","1"); //标识有更多数据在数据库中
	}
	else
	{
		kvReqSet.setParam("isMore","0");
	}
    
    value = kvReqSet.pack();

    return 0;
}


/**
*设置cache
*/
bool setFundProfitRecordToKV(CMySQL* mysql, FundProfitRecord& fundProfitRecord)
{
	string key = "profit_record_" + toString(fundProfitRecord.Ftrade_id);

    vector<FundProfitRecord> fundProfitRecordVec;
	if( !queryFundProfitRecord(mysql, 0, CACHE_USR_PROFIT_REC_MAX_NUM, fundProfitRecord, fundProfitRecordVec, false))
	{
		TRACE_DEBUG("no fund profit record");
		return true;
	}
    
    string szValue;
	packProfitRecordsCkvValue(fundProfitRecordVec, szValue);

    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_PROFIT_RECORD, key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool addFundProfitRecordToCache(const FundProfitRecord& fundProfitRecord)
{
    string key = "profit_record_" + toString(fundProfitRecord.Ftrade_id);

    vector<FundProfitRecord> fundProfitRecordVec;
	if( 0 != getFundProfitRecordFromCache(fundProfitRecord.Ftrade_id, fundProfitRecordVec))
	{
		TRACE_DEBUG("get fund profit record from cache fail");
		return false;
	}

    //如果CVK中已有该记录了，则直接返回不再更新到CKV
    vector<FundProfitRecord>::const_iterator cite;
    for (cite = fundProfitRecordVec.begin(); cite != fundProfitRecordVec.end(); ++cite) {
        if (string(cite->Flistid) == string(fundProfitRecord.Flistid)) {
            TRACE_DEBUG("fund profit record(Flistid=%s) already update to ckv",
                fundProfitRecord.Flistid);
            return true;
        }
    }

    addFundProfitRecord2List(fundProfitRecord, fundProfitRecordVec);
    
    string szValue;
    packProfitRecordsCkvValue(fundProfitRecordVec, szValue);
	
    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_PROFIT_RECORD, key, szValue,false))
	{
		return false;
	}
	else
	{
		return true;
	}
}

int getFundProfitRecordFromCache(const string trade_id, vector<FundProfitRecord> &list)
{
    string key = "profit_record_" + trade_id;
    string value;
    int ret = gCkvSvrOperator->get(key, value);
    //如果key不存在则返回true，表示ckv中没有数据,而不是读ckv失败
    if (ERR_KEY_NOT_EXIST == ret)
        return 0;
    
    if (0 != ret)
        return -1;

    parseProfitRecordCkvValue(value, list);      

    return 0;
}

static void addFundProfitRecord2List(const FundProfitRecord& fundProfitRecord, vector<FundProfitRecord> &list)
{   
    list.push_back(fundProfitRecord);
    //根据时间从大小到排序
    std::sort(list.begin(), list.end(), fundprofit_rec_cmp);    
}

static bool fundprofit_rec_cmp(const FundProfitRecord& rec1, const FundProfitRecord& rec2)
{
    //排序是从大到小排序
    if (strcmp(rec1.Fday, rec2.Fday) <= 0) {
        return false;
    } else {
        return true;
    }
}

