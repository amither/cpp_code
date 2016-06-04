#include "db_fund_profit.h" 
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

// 连接基金数据库句柄
extern CMySQL* gPtrFundDB;
bool queryFundProfit(CMySQL* pMysql, FundProfit& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ftrade_id,Fcurtype,Frecon_balance,Frecon_day,Fprofit, "
                    " Ftotal_profit,Flogin_ip,Fsign,Fcreate_time,Fmodify_time, "
                    " Fmemo,Fexplain,Freward_profit,Fvalid_money,Ftplus_redem_money,Fspid,Ffinancial_days, Fprecheck_money, Fprecheck_day, Fprecheck_profit,"
                    " Fstandby7"
                    " FROM fund_db_%02d.t_fund_profit_%d "
                    " WHERE "
                    " Ftrade_id='%s'  AND " 
                    " Fcurtype=%d " 
                    " %s ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype,
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
            strncpy(data.Ftrade_id,row[0] ? row[0] : "", sizeof(data.Ftrade_id) - 1);
            data.Fcurtype = row[1] ? atoi(row[1]) : 0;
            data.Frecon_balance = row[2] ? atoll(row[2]) : 0;
            strncpy(data.Frecon_day,row[3] ? row[3] : "", sizeof(data.Frecon_day) - 1);
            data.Fprofit = row[4] ? atoll(row[4]) : 0;
            data.Ftotal_profit = row[5] ? atoll(row[5]) : 0;
            strncpy(data.Flogin_ip,row[6] ? row[6] : "", sizeof(data.Flogin_ip) - 1);
            strncpy(data.Fsign,row[7] ? row[7] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[10] ? row[10] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[11] ? row[11] : "", sizeof(data.Fexplain) - 1);
            data.Freward_profit= row[12] ? atoll(row[12]) : 0;
            data.Fvalid_money= row[13] ? atoll(row[13]) : 0;
            data.Ftplus_redem_money= row[14] ? atoll(row[14]) : 0;
			strncpy(data.Fspid,row[15] ? row[15] : "", sizeof(data.Fspid) - 1);
			data.Ffinancial_days= row[16] ? atoi(row[16]) : 0;
            data.Fprecheck_money = row[17] ? atoll(row[17]) : 0;
            strncpy(data.Fprecheck_day,row[18] ? row[18] : "", sizeof(data.Fprecheck_day) - 1);
            data.Fprecheck_profit = row[19] ? atoll(row[19]) : 0;
			data.Fstandby7 = row[20] ? atoll(row[20]) : 0;
			checkSign( "t_fund_profit", data);
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

bool queryFundProfitList(CMySQL* pMysql, string trade_id, vector<FundProfit>& dataVec,  bool lock)
{
	MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ftrade_id,Fcurtype,Frecon_balance,Frecon_day,Fprofit, "
                    " Ftotal_profit,Flogin_ip,Fsign,Fcreate_time,Fmodify_time, "
                    " Fmemo,Fexplain,Freward_profit,Fspid,Ffinancial_days "
                    " FROM fund_db_%02d.t_fund_profit_%d "
                    " WHERE "
                    " Ftrade_id='%s'  " 
                    " %s ",
                    Sdb2(trade_id.c_str()),
                    Stb2(trade_id.c_str()),
                    pMysql->EscapeStr(trade_id).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
			FundProfit data;
			memset(&data, 0,sizeof(FundProfit));
			
            strncpy(data.Ftrade_id,row[0] ? row[0] : "", sizeof(data.Ftrade_id) - 1);
            data.Fcurtype = row[1] ? atoi(row[1]) : 0;
            data.Frecon_balance = row[2] ? atoll(row[2]) : 0;
            strncpy(data.Frecon_day,row[3] ? row[3] : "", sizeof(data.Frecon_day) - 1);
            data.Fprofit = row[4] ? atoll(row[4]) : 0;
            data.Ftotal_profit = row[5] ? atoll(row[5]) : 0;
            strncpy(data.Flogin_ip,row[6] ? row[6] : "", sizeof(data.Flogin_ip) - 1);
            strncpy(data.Fsign,row[7] ? row[7] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[10] ? row[10] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fexplain,row[11] ? row[11] : "", sizeof(data.Fexplain) - 1);
			data.Freward_profit= row[12] ? atoll(row[12]) : 0;
			strncpy(data.Fspid,row[13] ? row[13] : "", sizeof(data.Fspid) - 1);
			data.Ffinancial_days= row[14] ? atoi(row[14]) : 0;
            checkSign( "t_fund_profit", data);

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


void insertFundProfit(CMySQL* pMysql, FundProfit &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_profit_%d("
                    " Ftrade_id,Fcurtype,Frecon_balance,Frecon_day,Fprofit, "
                    " Ftotal_profit,Flogin_ip,Fsign,Fcreate_time,Fmodify_time,Fvalid_money,Ftplus_redem_money,Fspid,Ffinancial_days,Fstandby1)"
                    " VALUES("
                    " '%s',%d,%zd,'%s',%zd, "
                    " %zd,'%s','%s','%s','%s',%zd,%zd,'%s',%d,%d)",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype,
                    data.Frecon_balance,
                    pMysql->EscapeStr(data.Frecon_day).c_str(),
                    data.Fprofit,
                    data.Ftotal_profit,
                    pMysql->EscapeStr(data.Flogin_ip).c_str(),
                    pMysql->EscapeStr(genSign("t_fund_profit", data)).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    data.Fvalid_money,
                    data.Ftplus_redem_money,
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Ffinancial_days,
                    data.Fstandby1
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundProfit(CMySQL* pMysql, FundProfit& data )
{	
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_profit_%d SET "
                    " Frecon_balance=%zd,"
                    " Frecon_day='%s',"
                    " Fprofit=%zd,"
                    " Ftotal_profit=%zd,"
                    " Fvalid_money=%zd,"
                    " Ftplus_redem_money=%zd,"
                    " Ffinancial_days=%d,"
                    " Fsign='%s',"
                    " Flogin_ip='%s',"
                    " Fmodify_time='%s', "
                    " Fstandby1=%d "
                    " WHERE "
                    " Ftrade_id='%s' AND "
                    " Fcurtype=%d", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Frecon_balance,
                    pMysql->EscapeStr(data.Frecon_day).c_str(),
                    data.Fprofit,
                    data.Ftotal_profit,
                    data.Fvalid_money,
                    data.Ftplus_redem_money,
                    data.Ffinancial_days,
                    //代码检查确认UpdateProfitInfo这个函数之前均有获取全量t_fund_profit记录数据
                    pMysql->EscapeStr(genSign("t_fund_profit", data)).c_str(), 
                    pMysql->EscapeStr(data.Flogin_ip).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    data.Fstandby1,
                    //--------where条件--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void updateTotalFeeAndProfit(CMySQL* pMysql, FundProfit& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_profit_%d SET "
                    " Fprecheck_money=%zd,"
                    " Fprecheck_day='%s',"
                    " Fprecheck_profit=%zd,"
                    " Fmodify_time='%s',"
                    " Fstandby7='%zd'"
                    " WHERE "
                    " Ftrade_id='%s' AND "
                    " Fcurtype=%d", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Frecon_balance,
                    pMysql->EscapeStr(data.Frecon_day).c_str(),
                    data.Fprofit,
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    data.Fstandby7,
                    //--------where条件--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}
/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundProfitForReward(CMySQL* pMysql, FundProfit& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db_%02d.t_fund_profit_%d SET "
                    " Freward_profit=%zd,"
                    " Ftotal_profit=%zd,"
                    " Fsign='%s',"
                    " Fmodify_time='%s' "
                    " WHERE "
                    " Ftrade_id='%s' AND "
                    " Fcurtype=%d", 
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    data.Freward_profit,
                    data.Ftotal_profit,
                    //代码检查确认该函数已无外部调用
                    pMysql->EscapeStr(genSign("t_fund_profit", data)).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where条件--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}



/**
*data入参:要传入trade_id,
做出参数时，会返回所有基金的累计收益信息
*设置cache
*input timeout 过期时长，秒为单位
*fundProfitVec 出参，返回收益列表
*/
bool setTotalProfit(FundProfit& data, int uid, int timeout, vector<FundProfit> & fundProfitVec)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
    string key = "total_profit_"+ toString(uid);

	string szValue;
	CParams kvReqSet;
	LONG profit = 0;
	LONG total_profit = 0;

	//没查询到记录，当0处理
	if(queryFundProfitList(gPtrFundDB, data.Ftrade_id, fundProfitVec, false))
	{	
		char szParaName[64] = {0};
	    
		//设置要修改的数据szValue
		for(vector<FundProfit>::size_type i= 0; i != fundProfitVec.size(); ++i)
		{
			FundProfit& fundProfit = fundProfitVec[i];

			//如果对账日期为当日或前一日,认为都是昨日收益，其它情况认为用户昨日收益为0
			if((toString(GetDateToday()) != fundProfit.Frecon_day) && ( toString(GetDateToday()) != addDays(fundProfit.Frecon_day,1)))
			{
				fundProfit.Fprofit = 0;//更新结构体内容
			}
			
			snprintf(szParaName, sizeof(szParaName), "profit_%zd", i);
			kvReqSet.setParam(szParaName, fundProfit.Fprofit);
			
			snprintf(szParaName, sizeof(szParaName), "total_profit_%zd", i);
			kvReqSet.setParam(szParaName, fundProfit.Ftotal_profit);
			
			snprintf(szParaName, sizeof(szParaName), "Frecon_day_%zd", i);
			kvReqSet.setParam(szParaName, fundProfit.Frecon_day);

			snprintf(szParaName, sizeof(szParaName), "spid_%zd", i);
			kvReqSet.setParam(szParaName, fundProfit.Fspid);

			snprintf(szParaName, sizeof(szParaName), "financial_days_%zd", i);
			kvReqSet.setParam(szParaName, fundProfit.Ffinancial_days);

			snprintf(szParaName, sizeof(szParaName), "fcreate_time_%zd", i);
			kvReqSet.setParam(szParaName, fundProfit.Fcreate_time);

			profit += fundProfit.Fprofit;
			total_profit += fundProfit.Ftotal_profit;

		}
		
	}

	data.Ftotal_profit = total_profit;
	data.Fprofit = profit;
	strncpy(data.Frecon_day, (fundProfitVec.size() == 0) ? "" :fundProfitVec[0].Frecon_day, sizeof(data.Frecon_day) - 1);//当有多个账户有收益时，该字段无意义，请使用分户字段进行相关计算，这样可以对只有一条记录进行很好的兼容
	
	kvReqSet.setParam("total_num",
(int)(fundProfitVec.size()));
    //设置个基金分户的总收益
    kvReqSet.setParam("expire_time", (int)(time(NULL) + timeout));
	kvReqSet.setParam("total_profit",data.Ftotal_profit);
	kvReqSet.setParam("profit",data.Fprofit);
	//kvReqSet.setParam("reward_profit",reward_profit);
	kvReqSet.setParam("Frecon_day", data.Frecon_day);
    szValue = kvReqSet.pack();

    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_TOTAL_PROFIT, key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
	
}


bool getTotalProfit(FundProfit& data, int uid, vector<FundProfit> & fundProfitVec)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
    string key = "total_profit_"+ toString(uid);

    //取kv数据
    CParams kvRspGet;
    gCkvSvrOperator->get(key, kvRspGet);

	if( !kvRspGet.isExists("total_profit") || !kvRspGet.isExists("expire_time") )
	{
		return false;
	}

	if(time(NULL) > kvRspGet.getInt("expire_time"))
	{
		TRACE_DEBUG("ckv-cache value expired.expire_time[%d]", kvRspGet.getInt("expire_time"));
		return false;//过期返回不存在
	}

	//不超时的数据就是有效的，不用关心入账时间
	data.Ftotal_profit = kvRspGet.getLong("total_profit");
	data.Fprofit = kvRspGet.getLong("profit");
	//data.Freward_profit = kvRspGet.getLong("reward_profit");
	strncpy(data.Frecon_day,kvRspGet.getString("Frecon_day").c_str(), sizeof(data.Frecon_day) - 1);

	//分基金收益信息
	int total_num = atoi(kvRspGet.getString("total_num").c_str());

	for(int i = 0; i < total_num; i++)
	{
		FundProfit fundProfit;
		strncpy(fundProfit.Fspid, kvRspGet.getString(string("spid_" + toString(i)).c_str()).c_str(), sizeof(fundProfit.Fspid) - 1);
		strncpy(fundProfit.Frecon_day, kvRspGet.getString(string("Frecon_day_" + toString(i)).c_str()).c_str(), sizeof(fundProfit.Frecon_day) - 1);
		fundProfit.Fprofit = kvRspGet.getLong(string("profit_" + toString(i)).c_str());
		fundProfit.Ftotal_profit= kvRspGet.getLong(string("total_profit_" + toString(i)).c_str());
		fundProfit.Ffinancial_days = kvRspGet.getLong(string("financial_days_" + toString(i)).c_str());
		strncpy(fundProfit.Fcreate_time,kvRspGet.getString(string("fcreate_time_" + toString(i)).c_str()).c_str(), sizeof(fundProfit.Fcreate_time) - 1);
		
		fundProfitVec.push_back(fundProfit);
	}

	return true;

    
}

LONG countTotalReconBalance(CMySQL* pMysql,const string &trade_id)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    LONG totalfee=0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " sum(Frecon_balance)  "
                    " FROM fund_db_%02d.t_fund_profit_%d "
                    " WHERE "
                    " Ftrade_id='%s'  " ,
                    Sdb2(trade_id.c_str()),
                    Stb2(trade_id.c_str()),
                    pMysql->EscapeStr(trade_id).c_str()
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
        
        MYSQL_ROW row = mysql_fetch_row(pRes);  
        totalfee = row[0] ? atoll(row[0]) : 0;
        
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
    return totalfee;
}

/**
* 查询用户收益信息,检查对账日期
*/
bool queryFundProfitAndCheck(FundProfit& fund_profit, bool& fund_profit_exist, const char* date, LONG profit)
{


	fund_profit_exist = queryFundProfit(gPtrFundDB, fund_profit, true);

	//已完成后续日期对账的不可在对前面日期；
	//当日可重入；
	//程序不限制前一日无入账后续日期不可入账，因为用户账户无余额且无交易是不会对账的，所以可以直接在后续有交易的日期上对账。
	//入账日期的准确由两点保证: 1)入账批跑程序当日入账前要确定前一日已经入账；2)本程序根据已对账金额及当日交易流水进行计算，如果前一日该对账
	//而未对账，总金额无法匹配而导致对账失败。

	if(fund_profit_exist)
	{
		//已完成对账不可再次发起
		if(strncmp(date, fund_profit.Frecon_day, 8) < 0)
		{
			throw CException(ERR_HAVE_RECON, "the user have recon", __FILE__, __LINE__);
		}
		
		//当日允许重入对账，避免批量对账失败后无法再次处理
		if(0 == strncmp(date, fund_profit.Frecon_day, 8))
		{
			TRACE_NORMAL("the user have recon. trade_id[%s].recon_date[%s].", fund_profit.Ftrade_id, date);
			if(profit == fund_profit.Fprofit)
			{
				throw CException(ERR_REPEAT_ENTRY, "the user have recon", __FILE__, __LINE__);
			}
			else
			{
				TRACE_NORMAL("repeat enter param profit diff. db[%lld], input[%lld]", fund_profit.Fprofit, profit);
				throw CException(ERR_REPEAT_ENTRY_DIFF, "repeat enter param profit diff.", __FILE__, __LINE__);
			}
		}
        if(strncmp(date, fund_profit.Fprecheck_day, 8) == 0)
        {
            return true;
        }
	}

    return false;
}


int getTotalProfitFromCache(FundProfit& total_data, int uid, vector<FundProfit> & fundProfitVec)
{
    string key = "total_profit_"+ toString(uid);

    //取kv数据
    CParams kvRspGet;
    int ret = gCkvSvrOperator->get(key, kvRspGet);

    //如果key不存在则返回true，表示ckv中没有数据,而不是读ckv失败
    if (ERR_KEY_NOT_EXIST == ret)
        return 0;
    
    if (0 != ret)
        return -1;

	if( !kvRspGet.isExists("total_profit") || !kvRspGet.isExists("expire_time") )
	{
		return -1;
	}

	//不超时的数据就是有效的，不用关心入账时间
	total_data.Ftotal_profit = kvRspGet.getLong("total_profit");
	total_data.Fprofit = kvRspGet.getLong("profit");
	//data.Freward_profit = kvRspGet.getLong("reward_profit");
	SCPY(total_data.Frecon_day,kvRspGet.getString("Frecon_day").c_str());

	//分基金收益信息
	int total_num = atoi(kvRspGet.getString("total_num").c_str());

	for(int i = 0; i < total_num; i++)
	{
		FundProfit fundProfit;
		SCPY(fundProfit.Fspid, kvRspGet.getString(string("spid_" + toString(i)).c_str()).c_str());
		SCPY(fundProfit.Frecon_day, kvRspGet.getString(string("Frecon_day_" + toString(i)).c_str()).c_str());
		fundProfit.Fprofit = kvRspGet.getLong(string("profit_" + toString(i)).c_str());
		fundProfit.Ftotal_profit= kvRspGet.getLong(string("total_profit_" + toString(i)).c_str());
		fundProfit.Ffinancial_days = kvRspGet.getLong(string("financial_days_" + toString(i)).c_str());
		SCPY(fundProfit.Fcreate_time,kvRspGet.getString(string("fcreate_time_" + toString(i)).c_str()).c_str());
		fundProfitVec.push_back(fundProfit);
	}

	return 0;    
}

/**
 * 该函数只被收益入账接口调用，收益入账后将收益数据同步到ckv
 * 同步逻辑为:从CKV中查询现有CKV中有收益数据，再将入账的收益数据更新到CKV中
 * @param data 
 * @param uid 
 * @param timeout 
 * @return 
 */
bool addTotalProfit2Cache(const FundProfit& data, int uid, int timeout)
{
    vector<FundProfit> fundProfitVec;
    FundProfit ckvTotalData;

    //调用ckv-cache查询总收益
	if(0 != getTotalProfitFromCache(ckvTotalData, uid, fundProfitVec))
	{
        //查询不成功直接返回
        TRACE_DEBUG("get total profit from cache faild for uid:%d", uid);
		return false;
	}

    /**
     * 如果CVK中已有该基金记录则替换原有记录，如果没有该记录则新增加该记录
     */
    vector<FundProfit>::iterator ite;
    for(ite = fundProfitVec.begin(); ite != fundProfitVec.end(); ++ite) {
        //如果传入的收益记录spid在CKV中有，且recon_day比CKV中的要新则将
        //收益数据更新为最新的
        if (0 == strncmp(data.Fspid, ite->Fspid, sizeof(data.Fspid))) {
            //CKV中的时间比需要更新记录的时间早时，直接更新为最新，否则对于CKV中同
            //一商户CKV中时间大于等需要处理的记录时间时，不更新CKV
            if (strncmp(data.Frecon_day, ite->Frecon_day, sizeof(data.Frecon_day)) > 0) {
                *(ite) = data;
                break;
            } else {
                TRACE_DEBUG("fund total profit record(spid=%s,recon_day=%s) already update to ckv",
                    data.Fspid, data.Frecon_day);
                return true;
            }
        }
    }

    //如果已遍历完未到记录，则直接添加记录
    if (ite == fundProfitVec.end()) {
        fundProfitVec.push_back(data);
    }

    string key = "total_profit_"+ toString(uid);

	string szValue;
	CParams kvReqSet;

    FundProfit total_data;
    memset(&total_data, 0, sizeof(total_data));
    char szParaName[64] = {0};
    string strToday = toString(GetDateToday());
	    
	//设置要修改的数据szValue
	for(vector<FundProfit>::size_type i= 0; i != fundProfitVec.size(); ++i)
	{
		FundProfit& fundProfit = fundProfitVec[i];

		//如果对账日期为当日或前一日,认为都是昨日收益，其它情况认为用户昨日收益为0
		if(strToday != fundProfit.Frecon_day && strToday != addDays(fundProfit.Frecon_day,1))
		{
			fundProfit.Fprofit = 0;//更新结构体内容
		}
		
		snprintf(szParaName, sizeof(szParaName), "profit_%zd", i);
		kvReqSet.setParam(szParaName, fundProfit.Fprofit);
		
		snprintf(szParaName, sizeof(szParaName), "total_profit_%zd", i);
		kvReqSet.setParam(szParaName, fundProfit.Ftotal_profit);
		
		snprintf(szParaName, sizeof(szParaName), "Frecon_day_%zd", i);
		kvReqSet.setParam(szParaName, fundProfit.Frecon_day);

		snprintf(szParaName, sizeof(szParaName), "spid_%zd", i);
		kvReqSet.setParam(szParaName, fundProfit.Fspid);

		snprintf(szParaName, sizeof(szParaName), "financial_days_%zd", i);
		kvReqSet.setParam(szParaName, fundProfit.Ffinancial_days);
		
		snprintf(szParaName, sizeof(szParaName), "fcreate_time_%zd", i);
		kvReqSet.setParam(szParaName, fundProfit.Fcreate_time);

		total_data.Fprofit += fundProfit.Fprofit;
		total_data.Ftotal_profit += fundProfit.Ftotal_profit;

	}		

    //当有多个账户有收益时，该字段无意义，请使用分户字段进行相关计算，这样可以对只有一条记录进行很好的兼容
	SCPY(total_data.Frecon_day, (fundProfitVec.size() == 0) ? "" :fundProfitVec[0].Frecon_day);
	
	kvReqSet.setParam("total_num",(int)(fundProfitVec.size()));
    //设置个基金分户的总收益
    kvReqSet.setParam("expire_time", (int)(time(NULL) + timeout));
	kvReqSet.setParam("total_profit",total_data.Ftotal_profit);
	kvReqSet.setParam("profit",total_data.Fprofit);
	kvReqSet.setParam("Frecon_day", total_data.Frecon_day);
    szValue = kvReqSet.pack();

    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_TOTAL_PROFIT, key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}


