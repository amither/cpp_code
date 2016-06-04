#include "db_change_sp_record.h"


extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


/**
* 查询指定时间后转换成功的记录
*/
bool queryChangeSpRecord(CMySQL* pMysql, ChangeSpRecord& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Ftrade_id,Fori_spid,Fnew_spid,Fbuy_id, "
                    " Fredem_id,Fcft_bank_billno,Fstate,Fcreate_time,Fmodify_time "
                    " FROM fund_db.t_change_sp_record "
                    " WHERE "
                    " Ftrade_id='%s' and " 
                    " Fstate= 1 and " 
                    " Fmodify_time >= '%s' " 
                    " %s ",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
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
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fori_spid,row[2] ? row[2] : "", sizeof(data.Fori_spid) - 1);
            strncpy(data.Fnew_spid,row[3] ? row[3] : "", sizeof(data.Fnew_spid) - 1);
            strncpy(data.Fbuy_id,row[4] ? row[4] : "", sizeof(data.Fbuy_id) - 1);
            strncpy(data.Fredem_id,row[5] ? row[5] : "", sizeof(data.Fredem_id) - 1);
			strncpy(data.Fcft_bank_billno,row[6] ? row[6] : "", sizeof(data.Fcft_bank_billno) - 1);
            data.Fstate = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);

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
* 查询转换中的记录
*/
bool queryChangingSpRecord(CMySQL* pMysql, ChangeSpRecord& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fimt_id,Ftrade_id,Fori_spid,Fnew_spid,Fbuy_id, "
                    " Fredem_id,Fcft_bank_billno,Fstate,Fcreate_time,Fmodify_time "
                    " FROM fund_db.t_change_sp_record "
                    " WHERE Fstate = 0 "
                    " and Ftrade_id='%s' " 
                    " %s ",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
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
            data.Fimt_id = row[0] ? atoll(row[0]) : 0;
            strncpy(data.Ftrade_id,row[1] ? row[1] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fori_spid,row[2] ? row[2] : "", sizeof(data.Fori_spid) - 1);
            strncpy(data.Fnew_spid,row[3] ? row[3] : "", sizeof(data.Fnew_spid) - 1);
            strncpy(data.Fbuy_id,row[4] ? row[4] : "", sizeof(data.Fbuy_id) - 1);
            strncpy(data.Fredem_id,row[5] ? row[5] : "", sizeof(data.Fredem_id) - 1);
            strncpy(data.Fcft_bank_billno,row[6] ? row[6] : "", sizeof(data.Fcft_bank_billno) - 1);
            data.Fstate = row[7] ? atoi(row[7]) : 0;
            strncpy(data.Fcreate_time,row[8] ? row[8] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[9] ? row[9] : "", sizeof(data.Fmodify_time) - 1);

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


void insertChangeSpRecord(CMySQL* pMysql, ChangeSpRecord &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_change_sp_record("
                    " Ftrade_id,Fori_spid,Fnew_spid,Fbuy_id,Fredem_id, Fcft_bank_billno,"
                    " Fstate,Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " '%s','%s','%s','%s','%s', '%s', "
                    " %d,'%s','%s')",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fori_spid).c_str(),
                    pMysql->EscapeStr(data.Fnew_spid).c_str(),
                    pMysql->EscapeStr(data.Fbuy_id).c_str(),
                    pMysql->EscapeStr(data.Fredem_id).c_str(),
                    pMysql->EscapeStr(data.Fcft_bank_billno).c_str(),
                    data.Fstate,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


  

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateChangeSpRecord(CMySQL* pMysql, ChangeSpRecord& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " UPDATE fund_db.t_change_sp_record SET "
                    " Fstate=%d "
                    " WHERE "
                    " Fimt_id=%zd", 
                    data.Fstate,
                    //--------where条件--------
                    data.Fimt_id
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
/** 代码有BUG, 删除逻辑
bool checkCanChangeTradeSp(CMySQL* pMysql,bool throwExp)
{
	FundReconLog data;
	memset(&data, 0,sizeof(FundReconLog));
	data.Frecon_type = RECON_TYPE_PROFIT;
	data.Frecon_state = RECON_STATE_FINISH;

	//如果时间是配置的时间点之后，则判断当日的收益入账是否成功，不成功不能做基份额转换
	//如果时间是配置的时间点之前，则判断前一日的收益入账是否成功，不成功不能做基份额转换
	if(GetTimeToday() > gPtrConfig->m_AppCfg.change_sp_stop_time)
	{
		strncpy(data.Frecon_date, toString(GetDateToday()).c_str(), sizeof(data.Frecon_date) - 1);
	}
	else
	{
		strncpy(data.Frecon_date, getLastDate().c_str(), sizeof(data.Frecon_date) - 1);
	}

	if(queryFundReconLog(pMysql, data,  true))
	{
		return true;
	}

	if(throwExp)
	{
		throw CException(ERR_CONNOT_CHANGESP_TIME, "我们正在为您分配收益，为了确保您的收益不受损失，收益分配完成前，转换功能暂停", __FILE__, __LINE__);
	}
	return false;
}
*/



