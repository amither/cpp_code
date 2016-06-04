/**
  * FileName: fund_redem_sp_req_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-19
  * Description: 基金交易服务 基金赎回请求 源文件
  */

#include "fund_commfunc.h"
#include "fund_redem_sp_req_service.h"

FundRedemSpReq::FundRedemSpReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
	memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
	memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
	memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
    memset(&m_stUnFreezedata, 0, sizeof(ST_UNFREEZE_FUND));

	m_bBuyTradeExist = false;

}

/**
  * service step 1: 解析输入参数
  */
void FundRedemSpReq::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_redem_sp_req_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
	m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
	m_params.readStrParam(szMsg, "cft_trans_id", 1, 32);
	m_params.readStrParam(szMsg, "cft_fetch_id", 0, 32);
	m_params.readStrParam(szMsg, "cft_charge_ctrl_id", 0, 32);
	m_params.readStrParam(szMsg, "sp_fetch_id", 0, 32);
	m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
     m_params.readIntParam(szMsg, "fetch_type", 0,DRAW_ARRIVE_TYPE_BA);
	//m_params.readStrParam(szMsg, "transfer_id", 1, 32);
	m_params.readStrParam(szMsg, "spid", 10, 15);
    //m_params.readStrParam(szMsg, "fund_name", 0, 64);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
	m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
	m_params.readIntParam(szMsg, "purpose", 0,101);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
	m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

   //t+1开关关闭非白名单用户只有t0赎回
   if (gPtrConfig->m_AppCfg.tplus_redem_switch == 0
        && (gPtrConfig->m_AppCfg.tplus_redem_sps_white_list.find(string("|")+m_params.getString("uid")+"|") == string::npos))
   {
       if (m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T1)
       {
           throw EXCEPTION(ERR_BAD_PARAM, "input fetch_type error");    
       }
       if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_BA)
       {
           m_params.setParam("fetch_type", DRAW_ARRIVE_TYPE_T0);
       }
   }

}

/*
 * 生成基金注册用token
 */
string FundRedemSpReq::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uid|cft_bank_billno|spid|sp_billno|total_fee|key
    // 规则生成原串
    ss << m_params["uid"] << "|" ;
    ss << m_params["cft_bank_billno"] << "|" ;
    ss << m_params["spid"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << gPtrConfig->m_AppCfg.pre_regkey;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundRedemSpReq::CheckToken() throw (CException)
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


/**
  * 检查参数，获取内部参数
  */
void FundRedemSpReq::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
	/*
	if(m_params.getString("cft_bank_billno").substr(0,10) != m_params.getString("spid"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "input cft_bank_billno error"); 
	}
	*/

	//检查spid 及fund_code 是否有效
	strncpy(m_fund_sp_config.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_sp_config.Fspid) - 1);
	strncpy(m_fund_sp_config.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(m_fund_sp_config.Ffund_code) - 1);
	checkFundSpAndFundcode(m_pFundCon,m_fund_sp_config, false);//不强制必须是有效基金公司，用户已开通了该基金公司，可使用带限制的基金公司

	//检查是否支持赎回到财付通余额
	if(m_params.getInt("purpose") == PURPOSE_DEFAULT && !(gPtrConfig->m_AppCfg.support_redem_to_cft == 1))
	{
		throw CException(ERR_CANNOT_REDEM_TO_CFT, "not support redeem to cft balance.", __FILE__, __LINE__);
	}

    if (m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1 && m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_BA)
    {
        m_params.setParam("fetch_type", DRAW_ARRIVE_TYPE_T0);
    }
    /*if (m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T0 && 0 == m_params.getInt("bank_type"))
    {
        throw CException(ERR_BAD_PARAM, "缺少参数 bank_type.", __FILE__, __LINE__);
    }*/

    if ((DRAW_ARRIVE_TYPE_BA == m_params.getInt("fetch_type")&& m_params.getInt("purpose") != PURPOSE_REDEM_TO_BA)
        || (m_params.getInt("purpose") == PURPOSE_REDEM_TO_BA && DRAW_ARRIVE_TYPE_BA != m_params.getInt("fetch_type")))
    {
        throw CException(ERR_BAD_PARAM, "fetch_type check with purpose error.", __FILE__, __LINE__);
    }

	if(m_fund_sp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
	{	
		//定期产品暂时限制赎回
		throw EXCEPTION(ERR_BAD_PARAM, "Do not support the redemption of the Fund's");    
	}
}


/**
  * 执行申购请求
  */
void FundRedemSpReq::excute() throw (CException)
{
    try
    {
        CheckParams();
		

        /* 检查基金绑定记录 */
        CheckFundBind();

		/* 检查基金账户绑定基金公司交易账户记录 */
		CheckFundBindSpAcc();

         /* 开启事务 */
        m_pFundCon->Begin();

		/** 查询赎回交易记录 
		* 为了支持赎回补单重入，必须先查询单是否存在
		*/
        CheckFundTrade();

		//由cgi 在入口时检查，本处检查也无法完全避免余额不足
		//无份额基金公司会赎回失败，即时基金公司发生错误，在赎回确认时减子账户余额也会失败
		CheckFundBalance();

		checkSpLoaning();

		//检查赎回限额
		CheckAuthLimit();

        /* 记录赎回交易记录 */
        RecordFundTrade();

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

void FundRedemSpReq::CheckAuthLimit() throw (CException)
{
	//赎回到余额不检查赎回限制
	//強赎不用检查赎回限制
	if (m_params.getInt("purpose") == PURPOSE_REDEM_TO_BA || IsForceRedem(m_params.getString("cft_fetch_id")))
	{
		return;
	}
	
	UserClassify::UT user_type = g_user_classify->getUserType(m_params.getString("uin"));
	
	//普通用户T+0 T+1都检查赎回限制
	//vip用户只有T+0的时候检查赎回限制
	if ( user_type == UserClassify::NORMAL || m_params.getInt("fetch_type") != DRAW_ARRIVE_TYPE_T1)
	{
		//检查赎回限额
		checkExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,m_params.getInt("fetch_type"));
	}
}

bool FundRedemSpReq::IsForceRedem(string fetch_no)
{
	int ret = QueryFetchType(m_pFundCon, fetch_no);

	return ret == KF_FORCE_REDEM;
}
/*
 * 查询基金账户是否存在，以及验证参数的一致性
 */
void FundRedemSpReq::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    else
    {
        if(m_fund_bind.Flstate == LSTATE_FREEZE)
        {
            throw CException(ERR_ALREADY_FREEZE, "the user be frozen! ", __FILE__, __LINE__);
        }
    }

	// 记录存在，读出记录中的trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
	m_params.setParam("uin", m_fund_bind.Fqqid);
}

/*
*检查是否绑定基金公司帐号
*/
void FundRedemSpReq::CheckFundBindSpAcc() throw (CException)
{
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	//赎回不再限制必须是主交易帐号，只限制申购
	//queryValidMasterSpAcc(m_pFundCon, m_fund_bind_sp_acc,m_params.getString("spid"), false);
	queryValidFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);

}

/**
  * 检查基金交易记录是否已经生成
  */
void FundRedemSpReq::CheckFundTrade() throw (CException)
{
    // 没有赎回记录，继续下一步
    m_bBuyTradeExist = QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeBuy, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", m_bBuyTradeExist);

	if(!m_bBuyTradeExist)
	{
		return;
	}

	m_params.setParam("fund_trans_id", m_stTradeBuy.Flistid);
	
	// 检查关键参数
	if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}


	if(0 != strcmp(m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str()))
	{
		gPtrAppLog->error("fund trade exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_stTradeBuy.Ffund_code, m_params.getString("fund_code").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fund_code is different!", __FILE__, __LINE__);
	}

	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, total_fee is different!", __FILE__, __LINE__);
	}

	if(0 != strcmp(m_stTradeBuy.Fcft_trans_id, m_params.getString("cft_trans_id").c_str()))
	{
		gPtrAppLog->error("fund trade exists, cft_trans_id is different! cft_trans_id in db[%s], cft_trans_id input[%s] ", 
			m_stTradeBuy.Fcft_trans_id, m_params.getString("cft_trans_id").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, cft_trans_id is different!", __FILE__, __LINE__);
	}

       if ((m_params.getInt("purpose") == PURPOSE_UNFREEZE_FOR_FETCH || m_stTradeBuy.Fpurpose == PURPOSE_UNFREEZE_FOR_FETCH)
        && (m_params.getInt("purpose") != m_stTradeBuy.Fpurpose))
       {
		gPtrAppLog->error("fund trade exists, purpose is different! purpose in db[%d], purpose input[%d] ", 
			m_stTradeBuy.Fpurpose, m_params.getInt("purpose"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, purpose is different!", __FILE__, __LINE__);
       }
    
    if (((m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T1 || m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_BA)
        && m_stTradeBuy.Floading_type != DRAW_NOT_USE_LOADING)
        || (m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T0
        && m_stTradeBuy.Floading_type != DRAW_USE_LOADING))
    {
            gPtrAppLog->error("fund trade exists, Floading_type is confict! Floading_type in db[%d], fetch_type input[%s] ", 
    			m_stTradeBuy.Floading_type, m_params.getString("fetch_type").c_str());
            throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, fetch_type is confict!", __FILE__, __LINE__);
    }
    
	// 记录存在，物理状态无效，报错
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeBuy.Flistid, m_stTradeBuy.Ftrade_id, m_stTradeBuy.Fpur_type);
		throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
	}

    if(m_stTradeBuy.Fstate == REDEM_ININ || m_stTradeBuy.Fspe_tag == TRADE_RECORD_TIMEOUT)
    {
		//成功单超时的单，也报正常重入
		throw CException(ERR_REPEAT_ENTRY, "fund redem trade exist. ", __FILE__, __LINE__);
    }
	else if(m_stTradeBuy.Fstate == REDEM_SUC)
	{
		//到基金公司赎回已成功的单重入报特殊错误码，便于前置机识别后不再发起到基金公司的请求
		throw CException(ERR_REDEM_SP_SUC_REPEAT_ENTRY, "fund redem from sp success. ", __FILE__, __LINE__);
	}
	else
    {
		//其它状态的赎回不可以重入
		throw CException(ERR_REDEM_REPEAT_ENTRY, "fund redem trade exist. ", __FILE__, __LINE__);
    }
}

/**
* 查询余额通账户余额
*/
void FundRedemSpReq::CheckFundBalance()
{      
    //微信预付卡合作项目临时方案
    ST_FUND_CONTROL_INFO controlInfo;
    memset(&controlInfo,0,sizeof(controlInfo));
    strncpy(controlInfo.Ftrade_id,m_params["trade_id"],sizeof(controlInfo.Ftrade_id));
    bool isWxWfjUser = isWxPrePayCardBusinessUser(m_pFundCon,m_params["spid"],controlInfo);

    LONG freezeFee = 0;
    LONG balance = querySubaccBalance(m_params.getInt("uid"),querySubaccCurtype(m_pFundCon, m_params.getString("spid")),true,&freezeFee);
    if(m_params.getInt("purpose") == PURPOSE_UNFREEZE_FOR_FETCH)
    {
        //检查解冻单金额
        //ST_UNFREEZE_FUND unFreezedata;
        memset(&m_stUnFreezedata,0,sizeof(ST_UNFREEZE_FUND));
        strncpy(m_stUnFreezedata.Funfreeze_id,m_params["cft_trans_id"],sizeof(m_stUnFreezedata.Funfreeze_id)-1);
        if (false==queryFundUnFreezeByUnfreezeid(m_pFundCon,m_stUnFreezedata,false))
        {
            throw CException(ERR_QUERY_UNFREEZE_BILL, "query unfreeze bill fail ", __FILE__, __LINE__);
        }
          
        if (m_stUnFreezedata.Fredem_id[0] != 0)
        {
            throw CException(ERR_REPEAT_ENTRY_DIFF, "check unfreeze data  redem_id fail", __FILE__, __LINE__);
        }

        if (m_stUnFreezedata.Fcontrol_fee != m_params.getLong("total_fee"))
        {
            throw CException(ERR_CORE_USER_BALANCE, "check control_fee  money fail", __FILE__, __LINE__);
        }

        if ((string(m_stUnFreezedata.Fspid) != gPtrConfig->m_AppCfg.wx_wfj_spid)) //微信王府井预付卡不检查冻结
        {
            if (freezeFee < m_params.getLong("total_fee"))
            {
                TRACE_ERROR("not enough money,total_fee=%ld,freezeFee=%ld",m_params.getLong("total_fee"),freezeFee);
                throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
            }
        }
        else if (controlInfo.Ftotal_fee  < m_params.getLong("total_fee")) //微信王府井发起扣款检查王府井受限金额
        {
            throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
        }
          
    }
    else if (isWxWfjUser && balance<m_params.getLong("total_fee")+controlInfo.Ftotal_fee) //王府井用户发起赎回金额不能赎回赎回部分的金额
    {
        throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
    }
    else if(balance < m_params.getLong("total_fee"))
    {
        throw CException(ERR_CORE_USER_BALANCE, " not enough money", __FILE__, __LINE__);
    }
}

/**
* 检查赎回垫资账户额度
*/
void FundRedemSpReq::checkSpLoaning() throw (CException)
{
	//赎回类型为提现才累计赎回额度
	//创建记录时默认Fpurpose = PURPOSE_DRAW_T1,赎回成功才决定使用哪种类型
	
	//暂时决定所有赎回都走垫资
	/*
	if(m_stTradeBuy.Fpurpose != PURPOSE_DRAW_T1 && m_stTradeBuy.Fpurpose != PURPOSE_DRAW_T0)
	{
		return;
	}
	*/

	
	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));
	
	strncpy(fundSpConfig.Fspid, m_params.getString("spid").c_str(), sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(fundSpConfig.Ffund_code) - 1);
	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, false))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}


    if ((fundSpConfig.Fredem_valid&0x07) ==2) // 停止赎回
    {
        throw EXCEPTION(ERR_REDEM_DRAW_REFUSE, "sp redem  is stopped");
    }

    //目前只有T+1提现不走垫资
    if(m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T1 || DRAW_ARRIVE_TYPE_BA == m_params.getInt("fetch_type") )
    {
        return;
    }
    
    //垫资账户额度超限的处理
    checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),false);

	
}



/**
  * 生成基金赎回记录，状态: 初始赎回状态
  */
void FundRedemSpReq::RecordFundTrade()
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    string drawid;
    string listid;
    if (m_params.getInt("fetch_type")==DRAW_ARRIVE_TYPE_BA)
    {
        if (false == checkTransIdAndSpid(m_params["spid"],m_params["cft_bank_billno"]))
        {
            throw CException(ERR_BAD_PARAM, "spid check with cft_bank_billno error.", __FILE__, __LINE__);
        }
        //赎回到余额赎回单号由前端生成
        listid = m_params.getString("cft_bank_billno");
        drawid = m_params.getString("cft_bank_billno").substr(10);
        m_params.setParam("fund_trans_id", listid);
    }
    else
    {
        drawid = genSubaccDrawid();
        string cft_bank_billno = m_params.getString("cft_bank_billno");
        //赎回单号内部生成  10商户号+8位日期+10序列号+cft_bank_billno后3位，保证cft_bank_billno和listid的分库分表规则一致，不用在拆分表
        listid =  m_params.getString("spid") + drawid + cft_bank_billno.substr(cft_bank_billno.size()-3);
        m_params.setParam("fund_trans_id", listid);
    }


    strncpy(stRecord.Flistid, listid.c_str(), sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_params.getInt("uid");
    strncpy(stRecord.Ffund_name, m_fund_sp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    strncpy(stRecord.Fcard_no, m_params.getString("card_no").c_str(), sizeof(stRecord.Fcard_no)-1);
    if (m_params.getInt("fetch_type")==DRAW_ARRIVE_TYPE_BA)
    {
        stRecord.Fpur_type = PURTYPE_TRANSFER_REDEEM;
    }
    else
    {
        stRecord.Fpur_type = PURTYPE_REDEEM;
    }
    
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = REDEM_ININ;
    stRecord.Flstate = LSTATE_VALID;
    //strncpy(stRecord.Ftrade_date, m_params.getString("trade_date").c_str(), sizeof(stRecord.Ftrade_date)-1);
    //strncpy(stRecord.Ffund_value, m_params.getString("fund_value").c_str(), sizeof(stRecord.Ffund_value)-1);
    //strncpy(stRecord.Ffund_vdate, m_params.getString("fund_vdate").c_str(), sizeof(stRecord.Ffund_vdate)-1);
    //strncpy(stRecord.Ffund_type, m_params.getString("fund_type").c_str(), sizeof(stRecord.Ffund_type)-1);
    //strncpy(stRecord.Fnotify_url, m_params.getString("notify_url").c_str(), sizeof(stRecord.Fnotify_url)-1);
    //strncpy(stRecord.Frela_listid, "", sizeof(stRecord.Frela_listid)-1);
    //strncpy(stRecord.Fdrawid, "buy_no_drawid", sizeof(stRecord.Fdrawid)-1);
    strncpy(stRecord.Ffetchid, m_params.getString("cft_fetch_id").c_str(), sizeof(stRecord.Ffetchid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    //财付通核心是否支持重入? stRecord.Fstandby1 = 1; // 锁定记录
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid"));; // 币种类型
    strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
	strncpy(stRecord.Fcft_trans_id, m_params.getString("cft_trans_id").c_str(), sizeof(stRecord.Fcft_trans_id)-1);
	strncpy(stRecord.Fcft_charge_ctrl_id, m_params.getString("cft_charge_ctrl_id").c_str(), sizeof(stRecord.Fcft_charge_ctrl_id)-1);
	strncpy(stRecord.Fsp_fetch_id, m_params.getString("sp_fetch_id").c_str(), sizeof(stRecord.Fsp_fetch_id)-1);
	strncpy(stRecord.Fcft_bank_billno, m_params.getString("cft_bank_billno").c_str(), sizeof(stRecord.Fcft_bank_billno)-1);
	strncpy(stRecord.Fsub_trans_id, drawid.c_str(), sizeof(stRecord.Fsub_trans_id)-1);
	stRecord.Fpurpose = m_params.getInt("purpose");
	strncpy(stRecord.Fchannel_id, m_params.getString("channel_id").c_str(), sizeof(stRecord.Fchannel_id)-1);

    stRecord.Floading_type = ((m_params.getInt("fetch_type")==DRAW_ARRIVE_TYPE_T1||m_params.getInt("fetch_type")==DRAW_ARRIVE_TYPE_BA)?DRAW_NOT_USE_LOADING:DRAW_USE_LOADING);

    if (m_params.getInt("fetch_type") == DRAW_ARRIVE_TYPE_T1)
    {
        string fund_fetch_date;
        getTplusFetchDate(m_pFundCon,m_params.getString("systime"),fund_fetch_date);
        strncpy(stRecord.Ffund_vdate, fund_fetch_date.c_str(), sizeof(stRecord.Ffund_vdate)-1);
    }

    InsertTradeFund(m_pFundCon, &stRecord);
    InsertTradeUserFund(m_pFundCon, &stRecord);

	//记录垫资帐号提现单和基金交易单对应关系
	if(!m_params.getString("sp_fetch_id").empty())
	{
		FundFetch fundFetch;
		memset(&fundFetch, 0, sizeof(FundFetch));
		
		strncpy(fundFetch.Ffetchid, m_params.getString("sp_fetch_id").c_str(), sizeof(fundFetch.Ffetchid)-1);
		strncpy(fundFetch.Ffund_trans_id, stRecord.Flistid, sizeof(fundFetch.Ffund_trans_id)-1);
		strncpy(fundFetch.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fcreate_time)-1);
    	strncpy(fundFetch.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundFetch.Fmodify_time)-1);

		insertFundFetch(m_pFundCon, fundFetch);
	}
    
    if (m_params.getInt("purpose") == PURPOSE_UNFREEZE_FOR_FETCH)
    {
        ST_UNFREEZE_FUND unFreezedata;
        memset(&unFreezedata,0,sizeof(ST_UNFREEZE_FUND));
        strncpy(unFreezedata.Funfreeze_id,m_params["cft_trans_id"],sizeof(unFreezedata.Funfreeze_id)-1);
        strncpy(unFreezedata.Fredem_id,listid.c_str(),sizeof(unFreezedata.Fredem_id)-1);
        strncpy(unFreezedata.Fpay_trans_id,m_params["cft_fetch_id"],sizeof(unFreezedata.Fpay_trans_id)-1);
        updateFundUnFreeze(m_pFundCon, unFreezedata, m_stUnFreezedata);
    }
}


/**
  * 打包输出参数
  */
void FundRedemSpReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "acc_time", m_bBuyTradeExist ? m_stTradeBuy.Facc_time : m_params.getString("systime").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
	CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
	CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
	CUrlAnalyze::setParam(rqst->odata, "fund_trans_id", m_params.getString("fund_trans_id").c_str());
	CUrlAnalyze::setParam(rqst->odata, "spid", m_params.getString("spid").c_str());
	CUrlAnalyze::setParam(rqst->odata, "sp_name", m_fund_sp_config.Fsp_name);
	CUrlAnalyze::setParam(rqst->odata, "fund_code", m_params.getString("fund_code").c_str());
	CUrlAnalyze::setParam(rqst->odata, "fund_name", m_fund_sp_config.Ffund_name);

    rqst->olen = strlen(rqst->odata);
    return;
}


