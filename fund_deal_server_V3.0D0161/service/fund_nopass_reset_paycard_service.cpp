/**
  * FileName: fund_nopass_reset_paycard_service.cpp
  * Author: jiggersong
  * Version :1.0
  * Date: 2014-01-27
  * Description: 基金交易服务 重置用户的安全卡
  * 该接口为客服系统调用
  */

#include "fund_commfunc.h"
#include "db_fund_pay_card.h"
#include "db_fund_bank_config.h"
#include "fund_nopass_reset_paycard_service.h"

FundNopassResetPayCard::FundNopassResetPayCard(CMySQL* mysql)
{
    m_pFundCon = mysql;
    m_can_usr_rst_paycard = false;
}

/**
  * service step 1: 解析输入参数
  */
void FundNopassResetPayCard::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
	char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    m_spid = szSpId;
    
    TRACE_DEBUG("[fund_update_pay_card_service] receives: %s", szMsg);

    /**
     * 0-客服重置安全卡
     * 1-用户重置安全卡检查
     * 2-用户重置安全卡
     */    
    m_params.readIntParam(szMsg, "op_type", 0,2);
    int op_type = m_params.getInt("op_type");
        
	m_params.readStrParam(szMsg, "uin", 1, 64);
	m_params.readIntParam(szMsg, "uid", 1,MAX_INTEGER);

    if (RST_TYPE_KF == op_type 
        || RST_TYPE_USR_SELF == op_type) {
        //重置安全卡必须传入新安全卡的信息
        m_params.readStrParam(szMsg, "bind_serialno", 1, 64);
    	m_params.readIntParam(szMsg, "bank_type", 1,MAX_INTEGER);
    	m_params.readStrParam(szMsg, "card_tail", 1, 32);
        m_params.readStrParam(szMsg, "bank_id", 1, 32);
        m_params.readStrParam(szMsg, "mobile", 1, 21);
    }

    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口签名
    m_params.readStrParam(szMsg, "memo", 0, 255);   //备注信息

	GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}

/*
 * 生成基金注册用token
 */
string FundNopassResetPayCard::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照uin|uid|bind_serialno|bank_type|card_tail|bank_id|mobile|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["uid"] << "|" ;
    ss << m_params["bind_serialno"] << "|" ;
    ss << m_params["bank_type"] << "|" ;
    ss << m_params["card_tail"] << "|" ;
    ss << m_params["bank_id"] << "|" ;
    ss << m_params["mobile"] << "|" ;
    ss << gPtrConfig->m_AppCfg.nopass_reset_paycard_key;

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

/*
 * 检验token
 */
void FundNopassResetPayCard::CheckToken() throw (CException)
{
	// 生成token
	string token = GenFundToken();

    if (StrUpper(m_params.getString("token")) != StrUpper(token))
    {   
	    TRACE_ERROR("fund authen token check failed, input=%s", 
	                m_params.getString("token").c_str());
	    throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    }
}

void FundNopassResetPayCard::checkFundBind() throw (CException)
{
	memset(&m_fund_bind, 0, sizeof(m_fund_bind));
	if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
	{
		TRACE_WARN("query fund bind not exist,uin=[%s]", m_params.getString("uin").c_str());	
		throw EXCEPTION(ERR_FUNDBIND_NOTREG, "fund bind not exist");    
	}

    //用对用户重置的操作，检查用户信息的uid与传入的uid是否一致
    int op_type = m_params.getInt("op_type");
    if (RST_TYPE_USR_SELF == op_type
        || RST_TYPE_USR_SELF_CHK == op_type) {
        if (m_fund_bind.Fuid > 0 && m_params.getInt("uid") != m_fund_bind.Fuid)
            throw EXCEPTION(ERR_BAD_PARAM, "fund bind uid inconstistent");
    }
}

/**
  * 检查参数，获取内部参数
  */
void FundNopassResetPayCard::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

    //校验用户是否开通理财通账号
    checkFundBind();
}

void FundNopassResetPayCard::checkNewCardInfo() throw (CException)
{
    string uin = m_params.getString("uin");
    string bind_serialno = m_params.getString("bind_serialno");
    string bank_id = m_params.getString("bank_id");
    int bank_type = m_params.getInt("bank_type");
    
    //校验bank_type
	vector<FundBankConfig> fundBankConfigVec;
	if( !queryFundAllBankConfig(m_pFundCon, fundBankConfigVec, false))
	{
		TRACE_ERROR("query bank_type config error");
        throw EXCEPTION(ERR_BAD_PARAM, "query bank_type config error");    
	}
    
    bool bankTypeIsok=false;
    for(unsigned int i=0;i<fundBankConfigVec.size();i++)
    {
        if( fundBankConfigVec[i].Flstate!=1 ) //有效状态才匹配
        {
            continue;
        }
        
        if( fundBankConfigVec[i].Fsupport_type!=1 && fundBankConfigVec[i].Fsupport_type!=2 )    //1  全部；2  仅支持转入
        {
            continue;
        }
        
        if( fundBankConfigVec[i].Fbank_type==bank_type )
        {
            bankTypeIsok=true;
            break;
        }
    }
    
    if( !bankTypeIsok )
    {
		TRACE_ERROR("user new card bank_type error, uin=[%s]  bank_type=[%d]",uin.c_str(),bank_type);
        throw EXCEPTION(ERR_BAD_PARAM, "user new card bank_type error");   
    }
    
    map<string,string> cardMap;
    
    if( !queryBindCardInfo(uin,bind_serialno,cardMap) )
    {
        TRACE_ERROR("user new card not bind, uin=[%s]  bind_serialno=[%s]",uin.c_str(),bind_serialno.c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "user new card not bind");    
    }
    
    if( cardMap["bank_status"]!="2" )
    {
        TRACE_ERROR("user new card bind status error, uin=[%s]  bind_serialno=[%s] bank_status=[%s]",uin.c_str(),bind_serialno.c_str(),cardMap["bank_status"].c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "user new card bank status error");   
    }

    if( cardMap["bind_status"]!="2" )
    {
        TRACE_ERROR("user new card bind status error, uin=[%s]  bind_serialno=[%s] bind_status=[%s]",uin.c_str(),bind_serialno.c_str(),cardMap["bind_status"].c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "user new card bind status error");   
    }

    if( cardMap["bind_flag"]!="1" )
    {
        TRACE_ERROR("user new card bind status error, uin=[%s]  bind_serialno=[%s] bind_flag=[%s]",uin.c_str(),bind_serialno.c_str(),cardMap["bind_flag"].c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "user new card bind flag error");   
    }

    if( bank_id!= cardMap["bankid"] )
    {
        TRACE_ERROR("user new card cardid error, uin=[%s]  bind_serialno=[%s] bankid=[%s]",uin.c_str(),bind_serialno.c_str(),cardMap["bankid"].c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "user new card cardid error");  
    }
}

void FundNopassResetPayCard::checkCanRstPayCard() throw (CException)
{   
    LONG total_fee = 0;

    total_fee += getUsrBalance();
    if (total_fee >= gPtrConfig->m_AppCfg.usr_rst_paycard_fee_limit) {
        TRACE_ERROR("total balabce greater than 500.00");
        throw EXCEPTION(ERR_USR_NOT_RST_PAY_CARD, "total balabce greater than 500.00");
    }

    //检查是否有未处理完成的申购单
    total_fee += getBuyFee();
    if (total_fee >= gPtrConfig->m_AppCfg.usr_rst_paycard_fee_limit) {
        TRACE_ERROR("total balabce+undone buy fee greater than 500.00");
        throw EXCEPTION(ERR_USR_NOT_RST_PAY_CARD, "total balabce greater than 500.00");
    }

    //检查是否有未完成的赎回单
    total_fee += getRedemFee();
    if (total_fee >= gPtrConfig->m_AppCfg.usr_rst_paycard_fee_limit) {
        TRACE_ERROR("total balabce+undone buy fee+undone redem fee greater than 500.00");
        throw EXCEPTION(ERR_USR_NOT_RST_PAY_CARD, "total balabce greater than 500.00");
    }

    //检查是否有在途的提现
    total_fee += getFundFetchFee();
    if (total_fee >= gPtrConfig->m_AppCfg.usr_rst_paycard_fee_limit) {
        TRACE_ERROR("total balabce+undone buy fee+undone redem fee+fetching fee greater than 500.00");
        throw EXCEPTION(ERR_USR_NOT_RST_PAY_CARD, "total balabce greater than 500.00");
    }

    //检查是否有未完成的充值单
    total_fee += getChargingFee();
    if (total_fee >= gPtrConfig->m_AppCfg.usr_rst_paycard_fee_limit) {
        TRACE_ERROR("total_balabce+undone_buy_fee+undone_redem_fee+fetching_fee+charing_fee greater than 500.00");
        throw EXCEPTION(ERR_USR_NOT_RST_PAY_CARD, "total balabce greater than 500.00");
    }

    //前面逻辑检查完到此处说明用户已满足更换条件
    m_can_usr_rst_paycard = true;
}

LONG FundNopassResetPayCard::getBuyFee()throw (CException)
{
    time_t tmp_cur_time = time(NULL);
    tmp_cur_time += -1*60*gPtrConfig->m_AppCfg.undone_trans_timespan;
    string strCurTime = getSysTime(tmp_cur_time);
        
    string Tminus2Date;
    string TminusDate;
    string curDate = m_params.getString("systime");
    curDate = curDate.substr(0,4) + curDate.substr(5,2) + curDate.substr(8,2);
    bool isCurTDay = false;
    getTminus2TransDate(gPtrFundSlaveDB, curDate, Tminus2Date, TminusDate, isCurTDay);
    string start_time = changeDateFormat(Tminus2Date) + " 00:00:00";

    /**
     * 检查T-2 0点到当前时间是否有未完成的申购单
     * 申购单状态:申购成功、退款中、退款完成认为是最终状态
     * 对初始状态单只检查一段时间内(暂定15分钟)是否有初始单
     */
    char sql_cond[MAX_SQL_LEN] = {0};
    snprintf(sql_cond, sizeof(sql_cond)-1, " AND  Fpur_type in (1,2,3,9,10,11) " 
        " AND ( (Fstate not in(1,3,8,9) AND Facc_time>'%s' ) OR (Fstate=1 AND Fmodify_time>'%s')) ",
        escapeString(start_time).c_str(),
        escapeString(strCurTime).c_str());

    LONG total_fee = getBuyRecordsFee(gPtrFundSlaveDB, m_fund_bind.Fuid, sql_cond);

    return total_fee;
}

LONG FundNopassResetPayCard::getRedemFee() throw (CException)
{
    /**
     * 检查赎回单是否有非初始单和未到最终状态的单
     */    
    char sql_cond[MAX_SQL_LEN] = {0};
    snprintf(sql_cond, sizeof(sql_cond)-1," AND  Fpur_type in (4,12) AND Fstate not in(6,10,20) ");
    return getRedemRecordsFee(gPtrFundSlaveDB, m_fund_bind.Fuid, sql_cond);
}

LONG FundNopassResetPayCard::getFundFetchFee()throw (CException)
{
    LONG total_fee = getFetchingRecordsFee(gPtrFundSlaveDB,m_params["uin"],getTime_yyyymm());
    total_fee += getFetchingRecordsFee(gPtrFundSlaveDB,m_params["uin"],getTime_yyyymm(-1));

    return total_fee;
}

LONG FundNopassResetPayCard::getChargingFee()throw (CException)
{
    time_t tmp_cur_time = time(NULL);
    tmp_cur_time += -1*60*gPtrConfig->m_AppCfg.undone_trans_timespan;
    string strCurTime = getSysTime(tmp_cur_time);
    
    //检查是否有未完成的充值单
    char sql_cond[MAX_SQL_LEN] = {0};
    snprintf(sql_cond, sizeof(sql_cond)-1," Flstate=1 AND Ftype=1 AND Fstate=0 AND Fmodify_time>'%s' ",
        escapeString(strCurTime).c_str());
    LONG total_fee = getChargeRecordsFee(gPtrFundSlaveDB, m_fund_bind.Ftrade_id, sql_cond);

    return total_fee;
}

LONG FundNopassResetPayCard::getUsrBalance() throw (CException)
{
    //总资产(所有基金的资产和理财通余额)必须小于500元
    LONG total_asset = queryUserTotalAsset(m_fund_bind.Fuid, m_fund_bind.Ftrade_id);

    return total_asset;
}

/**
  * 执行申购请求
  */
void FundNopassResetPayCard::excute() throw (CException)
{

    CheckParams();

    int op_type = m_params.getInt("op_type");

    try {
        switch(op_type)
        {   
            //客服重置
            case RST_TYPE_KF:
            {
                checkNewCardInfo();
                
                restPayCard();
                
                break;
            }

            //用户重置安全卡的前置检查
            case RST_TYPE_USR_SELF_CHK:
            {
                checkCanRstPayCard();
                
                break;
            }

            //用户重置安全卡
            case RST_TYPE_USR_SELF:
            {
                checkCanRstPayCard();
                
                checkNewCardInfo();
                
                restPayCard();
                
                break;
            }
                
            default:
                throw EXCEPTION(ERR_BAD_PARAM, "invalid op_type"); 
                break;
        }
    }
    catch (CException &e) 
    {
        if (ERR_USR_NOT_RST_PAY_CARD == (unsigned int)e.error()) {
            m_can_usr_rst_paycard = false;
        } else {
            throw;
        }
    }
}

void FundNopassResetPayCard::restPayCard() throw (CException)
{
    try
    {
         /* 开启事务 */
        m_pFundCon->Begin();
        /* 更新用户支付卡 */
        UpdatePayCard(); 
        /* 提交事务 */
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
        m_pFundCon->Rollback();
    	TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
    	
    	throw;
    }
}

void FundNopassResetPayCard::UpdatePayCard()
{
    ST_FUND_BIND fund_bind;
    memset(&fund_bind, 0, sizeof(fund_bind));
    //加锁用户信息
	QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &fund_bind, true);
        
    FundPayCard old_pay_card;
	memset(&old_pay_card, 0, sizeof(old_pay_card));
	strncpy(old_pay_card.Fqqid, m_params.getString("uin").c_str(), sizeof(old_pay_card.Fqqid) - 1);

	if(!queryFundPayCard(m_pFundCon,old_pay_card,true))
	{
		TRACE_WARN("user pay card not exist,uin=[%s]", m_params.getString("uin").c_str());	
		throw EXCEPTION(ERR_PAY_CARD_NOT_EXIST, "user pay card not exist");   
	}

    //重置支付卡信息
	FundPayCard fund_pay_card;
	
	memset(&fund_pay_card, 0, sizeof(FundPayCard));
	strncpy(fund_pay_card.Fqqid, m_params.getString("uin").c_str(), sizeof(fund_pay_card.Fqqid) - 1);
	strncpy(fund_pay_card.Fbind_serialno,  m_params.getString("bind_serialno").c_str(), sizeof(fund_pay_card.Fbind_serialno) - 1);
	fund_pay_card.Fbank_type=m_params.getInt("bank_type") ;
	strncpy(fund_pay_card.Fcard_tail,  m_params.getString("card_tail").c_str(), sizeof(fund_pay_card.Fcard_tail) - 1);
	strncpy(fund_pay_card.Fbank_id,  m_params.getString("bank_id").c_str(), sizeof(fund_pay_card.Fbank_id) - 1);
	strncpy(fund_pay_card.Fmobile,  m_params.getString("mobile").c_str(), sizeof(fund_pay_card.Fmobile) - 1);
	strncpy(fund_pay_card.Fmodify_time,  m_params.getString("systime").c_str(), sizeof(fund_pay_card.Fmodify_time) - 1);
    
    strncpy(fund_pay_card.Ftrade_id, old_pay_card.Ftrade_id, sizeof(fund_pay_card.Ftrade_id) - 1);
    strncpy(fund_pay_card.Fcreate_time, old_pay_card.Fcreate_time, sizeof(fund_pay_card.Fcreate_time) - 1);
    
    //将修改前的安全卡信息写入日志
    saveFundPayCardLog(m_pFundCon, old_pay_card, m_params.getString("bind_serialno"),
         "[fund_nopass_reset_paycard_service]"+m_params.getString("memo"));
    
	updateFundPayCard(m_pFundCon, fund_pay_card);
	
	setPayCardToKV(m_pFundCon, fund_pay_card);
}

/**
  * 打包输出参数
  */
void FundNopassResetPayCard::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    //只有在用户更换安全卡时才返回该字段，对客服重置保持原逻辑
    int op_type = m_params.getInt("op_type");
    if (RST_TYPE_USR_SELF_CHK == op_type
        || RST_TYPE_USR_SELF == op_type) {
        CUrlAnalyze::setParam(rqst->odata, "can_rst_paycard", 
            m_can_usr_rst_paycard ? "1" : "0");
    }

    rqst->olen = strlen(rqst->odata);
    return;
}

