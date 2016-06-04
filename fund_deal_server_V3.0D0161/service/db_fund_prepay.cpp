#include "db_fund_prepay.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 






bool queryFundPrepay(CMySQL* pMysql, FundPrepay& data,  bool lock) //标题
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Fuin,Fuid,Facct_type,Fspid, "
                    " Ffund_name,Ffund_code,Ftotal_fee,Fcft_trans_id,Fbank_type, "
                    " Fcard_no,Fstate,Fauthen_state,Frefund_id,Frefund_time,Fmemo, "
                    " Fcreate_time,Fmodify_time,Fopenid,Fstandby3  "
                    " FROM fund_db.t_fund_prepay_%d "
                    " WHERE "
                    " Flistid='%s' " 
                    " %s ",
                    Stb2_listid(data.Flistid),
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
            strncpy(data.Fuin,row[1] ? row[1] : "", sizeof(data.Fuin) - 1);
            data.Fuid = row[2] ? atoi(row[2]) : 0;
            data.Facct_type = row[3] ? atoi(row[3]) : 0;
            strncpy(data.Fspid,row[4] ? row[4] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ffund_name,row[5] ? row[5] : "", sizeof(data.Ffund_name) - 1);
            strncpy(data.Ffund_code,row[6] ? row[6] : "", sizeof(data.Ffund_code) - 1);
            data.Ftotal_fee = row[7] ? atoll(row[7]) : 0;
            strncpy(data.Fcft_trans_id,row[8] ? row[8] : "", sizeof(data.Fcft_trans_id) - 1);
            data.Fbank_type = row[9] ? atoi(row[9]) : 0;
            strncpy(data.Fcard_no,row[10] ? row[10] : "", sizeof(data.Fcard_no) - 1);
            data.Fstate = row[11] ? atoi(row[11]) : 0;
			data.Fauthen_state= row[12] ? atoi(row[12]) : 0;
            strncpy(data.Frefund_id,row[13] ? row[13] : "", sizeof(data.Frefund_id) - 1);
            strncpy(data.Frefund_time,row[14] ? row[14] : "", sizeof(data.Frefund_time) - 1);
            strncpy(data.Fmemo,row[15] ? row[15] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Fcreate_time,row[16] ? row[16] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[17] ? row[17] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fopenid,row[18] ? row[18] : "", sizeof(data.Fopenid) - 1);
            strncpy(data.Fstandby3,row[19] ? row[19] : "", sizeof(data.Fstandby3) - 1);

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



void insertFundPrepay(CMySQL* pMysql, FundPrepay &data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_prepay_%d("
                    " Flistid,Fuin,Fuid,Facct_type,Fspid, "
                    " Ffund_name,Ffund_code,Ftotal_fee,Fcft_trans_id,Fbank_type, "
                    " Fcard_no,Fstate,Frefund_id,Frefund_time,Fmemo, "
                    " Fcreate_time,Fmodify_time,Fopenid,Fstandby3 )"
                    " VALUES("
                    " '%s','%s',%d,%d,'%s', "
                    " '%s','%s',%zd,'%s',%d, "
                    " '%s',%d,'%s','%s','%s', "
                    " '%s','%s','%s','%s')",
                    Stb2_listid(data.Flistid),
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    pMysql->EscapeStr(data.Fuin).c_str(),
                    data.Fuid,
                    data.Facct_type,
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ffund_name).c_str(),
                    pMysql->EscapeStr(data.Ffund_code).c_str(),
                    data.Ftotal_fee,
                    pMysql->EscapeStr(data.Fcft_trans_id).c_str(),
                    data.Fbank_type,
                    pMysql->EscapeStr(data.Fcard_no).c_str(),
                    data.Fstate,
                    pMysql->EscapeStr(data.Frefund_id).c_str(),
                    pMysql->EscapeStr(data.Frefund_time).c_str(),
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Fopenid).c_str(),
                    pMysql->EscapeStr(data.Fstandby3).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


  

/*
*update函数
*update影响行数为1时正确，为0或>1都会抛出异常
*/  
void updateFundPrepay(CMySQL* pMysql, FundPrepay& data )
{

	stringstream tb_name;
    stringstream ss_cond;
    map<string, string> kv_map;
    
	tb_name << "fund_db.t_fund_prepay_" << toString(Stb2_listid(data.Flistid));

    // 设置需要更新的字段
    if(!(0 == strcmp("", data.Fcft_trans_id)))
	{
		kv_map["Fcft_trans_id"] = data.Fcft_trans_id;
	}

	if(data.Fuid != 0)
	{
		kv_map["Fuid"] = toString(data.Fuid);
	}

	if(data.Fstate!= 0)
	{
		kv_map["Fstate"] = toString(data.Fstate);
	}

	if(data.Fauthen_state!= 0)
	{
		kv_map["Fauthen_state"] = toString(data.Fauthen_state);
	}

    kv_map["Fmodify_time"] = data.Fmodify_time;
    
    ss_cond << "Flistid='" << escapeString(data.Flistid) << "'";
    
    // 执行更新数据表操作
    int affect_row = UpdateTable(pMysql, tb_name.str(), ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }

}


