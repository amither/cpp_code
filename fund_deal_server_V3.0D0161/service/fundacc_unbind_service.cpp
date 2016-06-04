/**
  * FileName: fundacc_unbind_service.cpp
  * Author: louisjiang	
  * Version :1.0
  * Date: 2014-04-11
  * Description: 基金交易服务 基金销户文件
  */

#include "fund_commfunc.h"
#include "fundacc_unbind_service.h"


FundUnbindAcc::FundUnbindAcc(CMySQL* mysql,int para)
{
    m_fund_conn = mysql;
    memset(&m_fund_bind,0,sizeof(m_fund_bind));
    m_op_type = para;
}


/**
  * 解析输入参数
  */
void FundUnbindAcc::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fundacc_unbind_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);     //财付通帐号
    m_params.readStrParam(szMsg, "trade_id", 0, 32); 
    m_params.readStrParam(szMsg, "cre_type", 0, 5);//支持回乡证
    m_params.readStrParam(szMsg, "cre_id", 0, 32);//证件号的md5
    m_params.readStrParam(szMsg, "true_name", 0, 64); //开户姓名
    m_params.readStrParam(szMsg, "mobile", 0, 21); // 手机号
    m_params.readStrParam(szMsg, "client_ip", 0, 16);
    m_params.readStrParam(szMsg, "token", 0, 32);   // 接口token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}


/*
 * 生成基金注销用token
 */
string FundUnbindAcc::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uin|cre_type|cre_id|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["cre_type"] << "|" ;
    ss << m_params["cre_id"] << "|" ;
    ss << gPtrConfig->m_AppCfg.key_unbind;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/**
  * 校验token
  */
void FundUnbindAcc::checkToken()throw (CException)
{
    // 生成token
    string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }  
}

void FundUnbindAcc::checkWhiteList() throw (CException)
{
    size_t len = strlen(m_fund_bind.Ftrade_id);
    if (len <10)
    {
        throw EXCEPTION(ERR_BAD_PARAM, "invalid Ftrade_id");    
    }
    string tradeidtail = m_fund_bind.Ftrade_id+len-3;
    if (gPtrConfig->m_AppCfg.unbindwhitelist == "ALL") //全部放开
    {
        return;
    }
    else if (gPtrConfig->m_AppCfg.unbindwhitelist.find("|") != string::npos)  //按照最后一位灰度
    {
         if (gPtrConfig->m_AppCfg.unbindwhitelist.find(tradeidtail.substr(2,1)+"|") != string::npos)
         {
            throw EXCEPTION(ERR_NOT_UNBINDWHITE_LISTUSER, "already bind ,unregidter not allowed!");  
         }
    }
    else  //按照后3位的大小灰度
    {
        if (atoi(tradeidtail.c_str()) >= atoi(gPtrConfig->m_AppCfg.unbindwhitelist.c_str()))
         {
            throw EXCEPTION(ERR_NOT_UNBINDWHITE_LISTUSER, "already bind ,unregidter not allowed!");  
         }
    }
    
}

void FundUnbindAcc::checkUnbind()  throw (CException)
{
    //按照tradeid后三位灰度发布只有白名单用户才允许注销
    checkWhiteList();
    
    //校验当前份额必须是0
    checkBalance();

    //校验最近3天无申购赎回记录包括初始状态的单
    checkBuyRecord();

    //校验无在途t+1赎回
    checkTplusRedem();

    //无在途提现单
    checkFundFetch();

    //校验无处于 5状态的赎回单
    checkRedem();

	//校验是否有未确认份额
	checkUnconfirm();

    //检查一段时间内是否有初始状态的充值单
    checkChargingRecord();
}


void FundUnbindAcc::doUnbind()  throw (CException)
{
    //校验token
    checkToken();
    
    try
    {
        //启动mysql事物
        m_fund_conn->Begin();

        //校验开户信息
        checkFundbind();

        //校验是否满足注销条件
        checkUnbind();

        //记录销户信息表
        addUnbindRecord();

        //更新绑定表状态为注销
        updateFundBindRecord();

        //更新安全卡表状态为注销
        updateFundPayCardRecord();

        //提交事物
        m_fund_conn->Commit();
    }
    catch(CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        //失败回滚
        m_fund_conn->Rollback();

        if ((ERR_REPEAT_ENTRY != (unsigned)e.error()) 
          && (ERR_REGOK_ALREADY != (unsigned)e.error()))
        {
            throw;
        }
    }
}

/**
  * 执行基金账户销户
  */
void FundUnbindAcc::excute()  throw (CException)
{
    if (false == QueryFundBindByUin(m_fund_conn, m_params.getString("uin"), &m_fund_bind, false)
        || m_fund_bind.Ftrade_id[0] == 0)
    {
        return;// 未注册理财通用户直接返回注销成功
    }

    if (m_op_type == 1) // 1 查询是否可注销  2 注销
    {
        checkUnbind();
    }
    else
    {
        doUnbind();
    }
}


//校验开户信息
void FundUnbindAcc::checkFundbind() throw (CException)
{
    memset(&m_fund_bind,0,sizeof(m_fund_bind));
    if (false == QueryFundBindByUin(m_fund_conn, m_params.getString("uin"), &m_fund_bind, true))
    {
        TRACE_ERROR("the fund bind record not exist by bind_ack");
        throw EXCEPTION(0, "the fund bind record not exist");  // 未注册理财通用户直接返回注销成功
    }

    // 检查关键参数
    if (m_params.getString("true_name").length()>0 && m_params.getString("true_name") != m_fund_bind.Ftrue_name)
    {
        TRACE_ERROR("true_name in db=%s diff with input=%s", 
                    m_fund_bind.Ftrue_name, m_params.getString("true_name").c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "true_name in db diff with input");
    }

    string enc_cre_id=getMd5(m_fund_bind.Fcre_id+getMd5(m_params.getString("uin")));
    if (m_params.getString("cre_id") != enc_cre_id)
    {
        TRACE_ERROR("cre_id in db=%s diff with input=%s", 
                    enc_cre_id.c_str(), m_params.getString("cre_id").c_str());
        //throw EXCEPTION(ERR_BAD_PARAM, "cre_id in db diff with input");
    }

    if(m_params.getString("trade_id").length()>0 && m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        TRACE_ERROR("trade_id in db=%s diff with input=%s", 
                    m_fund_bind.Ftrade_id, m_params.getString("trade_id").c_str());
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "trade_id in db diff with input");
    }

}

//校验当前份额必须是0
void FundUnbindAcc::checkBalance() throw (CException)
{
    //总账户余额是 0
    if (0 != queryUserTotalAsset(m_fund_bind.Fuid, m_fund_bind.Ftrade_id,true,true))
    {
        TRACE_ERROR("total balabce not 0!");
        throw EXCEPTION(ERR_BALANCE_CANNOT_UNBIND, "total balabce not 0!");
    }

    //已经对账余额是 0
    if (0 != countTotalReconBalance(gPtrFundSlaveDB,m_fund_bind.Ftrade_id))
    {
        TRACE_ERROR("countTotalReconBalance not 0!");
        throw EXCEPTION(ERR_TRANS_NOT_COMPLETE_UNBIND, "countTotalReconBalancenot not  0!");
    }
 
}



//3天内无申购赎回记录(包括预申购)
void FundUnbindAcc::checkBuyRecord() throw (CException)
{
    string start_time;
    string shortTime = m_params.getString("systime");
    shortTime = shortTime.substr(0,4) + shortTime.substr(5,2) + shortTime.substr(8,2);
    int offsetDate = gPtrConfig->m_AppCfg.unbind_no_trans_days; 
    start_time = changeDateFormat(addDays(shortTime, -offsetDate)) + " 00:00:00";
    string cond;
    cond = " AND ((Fmodify_time>='"+start_time+"' AND Fpur_type<>4 AND Fpur_type<>12) OR ((Fpur_type=4 OR Fpur_type=12) AND Facc_time>='"+start_time+"'))";
    if (countTranRecords(gPtrFundSlaveDB,m_fund_bind.Fuid,cond) != 0)
    {
        TRACE_ERROR("trans record exist!");
        throw EXCEPTION(ERR_TRANS_NOT_COMPLETE_UNBIND, "trans record exist!");
    }
}

void FundUnbindAcc::checkRedem() throw (CException)
{
    string cond;
    cond = " AND Fpur_type="+toString(PURTYPE_REDEEM)+" AND Fstate in (13,5) AND Flstate=1 ";
    if (countTranRecords(gPtrFundSlaveDB,m_fund_bind.Fuid,cond) != 0)
    {
        TRACE_ERROR("checkRedem trans record exist!");
        throw EXCEPTION(ERR_TRANS_NOT_COMPLETE_UNBIND, "redem trans record exist!");
    }
}

void FundUnbindAcc::checkUnconfirm() throw (CException)
{
	vector<FUND_UNCONFIRM> dataVec;
	//TODO:fundUnconfirm 删除逻辑:第三步删除unconfirm
	if(gPtrConfig->m_AppCfg.fund_index_trans_grey<3)
	{
	    int count = queryValidFundUnconfirmByTradeId(gPtrFundSlaveDB,m_fund_bind.Ftrade_id,dataVec);
		if(count>0)
		{
	        TRACE_ERROR("checkUnconfirm trans record exist!");
	        throw EXCEPTION(ERR_HAS_UNCONFIRM, "checkUnconfirm trans record exist!");
		}
	}
	if(queryUnfinishTransExists(gPtrFundSlaveDB,m_fund_bind.Ftrade_id))
	{
	        TRACE_ERROR("checkUnfinish Index trans record exist!");
	        throw EXCEPTION(ERR_HAS_UNCONFIRM, "checkUnfinish Index trans record exist!");
	}
}

//无在途提现单
void FundUnbindAcc::checkFundFetch()throw (CException)
{
    //查询本月是否有提现中记录
    if (countFetchingRecords(gPtrFundSlaveDB,m_params["uin"],getTime_yyyymm()) != 0)
    {
        TRACE_ERROR("checkFundFetch fetching record exist!");
        throw EXCEPTION(ERR_TRANS_NOT_COMPLETE_UNBIND, "checkFundFetch fetching record exist!");
    }

    //查询上个月是否有提现中记录
    if (countFetchingRecords(gPtrFundSlaveDB,m_params["uin"],getTime_yyyymm(-1)) != 0)
    {
        TRACE_ERROR("checkFundFetch fetching record exist!");
        throw EXCEPTION(ERR_TRANS_NOT_COMPLETE_UNBIND, "checkFundFetch fetching record exist!");
    }

}

void FundUnbindAcc::checkChargingRecord() throw (CException)
{
    time_t tmp_cur_time = time(NULL);
    tmp_cur_time += -1*60*gPtrConfig->m_AppCfg.undone_trans_timespan;
    string strCurTime = getSysTime(tmp_cur_time);
    
    //检查是否有未完成的充值单
    char sql_cond[MAX_SQL_LEN] = {0};
    snprintf(sql_cond, sizeof(sql_cond)-1," Flstate=1 AND Ftype=1 AND Fstate=0 AND Fmodify_time>'%s' ",
        escapeString(strCurTime).c_str());
    LONG total_fee = getChargeRecordsFee(gPtrFundSlaveDB, m_fund_bind.Ftrade_id, sql_cond);

    if (total_fee > 0)
    {
        TRACE_ERROR("checkChargingRecord trans record exist!");
        throw EXCEPTION(ERR_TRANS_NOT_COMPLETE_UNBIND, "charging trans record exist!");
    }
}


//校验无在途t+1赎回
void FundUnbindAcc::checkTplusRedem() throw (CException)
{
    //T-2日15点之后无t+1赎回操作
    string Tminus2Date;
    string TminusDate;
    bool isCurTDay;
    getTminus2TransDate(m_fund_conn, nowdate(m_params.getString("systime").substr(0,10)) ,Tminus2Date,TminusDate,isCurTDay);

    string start_time;
	
    if(gPtrConfig->m_AppCfg.trans_recon_type == 1)
    {
        start_time = changeDateFormat(Tminus2Date) + " 15:00:00";
    }
    else
    {
        start_time = Tminus2Date;
    }
   

    string cond;
    cond = " AND Fpur_type="+toString(PURTYPE_REDEEM)
        +" AND Floading_type=0 AND (Fstate=5 OR Fstate=10) AND Flstate=1 AND Facc_time>='"
        +start_time+"' ";
    if (countTranRecords(gPtrFundSlaveDB,m_fund_bind.Fuid,cond) != 0)
    {
        TRACE_ERROR("tplus redem exist,unbind not allowed!");
        throw EXCEPTION(ERR_TPLUS_REDEM_UNBIND, "tplus redem fee exist!");
    }
}



//记录销户信息表
void FundUnbindAcc::addUnbindRecord() throw (CException)
{
    ST_FUND_UNBIND unbindRecord;
    memset(&unbindRecord,0,sizeof(unbindRecord));
    unbindRecord.Fcre_type = m_fund_bind.Fcre_type;         
    strncpy(unbindRecord.Fcre_id,m_fund_bind.Fcre_id,sizeof(unbindRecord.Fcre_id)-1);
    strncpy(unbindRecord.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(unbindRecord.Ftrade_id)-1);
    strncpy(unbindRecord.Fqqid,m_fund_bind.Fqqid,sizeof(unbindRecord.Fqqid)-1);
    unbindRecord.Fuid = m_fund_bind.Fuid;    
    strncpy(unbindRecord.Ftrue_name,m_fund_bind.Ftrue_name,sizeof(unbindRecord.Ftrue_name)-1);
    strncpy(unbindRecord.Fcre_id_orig,m_fund_bind.Fcre_id_orig,sizeof(unbindRecord.Fcre_id_orig)-1);
    strncpy(unbindRecord.Fphone,m_fund_bind.Fphone,sizeof(unbindRecord.Fphone)-1);
    strncpy(unbindRecord.Fmobile,m_fund_bind.Fmobile,sizeof(unbindRecord.Fmobile)-1);
    unbindRecord.Fstate = 1;         
    unbindRecord.Flstate = 1;       
    strncpy(unbindRecord.Fcreate_time,m_params["systime"],sizeof(unbindRecord.Fcreate_time)-1);
    strncpy(unbindRecord.Fmodify_time,m_params["systime"],sizeof(unbindRecord.Fmodify_time)-1);
    strncpy(unbindRecord.Facc_time,m_params["systime"],sizeof(unbindRecord.Facc_time)-1);
    unbindRecord.Facct_type = m_fund_bind.Facct_type;         
    strncpy(unbindRecord.Fchannel_id,m_fund_bind.Fchannel_id,sizeof(unbindRecord.Fchannel_id)-1);
    strncpy(unbindRecord.Fopenid,m_fund_bind.Fopenid,sizeof(unbindRecord.Fopenid)-1);
    
    InsertFundUnBind(m_fund_conn,&unbindRecord);
}



//更新绑定表状态为注销
void FundUnbindAcc::updateFundBindRecord() throw (CException)
{
    strncpy(m_fund_bind.Fmodify_time,m_params["systime"],sizeof(m_fund_bind.Fmodify_time)-1);
    disableFundBind(m_fund_conn,&m_fund_bind);
    delFundbindToKV(m_params["uin"]);
}



//更新安全卡表状态为注销
void FundUnbindAcc::updateFundPayCardRecord() throw (CException)
{
    FundPayCard data;
    memset(&data,0,sizeof(FundPayCard));
    strncpy(data.Fqqid,m_params["uin"],sizeof(data.Fqqid)-1);
    if (queryFundPayCard(m_fund_conn,data,true))
    {
        disableFundPayCard(m_fund_conn,data); //不更新modifytime
        delPayCardToKV(data.Fqqid); //删除安全卡数据
    }
}

/**
  * 打包返回
  */
void FundUnbindAcc::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    if (m_fund_bind.Ftrade_id[0] == 0)
    {
        CUrlAnalyze::setParam(rqst->odata, "user_not_reg", "1");
    }
    rqst->olen = strlen(rqst->odata);
    return;
}

