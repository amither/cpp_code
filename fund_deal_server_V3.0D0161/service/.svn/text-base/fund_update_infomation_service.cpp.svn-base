/**
  * FileName: fund_update_infomation_service.cpp
  * Author: sivenli
  * Version :1.0
  * Date: 2015-4-23
  * Description: 基金交易服务 更新资讯状态
  */

#include "fund_commfunc.h"
#include "fund_update_infomation_service.h"

FundUpdateInfomation::FundUpdateInfomation(CMySQL* mysql)
{
    m_pFundCon = mysql;                 
}

/**
  * service step 1: 解析输入参数
  */
void FundUpdateInfomation::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char *pMsg = (char*)(rqst->idata);

    TRACE_DEBUG("[fund_update_infomation_state_service] receives: %s", pMsg);

	m_params.readStrParam(pMsg, "info_id", 1, 64);
    m_params.readIntParam(pMsg, "op_type", 1, 6); //1 挂起  2 发布 3 撤回   4对外已发布(批跑使用状态) 5更新html 6更新标题
    m_params.readStrParam(pMsg, "html_content", 0, 4096);
    m_params.readStrParam(pMsg, "title", 0, 512);
    m_params.readIntParam(pMsg, "add_to_content", 0, 1);
}

/**
  * 执行申购请求
  */
void FundUpdateInfomation::excute() throw (CException)
{
	try
	{
         /* 开启事务 */
        m_pFundCon->Begin();

		UpdateInfomation();

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


void FundUpdateInfomation::UpdateInfomation()
{
	FundInfomation fund_infomation; 
	memset(&fund_infomation, 0, sizeof(FundInfomation));
    strncpy(fund_infomation.info_id,m_params.getString("info_id").c_str(), sizeof(fund_infomation.info_id) - 1);
	if(!queryInfomation(m_pFundCon, fund_infomation,true))
	{
		throw CException(ERR_INFO_NOT_EXIST, "the infomation record not exist! ", __FILE__, __LINE__);	
	}
    else
    {
        int op_type = m_params.getInt("op_type");

        if(op_type == OP_INFO_UPDATE_TITLE)
        {
            char gbkStr[512];
            strncpy(gbkStr,m_params.getString("title").c_str(), sizeof(gbkStr) - 1);
            char utf8Str[512];
            int utfLen = 512;
            memset(utf8Str,0,512);
            ConvertCharSet(gbkStr,utf8Str,utfLen);
            memset(fund_infomation.title,0,513);
            strncpy(fund_infomation.title,utf8Str, sizeof(fund_infomation.title) - 1);
            updateInfomationTitle(m_pFundCon, fund_infomation);
        }
        else if(op_type == OP_INFO_UPDATE_HTML_CONTENT)
        {
            char gbkStr[32767];
            strncpy(gbkStr,m_params.getString("html_content").c_str(), sizeof(gbkStr) - 1);
            TRACE_DEBUG("receives gbk: %s", gbkStr);
            char utf8Str[32767];
            int utfLen = 32767;
            memset(utf8Str,0,32767);
            ConvertCharSet(gbkStr,utf8Str,utfLen);
            TRACE_DEBUG("convert utf8: %s", utf8Str);
            if(m_params.getInt("add_to_content") == 1)
            {
                AddToHtmlContent(m_pFundCon,m_params.getString("info_id"),string(utf8Str));
            }
            else
            {
                memset(fund_infomation.html_content,0,32768);
                strncpy(fund_infomation.html_content,utf8Str, sizeof(fund_infomation.html_content) - 1);
                updateInfomationHtmlContent(m_pFundCon, fund_infomation);
            }   
        } 
        else
        {
            if(op_type == OP_INFO_HANG_UP)
            {
                fund_infomation.state = 1;
                fund_infomation.state_after_audit = INFO_HANG_UP;
            }
            else if(op_type == OP_INFO_RELEASE)
            {
                fund_infomation.state = 1;
                fund_infomation.state_after_audit = INFO_RELEASE;
            }
            else if(op_type == OP_INFO_RECALL)
            {
                fund_infomation.state = 1;
                fund_infomation.state_after_audit = INFO_RECALL;
                fund_infomation.published = 0;
            }
            else if(op_type == OP_INFO_RELEASE_REAL)
            {
                fund_infomation.published = 1;
            }
            updateInfomationState(m_pFundCon, fund_infomation);
        }
    }
}


/**
  * 打包输出参数
  */
void FundUpdateInfomation::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


