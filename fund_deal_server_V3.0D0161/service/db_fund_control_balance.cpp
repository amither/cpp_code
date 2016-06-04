#include "fund_commfunc.h"
#include "db_fund_control_balance.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 


bool isConsumFundSpid(const string&fundSpid)
{
     //只有南方基金才可能是预付卡购买用户
    return(fundSpid == gPtrConfig->m_AppCfg.consum_fund_spid);
}

bool isWxPrePayCardBusinessUser(CMySQL* pMysql,const string&fundSpid,ST_FUND_CONTROL_INFO &controlInfo)
{
   
    if (false == isConsumFundSpid(fundSpid))
    {
        return false;
    }
    
    strncpy(controlInfo.Ftrade_id,controlInfo.Ftrade_id,sizeof(controlInfo.Ftrade_id)),
    controlInfo.Ftype=1;
    
    return queryFundControlInfo(pMysql,controlInfo,false);
}

bool isWxPrePayCardBusinessUser(CMySQL* pMysql,const string&fundSpid,const string& trade_id)
{
   
    if (false == isConsumFundSpid(fundSpid))
    {
        return false;
    }
    
    ST_FUND_CONTROL_INFO data;
    memset(&data,0,sizeof(ST_FUND_CONTROL_INFO));
    strncpy(data.Ftrade_id,trade_id.c_str(),sizeof(data.Ftrade_id)),
    data.Ftype=1;
    
    return queryFundControlInfo(pMysql,data,false);
}

/**
  * 检查预付卡购买
  */
bool checkWxPreCardBuy(CMySQL* pMysql, ST_FUND_CONTROL_INFO& controlParams, ST_FUND_CONTROL_INFO& controlInfo,bool lock)
{	
    //微信王府井预付卡商户临时方案检查
    memset(&controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    strncpy(controlInfo.Ftrade_id,controlParams.Ftrade_id,sizeof(controlInfo.Ftrade_id));
    controlInfo.Ftype=1; 	
    bool hasControl = queryFundControlInfo(pMysql, controlInfo,  lock);
	char msg[128]={0};
    //首次充值,可以购买
	if(!hasControl)
    {
        memset(&controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
		return true;
    }
	if(controlInfo.Ftotal_fee==0)
	{
		return true;
	}
	
	// 已有预付卡:预付卡号与购买卡号不一致,禁止购买
    if(0!=strcmp(controlInfo.Fcard_no,controlParams.Fcard_no))
    {	
		return false;
    }
	// 已有预付卡:预付卡商户号与购买商户号不一致,禁止购买
    if(0!=strcmp(controlInfo.Fspid,controlParams.Fspid))
    {
		return false;
    }
	return true;
}


bool queryFundControlInfo(CMySQL* pMysql, ST_FUND_CONTROL_INFO& data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Ftrade_id,Fspid,Ffund_spid,Ftotal_fee, Fuin,"
                    " Ftotal_profit,Ftype,Ffirst_profit_day,Flast_profit, Fcur_type, "
                    " Fcard_no,Fcard_partner "
                    " FROM fund_db.t_fund_control_balance  "
                    " WHERE "
                    " Ftrade_id='%s' AND Ftype=%d  AND Flstate=1 " 
                    " %s ",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Ftype,
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
        for (int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Ftrade_id,row[0] ? row[0] : "", sizeof(data.Ftrade_id) - 1);
            strncpy(data.Fspid,row[1] ? row[1] : "", sizeof(data.Fspid) - 1);
            strncpy(data.Ffund_spid,row[2] ? row[2] : "", sizeof(data.Ffund_spid) - 1);
            data.Ftotal_fee = row[3] ? atoll(row[3]) : 0;
            strncpy(data.Fuin,row[4] ? row[4] : "", sizeof(data.Fuin) - 1);
            data.Ftotal_profit = row[5] ? atoll(row[5]) : 0;
            data.Ftype = row[6] ? atoi(row[6]) : 0;
            strncpy(data.Ffirst_profit_day,row[7] ? row[7] : "", sizeof(data.Ffirst_profit_day) - 1);
            data.Flast_profit= row[8] ? atoll(row[8]) : 0;
            data.Fcur_type = row[9] ? atoi(row[9]) : 0;
            strncpy(data.Fcard_no,row[10] ? row[10] : "", sizeof(data.Fcard_no) - 1);
            strncpy(data.Fcard_partner,row[11] ? row[11] : "", sizeof(data.Fcard_partner) - 1);
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


void  insertFundControlInfo(CMySQL* pMysql, ST_FUND_CONTROL_INFO& data )
{
    char signBuf[65]={0};
    string signSrc = string(data.Ftrade_id)+"|"+data.Ftrade_id+"|"+toString(data.Ftotal_fee);
    getMd5(signSrc.c_str(), signSrc.length(), signBuf);
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO  fund_db.t_fund_control_balance("
                    " Ftrade_id,Fspid,Ffund_spid,Ftotal_fee, Fuin,"
                    " Ftotal_profit,Ftype,Ffirst_profit_day,Flast_profit, Fcur_type,"
                    " Fcreate_time,Fmodify_time,Flstate,Fcard_no,Fcard_partner )"
                    " VALUES("
                    " '%s','%s','%s',%ld,'%s', "
                    " %ld,%d,'%s',%ld,%d, "
                    "  '%s','%s',%d,'%s','%s')",
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    pMysql->EscapeStr(data.Fspid).c_str(),
                    pMysql->EscapeStr(data.Ffund_spid).c_str(),
                    data.Ftotal_fee,
                    pMysql->EscapeStr(data.Fuin).c_str(),
                    data.Ftotal_profit,
                    data.Ftype,
                    pMysql->EscapeStr(data.Ffirst_profit_day).c_str(),
                    data.Flast_profit,
                    data.Fcur_type,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    1,
                    pMysql->EscapeStr(data.Fcard_no).c_str(),
                    pMysql->EscapeStr(data.Fcard_partner).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

void addFundControlBalance(CMySQL* pMysql, LONG addFee ,const string&systime,const string& trade_id,int business_type)
{
    string querySql = "UPDATE  fund_db.t_fund_control_balance  SET ";

    querySql += string(" Ftotal_fee = Ftotal_fee + ")+toString(addFee)+" ,"; 
    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(systime)+"' ";   
    
    querySql += string(" WHERE Ftrade_id='")+pMysql->EscapeStr(trade_id)+string("' AND Flstate=1 AND Ftype= ")+toString(business_type);   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


void subFundControlBalance(CMySQL* pMysql, LONG subFee ,const string&systime,const string& trade_id,int business_type)
{
    string querySql = "UPDATE  fund_db.t_fund_control_balance  SET ";

    querySql += string(" Ftotal_fee = Ftotal_fee - ")+toString(subFee)+" ,"; 
    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(systime)+"' ";   
    
    querySql += string(" WHERE Ftrade_id='")+pMysql->EscapeStr(trade_id)+"' AND Flstate=1 AND Ftype="+toString(business_type)+" AND Ftotal_fee>="+toString(subFee);   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void addFundControlProfit(CMySQL* pMysql, LONG addProfitFee,const string &profitDay,const string&systime,const string& trade_id,int business_type)
{
    string querySql = "UPDATE  fund_db.t_fund_control_balance  SET ";

    querySql += string(" Ftotal_profit = Ftotal_profit + ")+toString(addProfitFee)+" ,"; 
    
    querySql += string(" Ftotal_fee = Ftotal_fee + ")+toString(addProfitFee)+" ,"; 

    querySql += string(" Flast_profit = ")+toString(addProfitFee)+" ,";
    
    querySql += string(" Flast_profit_day = '")+pMysql->EscapeStr(profitDay)+"' ,";
    
    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(systime)+"' ";   
    
    querySql += string(" WHERE Ftrade_id='")+pMysql->EscapeStr(trade_id)+"' AND Flstate=1 AND Ftype= "+toString(business_type);   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


