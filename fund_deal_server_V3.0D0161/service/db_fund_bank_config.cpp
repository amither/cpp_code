#include "db_fund_bank_config.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 






bool queryFundBankConfig(CMySQL* pMysql, FundBankConfig& data,  bool lock) //标题
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
		iLen = snprintf(szSql, sizeof(szSql),
			" SELECT "
			" Fbank_type,Fbank_name,Flstate,Fsupport_type,Fonce_quota, "
			" Fday_quota,Farrival_type,Fcreate_time,Fmodify_time, "
			" Fmemo, Fbank_abbr,Farrival_time "
			" FROM fund_db.t_fund_bank_config "
			" WHERE "
			" Fbank_type=%d  AND " 
			" Flstate=%d " 
			" %s ",
			data.Fbank_type,
			LSTATE_VALID,
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
			data.Fbank_type = row[0] ? atoi(row[0]) : 0;
			strncpy(data.Fbank_name,row[1] ? row[1] : "", sizeof(data.Fbank_name) - 1);
			data.Flstate = row[2] ? atoi(row[2]) : 0;
			data.Fsupport_type = row[3] ? atoi(row[3]) : 0;
			data.Fonce_quota = row[4] ? atoll(row[4]) : 0;
			data.Fday_quota = row[5] ? atoll(row[5]) : 0;
			data.Farrival_type = row[6] ? atoi(row[6]) : 0;
			strncpy(data.Fcreate_time,row[7] ? row[7] : "", sizeof(data.Fcreate_time) - 1);
			strncpy(data.Fmodify_time,row[8] ? row[8] : "", sizeof(data.Fmodify_time) - 1);
			strncpy(data.Fmemo,row[9] ? row[9] : "", sizeof(data.Fmemo) - 1);
			strncpy(data.Fbank_abbr,row[10] ? row[10] : "", sizeof(data.Fbank_abbr) - 1);
			strncpy(data.Farrival_time,row[11] ? row[11] : "", sizeof(data.Farrival_time) - 1);

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


bool queryFundAllBankConfig(CMySQL* pMysql, vector<FundBankConfig>& dataVec,  bool lock)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
		iLen = snprintf(szSql, sizeof(szSql),
			" SELECT "
			" Fbank_type,Fbank_name,Flstate,Fsupport_type,Fonce_quota, "
			" Fday_quota,Farrival_type,Fcreate_time,Fmodify_time, "
			" Fmemo, Fbank_abbr,Farrival_time,Fbank_area,Fbank_city "
			" FROM fund_db.t_fund_bank_config "
			" %s ",
			lock ? "FOR UPDATE" : ""
			);
		gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
		// 执行查询
		pMysql->Query(szSql, iLen);
		// 取结果集
		pRes = pMysql->FetchResult();
		// 获取结果行
		iRow = mysql_num_rows(pRes);
		if(iRow <0 )
		{
			throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
		}
		for(int i=0; i<iRow; i++) 
		{
			MYSQL_ROW row = mysql_fetch_row(pRes);
			FundBankConfig data;
			memset(&data, 0,sizeof(FundBankConfig));

			data.Fbank_type = row[0] ? atoi(row[0]) : 0;
			strncpy(data.Fbank_name,row[1] ? row[1] : "", sizeof(data.Fbank_name) - 1);
			data.Flstate = row[2] ? atoi(row[2]) : 0;
			data.Fsupport_type = row[3] ? atoi(row[3]) : 0;
			data.Fonce_quota = row[4] ? atoll(row[4]) : 0;
			data.Fday_quota = row[5] ? atoll(row[5]) : 0;
			data.Farrival_type = row[6] ? atoi(row[6]) : 0;
			strncpy(data.Fcreate_time,row[7] ? row[7] : "", sizeof(data.Fcreate_time) - 1);
			strncpy(data.Fmodify_time,row[8] ? row[8] : "", sizeof(data.Fmodify_time) - 1);
			strncpy(data.Fmemo,row[9] ? row[9] : "", sizeof(data.Fmemo) - 1);
			strncpy(data.Fbank_abbr,row[10] ? row[10] : "", sizeof(data.Fbank_abbr) - 1);
			strncpy(data.Farrival_time,row[11] ? row[11] : "", sizeof(data.Farrival_time) - 1);
			strncpy(data.Fbank_area,row[12] ? row[12] : "", sizeof(data.Fbank_area) - 1);
			strncpy(data.Fbank_city,row[13] ? row[13] : "", sizeof(data.Fbank_city) - 1);

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
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundBankConfig(CMySQL* pMysql, FundBankConfig& data )
{
	char szSql[MAX_SQL_LEN + 1]={0};
	// 构造SQL
	int iLen = snprintf(szSql, sizeof(szSql),
		" UPDATE fund_db.t_fund_bank_config SET "
		" Fonce_quota='%zd',"
		" Fday_quota='%zd',"
		" Fmodify_time='%s' "
		" WHERE "
		" Fbank_type=%d", 
		data.Fonce_quota,
		data.Fday_quota,
		pMysql->EscapeStr(data.Fmodify_time).c_str(),
		//--------where条件--------
		data.Fbank_type
		);
	gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
	// 执行SQL
	pMysql->Query(szSql, iLen);
	// 判断影响行数是否唯一
	if (pMysql->AffectedRows() != 1)
	{
		TRACE_ERROR("affected row[%d] not equal 1. Fbank_type=[%d], ", pMysql->AffectedRows(), data.Fbank_type);
		throw CException(ERR_DB_AFFECTED, "affected row not equal 1!", __FILE__, __LINE__);
	}
}


bool setSupportBankToKV(FundBankConfig& data)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
	//string key = gPtrConfig->m_KvCfg.kvAddress + "fund_support_bank_" + toString(data.Fbank_type);
	string key = "fund_support_bank_" + toString(data.Fbank_type);
	string szValue;

	CParams kvReqSet;
	//设置要修改的数据szValue
	kvReqSet.setParam("Fbank_type",data.Fbank_type);
	kvReqSet.setParam("Fbank_abbr",data.Fbank_abbr);
	kvReqSet.setParam("Fbank_name",data.Fbank_name);
	kvReqSet.setParam("Flstate",data.Flstate);
	kvReqSet.setParam("Fsupport_type",data.Fsupport_type);
	kvReqSet.setParam("Fonce_quota",data.Fonce_quota);
	kvReqSet.setParam("Fday_quota",data.Fday_quota);
	kvReqSet.setParam("Farrival_type",data.Farrival_type);
	kvReqSet.setParam("Farrival_time",data.Farrival_time);
	kvReqSet.setParam("Fmemo",data.Fmemo);
	kvReqSet.setParam("Fbank_area",data.Fbank_area);
	kvReqSet.setParam("Fbank_city",data.Fbank_city);
	szValue = kvReqSet.pack();

	//将szValue写入ckv
	if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_FUND_SUPPORT_BANK), key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}

}

void delBankFromKV(int bank_type)
{
	string key = "fund_support_bank_" + toString(bank_type);
	gCkvSvrOperator->del(NOT_REUPDATE_CKV(CKV_KEY_FUND_SUPPORT_BANK),key);
}


bool setAllSupportBankToKV(CMySQL* pMysql)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
	//string key = gPtrConfig->m_KvCfg.kvAddress + "fund_support_bank_all";
	string key = "fund_support_bank_all";

	vector<FundBankConfig> fundBankConfigVec;
	if( !queryFundAllBankConfig(pMysql, fundBankConfigVec, false))
	{
		TRACE_DEBUG("no fund sp config");
		return true;
	}


	string szValue;
	CParams kvReqSet;
	char szParaName[64] = {0};

	//设置要修改的数据szValue
	int total_num = 0;
	for(vector<FundBankConfig>::iterator iter = fundBankConfigVec.begin(); iter != fundBankConfigVec.end(); ++iter)
	{
		FundBankConfig fundBankConfig = *iter;

		/**
		* 失效的也要能查询出来，要告知用户为什么失效
		if(fundBankConfig.Flstate != LSTATE_VALID)
		{
		//将失效的从cache 中删除
		delBankFromKV(fundBankConfig.Fbank_type);
		continue;
		}
		*/

		snprintf(szParaName, sizeof(szParaName), "Fbank_type_%d", total_num);
		kvReqSet.setParam(szParaName, fundBankConfig.Fbank_type);
		snprintf(szParaName, sizeof(szParaName), "Fbank_abbr_%d", total_num);
		kvReqSet.setParam(szParaName, fundBankConfig.Fbank_abbr);
		snprintf(szParaName, sizeof(szParaName), "Fbank_name_%d", total_num);
		kvReqSet.setParam(szParaName, fundBankConfig.Fbank_name);
		snprintf(szParaName, sizeof(szParaName), "Flstate_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Flstate);
		snprintf(szParaName, sizeof(szParaName), "Fsupport_type_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Fsupport_type);
		snprintf(szParaName, sizeof(szParaName), "Fonce_quota_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Fonce_quota);
		snprintf(szParaName, sizeof(szParaName), "Fday_quota_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Fday_quota);
		snprintf(szParaName, sizeof(szParaName), "Farrival_type_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Farrival_type);
		snprintf(szParaName, sizeof(szParaName), "Farrival_time_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Farrival_time);
		snprintf(szParaName, sizeof(szParaName), "Fmemo_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Fmemo);
		snprintf(szParaName, sizeof(szParaName), "Fbank_area_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Fbank_area);
		snprintf(szParaName, sizeof(szParaName), "Fbank_city_%d", total_num);
		kvReqSet.setParam(szParaName,fundBankConfig.Fbank_city);

		//写入单条银行记录
		if(!setSupportBankToKV(fundBankConfig))
		{
			return false;
		}

		total_num += 1;

	}

	kvReqSet.setParam("total_num",
		total_num);
	szValue = kvReqSet.pack();

	//将szValue写入kv
	//将szValue写入ckv
	if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_FUND_SUPPORT_BANK_ALL), key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}

}

bool getSupportBankFromKV(int iBankType, string &sBankName)
{
	string key = "fund_support_bank_" + toString(iBankType);
	string szValue;

	//取kv数据
	CParams kvRspGet;
	int iRet = gCkvSvrOperator->get(key, kvRspGet);

	if(iRet != 0)
	{
		return false;
	}

	if(kvRspGet.getInt("Flstate") != 1)
	{
		return false;	
	}

	sBankName = kvRspGet.getString("Fbank_name");
	return true;
}


