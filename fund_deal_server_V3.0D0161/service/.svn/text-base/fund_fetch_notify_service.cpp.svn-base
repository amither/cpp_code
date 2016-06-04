/**
  * FileName: fund_fetch_notify_service.cpp
  * Version :1.0
  * Date: 2015-03-02
  * Description: 余额提现冻结通知确认接口
  */

#include "fund_commfunc.h"
#include "fund_fetch_notify_service.h"

FundFetchNotify::FundFetchNotify(CMySQL* mysql)
{
    m_pFundCon = mysql;
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fetch_data,0, sizeof(ST_BALANCE_ORDER));

}

void FundFetchNotify::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);

    // 要保留请求数据，抛差错使用
    m_request = rqst;
    
    
    TRACE_DEBUG("[fund_fetch_notify_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "spid", 1, 15);
    m_params.readLongParam(szMsg, "total_fee", 1,MAX_LONG);
    m_params.readIntParam(szMsg, "cur_type", 1,1);
    m_params.readStrParam(szMsg, "buy_id", 0, 32);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * 检验token
 */
void FundFetchNotify::CheckToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
    //uin|total_fee|buy_id|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["buy_id"] << "|" ;
    ss << gPtrConfig->m_AppCfg.fetch_service_key;

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
void FundFetchNotify::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
    
}

/*
 * 查询基金账户是否存在
 */
void FundFetchNotify::CheckFundBind() throw (CException)
{
    if (!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}

void FundFetchNotify::CheckFetchOrder()throw (CException)
{
    strncpy(m_fetch_data.Flistid,m_params["buy_id"],sizeof(m_fetch_data.Flistid)-1);
    strncpy(m_fetch_data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_fetch_data.Ftrade_id)-1);
    m_fetch_data.Ftype =OP_TYPE_BA_BUY ;
    if (false == queryFundBalanceOrder(m_pFundCon, m_fetch_data,  false))
    {
        throw CException(ERR_BAD_PARAM, "fetch data not exists !", __FILE__, __LINE__);
    }

    // 检查关键参数

    if (m_params.getString("uin") != m_fetch_data.Fuin)
    {
        gPtrAppLog->error("fetch data exists, uin is different! uin in db[%s], uin input[%s]", 
			m_fetch_data.Fuin, m_params["uin"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, uin is different!", __FILE__, __LINE__);
    }

    if (m_params.getLong("total_fee") != m_fetch_data.Ftotal_fee)
    {
        gPtrAppLog->error("fetch data exists, total_fee is different! total_fee in db[%zd], total_fee input[%zd]", 
			m_fetch_data.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, total_fee is different!", __FILE__, __LINE__);
    }

    if (m_params.getString("buy_id") != m_fetch_data.Flistid)
    {
        gPtrAppLog->error("fetch data exists, listid is different! listid in db[%s], listid input[%s]", 
			m_fetch_data.Flistid, m_params["buy_id"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fetch data exists, listid is different!", __FILE__, __LINE__);
    }

    if (m_fetch_data.Fstandby2 == BA_FETCH_NOTIFY  )
    {
        throw CException(ERR_REPEAT_ENTRY, "fetch data already notify!", __FILE__, __LINE__);
    }
		
   if (m_fetch_data.Fstate != FUND_FETCH_SUBACC_OK)
    {
        gPtrAppLog->error("fetch record state =%d invalid",m_fetch_data.Fstate);
        throw CException(ERR_INVALID_STATE, "fetch record state invalid. ", __FILE__, __LINE__);
    }
    
}
      

void FundFetchNotify::UpdateFetchOrder()throw (CException)
{
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["buy_id"],sizeof(data.Flistid)-1);
    data.Ftype = OP_TYPE_BA_BUY;
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);
    data.Fstandby2 = BA_FETCH_NOTIFY;
    updateFundBalanceOrder(m_pFundCon,data);
}


void FundFetchNotify::CheckFundTrade()  throw (CException)
{
    ST_TRADE_FUND stTradeBuy;
    memset(&stTradeBuy,0,sizeof(ST_TRADE_FUND));
    if (false == QueryTradeFund(m_pFundCon, m_params.getString("buy_id").c_str(), 
		PURTYPE_BUY, &stTradeBuy, false))
    {
        gPtrAppLog->debug("fund buy  trade record not exist : %s", m_params.getString("buy_id").c_str());
        throw CException(ERR_BAD_PARAM, "fund buy  trade record not exis !", __FILE__, __LINE__);
    }
	// 记录存在，物理状态无效，报错
	if(LSTATE_INVALID == stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			stTradeBuy.Flistid, stTradeBuy.Ftrade_id, stTradeBuy.Fpur_type);
		throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
	}

    if(stTradeBuy.Fstate != PAY_ACK_SUC && stTradeBuy.Fstate != PAY_OK)
    {
		//其它状态的不可以更新
		gPtrAppLog->error("fund trade state cannot update. listid[%s], trade_id[%s] , state[%d]", 
			stTradeBuy.Flistid, stTradeBuy.Ftrade_id, stTradeBuy.Fstate);
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund trade state cannot update. ", __FILE__, __LINE__);
    }
}



void FundFetchNotify::CheckOrderState() throw (CException)
{
    ST_BALANCE_ORDER fetchData;
    memset(&fetchData,0,sizeof(fetchData));
    
    strncpy(fetchData.Flistid,m_params["buy_id"],sizeof(fetchData.Flistid)-1);
    strncpy(fetchData.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(fetchData.Ftrade_id)-1);
    fetchData.Ftype = OP_TYPE_BA_BUY;
    if (false == queryFundBalanceOrder(m_pFundCon, fetchData,  true))
    {
        throw CException(ERR_BAD_PARAM, "fetch data not exists !", __FILE__, __LINE__);
    }

    if (fetchData.Fstandby2 == BA_FETCH_NOTIFY  )
    {
        throw CException(ERR_REPEAT_ENTRY, "fetch data already notify!", __FILE__, __LINE__);
    }
		
    if (fetchData.Fstate != FUND_FETCH_SUBACC_OK)
    {
        gPtrAppLog->error("fetch record state =%d invalid",m_fetch_data.Fstate);
        throw CException(ERR_INVALID_STATE, "fetch record state invalid. ", __FILE__, __LINE__);
    } 
    
}


void FundFetchNotify::excute()  throw (CException)
{
    //参数检查
    CheckParams();

    // 检查基金账户记录 
    CheckFundBind();

    try
    {
        //检查基金交易单
        CheckFundTrade();
            
        //重入检查
        CheckFetchOrder();

        //启动事物
        m_pFundCon->Begin();

        //加锁查询提现单
        CheckOrderState();
        
        //更新提现冻结成功通知状态
        UpdateFetchOrder();
        
        m_pFundCon->Commit();
        
    }
    catch(CException &e)
    {
        m_pFundCon->Rollback();
        
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
    
}

void FundFetchNotify::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    rqst->olen = strlen(rqst->odata);
    return;
}




