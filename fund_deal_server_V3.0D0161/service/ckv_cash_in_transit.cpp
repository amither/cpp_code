#include "ckv_cash_in_transit.h"
#include "dbsign.h"

int queryCashInTransitBAFetch4CKV(CMySQL* pMysql, const char* tradeId, vector<CashInTransit> &dataVec)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Ftrade_id,Ftotal_fee,Ftype,Facc_time "
                    " FROM fund_db_%02d.t_fund_balance_order_%d "
                    " WHERE Ftrade_id='%s' "
                    "   AND Fstate in (%d,%d) "
                    "   AND Ftype in (%d,%d) "
                    "   AND Ffetch_result=%d "
                    "   AND Flstate=1 ",
                    Sdb2(tradeId),Stb2(tradeId),
                    pMysql->EscapeStr(tradeId).c_str(),
                    FUND_FETCH_SUBACC_OK,
                    FUND_FETCH_OK,
                    OP_TYPE_BA_FETCH,
                    OP_TYPE_BA_FETCH_T1,
                    FETCH_RESULT_INIT
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);

		BalanceConfigCache balanceConfigCache = getCacheBalanceConfig(pMysql);
		
        for(int i=0; i<iRow; i++) 
        {
        	int j=-1;
            MYSQL_ROW row = mysql_fetch_row(pRes);
            CashInTransit data;
            strncpy(data.listid,row[++j] ? row[j] : "", sizeof(data.listid) - 1);
            strncpy(data.trade_id,row[++j] ? row[j] : "", sizeof(data.trade_id) - 1);
            data.total_fee=row[++j] ? atoi(row[j]) : 0;
			int type=row[++j] ? atoi(row[j]) : 0;
			if(type==OP_TYPE_BA_FETCH)
            	data.state=CASH_IN_TRANSIT_BALANCE_T0FETCH;
			else
				data.state=CASH_IN_TRANSIT_BALANCE_T1FETCH;
            data.curtype=balanceConfigCache.curtype;
            strncpy(data.spid, balanceConfigCache.spid.c_str(), sizeof(data.spid) - 1);
            strncpy(data.acc_time,row[++j] ? row[j] : "", sizeof(data.acc_time) - 1);
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
/**
  * 显示未完成的过程数据(后续增加)
 
int queryUnfinishProcessByTradeId4CKV(CMySQL* pMysql, const char* tradeId, vector<CashInTransit> &dataVec)
{
	MYSQL_RES* pRes = NULL;
	char szSql[MAX_SQL_LEN] = {0};
	int iLen = 0, iRow = 0;
	try
	{
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Ftrade_id,Ftotal_fee,Fspid,Fstate,date_format(Facc_time,'%%Y%%m%%d') "
                    " FROM fund_db_%02d.t_fund_trans_process_%d "
                    " WHERE Ftrade_id='%s' "
                    "   AND Ffinish_time > curdate() "
                    "   AND Flstate = 1 ",
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
            CashInTransit data;
            strncpy(data.listid,row[++j] ? row[j] : "", sizeof(data.listid) - 1);
            strncpy(data.trade_id,row[++j] ? row[j] : "", sizeof(data.trade_id) - 1);
            data.total_fee=row[++j] ? atoi(row[j]) : 0;

			strncpy(data.spid,row[++j] ? row[j] : "", sizeof(data.spid) - 1);	
            data.curtype=querySubaccCurtype(pMysql,data.spid);
			
            data.state=row[++j] ? atoi(row[j]) : 0;
			strncpy(data.acc_time,row[++j] ? row[j] : "", sizeof(data.acc_time) - 1);
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
*/

bool setCashInTransitCKV(CMySQL* mysql, const string& tradeId)
{
	vector<CashInTransit> cashVec;

	gPtrAppLog->debug("setCashInTransitCKV. trade_id=[%s]", tradeId.c_str());
	queryCashInTransitBAFetch4CKV(mysql,tradeId.c_str(),cashVec);
	// 后续增加过程中的交易到CKV
	//queryUnfinishProcessByTradeId4CKV(mysql, tradeId.c_str(), cashVec);
	
    string key = "cash_in_transit_" + tradeId;
	
	string szValue;
	packCashInTransitCKV(cashVec,szValue);
	
	gPtrAppLog->debug("setCashInTransitCKV. key=[%s] value=[%s]", key.c_str(), szValue.c_str());
    //将szValue写入ckv
	if(gCkvSvrOperator->set(CKV_KEY_CASH_IN_TRANSIT,key, szValue))
	{
		// 重试
		int ret =gCkvSvrOperator->set(CKV_KEY_CASH_IN_TRANSIT,key, szValue);
		return ret==0;
	}
	else
	{
		return true;
	}
}

void packCashInTransitCKV(vector<CashInTransit>& cashInTransitVec, string& szValue)
{
	if(cashInTransitVec.size()<=0)
	{
		szValue="";
		return;
	}
	CParams kvReqSet;
	char szParaName[64] = {0};
	    
	//设置要修改的数据szValue
	for(vector<CashInTransit>::size_type i= 0; i != cashInTransitVec.size(); ++i)
	{
		CashInTransit& data = cashInTransitVec[i];
		
		snprintf(szParaName, sizeof(szParaName), "listid_%zd", i);
		kvReqSet.setParam(szParaName, data.listid);
		
		snprintf(szParaName, sizeof(szParaName), "spid_%zd", i);
		kvReqSet.setParam(szParaName, data.spid);
		
		snprintf(szParaName, sizeof(szParaName), "curtype_%zd", i);
		kvReqSet.setParam(szParaName, data.curtype);
		
		snprintf(szParaName, sizeof(szParaName), "state_%zd", i);
		kvReqSet.setParam(szParaName, data.state);

		snprintf(szParaName, sizeof(szParaName), "total_fee_%zd", i);
		kvReqSet.setParam(szParaName, data.total_fee);
		
		snprintf(szParaName, sizeof(szParaName), "acc_time_%zd", i);
		kvReqSet.setParam(szParaName, data.acc_time);

	}
		
	kvReqSet.setParam("total_num",(int)(cashInTransitVec.size()));
    szValue = kvReqSet.pack();
}

