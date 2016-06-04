#include "db_fund_user_risk.h"
#include "common.h"
#include "fund_commfunc.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

/**
 * 记录用户评测流水
 * @param pMysql 
 * @param data 
 */
void saveFundUserRisk(CMySQL* pMysql,  ST_USER_RISK &data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    
    // 构造SQL
    string cur_time = getSysTime();
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db_%02d.t_fund_user_risk_%d("
                    " Fqqid,Fspid,Frisk_score,Fsubject_no,Fanswer, Fclient_ip,"
                    " Frisk_type,Fmemo,Fcreate_time,Fmodify_time)"
                    " VALUES("
                    " '%s','%s',%d,'%s','%s','%s' ,"
                    " '%d','%s','%s','%s')",
                    Sdb2_qqid(data.Fqqid),Stb2_qqid(data.Fqqid),
                    pMysql->EscapeStr(data.Fqqid).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    data.Frisk_score,
                    pMysql->EscapeStr(data.Fsubject_no).c_str(),
                    pMysql->EscapeStr(data.Fanswer).c_str(),
                    pMysql->EscapeStr(data.Fclient_ip).c_str(),
                    data.Frisk_type,
                    pMysql->EscapeStr(data.Fmemo).c_str(),
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str() );
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}


/**
 * 查询用户评测流水
 * @param pMysql 
 * @param data 
 */
bool queryFundUserRisk(CMySQL* pMysql,  ST_USER_RISK &data)
{
      MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
  try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fqqid,Fspid,Frisk_score,Fsubject_no,Fanswer, "
                    " Frisk_type,Fmemo,Fcreate_time,Fmodify_time"
                    " FROM fund_db_%02d.t_fund_user_risk_%d "
                    " WHERE Fqqid = '%s' "
                    " order by Fcreate_time DESC limit 1 ",
                    Sdb2_qqid(data.Fqqid),Stb2_qqid(data.Fqqid),
                    pMysql->EscapeStr(data.Fqqid).c_str()
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
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Fqqid,row[0] ? row[0] : "", sizeof(data.Fqqid) - 1);
	     strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
	     data.Frisk_score= row[2] ? atoi(row[2]) : 0;
	     strncpy(data.Fsubject_no,row[3] ? row[3] : "", sizeof(data.Fsubject_no) - 1);
	     strncpy(data.Fanswer,row[4] ? row[4] : "", sizeof(data.Fanswer) - 1);
            data.Frisk_type= row[5] ? atoi(row[5]) : 0;
	     strncpy(data.Fmemo,row[6] ? row[6] : "", sizeof(data.Fmemo) - 1);
	     strncpy(data.Fcreate_time,row[7] ? row[7] : "", sizeof(data.Fcreate_time) - 1);
	     strncpy(data.Fmodify_time,row[8] ? row[8] : "", sizeof(data.Fmodify_time) - 1);
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
