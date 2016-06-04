/**
  * FileName: fund_insert_infomation_service.cpp
  * Author: sivenli
  * Version :1.0
  * Date: 2015-4-23
  * Description: 基金交易服务 插入资讯信息
  */

#include "fund_commfunc.h"
#include "fund_insert_infomation_service.h"

FundInsertInfomation::FundInsertInfomation(CMySQL* mysql)
{
    m_pFundCon = mysql;                 
}

/**
  * service step 1: 解析输入参数
  */
void FundInsertInfomation::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    char *pMsg = (char*)(rqst->idata);

    TRACE_DEBUG("[fund_insert_infomation_service] receives: %s", pMsg);

	m_params.readStrParam(pMsg, "info_id", 1, 64);
    m_params.readStrParam(pMsg, "title", 1, 512);  //标题
    m_params.readStrParam(pMsg, "link", 1, 1024);  //链接
    m_params.readStrParam(pMsg, "info_time", 1, 32); //发布时间
    m_params.readStrParam(pMsg, "source", 1, 64);    //来源  好买或自选股
    m_params.readStrParam(pMsg, "html_content", 1, 4096); // html文件内容
    m_params.readIntParam(pMsg, "add_to_content", 0, 1); // 是否追加html_content 0插入  1追加
	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

/**
  * 执行申购请求
  */
void FundInsertInfomation::excute() throw (CException)
{
	try
	{
	     /* 开启事务 */
	    m_pFundCon->Begin();
		 
		InsertInfomation();

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


void FundInsertInfomation::InsertInfomation()
{
	FundInfomation fund_infomation; 
	memset(&fund_infomation, 0, sizeof(FundInfomation));
    strncpy(fund_infomation.info_id,m_params.getString("info_id").c_str(), sizeof(fund_infomation.info_id) - 1);
	if(queryInfomation(m_pFundCon, fund_infomation,false))
	{
		TRACE_DEBUG("query fund_infomation existed,info_id=[%ld]", m_params.getLong("info_id"));
        if(m_params.getInt("add_to_content") == 1)
        {
            AddToHtmlContent(m_pFundCon,m_params.getString("info_id"),m_params.getString("html_content"));
        }
        else
        {
            throw EXCEPTION(ERR_INFO_EXIST, "info exist");
        }
	}
    else
    {
        if(m_params.getInt("add_to_content") == 0)
        {
            strncpy(fund_infomation.title,m_params.getString("title").c_str(), sizeof(fund_infomation.title) - 1);
            strncpy(fund_infomation.link,m_params.getString("link").c_str(), sizeof(fund_infomation.link) - 1);
            strncpy(fund_infomation.info_time,m_params.getString("info_time").c_str(), sizeof(fund_infomation.info_time) - 1);
            strncpy(fund_infomation.source,m_params.getString("source").c_str(), sizeof(fund_infomation.source) - 1);
            strncpy(fund_infomation.html_content,m_params.getString("html_content").c_str(), sizeof(fund_infomation.html_content) - 1);
            strncpy(fund_infomation.create_time,m_params.getString("systime").c_str(), sizeof(fund_infomation.create_time) - 1);
            insertInfomation(m_pFundCon, fund_infomation);
        }
    }
}


/**
  * 打包输出参数
  */
void FundInsertInfomation::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


