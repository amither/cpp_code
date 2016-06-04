/**
  * FileName: fund_pre_record_user_acc_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2015-03-04
  * Description: 记录潜在新理财通用户账户
  */

#include "fund_commfunc.h"
#include "fund_pre_record_user_acc_service.h"

FundPreRecordUserAcc::FundPreRecordUserAcc(CMySQL* mysql)
{
    m_pFundCon = mysql;
}

void FundPreRecordUserAcc::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    
    TRACE_DEBUG("[fund_pre_record_user_acc_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 0, 64);
    m_params.readStrParam(szMsg, "acc_id", 1, 64);
    m_params.readIntParam(szMsg, "acc_type", 1,MAX_INTEGER);
    m_params.readStrParam(szMsg, "channel_id", 1, 64);
    m_params.readStrParam(szMsg, "business_type", 0, 32);
    m_params.readIntParam(szMsg, "omit_old_user", 0,1);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * 检验token
 */
void FundPreRecordUserAcc::CheckToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["acc_id"] << "|" ;
    ss << m_params["acc_type"] << "|" ;
    ss << m_params["channel_id"] << "|" ;
    ss << "d3540af57272bdd0fbfeeae3b9435955";

    TRACE_DEBUG("token src=%s", ss.str().c_str());
    getMd5(ss.str().c_str(), ss.str().size(), buff);

    if (StrUpper(m_params.getString("token")) != StrUpper(buff))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }   
}



/**
  * 检查参数，获取内部参数
  */
void FundPreRecordUserAcc::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
}

/*
 * 查询基金账户是否存在
 */
bool FundPreRecordUserAcc::isOldUser() throw (CException)
{
    if (m_params.getString("uin").empty())
    {
        return false;
    }
    ST_FUND_BIND m_fund_bind; 
    memset(&m_fund_bind,0,sizeof(ST_FUND_BIND));
    return QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false);
}

bool FundPreRecordUserAcc::isAlreadyExist(ST_PREUSER_ACC& data)throw (CException)
{
    if (m_params.getString("channel_id") != data.Fchannel_id)
    {
        return false;
    }
    if (m_params.getString("business_type") != data.Fbusiness_type)
    {
        return false;
    }
    return true;
}

void FundPreRecordUserAcc::RecordAccInfo()throw (CException)
{
    ST_PREUSER_ACC data;
    memset(&data,0,sizeof(data));
    strncpy(data.Facc_id,m_params["acc_id"],sizeof(data.Facc_id)-1);
    data.Facc_type = m_params.getInt("acc_type");
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);
    strncpy(data.Fchannel_id,m_params["channel_id"],sizeof(data.Fchannel_id)-1);
    strncpy(data.Fbusiness_type,m_params["business_type"],sizeof(data.Fbusiness_type)-1);
    strncpy(data.Fuin,m_params["uin"],sizeof(data.Fuin)-1);
    strncpy(data.Fcreate_time,m_params["systime"],sizeof(data.Fcreate_time)-1);

    //非关键数据，不需要启动mysql事物
    
    if (false == queryPreUserAcc(m_pFundCon,data,false))
    {
        //新用户直接插入db
        insertPreUserAcc(m_pFundCon,data);
    }
    else
    {  
        //判断是否已经存在相同记录
        if (isAlreadyExist(data))
        {
            TRACE_DEBUG("record already exist.");
            return;
        }
        //更新渠道号信息
        strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);
        strncpy(data.Fchannel_id,m_params["channel_id"],sizeof(data.Fchannel_id)-1);
        strncpy(data.Fbusiness_type,m_params["business_type"],sizeof(data.Fbusiness_type)-1);
        strncpy(data.Fuin,m_params["uin"],sizeof(data.Fuin)-1);
        updatePreUserAcc(m_pFundCon,data);
    }
   
}


void FundPreRecordUserAcc::excute()  throw (CException)
{
    //参数检查
    CheckParams();

    //如果请求指定不记录老用户，且判断用户是老用户则直接返回
    if (m_params.getInt("omit_old_user") && isOldUser())
    {
        return;
    }

    //保存账户信息
    RecordAccInfo();
}

void FundPreRecordUserAcc::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}

