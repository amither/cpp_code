/**
  * FileName: fund_query_total_profit_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-11-05
  * Description: �����׷��� ���𿪻� Դ�ļ�
  */

#include "fund_commfunc.h"
#include "fund_query_total_profit_service.h"


FundQueryTotalProfit::FundQueryTotalProfit(CMySQL* mysql,int type)
{
    m_fund_conn = mysql;

    memset(&m_fund_profit, 0, sizeof(FundProfit));
    m_servicetype = type;
}


/**
  * service step 1: �����������
  */
void FundQueryTotalProfit::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char *pMsg = (char*)(rqst->idata);

    // ��ȡ����
    m_params.readIntParam(pMsg, "uid", 10000,MAX_INTEGER);
    m_params.readStrParam(pMsg, "qlskey", 0, 64);
    m_params.readStrParam(pMsg, "qluin", 0, 64);
}


/**
  * ����������ȡ�ڲ�����
  */
void FundQueryTotalProfit::CheckParams() throw (CException)
{
    if (m_servicetype == CHECK_LOGIN)
    {
        checkSession(m_params["qlskey"], m_params["qluin"], "100096");
    }
}

/**
  * ִ�л����˻�����
  */
void FundQueryTotalProfit::excute() throw (CException)
{

	CheckParams();

	queryTotalProfit();
    
}


void FundQueryTotalProfit::queryTotalProfit() throw (CException)
{

	//����ckv-cache��ѯ������
	if(getTotalProfit(m_fund_profit, m_params.getInt("uid"), m_fundProfitVec))
	{
		return;//��ѯ�ɹ�ֱ�ӷ���
	}

	//cache �в����ڻ�������ѯDB�����cache
	CheckFundBind();

	strncpy(m_fund_profit.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_profit.Ftrade_id) - 1);
	
	setTotalProfit(m_fund_profit,m_params.getInt("uid"), 60*60*2, m_fundProfitVec);//������Ч��1Сʱ

}



/*
 * ��ѯ�����˻��Ƿ����
 */
void FundQueryTotalProfit::CheckFundBind() throw (CException)
{
	ST_FUND_BIND fund_bind;
	memset(&fund_bind, 0, sizeof(ST_FUND_BIND));
	
	if(!QueryFundBindByUid(m_fund_conn, m_params.getInt("uid"), &fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist", __FILE__, __LINE__);
    }
	// ��¼���ڣ�������¼�е�trade_id
    m_params.setParam("trade_id", fund_bind.Ftrade_id);

    if (m_servicetype == CHECK_LOGIN && m_params.getString("qluin") != fund_bind.Fqqid)
    {
        throw CException(ERR_SESSION_PARA_CHECK, "qluin diff db ", __FILE__, __LINE__);
    }
}


/**
  * ����������
  */
void FundQueryTotalProfit::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
	CUrlAnalyze::setParam(rqst->odata, "yday_profit", toString(m_fund_profit.Fprofit).c_str());
	CUrlAnalyze::setParam(rqst->odata, "total_profit", toString(m_fund_profit.Ftotal_profit).c_str());
	CUrlAnalyze::setParam(rqst->odata, "Frecon_day", m_fund_profit.Frecon_day);
	//CUrlAnalyze::setParam(rqst->odata, "reward_profit", toString(m_fund_profit.Freward_profit).c_str());
	CUrlAnalyze::setParam(rqst->odata, "ret_num", (int)m_fundProfitVec.size());

	for(vector<FundProfit>::size_type i= 0; i< m_fundProfitVec.size(); i++)
	{
		FundProfit tempFundProfit = m_fundProfitVec[i];
		CUrlAnalyze::setParam(rqst->odata, string("spid_"+toString(i)).c_str(), tempFundProfit.Fspid);
		CUrlAnalyze::setParam(rqst->odata, string("Frecon_day_"+toString(i)).c_str(), tempFundProfit.Frecon_day);
		CUrlAnalyze::setParam(rqst->odata, string("yday_profit_"+toString(i)).c_str(), toString(tempFundProfit.Fprofit).c_str());
		CUrlAnalyze::setParam(rqst->odata, string("total_profit_"+toString(i)).c_str(), toString(tempFundProfit.Ftotal_profit).c_str());
		CUrlAnalyze::setParam(rqst->odata, string("financial_days_"+toString(i)).c_str(), toString(tempFundProfit.Ffinancial_days).c_str());
		CUrlAnalyze::setParam(rqst->odata, string("fcreate_time_"+toString(i)).c_str(), toString(tempFundProfit.Fcreate_time).c_str());	
		
	}

    rqst->olen = strlen(rqst->odata);
    return;
}


