#include "db_fund_profit.h" 
#include "dbsign.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig; 

// ���ӻ������ݿ���
extern CMySQL* gPtrFundDB;
bool queryFundProfit(CMySQL* pMysql, FundProfit& data,  bool lock) //����
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
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
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
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
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
    // ����SQL
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
    // ִ��SQL
    pMysql->Query(szSql, iLen);
}


/*
*update����
*updateӰ������Ϊ1ʱ��ȷ��Ϊ0��>1�����׳��쳣
*/  
void updateFundProfit(CMySQL* pMysql, FundProfit& data )
{	
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
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
                    //������ȷ��UpdateProfitInfo�������֮ǰ���л�ȡȫ��t_fund_profit��¼����
                    pMysql->EscapeStr(genSign("t_fund_profit", data)).c_str(), 
                    pMysql->EscapeStr(data.Flogin_ip).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    data.Fstandby1,
                    //--------where����--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

void updateTotalFeeAndProfit(CMySQL* pMysql, FundProfit& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
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
                    //--------where����--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}
/*
*update����
*updateӰ������Ϊ1ʱ��ȷ��Ϊ0��>1�����׳��쳣
*/  
void updateFundProfitForReward(CMySQL* pMysql, FundProfit& data )
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // ����SQL
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
                    //������ȷ�ϸú��������ⲿ����
                    pMysql->EscapeStr(genSign("t_fund_profit", data)).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    //--------where����--------
                    pMysql->EscapeStr(data.Ftrade_id).c_str(),
                    data.Fcurtype
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // ִ��SQL
    pMysql->Query(szSql, iLen);
    // �ж�Ӱ�������Ƿ�Ψһ
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}



/**
*data���:Ҫ����trade_id,
��������ʱ���᷵�����л�����ۼ�������Ϣ
*����cache
*input timeout ����ʱ������Ϊ��λ
*fundProfitVec ���Σ����������б�
*/
bool setTotalProfit(FundProfit& data, int uid, int timeout, vector<FundProfit> & fundProfitVec)
{
	//int appid = gPtrConfig->m_KvCfg.appid;
    string key = "total_profit_"+ toString(uid);

	string szValue;
	CParams kvReqSet;
	LONG profit = 0;
	LONG total_profit = 0;

	//û��ѯ����¼����0����
	if(queryFundProfitList(gPtrFundDB, data.Ftrade_id, fundProfitVec, false))
	{	
		char szParaName[64] = {0};
	    
		//����Ҫ�޸ĵ�����szValue
		for(vector<FundProfit>::size_type i= 0; i != fundProfitVec.size(); ++i)
		{
			FundProfit& fundProfit = fundProfitVec[i];

			//�����������Ϊ���ջ�ǰһ��,��Ϊ�����������棬���������Ϊ�û���������Ϊ0
			if((toString(GetDateToday()) != fundProfit.Frecon_day) && ( toString(GetDateToday()) != addDays(fundProfit.Frecon_day,1)))
			{
				fundProfit.Fprofit = 0;//���½ṹ������
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
	strncpy(data.Frecon_day, (fundProfitVec.size() == 0) ? "" :fundProfitVec[0].Frecon_day, sizeof(data.Frecon_day) - 1);//���ж���˻�������ʱ�����ֶ������壬��ʹ�÷ֻ��ֶν�����ؼ��㣬�������Զ�ֻ��һ����¼���кܺõļ���
	
	kvReqSet.setParam("total_num",
(int)(fundProfitVec.size()));
    //���ø�����ֻ���������
    kvReqSet.setParam("expire_time", (int)(time(NULL) + timeout));
	kvReqSet.setParam("total_profit",data.Ftotal_profit);
	kvReqSet.setParam("profit",data.Fprofit);
	//kvReqSet.setParam("reward_profit",reward_profit);
	kvReqSet.setParam("Frecon_day", data.Frecon_day);
    szValue = kvReqSet.pack();

    //��szValueд��ckv
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

    //ȡkv����
    CParams kvRspGet;
    gCkvSvrOperator->get(key, kvRspGet);

	if( !kvRspGet.isExists("total_profit") || !kvRspGet.isExists("expire_time") )
	{
		return false;
	}

	if(time(NULL) > kvRspGet.getInt("expire_time"))
	{
		TRACE_DEBUG("ckv-cache value expired.expire_time[%d]", kvRspGet.getInt("expire_time"));
		return false;//���ڷ��ز�����
	}

	//����ʱ�����ݾ�����Ч�ģ����ù�������ʱ��
	data.Ftotal_profit = kvRspGet.getLong("total_profit");
	data.Fprofit = kvRspGet.getLong("profit");
	//data.Freward_profit = kvRspGet.getLong("reward_profit");
	strncpy(data.Frecon_day,kvRspGet.getString("Frecon_day").c_str(), sizeof(data.Frecon_day) - 1);

	//�ֻ���������Ϣ
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
        // ִ�в�ѯ
        pMysql->Query(szSql, iLen);
        // ȡ�����
        pRes = pMysql->FetchResult();
        // ��ȡ�����
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
* ��ѯ�û�������Ϣ,����������
*/
bool queryFundProfitAndCheck(FundProfit& fund_profit, bool& fund_profit_exist, const char* date, LONG profit)
{


	fund_profit_exist = queryFundProfit(gPtrFundDB, fund_profit, true);

	//����ɺ������ڶ��˵Ĳ����ڶ�ǰ�����ڣ�
	//���տ����룻
	//��������ǰһ�������˺������ڲ������ˣ���Ϊ�û��˻���������޽����ǲ�����˵ģ����Կ���ֱ���ں����н��׵������϶��ˡ�
	//�������ڵ�׼ȷ�����㱣֤: 1)�������ܳ���������ǰҪȷ��ǰһ���Ѿ����ˣ�2)����������Ѷ��˽����ս�����ˮ���м��㣬���ǰһ�ոö���
	//��δ���ˣ��ܽ���޷�ƥ������¶���ʧ�ܡ�

	if(fund_profit_exist)
	{
		//����ɶ��˲����ٴη���
		if(strncmp(date, fund_profit.Frecon_day, 8) < 0)
		{
			throw CException(ERR_HAVE_RECON, "the user have recon", __FILE__, __LINE__);
		}
		
		//��������������ˣ�������������ʧ�ܺ��޷��ٴδ���
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

    //ȡkv����
    CParams kvRspGet;
    int ret = gCkvSvrOperator->get(key, kvRspGet);

    //���key�������򷵻�true����ʾckv��û������,�����Ƕ�ckvʧ��
    if (ERR_KEY_NOT_EXIST == ret)
        return 0;
    
    if (0 != ret)
        return -1;

	if( !kvRspGet.isExists("total_profit") || !kvRspGet.isExists("expire_time") )
	{
		return -1;
	}

	//����ʱ�����ݾ�����Ч�ģ����ù�������ʱ��
	total_data.Ftotal_profit = kvRspGet.getLong("total_profit");
	total_data.Fprofit = kvRspGet.getLong("profit");
	//data.Freward_profit = kvRspGet.getLong("reward_profit");
	SCPY(total_data.Frecon_day,kvRspGet.getString("Frecon_day").c_str());

	//�ֻ���������Ϣ
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
 * �ú���ֻ���������˽ӿڵ��ã��������˺���������ͬ����ckv
 * ͬ���߼�Ϊ:��CKV�в�ѯ����CKV�����������ݣ��ٽ����˵��������ݸ��µ�CKV��
 * @param data 
 * @param uid 
 * @param timeout 
 * @return 
 */
bool addTotalProfit2Cache(const FundProfit& data, int uid, int timeout)
{
    vector<FundProfit> fundProfitVec;
    FundProfit ckvTotalData;

    //����ckv-cache��ѯ������
	if(0 != getTotalProfitFromCache(ckvTotalData, uid, fundProfitVec))
	{
        //��ѯ���ɹ�ֱ�ӷ���
        TRACE_DEBUG("get total profit from cache faild for uid:%d", uid);
		return false;
	}

    /**
     * ���CVK�����иû����¼���滻ԭ�м�¼�����û�иü�¼�������Ӹü�¼
     */
    vector<FundProfit>::iterator ite;
    for(ite = fundProfitVec.begin(); ite != fundProfitVec.end(); ++ite) {
        //�������������¼spid��CKV���У���recon_day��CKV�е�Ҫ����
        //�������ݸ���Ϊ���µ�
        if (0 == strncmp(data.Fspid, ite->Fspid, sizeof(data.Fspid))) {
            //CKV�е�ʱ�����Ҫ���¼�¼��ʱ����ʱ��ֱ�Ӹ���Ϊ���£��������CKV��ͬ
            //һ�̻�CKV��ʱ����ڵ���Ҫ����ļ�¼ʱ��ʱ��������CKV
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

    //����ѱ�����δ����¼����ֱ����Ӽ�¼
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
	    
	//����Ҫ�޸ĵ�����szValue
	for(vector<FundProfit>::size_type i= 0; i != fundProfitVec.size(); ++i)
	{
		FundProfit& fundProfit = fundProfitVec[i];

		//�����������Ϊ���ջ�ǰһ��,��Ϊ�����������棬���������Ϊ�û���������Ϊ0
		if(strToday != fundProfit.Frecon_day && strToday != addDays(fundProfit.Frecon_day,1))
		{
			fundProfit.Fprofit = 0;//���½ṹ������
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

    //���ж���˻�������ʱ�����ֶ������壬��ʹ�÷ֻ��ֶν�����ؼ��㣬�������Զ�ֻ��һ����¼���кܺõļ���
	SCPY(total_data.Frecon_day, (fundProfitVec.size() == 0) ? "" :fundProfitVec[0].Frecon_day);
	
	kvReqSet.setParam("total_num",(int)(fundProfitVec.size()));
    //���ø�����ֻ���������
    kvReqSet.setParam("expire_time", (int)(time(NULL) + timeout));
	kvReqSet.setParam("total_profit",total_data.Ftotal_profit);
	kvReqSet.setParam("profit",total_data.Fprofit);
	kvReqSet.setParam("Frecon_day", total_data.Frecon_day);
    szValue = kvReqSet.pack();

    //��szValueд��ckv
	if(gCkvSvrOperator->set(CKV_KEY_TOTAL_PROFIT, key, szValue))
	{
		return false;
	}
	else
	{
		return true;
	}
}


