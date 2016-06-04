/**
  * FileName: fund_update_trans_date.cpp
  * Author: dianaliu
  * Version :1.0
  * Date: 2015-6-19
  * Description: 基金交易服务 更新交易日信息
  */

#include "fund_commfunc.h"
#include "fund_update_trans_date.h"

FundUpdateTransDate::FundUpdateTransDate(CMySQL* mysql)
{
    m_pFundCon = mysql;
}

/**
  * service step 1: 解析输入参数
  */
void FundUpdateTransDate::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char *pMsg = (char*)(rqst->idata);
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    m_params.readStrParam(pMsg, "date", 8, 8);
    m_params.readStrParam(pMsg, "memo", 0, 128);
    m_params.readIntParam(pMsg, "standby1", 0, 2);
    m_params.readStrParam(pMsg, "standby3", 0, 64);
    m_params.readStrParam(pMsg, "standby4", 1, 128);
    m_params.readStrParam(pMsg, "token", 32, 32);

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

void FundUpdateTransDate::CheckParams() throw (CException)
{
	/*if(m_params.getInt("standby1") == ETF_TYPE_PART_NOT_TRADE_TIME)
	{
		CHECK_PARAM_EMPTY("standby3");   
		
		if (m_params.getString("standby3").length() < 17)
            	{
                	throw CException(ERR_TRADE_DATE_FORMAT, "standby3 format not legal ", __FILE__, __LINE__);
            	}
		//检查standby3的格式
		vector<string> vecTimePeriod;
		str_split(m_params.getString("standby3"), '-', vecTimePeriod,false);
		for (vector<string>::size_type i=0; i<vecTimePeriod.size(); ++i)
		{
            		if (vecTimePeriod[i].length() < 8)
            		{
                		throw CException(ERR_TRADE_DATE_FORMAT, "standby3 format not legal ", __FILE__, __LINE__);
            		}
			
			int hour=0, minute=0, second=0;
    			sscanf(vecTimePeriod[i].c_str(), "%02d:%02d:%02d",&hour, &minute, &second);

			TRACE_DEBUG("vecTime=%s,j=%d,%d:%d:%d", vecTimePeriod[i].c_str(),i,hour,minute,second);
    			if(hour < 9 || hour > 15 ||minute >= 60 || second >= 60)
    			{
    				throw CException(ERR_TRADE_DATE_FORMAT, "standby3 format not legal ", __FILE__, __LINE__);	
    			}
		}
	}*/

	//检查操作时间
	time_t tt = time(NULL);
       struct tm stTm;
       memset(&stTm, 0, sizeof(stTm));
       localtime_r(&tt, &stTm);
	
	int iNow = stTm.tm_hour * 3600 + stTm.tm_min * 60 + stTm.tm_sec;
	int  iBegin = 3600*14 + 60*55;
	int  iEnd = 3600*15 + 60*5;
	if(iNow >= iBegin && iNow <= iEnd)
	{
		throw CException(ERR_HK_OPER_TIME, "operator time not legal ", __FILE__, __LINE__);
	}

	CheckToken();
}

string FundUpdateTransDate::GenFundToken()
{
	stringstream ss;
	char buff[128] = {0};

	ss << m_params["date"] << "|" ;
	ss << m_params["memo"] << "|" ;
	ss << m_params["standby1"] << "|";
	ss << m_params["standby3"] << "|";
	ss << m_params["standby4"] << "|" ;
	ss << gPtrConfig->m_AppCfg.update_trans_date_key;

	getMd5(ss.str().c_str(), ss.str().size(), buff);

	return buff;
}

void FundUpdateTransDate::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

	if (StrUpper(m_params.getString("token")) != StrUpper(token))
	{   
		TRACE_DEBUG("fund authen token check failed, input=%s,real=%s", 
			m_params.getString("token").c_str(), token.c_str());
		throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
	}   
}

/**
  * 执行申购请求
  */
void FundUpdateTransDate::excute() throw (CException)
{
	CheckParams();
	
	try
	{
	     /* 开启事务 */
	    m_pFundCon->Begin();
		 
          UpdateTransDate();

	    /* 提交事务 */
	    m_pFundCon->Commit();
	}
	catch (CException& e)
	{
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		m_pFundCon->Rollback();

		if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
		  && (ERR_REGOK_ALREADY != (unsigned)e.error()))
		{
		    throw;
		}
	}
}


void FundUpdateTransDate::UpdateTransDate()
{
	FundTransDate stTDate; 
	memset(&stTDate, 0, sizeof(FundTransDate));
       strncpy(stTDate.Fdate, m_params.getString("date").c_str(), sizeof(stTDate.Fdate) - 1);
	if(!queryExactTransDate(m_pFundCon, stTDate, false))
	{
		throw CException(ERR_DB_NULL_RESULT, "fund trade date not exist. ", __FILE__, __LINE__);
	}
      else
      {
        strncpy(stTDate.Fmemo,m_params.getString("memo").c_str(), sizeof(stTDate.Fmemo) - 1);
        strncpy(stTDate.Fmodify_time,m_params.getString("systime").c_str(), sizeof(stTDate.Fmodify_time) - 1);
        stTDate.Fstandby1= m_params.getInt("standby1");
        //strncpy(stTDate.Fstandby3,m_params.getString("standby3").c_str(), sizeof(stTDate.Fstandby3) - 1);
	  strncpy(stTDate.Fstandby4,m_params.getString("standby4").c_str(), sizeof(stTDate.Fstandby4) - 1);
        updateTransDate(m_pFundCon, stTDate);
    }
}


/**
  * 打包输出参数
  */
void FundUpdateTransDate::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


