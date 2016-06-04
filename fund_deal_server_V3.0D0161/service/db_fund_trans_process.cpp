#include "db_fund_trans_process.h"
#include "dbsign.h"

bool queryFundTransProcess(CMySQL* pMysql,  FundTransProcess &data,  bool islock/* = false */)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Flistid,Ftrade_id,Fspid,Ftype,Ffund_code,date_format(Ftrade_date,'%%Y%%m%%d'),date_format(Fconfirm_date,'%%Y%%m%%d'), "
                    " Fpur_type,Fpurpose,Ftotal_fee,Ffund_units,Ffund_net,Fstate,Flstate,Fsign,Fmemo, "
                    " Fcreate_time,Fmodify_time,Facc_time,Fsubacc_time,Fconfirm_time,Ffetch_time,Ffinish_time "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id='%s' and Flistid = '%s' "
                    " %s ",
                    Sdb2(data.Ftrade_id),Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    islock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 ||iRow>1 )
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
        	int j=-1;
            MYSQL_ROW row = mysql_fetch_row(pRes);			
            data.Fid = row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Flistid,row[++j] ? row[j] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Ftrade_id,row[++j] ? row[j] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[++j] ? row[j] : "", sizeof(data.Fspid) - 1);
            data.Ftype =row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Ffund_code,row[++j] ? row[j] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrade_date,row[++j] ? row[j] : "", sizeof(data.Ftrade_date) - 1);
            strncpy(data.Fconfirm_date,row[++j] ? row[j] : "", sizeof(data.Fconfirm_date) - 1);
            data.Fpur_type=row[++j] ? atoi(row[j]) : 0;
			data.Fpurpose=row[++j] ? atoi(row[j]) : 0;
            data.Ftotal_fee= row[++j] ? atoll(row[j]) : 0;
            data.Ffund_units= row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ffund_net,row[++j] ? row[j] : "", sizeof(data.Ffund_net) - 1);
            data.Fstate= row[++j] ? atoi(row[j]) : 0;
            data.Flstate= row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Fsign,row[++j] ? row[j] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fmemo,row[++j] ? row[j] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[++j] ? row[j] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[++j] ? row[j] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fsubacc_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Fsubacc_time) - 1);
            strncpy(data.Fconfirm_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Fconfirm_time) - 1);
            strncpy(data.Ffetch_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Ffetch_time) - 1);
            strncpy(data.Ffinish_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Ffinish_time) - 1);
			checkSign( "t_fund_trans_process", data);
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

int queryUnfinishTransByTradeId(CMySQL* pMysql, const char* tradeId, vector<FundTransProcess> &dataVec)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Flistid,Ftrade_id,Fspid,Ftype,Ffund_code,date_format(Ftrade_date,'%%Y%%m%%d'),date_format(Fconfirm_date,'%%Y%%m%%d'), "
                    " Fpur_type,Fpurpose,Ftotal_fee,Ffund_units,Ffund_net,Fstate,Flstate,Fsign,Fmemo, "
                    " Fcreate_time,Fmodify_time,Facc_time,Fsubacc_time,Fconfirm_time,Ffetch_time,Ffinish_time "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id='%s' "
                    "   AND Fstate in (%d,%d,%d,%d,%d) AND Flstate=1 ",
                    Sdb2(tradeId),Stb2(tradeId),
                    pMysql->EscapeStr(tradeId).c_str(),
                    PROCESS_TRANS_STATE_BUY_UNCONFIRM,
                    PROCESS_TRANS_STATE_BUY_CONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_UNCONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_CONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_FETCH
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
        	int j=-1;
            MYSQL_ROW row = mysql_fetch_row(pRes);
            FundTransProcess data;
            data.Fid = row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Flistid,row[++j] ? row[j] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Ftrade_id,row[++j] ? row[j] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[++j] ? row[j] : "", sizeof(data.Fspid) - 1);
            data.Ftype =row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Ffund_code,row[++j] ? row[j] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrade_date,row[++j] ? row[j] : "", sizeof(data.Ftrade_date) - 1);
            strncpy(data.Fconfirm_date,row[++j] ? row[j] : "", sizeof(data.Fconfirm_date) - 1);
            data.Fpur_type=row[++j] ? atoi(row[j]) : 0;
			data.Fpurpose=row[++j] ? atoi(row[j]) : 0;
            data.Ftotal_fee= row[++j] ? atoll(row[j]) : 0;
            data.Ffund_units= row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ffund_net,row[++j] ? row[j] : "", sizeof(data.Ffund_net) - 1);
            data.Fstate= row[++j] ? atoi(row[j]) : 0;
            data.Flstate= row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Fsign,row[++j] ? row[j] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fmemo,row[++j] ? row[j] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[++j] ? row[j] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[++j] ? row[j] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fsubacc_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Fsubacc_time) - 1);
            strncpy(data.Fconfirm_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Fconfirm_time) - 1);
            strncpy(data.Ffetch_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Ffetch_time) - 1);
            strncpy(data.Ffinish_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Ffinish_time) - 1);
            dataVec.push_back(data);
			checkSign( "t_fund_trans_process", data);
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
    return iRow;
}

void updateFundTransProcessWithSign(CMySQL* pMysql, FundTransProcess &data, FundTransProcess &dbData)
{
	string sign = genMergeSign("t_fund_trans_process",data,dbData);
	strncpy(data.Fsign,sign.c_str(),sizeof(data.Fsign)-1);
	updateFundTransProcess(pMysql,data);
}
void updateFundTransProcess(CMySQL* pMysql,  FundTransProcess &data)
{
    stringstream tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
    TRACE_DEBUG("updateFundTransProcess:[%s][%ld]",data.Ftrade_id,data.Fid);
	// 灰度期间可能没有数据
    if (data.Fid==MIN_INTEGER)
    {
    	TRACE_DEBUG("updateFundTransProcess没有要更新的数据!:[%s][%ld]",data.Ftrade_id,data.Fid);
		return;
    }
    string trade_id = escapeString(data.Ftrade_id);
    
    tb_name << "fund_db_" << trade_id.substr(trade_id.size() - 2);
    tb_name << ".t_fund_trans_process_" << trade_id.substr(trade_id.size() - 3, 1);

    // 设置需要更新的字段
    kv_map["Fmodify_time"] = toString(data.Fmodify_time);

	TRACE_DEBUG("[Fmodify_time:%s]", kv_map["Fmodify_time"].c_str());
	
	if(!(0 == strcmp("", data.Ftrade_date )))
	{
		kv_map["Ftrade_date"] = data.Ftrade_date;
	}
	if(!(0 == strcmp("", data.Fconfirm_date)))
	{
		kv_map["Fconfirm_date"] = data.Fconfirm_date;
	}
	if(data.Fpur_type!=MIN_INTEGER) 
	{                                               
		kv_map["Fpur_type"] = toString(data.Fpur_type);   
	}
	if(data.Fpurpose!=MIN_INTEGER) 
	{                                               
		kv_map["Fpurpose"] = toString(data.Fpurpose);   
	}
	if(data.Ftotal_fee!=MIN_INTEGER) 
	{                                               
		kv_map["Ftotal_fee"] = toString(data.Ftotal_fee);   
	}
	if(data.Ffund_units!=MIN_INTEGER) 
	{                                               
		kv_map["Ffund_units"] = toString(data.Ffund_units);   
	}
	if(!(0 == strcmp("", data.Ffund_net)))
	{
		kv_map["Ffund_net"] = data.Ffund_net;
	}
	if(data.Fstate!=MIN_INTEGER) 
	{                                               
		kv_map["Fstate"] = toString(data.Fstate);   
	}
	if(data.Flstate!=MIN_INTEGER) 
	{                                               
		kv_map["Flstate"] = toString(data.Flstate);   
	}
	if(!(0 == strcmp("", data.Fsign)))
	{
		kv_map["Fsign"] = data.Fsign;
	} 
	if(!(0 == strcmp("", data.Fmemo)))
	{
		kv_map["Fmemo"] = data.Fmemo;
	}

	if(!(0 == strcmp("", data.Facc_time)))
	{
		kv_map["Facc_time"] = data.Facc_time;
	}

	if(!(0 == strcmp("", data.Fsubacc_time)))
	{
		kv_map["Fsubacc_time"] = data.Fsubacc_time;
	}

	if(!(0 == strcmp("", data.Fconfirm_time)))
	{
		kv_map["Fconfirm_time"] = data.Fconfirm_time;
	}

	if(!(0 == strcmp("", data.Ffetch_time)))
	{
		kv_map["Ffetch_time"] = data.Ffetch_time;
	}

	if(!(0 == strcmp("", data.Ffinish_time)))
	{
		kv_map["Ffinish_time"] = data.Ffinish_time;
	}
	
    ss_cond << "Fid='" <<  toString(data.Fid) <<"' ";

    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, tb_name.str(), ss_cond, kv_map);
	// 灰度期间可能没有数据
    if (affect_row != 1&&gPtrConfig->m_AppCfg.fund_index_trans_grey>=2)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}

// 查询是否存在未确认指数 ,兼容期间增加查询unfirm_fund
bool queryUnfinishTransExistsBySp(CMySQL* pMysql, const string& spid, const string& tradeId)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    int count = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " count(1) "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id = '%s' AND Fspid = '%s' "
                    " AND Fstate in (%d,%d,%d,%d,%d) AND Flstate=1 limit 1",
                    Sdb2(tradeId.c_str()),Stb2(tradeId.c_str()),
                    pMysql->EscapeStr(tradeId).c_str(),
                    pMysql->EscapeStr(spid).c_str(),
                    PROCESS_TRANS_STATE_BUY_UNCONFIRM,
                    PROCESS_TRANS_STATE_BUY_CONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_UNCONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_CONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_FETCH
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 ||iRow>1 )
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        MYSQL_ROW row = mysql_fetch_row(pRes);
        count= row[0] ? atoi(row[0]) : 0;
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
    return count > 0;
}

// 查询是否存在未确认指数 ,兼容期间增加查询unfirm_fund
bool queryUnfinishTransExists(CMySQL* pMysql, const string& tradeId)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
	int count = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " count(1) "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id = '%s' "
                    " AND Fstate in (%d,%d,%d,%d,%d) AND Flstate=1 limit 1",
                    Sdb2(tradeId.c_str()),Stb2(tradeId.c_str()),
                    pMysql->EscapeStr(tradeId).c_str(),
                    PROCESS_TRANS_STATE_BUY_UNCONFIRM,
                    PROCESS_TRANS_STATE_BUY_CONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_UNCONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_CONFIRM,
                    PROCESS_TRANS_STATE_REDEEM_FETCH
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 ||iRow>1 )
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        MYSQL_ROW row = mysql_fetch_row(pRes);
        count= row[0] ? atoi(row[0]) : 0;
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
    return count > 0;
}

void insertFundTransProcess(CMySQL* pMysql, FundTransProcess &data )
{
	//genSign(data);
	string sign = genMergeSign("t_fund_trans_process",data,data);
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_trans_process_%d("
                    " Flistid,Ftrade_id,Fspid,Ffund_code,Ftype,Ftrade_date,Fconfirm_date,Fpur_type,Fpurpose,Ftotal_fee,Ffund_units, "
                    " Ffund_net,Fstate,Flstate,Fsign,Fmemo,Fcreate_time,Fmodify_time,Facc_time,Fsubacc_time,Fconfirm_time,Ffetch_time,Ffinish_time )"
                    " VALUES("
                    " '%s','%s','%s','%s',%d,'%s','%s',%d,%d,%ld,%ld, "
                    " '%s',%d, %d,'%s','%s','%s','%s','%s','%s','%s','%s','%s') ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    data.Ftype,
                    pMysql->EscapeStr(data.Ftrade_date).c_str(),
                    pMysql->EscapeStr(data.Fconfirm_date).c_str(),
                    data.Fpur_type,
                    data.Fpurpose,
                    data.Ftotal_fee,
                    data.Ffund_units,
                    pMysql->EscapeStr(data.Ffund_net).c_str(),
                    data.Fstate,
                    data.Flstate,
                    pMysql->EscapeStr(sign).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    pMysql->EscapeStr(data.Fsubacc_time).c_str(),
                    pMysql->EscapeStr(data.Fconfirm_time).c_str(),
                    pMysql->EscapeStr(data.Ffetch_time).c_str(),
                    pMysql->EscapeStr(data.Ffinish_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

/**
  * 目前只查询出指数数据,前端根据指数数据要求个性展示
  * 已完成的延迟一天显示
  */
int queryUnfinishTransByTradeId4CKV(CMySQL* pMysql, const char* tradeId, vector<FundTransProcess> &dataVec)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Flistid,Ftrade_id,Fspid,Ftype,Ffund_code,date_format(Ftrade_date,'%%Y%%m%%d'),date_format(Fconfirm_date,'%%Y%%m%%d'), "
                    " Fpur_type,Fpurpose,Ftotal_fee,Ffund_units,Ffund_net,Fstate,Flstate,Fsign,Fmemo, "
                    " Fcreate_time,Fmodify_time,Facc_time,Fconfirm_time,Ffetch_time,Ffinish_time "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id='%s' "
                    "   AND Ffinish_time >= curdate() "
                    "   AND Flstate = 1 "
                    "   AND Ftype=%d ",
                    Sdb2(tradeId),Stb2(tradeId),
                    pMysql->EscapeStr(tradeId).c_str(),
                    SPCONFIG_TYPE_ETF
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
            int j=-1;
            MYSQL_ROW row = mysql_fetch_row(pRes);
            FundTransProcess data;
            data.Fid = row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Flistid,row[++j] ? row[j] : "", sizeof(data.Flistid) - 1);
            strncpy(data.Ftrade_id,row[++j] ? row[j] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[++j] ? row[j] : "", sizeof(data.Fspid) - 1);
            data.Ftype =row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Ffund_code,row[++j] ? row[j] : "", sizeof(data.Ffund_code) - 1);
            strncpy(data.Ftrade_date,row[++j] ? row[j] : "", sizeof(data.Ftrade_date) - 1);
            strncpy(data.Fconfirm_date,row[++j] ? row[j] : "", sizeof(data.Fconfirm_date) - 1);
            data.Fpur_type=row[++j] ? atoi(row[j]) : 0;
			data.Fpurpose=row[++j] ? atoi(row[j]) : 0;
            data.Ftotal_fee= row[++j] ? atoll(row[j]) : 0;
            data.Ffund_units= row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ffund_net,row[++j] ? row[j] : "", sizeof(data.Ffund_net) - 1);
            data.Fstate= row[++j] ? atoi(row[j]) : 0;
            data.Flstate= row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Fsign,row[++j] ? row[j] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fmemo,row[++j] ? row[j] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[++j] ? row[j] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[++j] ? row[j] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Facc_time) - 1);
            strncpy(data.Fconfirm_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Fconfirm_time) - 1);
            strncpy(data.Ffetch_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Ffetch_time) - 1);
            strncpy(data.Ffinish_time,row[++j] ? ValiDateStr(row[j]) : "", sizeof(data.Ffinish_time) - 1);
			checkSign( "t_fund_trans_process", data);
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
    return iRow;
}

bool setFundUnfinishTransCKV(CMySQL* mysql, const string& tradeId)
{
	vector<FundTransProcess> fundIndexTransVec;

	gPtrAppLog->debug("setFundUnfinishTransCKV. trade_id=[%s]", tradeId.c_str());
	queryUnfinishTransByTradeId4CKV(mysql, tradeId.c_str(), fundIndexTransVec);
	
	gPtrAppLog->debug("setFundUnfinishTransCKV. fundIndexTransVec.size=[%d].trade_id=[%s]", fundIndexTransVec.size(),tradeId.c_str());
    string key = "unfinish_index_" + tradeId;
	
	string szValue;
	packFundUnfinishTransCKV(fundIndexTransVec,szValue);
	
	gPtrAppLog->debug("setFundUnfinishTransCKV. key=[%s] value=[%s]", key.c_str(), szValue.c_str());
    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_UNFINISH_INDEX,key, szValue))
	{
		// 重试
		int ret =gCkvSvrOperator->set(CKV_KEY_UNFINISH_INDEX,key, szValue);
		return ret==0;
	}
	else
	{
		return true;
	}
}

void packFundUnfinishTransCKV(vector<FundTransProcess>& fundUnfinishVec, string& szValue)
{
	if(fundUnfinishVec.size()<=0)
	{
		szValue="";
		return;
	}
	CParams kvReqSet;
	char szParaName[64] = {0};
	    
	//设置要修改的数据szValue
	for(vector<FundTransProcess>::size_type i= 0; i != fundUnfinishVec.size(); ++i)
	{
		FundTransProcess& unfinishIndex = fundUnfinishVec[i];
		
		snprintf(szParaName, sizeof(szParaName), "Flistid_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Flistid);

		snprintf(szParaName, sizeof(szParaName), "Ftrade_id_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ftrade_id);
		
		snprintf(szParaName, sizeof(szParaName), "Fspid_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fspid);
		
		snprintf(szParaName, sizeof(szParaName), "Ffund_code_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ffund_code);
		
		snprintf(szParaName, sizeof(szParaName), "Ftrade_date_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ftrade_date);

		snprintf(szParaName, sizeof(szParaName), "Fconfirm_date_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fconfirm_date);
		
		snprintf(szParaName, sizeof(szParaName), "Fpur_type_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fpur_type);
		
		snprintf(szParaName, sizeof(szParaName), "Fpurpose_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fpurpose);
		
		snprintf(szParaName, sizeof(szParaName), "Ftotal_fee_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ftotal_fee);
		
		snprintf(szParaName, sizeof(szParaName), "Ffund_units_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ffund_units);		
		
		snprintf(szParaName, sizeof(szParaName), "Ffund_net_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ffund_net);
		
		snprintf(szParaName, sizeof(szParaName), "Fstate_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fstate);
		
		snprintf(szParaName, sizeof(szParaName), "Fcreate_time_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fcreate_time);
		
		snprintf(szParaName, sizeof(szParaName), "Facc_time_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Facc_time);
		
		snprintf(szParaName, sizeof(szParaName), "Fconfirm_time_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Fconfirm_time);
		
		snprintf(szParaName, sizeof(szParaName), "Ffetch_time_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ffetch_time);
		
		snprintf(szParaName, sizeof(szParaName), "Ffinish_time_%zd", i);
		kvReqSet.setParam(szParaName, unfinishIndex.Ffinish_time);

	}
		
	kvReqSet.setParam("total_num",(int)(fundUnfinishVec.size()));
    szValue = kvReqSet.pack();
}

/**
  * 统计当前未完成的申购资产
  * 规则:当Ffund_units为0时,使用Ftotal_fee计算，当申购份额确认Ffund_units不为0，使用fund_units*fund_net计算
  * 受限于申购份额确认当天可以使份额可用，
  * 因此,可以 使用申购确认时候的净值来统计当前资产
  * 输出map key: fundcode
  */
int statUnfinishBuyAssetByTradeId(CMySQL* pMysql, const char* tradeId, map<string,FundUnfinishAssert> &dataMap)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fspid,Ffund_code,sum(if((Ffund_units=0),Ftotal_fee,round(Ffund_units*Ffund_net))) "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id='%s' "
                    "   AND Fstate in (%d,%d) "
                    "   AND Flstate = 1 "
                    " group by Fspid,Ffund_code "
                    ,
                    Sdb2(tradeId),Stb2(tradeId),
                    pMysql->EscapeStr(tradeId).c_str(),
                    PROCESS_TRANS_STATE_BUY_UNCONFIRM,
                    PROCESS_TRANS_STATE_BUY_CONFIRM
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
		dataMap.clear();
        for(int i=0; i<iRow; i++) 
        {
            int j=-1;
            MYSQL_ROW row = mysql_fetch_row(pRes);
            FundUnfinishAssert data;
            strncpy(data.Ftrade_id,tradeId, sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[++j] ? row[j] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ffund_code,row[++j] ? row[j] : "", sizeof(data.Ffund_code) - 1);
            data.Ftotal_fee= row[++j] ? atoll(row[j]) : 0;
			
            dataMap[string(data.Ffund_code)]=data;
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
    return iRow;
}

