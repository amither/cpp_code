/**
  * FileName: abstract_buy_sp_req_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-02-28
  * Description: 父类：基金申购请求
  */


#include "fund_commfunc.h"
#include "abstract_buy_sp_req_service.h"

AbstractBuySpReq::AbstractBuySpReq(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fund_bind_sp_acc, 0, sizeof(FundBindSp));
    memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
    memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));

    m_bBuyTradeExist = false;
}

/**
  * service step 1: 解析输入参数
  */
void AbstractBuySpReq::parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg)  throw (CException)
{
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息    
    TRACE_DEBUG("[abstract_buy_sp_req_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "uid", 0,MAX_INTEGER);
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "fund_trans_id", 1, 32);
    m_params.readStrParam(szMsg, "spid", 10, 15);
    m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
    m_params.readStrParam(szMsg, "fund_code", 0, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readIntParam(szMsg, "purpose", 0, 101);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readIntParam(szMsg, "agree_risk", 0, 64);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
    
    parseBizInputMsg(szMsg); // 读取业务参数

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

    //申购请求支持券消费，新增入参用于记录增值券主键，记录在Fstandby4，varchar64 非必填
    m_params.readStrParam(szMsg, "coupon_id", 0, 32);

}

/**
  * 传入已有的商户配置,避免重复查询
  */
void AbstractBuySpReq::setSpConfig(FundSpConfig fundSpConfig)
{
	m_fund_sp_config = fundSpConfig;
}


/**
  * 检查参数，获取内部参数
  */
void AbstractBuySpReq::CheckParams() throw (CException)
{
    if(m_params.getString("fund_trans_id").substr(0,10) != m_params.getString("spid"))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input fund_trans_id error"); 
    }
	m_params.setParam("fund_code",m_fund_sp_config.Ffund_code);//对于没传入fund_code的选择一个
    
    // 检查基金是否可以进行申购
    checkFundcodePurchaseValid(m_fund_sp_config.Fbuy_valid);

    //检查及解析channel_id
    CheckChannelId();
}

/**
  * 执行申购请求
  */
void AbstractBuySpReq::excute() throw (CException)
{
	try{
		
        CheckParams();

        /* 检查基金账户记录 */
        CheckFundBind();

        /* 检查基金账户绑定基金公司交易账户记录 */
        CheckFundBindSpAcc();

        //检查用户持有份额
        CheckUserTotalShare();

         /* 开启事务 */
        m_pFundCon->Begin();
		 
        /* 查询基金交易记录 */
        CheckFundTrade();

        /* 检查限额 必须在查询基金交易单之后 */
        CheckFundSpLimit();
        
        /* 记录基金交易记录 */
        RecordFundTrade();

        /* 提交事务 */
        m_pFundCon->Commit();

        /* 记录成功后检查用户的状态判断返回值*/
        CheckUserTempFail();
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
/*
 * 查询基金账户是否存在
 */
void AbstractBuySpReq::CheckFundBind() throw (CException)
{
	//对于新用户首次申购鉴权成功，支付失败(额度不足，或其它问题)，用户基本信息表中没有uid
	bool bind_exist = QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false);
	
	if(!bind_exist)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }else
    {
        if(m_fund_bind.Flstate == LSTATE_FREEZE)
        {
            throw CException(ERR_ALREADY_FREEZE, "the user be frozen! ", __FILE__, __LINE__);
        }
    }

	// 记录存在，读出记录中的trade_id
    m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
}

/*
*检查是否绑定基金公司帐号，并且可交易
*/
void AbstractBuySpReq::CheckFundBindSpAcc() throw (CException)
{
	
	strncpy(m_fund_bind_sp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_sp_acc.Ftrade_id) - 1);
	strncpy(m_fund_bind_sp_acc.Fspid, m_params.getString("spid").c_str(), sizeof(m_fund_bind_sp_acc.Fspid) - 1);

	bool existsBindSp = queryFundBindSp(m_pFundCon, m_fund_bind_sp_acc, false);
	if(!existsBindSp){		
		TRACE_ERROR("the fund bind sp account record not exist.spid:%s", m_params.getString("spid").c_str());
        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record not exist");
	}

	if(LSTATE_FREEZE == m_fund_bind_sp_acc.Flstate){
		//账户被冻结不允许操作
		TRACE_ERROR("the fund bind sp account record has been frozen.");
        throw EXCEPTION(ERR_SP_ACC_FREEZE, "the fund bind sp account record has been frozen.");
	}

	if(BIND_SPACC_SUC != m_fund_bind_sp_acc.Fstate&&BIND_SPACC_TEMP_FAIL != m_fund_bind_sp_acc.Fstate){
		//账户未绑定不允许购买:此处允许绑定暂时失败的用户生成交易单,支付回调时候进行退款
		TRACE_ERROR("the fund bind sp account record not allowed buy.");
        throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record state not allowed buy.");
	}
		
}


/**
  * 检查基金交易记录是否已经生成
  */
void AbstractBuySpReq::CheckFundTrade() throw (CException)
{
    // 没有购买记录，继续下一步
    m_bBuyTradeExist = QueryTradeFund(m_pFundCon, m_params.getString("fund_trans_id").c_str(), 
		PURTYPE_BUY, &m_stTradeBuy, true);

    gPtrAppLog->debug("fund buy req trade record exist : %d", m_bBuyTradeExist);

    if(!m_bBuyTradeExist)
        return;

    // 检查关键参数
    if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_stTradeBuy.Ftrade_id, "") 
		&& 0 != strcmp(m_stTradeBuy.Ftrade_id, m_params.getString("trade_id").c_str()))
    {
        gPtrAppLog->error("fund trade exists, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
			m_stTradeBuy.Ftrade_id, m_params.getString("trade_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, trade_id is different!", __FILE__, __LINE__);
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

    // 记录存在，物理状态无效，报错
    if(LSTATE_INVALID == m_stTradeBuy.Flstate)
    {
        gPtrAppLog->error("fund trade exists, lstate is invalid. listid[%s], trade_id[%s] , purtype[%d]", 
			m_stTradeBuy.Flistid, m_stTradeBuy.Ftrade_id, m_stTradeBuy.Fpur_type);
        throw CException(ERR_TRADE_INVALID, "fund trade exists, lstate is invalid. ", __FILE__, __LINE__);
    }

    // 记录已经被锁定，报错
    if (m_stTradeBuy.Fstandby1 != 0)
    {
        throw EXCEPTION(ERR_BUYPAY_LOCK, "the trade exists and locked");
    }

	// 记录存在且已经到基金公司预申购成功
    if((PAY_INIT <= m_stTradeBuy.Fstate && m_stTradeBuy.Fstate <= PURCHASE_SUC)||m_stTradeBuy.Fstate==PAY_ACK_SUC)
    {
        throw CException(ERR_BUY_REQ_SP_OK, "the state in db is newer. ", __FILE__, __LINE__);
    }

	// 非成功状态且不为初始状态的订单不能重入创建
	if(CREATE_INIT != m_stTradeBuy.Fstate)
    {
        throw CException(ERR_BUY_RECORD_INVALID, "fund trade record state invalid. ", __FILE__, __LINE__);
    }

	// 重入情况检查用户状态返回值
	CheckUserTempFail();
	
    // 记录已存在
    throw EXCEPTION(ERR_REPEAT_ENTRY, "the trade exists");
}

/**
  * 检查基金资产
  */
void AbstractBuySpReq::CheckFundSpLimit() throw (CException)
{	
    /* 需要根据是否存在基金交易单以及uid为0判断是否是全新用户支付后的开户，如果是，请求需要放过*/
    if ((m_bBuyTradeExist==false ||  m_stTradeBuy.Fuid != 0) && m_params.getInt("purpose") != 101) //赠送申购不检查限额
    {
        checkFundcodeToScopeUpperLimit(m_fund_bind.Fuid,m_params["systime"],m_params.getLong("total_fee"), m_fund_sp_config, true);
    }
}

/**
  * 生成基金购买记录，状态: 等待付款
  */
void AbstractBuySpReq::RecordFundTrade()
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

    strncpy(stRecord.Flistid, m_params.getString("fund_trans_id").c_str(), sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_params.getString("spid").c_str(), sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_fund_bind.Fuid;
    strncpy(stRecord.Ffund_name, m_fund_sp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    strncpy(stRecord.Fcard_no, m_params.getString("card_no").c_str(), sizeof(stRecord.Fcard_no)-1);
    
	//赠送申购
	if(m_params.getInt("purpose") ==100)
	{
		//stRecord.Fpur_type = PURTYPE_REWARD_PROFIT;
		throw EXCEPTION(ERR_BAD_PARAM, "input purpose error");   //先不支持赠送收益
	}else if(m_params.getInt("purpose") ==101)
	{
		stRecord.Fpur_type = PURTYPE_REWARD_SHARE;
		
	}
       else if (m_params.getInt("purpose") ==PURPOSE_BALANCE_BUY)
       {
           stRecord.Fpur_type = PURTYPE_TRANSFER_PURCHASE;
       }
	else
	{
		stRecord.Fpur_type = PURTYPE_PURCHASE;
	}
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = CREATE_INIT;
    stRecord.Flstate = LSTATE_VALID;
    strncpy(stRecord.Ftrade_date, m_params.getString("trade_date").c_str(), sizeof(stRecord.Ftrade_date)-1);
    strncpy(stRecord.Ffund_value, m_params.getString("fund_value").c_str(), sizeof(stRecord.Ffund_value)-1);
    strncpy(stRecord.Ffund_vdate, m_params.getString("fund_vdate").c_str(), sizeof(stRecord.Ffund_vdate)-1);
    strncpy(stRecord.Ffund_type, m_params.getString("fund_type").c_str(), sizeof(stRecord.Ffund_type)-1);
    strncpy(stRecord.Fnotify_url, m_params.getString("notify_url").c_str(), sizeof(stRecord.Fnotify_url)-1);
    strncpy(stRecord.Frela_listid, "", sizeof(stRecord.Frela_listid)-1);
    //strncpy(stRecord.Fdrawid, "buy_no_drawid", sizeof(stRecord.Fdrawid)-1);
    strncpy(stRecord.Ffetchid, "buy_no_fetchid", sizeof(stRecord.Ffetchid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    //财付通核心是否支持重入? stRecord.Fstandby1 = 1; // 锁定记录
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("spid")); // 币种类型

    stRecord.Fpurpose = m_params.getInt("purpose");

	strncpy(stRecord.Fchannel_id, m_channelId.c_str(), sizeof(stRecord.Fchannel_id)-1);
    stRecord.Fpay_channel = m_payChannel;
	
	//券id
	strncpy(stRecord.Fcoupon_id, m_params.getString("coupon_id").c_str(), sizeof(stRecord.Fcoupon_id)-1);

    InsertTradeFund(m_pFundCon, &stRecord);
	if(m_fund_bind.Fuid >= 10000)
	{
    	InsertTradeUserFund(m_pFundCon, &stRecord);
	}
}

//解析请求字段中的channel_id，生成真正的channel_id和pay_channel(0：银行卡   1：理财通余额 2: 网银 )
//如， 0_68|fm_3_unknown,  channel_id为68|fm_3_unknown, pay_channel为0
void AbstractBuySpReq::CheckChannelId() throw (CException)
{
    const string channelIdOrig = m_params.getString("channel_id");
    m_channelId = channelIdOrig;
    if (m_params.getInt("purpose") == PURPOSE_BALANCE_BUY)
    {
        m_payChannel = PAY_TYPE_BALANCE;
        return;
    }

    if (channelIdOrig.empty())
    {
        m_payChannel = PAY_TYPE_CARD;
        return;
    }

    string str1 = channelIdOrig.substr(0, channelIdOrig.find('|'));
    size_t it = str1.find('_');
    if (it != string::npos)
    {
        m_payChannel = atoi(str1.substr(0, it).c_str());
        if (m_payChannel < PAY_TYPE_CARD || m_payChannel >= PAY_TYPE_END)
        {
            TRACE_ERROR("input channel_id %s , payChannel:%d error", channelIdOrig.c_str(), m_payChannel);
            throw EXCEPTION(ERR_BAD_PARAM, "input channel_id error");   
        }
        m_channelId = channelIdOrig.substr(it+1);
    }
    else
    {
        m_payChannel = PAY_TYPE_CARD;
    }

    TRACE_DEBUG("channelIdOrig: %s, m_channelId: %s, m_payChannel: %d", channelIdOrig.c_str(), m_channelId.c_str(), m_payChannel);
}

/**
* 非实名认证用户在开户前会先创建订单记录，但缺少trade_id，在实名认证通过并开通理财账户后补填trade_id
*/
void AbstractBuySpReq::UpdateFundTradeForReq()
{
	
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id) - 1);
	stRecord.Fstate = m_stTradeBuy.Fstate;
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fbank_type = m_params.getInt("bank_type");
	stRecord.Fuid = m_stTradeBuy.Fuid; //修复原先存在的uid为空导致更新用户交易单表失败的问题
    
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"), (m_fund_bind.Fuid == 0 || m_stTradeBuy.Fuid == 0) ? false : true);
}

void AbstractBuySpReq::CheckUserTempFail() throw (CException)
{
	// 记录创建成功后返回错误
	if(BIND_SPACC_TEMP_FAIL == m_fund_bind_sp_acc.Fstate){
		
    	throw EXCEPTION(ERR_NOT_BIND_SP_ACC, "the fund bind sp account record state not allowed buy.");
	}
}

void AbstractBuySpReq::CheckUserTotalShare() throw (CException)
{
		//余额申购和活动赠送都不检查限额
    if (m_params.getInt("purpose") == PURPOSE_BALANCE_BUY || m_params.getInt("purpose") == PURPOSE_ACTION_BUY)
    {
        return;
    }
      

    LONG currentTotalAsset = queryUserTotalAsset(m_fund_bind.Fuid,m_fund_bind.Ftrade_id);
    if (true == isUserAssetOverLimit(m_fund_bind.Fasset_limit_lev,currentTotalAsset, m_params.getLong("total_fee")))
    {
        //提现失败重新申购不限额
        if (true == checkInnerBalancePayForReBuy(gPtrFundDB, m_params.getString("fund_trans_id"), m_params.getString("total_fee"),""))
        {
            return;
        }
        
        TRACE_ERROR("user total Asset Over Limit!");
        throw CException(ERR_FUND_ASSET_OVER_LIMIT, "user total Asset Over Limit! ", __FILE__, __LINE__);
    }
}


/**
  * 打包输出参数
  */
void AbstractBuySpReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "sp_user", m_fund_bind_sp_acc.Fsp_user_id);
    CUrlAnalyze::setParam(rqst->odata, "sp_trans_id", m_fund_bind_sp_acc.Fsp_trans_id);
    CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
    CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
    CUrlAnalyze::setParam(rqst->odata, "fund_code", m_params.getString("fund_code").c_str());
    CUrlAnalyze::setParam(rqst->odata, "agree_risk", getAgreeRiskType(m_fund_sp_config,m_fund_bind));

	packBizReturnMsg(rqst);
		
    rqst->olen = strlen(rqst->odata);
    return;
}


