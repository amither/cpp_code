/**
  * FileName: fund_query_balance_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-12-31
  * Description: 基金交易服务 查询余额增值账户余额 源文件
  */

#include "fund_commfunc.h"
#include "fund_query_balance_service.h"
#include <algorithm>

FundQueryBalance::FundQueryBalance(CMySQL* mysql,int type)
{
    m_fund_conn = mysql;
    m_servicetype = type;
}


/**
  * service step 1: 解析输入参数
  */
void FundQueryBalance::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char *pMsg = (char*)(rqst->idata);

    // 读取参数
    m_params.readIntParam(pMsg, "uid", 10000,MAX_INTEGER);
	m_params.readIntParam(pMsg, "query_type", 0,4);//0:查询总份额（默认）	1:查询总份额及各基金公司份额	2：查询指定基金公司份额+总份额	3：查询指定基金公司4:查询理财通余额
	m_params.readStrParam(pMsg, "spid", 0, 256);
    m_params.readStrParam(pMsg, "qlskey", 0, 64);
    m_params.readStrParam(pMsg, "qluin", 0, 64);
}


/**
  * 检查参数，获取内部参数
  */
void FundQueryBalance::CheckParams() throw (CException)
{
	if(2 == m_params.getInt("query_type") || 3 == m_params.getInt("query_type"))
	{
		CHECK_PARAM_EMPTY("spid");
	}
    
    if (m_servicetype == CHECK_LOGIN)
    {
        checkSession(m_params["qlskey"], m_params["qluin"], "100095");
    }
}

/**
  * 执行基金账户开户
  */
void FundQueryBalance::excute() throw (CException)
{

	CheckParams();

	CheckFundBind();

	queryBalance();
    
}

/**
* 查询余额通账户余额
*/
void FundQueryBalance::queryBalance()
{
   int query_type = m_params.getInt("query_type");
   if (4 == query_type)
   {
       LONG balance = querySubaccBalance(m_params.getInt("uid"), CUR_FUND_BALANCE, false);
       m_params.setParam("balance", balance); //兼容老cgi逻辑
       m_params.setParam("fund_balance", balance);
   }
   else
   {
       m_subacc_curtype_list.clear();
       if(3 == query_type)
       {
            vector<string> spidVec = split(m_params.getString("spid"),",");
	        for(size_t i=0;i<spidVec.size();i++)
            {
		        int subacc_curtype = querySubaccCurtype(m_fund_conn, spidVec[i]);
                m_subacc_curtype_list.push_back(subacc_curtype);
                m_cutypeToSpid.insert(std::pair<int,string>(subacc_curtype,spidVec[i]));
	        }
            querySubaccBalanceListFromCKV(m_params.getInt("uid"), m_subacc_curtype_list, m_subaccListUser);
       }
       else //0 1 2
       {
            vector<FundBindSp> fundBindSpVec;
	        getFundBindAllSpFromKV(m_params.getString("trade_id"), fundBindSpVec);
	        for(vector<FundBindSp>::iterator iter = fundBindSpVec.begin();iter != fundBindSpVec.end(); ++iter)
	        {
                int subacc_curtype = querySubaccCurtype(gPtrFundDB,(*iter).Fspid);
		        m_subacc_curtype_list.push_back(subacc_curtype);
                if(1 == query_type)
                {
                    m_cutypeToSpid.insert(std::pair<int,string>(subacc_curtype,string((*iter).Fspid)));
                }
                else if(2 == query_type )
                {
                    vector<string> spidVec = split(m_params.getString("spid"),",");
                    vector<string>::iterator it = std::find(spidVec.begin(),spidVec.end(),string((*iter).Fspid));
                    if(it != spidVec.end())
                    {
                        m_cutypeToSpid.insert(std::pair<int,string>(subacc_curtype,string((*iter).Fspid)));
                    }
                }
	        }
            querySubaccBalanceListFromCKV(m_params.getInt("uid"), m_subacc_curtype_list, m_subaccListUser);
            LONG total_balance = 0;
            for(vector<SubaccUser>::iterator iter = m_subaccListUser.begin(); iter!=m_subaccListUser.end(); ++iter)
            {
                total_balance += iter->Fbalance;
            }
            m_params.setParam("balance", total_balance);
             //增加返回理财通余额
		    LONG fund_balance = querySubaccBalance(m_params.getInt("uid"), CUR_FUND_BALANCE, false);
		    m_params.setParam("fund_balance", fund_balance);
       }      
   }
}

/*
 * 查询基金账户是否存在
 */
void FundQueryBalance::CheckFundBind() throw (CException)
{
	ST_FUND_BIND fund_bind;
	memset(&fund_bind, 0, sizeof(ST_FUND_BIND));
	
	if(!QueryFundBindByUid(m_fund_conn, m_params.getInt("uid"), &fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist", __FILE__, __LINE__);
    }
	// 记录存在，读出记录中的trade_id
    m_params.setParam("trade_id", fund_bind.Ftrade_id);

    if (m_servicetype == CHECK_LOGIN && m_params.getString("qluin") != fund_bind.Fqqid)
    {
        throw CException(ERR_SESSION_PARA_CHECK, "qluin diff with db ", __FILE__, __LINE__);
    }
}


/**
  * 打包输出参数
  */
void FundQueryBalance::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "balance",m_params.getString("balance").c_str());
    int query_type = m_params.getInt("query_type");
    if (3 != query_type )
    {
        CUrlAnalyze::setParam(rqst->odata, "fund_balance",m_params.getString("fund_balance").c_str());
    }
    
    if(0 == query_type || 4 == query_type)
    {
	    CUrlAnalyze::setParam(rqst->odata, "ret_num",0);
    }
    else
    {
        CUrlAnalyze::setParam(rqst->odata, "ret_num",(int)m_cutypeToSpid.size());
    }

    int j = 0;
	for(vector<SubaccUser>::size_type i= 0; i< m_subaccListUser.size(); i++)
	{
      
		SubaccUser subaccUserInfo = m_subaccListUser[i];
        map<int,string>::iterator it = m_cutypeToSpid.find(subaccUserInfo.Fcurtype);
        if(it != m_cutypeToSpid.end())
        {
		    CUrlAnalyze::setParam(rqst->odata, string("spid_"+toString(j)).c_str(), it->second.c_str());
		    CUrlAnalyze::setParam(rqst->odata, string("balance_"+toString(j)).c_str(), toString(subaccUserInfo.Fbalance).c_str());
		    CUrlAnalyze::setParam(rqst->odata, string("con_"+toString(j)).c_str(), toString(subaccUserInfo.Fcon).c_str());
            j++;
        }
	}

    rqst->olen = strlen(rqst->odata);
    return;
}


