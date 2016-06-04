#include "db_fund_unconfirm.h"
#include "common.h"
#include "fund_commfunc.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


/**
 * 查询用户未确认份额
 * @param pMysql 
 * @param data 
 */
bool queryFundUnconfirm(CMySQL* pMysql,  FUND_UNCONFIRM &data, bool islock /* = false */)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Fspid,date_format(Ftrade_date,'%%Y%%m%%d'),date_format(Fconfirm_date,'%%Y%%m%%d'), "
                    " Ftotal_fee,Fcfm_total_fee,Fcfm_units,Funuse_units,Ffund_net,Fstate,Flstate, "
                    " Fsign,Fmemo,Fcreate_time,Fmodify_time "
                    " FROM fund_db_%02d.t_unconfirm_fund_%d "
                    " WHERE Ftrade_id = '%s' AND Fspid = '%s' AND Ftrade_date='%s' "
                    " %s ",
                    Sdb2(data.Ftrade_id),Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ftrade_date).c_str(),
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
            strncpy(data.Ftrade_id,row[++j] ? row[j] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[++j] ? row[j] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ftrade_date,row[++j] ? row[j] : "", sizeof(data.Ftrade_date) - 1);
            strncpy(data.Fconfirm_date,row[++j] ? row[j] : "", sizeof(data.Fconfirm_date) - 1);
            data.Ftotal_fee= row[++j] ? atoll(row[j]) : 0;
            data.Fcfm_total_fee= row[++j] ? atoll(row[j]) : 0;
            data.Fcfm_units= row[++j] ? atoll(row[j]) : 0;
            data.Funuse_units= row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ffund_net,row[++j] ? row[j] : "", sizeof(data.Ffund_net) - 1);
            data.Fstate= row[++j] ? atoi(row[j]) : 0;
            data.Flstate= row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Fsign,row[++j] ? row[j] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fmemo,row[++j] ? row[j] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[++j] ? row[j] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[++j] ? row[j] : "", sizeof(data.Fmodify_time) - 1);
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
 * 查询用户未确认份额
 * @param pMysql 
 * @param data 
 */
int queryValidFundUnconfirmByTradeId(CMySQL* pMysql, const char* tradeId, vector<FUND_UNCONFIRM> &dataVec)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Ftrade_id,Fspid,date_format(Ftrade_date,'%%Y%%m%%d'),date_format(Fconfirm_date,'%%Y%%m%%d'), "
                    " Ftotal_fee,Fcfm_total_fee,Fcfm_units,Funuse_units,Ffund_net,Fstate,Flstate, "
                    " Fsign,Fmemo,Fcreate_time,Fmodify_time "
                    " FROM fund_db_%02d.t_unconfirm_fund_%d "
                    " WHERE Ftrade_id = '%s' AND Ftrade_date>=date_add(now(), interval -15 day) AND Fstate<>4 ",
                    Sdb2(tradeId),Stb2(tradeId),
                    pMysql->EscapeStr(tradeId).c_str()
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
			FUND_UNCONFIRM data;
            data.Fid = row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ftrade_id,row[++j] ? row[j] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[++j] ? row[j] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ftrade_date,row[++j] ? row[j] : "", sizeof(data.Ftrade_date) - 1);
            strncpy(data.Fconfirm_date,row[++j] ? row[j] : "", sizeof(data.Fconfirm_date) - 1);
            data.Ftotal_fee= row[++j] ? atoll(row[j]) : 0;
            data.Fcfm_total_fee= row[++j] ? atoll(row[j]) : 0;
            data.Fcfm_units= row[++j] ? atoll(row[j]) : 0;
            data.Funuse_units= row[++j] ? atoll(row[j]) : 0;
            strncpy(data.Ffund_net,row[++j] ? row[j] : "", sizeof(data.Ffund_net) - 1);
            data.Fstate= row[++j] ? atoi(row[j]) : 0;
            data.Flstate= row[++j] ? atoi(row[j]) : 0;
            strncpy(data.Fsign,row[++j] ? row[j] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fmemo,row[++j] ? row[j] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[++j] ? row[j] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[++j] ? row[j] : "", sizeof(data.Fmodify_time) - 1);
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

void updateFundUnconfirmById(CMySQL* pMysql,  FUND_UNCONFIRM &data)
{ 	
	stringstream tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
	TRACE_DEBUG("updateFundUnconfirmById:[%s][%ld][%s]",data.Ftrade_id,data.Fid, data.Fmodify_time);
    string trade_id = escapeString(data.Ftrade_id);
    
    tb_name << "fund_db_" << trade_id.substr(trade_id.size() - 2);
    tb_name << ".t_unconfirm_fund_" << trade_id.substr(trade_id.size() - 3, 1);

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
	if(data.Ftotal_fee!=MIN_INTEGER) 
	{                                               
		kv_map["Ftotal_fee"] = toString(data.Ftotal_fee);   
	}
	if(data.Fcfm_total_fee!=MIN_INTEGER) 
	{                                               
		kv_map["Fcfm_total_fee"] = toString(data.Fcfm_total_fee);   
	}
	if(data.Fcfm_units!=MIN_INTEGER) 
	{                                               
		kv_map["Fcfm_units"] = toString(data.Fcfm_units);   
	}
	if(data.Funuse_units!=MIN_INTEGER) 
	{                                               
		kv_map["Funuse_units"] = toString(data.Funuse_units);   
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

    ss_cond << "Fid='" <<  toString(data.Fid) <<"' ";

    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, tb_name.str(), ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}


/**
 * 查询是否存在未确认资产份额 
 * 存在:return true
 * 不存在: return false
 */
bool queryFundUnconfirmExists(CMySQL* pMysql, const string& spid, const string& tradeId)
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
                    " FROM fund_db_%02d.t_unconfirm_fund_%d "
                    " WHERE Ftrade_id = '%s' AND Fspid = '%s' "
                    " AND Fstate=1 AND Flstate=1 ",
                    Sdb2(tradeId.c_str()),Stb2(tradeId.c_str()),
                    pMysql->EscapeStr(tradeId).c_str(),
                    pMysql->EscapeStr(spid).c_str()
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

//计算签名，更新操作会将全量数据传进来，所以在该类中计算签名即可
void genSign(FUND_UNCONFIRM &data)
{
	stringstream ss;
    char buff[128] = {0};
    
    ss << data.Ftrade_id<< "|" ;
    ss << data.Fspid<< "|" ;
    ss << data.Ftotal_fee<< "|" ;
    ss << data.Fcfm_total_fee<< "|" ;
    ss << "020494b6787f58fabaae42bf544ca4c4";

    getMd5(ss.str().c_str(), ss.str().size(), buff);

	strncpy(data.Fsign, buff, sizeof(data.Fsign) - 1);
}

/**
 * 增加未确认份额
 */
void insertFundUnconfirm(CMySQL* pMysql, FUND_UNCONFIRM &data )
{
	genSign(data);
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_unconfirm_fund_%d("
                    " Ftrade_id,Fspid,Ftrade_date,Fconfirm_date,Ftotal_fee,Fcfm_total_fee,Fcfm_units,Funuse_units, "
                    " Ffund_net,Fstate,Flstate,Fsign,Fmemo,Fcreate_time,Fmodify_time )"
                    " VALUES("
                    " '%s','%s','%s','%s',%ld,%ld,%ld, "
                    " %ld,'%s',%d,%d,'%s','%s','%s','%s') ",
                    Sdb2(data.Ftrade_id),
                    Stb2(data.Ftrade_id),
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ftrade_date).c_str(),
                    pMysql->EscapeStr(data.Fconfirm_date).c_str(),
                    data.Ftotal_fee,
                    data.Fcfm_total_fee,
                    data.Fcfm_units,
                    data.Funuse_units,
                    pMysql->EscapeStr(data.Ffund_net).c_str(),
                    data.Fstate,
                    data.Flstate,
                    pMysql->EscapeStr(data.Fsign).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

bool setFundUnconfirm(CMySQL* mysql,const string& tradeId)
{
	vector<FUND_UNCONFIRM> fundUnconfirmVec;

	queryValidFundUnconfirmByTradeId(mysql, tradeId.c_str(), fundUnconfirmVec);
	
	gPtrAppLog->debug("setFundUnconfirm ckv. fundUnconfirmVec.size=[%d].trade_id=[%s]", fundUnconfirmVec.size(),tradeId.c_str());
    string key = "unconfirm_"+ tradeId;
	
	string szValue;
	packFundUnconfirm(fundUnconfirmVec,szValue);
	
	gPtrAppLog->debug("setFundUnconfirm ckv. key=[%s] value=[%s]", key.c_str(), szValue.c_str());
    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_UNCONFIRM,key, szValue))
	{
		// 重试
		int ret =gCkvSvrOperator->set(CKV_KEY_UNCONFIRM,key, szValue);
		return ret==0;
	}
	else
	{
		return true;
	}
}

void packFundUnconfirm(vector<FUND_UNCONFIRM>& fundUnconfirmVec, string& szValue)
{
	if(fundUnconfirmVec.size()<=0)
	{
		szValue="";
		return;
	}
	CParams kvReqSet;
	char szParaName[64] = {0};
	    
	//设置要修改的数据szValue
	for(vector<FUND_UNCONFIRM>::size_type i= 0; i != fundUnconfirmVec.size(); ++i)
	{
		FUND_UNCONFIRM& unconfirm = fundUnconfirmVec[i];
		
		snprintf(szParaName, sizeof(szParaName), "Fid_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Fid);

		snprintf(szParaName, sizeof(szParaName), "Ftrade_id_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Ftrade_id);
		
		snprintf(szParaName, sizeof(szParaName), "Fspid_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Fspid);
		
		snprintf(szParaName, sizeof(szParaName), "Ftrade_date_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Ftrade_date);

		snprintf(szParaName, sizeof(szParaName), "Fconfirm_date_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Fconfirm_date);
		
		snprintf(szParaName, sizeof(szParaName), "Ftotal_fee_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Ftotal_fee);
		
		snprintf(szParaName, sizeof(szParaName), "Fcfm_total_fee_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Fcfm_total_fee);
		
		snprintf(szParaName, sizeof(szParaName), "Fcfm_units_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Fcfm_units);		
		
		snprintf(szParaName, sizeof(szParaName), "Funuse_units_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Funuse_units);
		
		snprintf(szParaName, sizeof(szParaName), "Ffund_net_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Ffund_net);
		
		snprintf(szParaName, sizeof(szParaName), "Fstate_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Fstate);
		
		snprintf(szParaName, sizeof(szParaName), "Flstate_%zd", i);
		kvReqSet.setParam(szParaName, unconfirm.Flstate);

	}
		
	kvReqSet.setParam("total_num",(int)(fundUnconfirmVec.size()));
    szValue = kvReqSet.pack();
	
}

