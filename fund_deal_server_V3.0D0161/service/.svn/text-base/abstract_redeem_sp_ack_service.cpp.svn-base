/**
  * FileName: abstract_redeem_sp_ack_service.cpp
  * Author: jessiegao	
  * Version :1.0
  * Date: 2015-3-12
  * Description: 基金交易服务 基金赎回确认
  */

#include "fund_commfunc.h"
#include "abstract_redeem_sp_ack_service.h"

AbstractRedeemSpAck::AbstractRedeemSpAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_stTradeBuy, 0, sizeof(ST_TRADE_FUND));
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fundUserTotalAcc, 0, sizeof(FundUserTotalAcc));
    memset(&m_fund_sp_config, 0, sizeof(FundSpConfig));
    
    m_need_updateExauAuthLimit = false;
    m_subAccDrawOk = false;
	m_balanceChanged = false;
}

/**
  * service step 1: 解析输入参数
  */
void AbstractRedeemSpAck::parseInputMsg(TRPC_SVCINFO* rqst,char* szMsg)  throw (CException)
{
	// 要保留请求数据，抛差错使用
    m_request = rqst;
    
    TRACE_DEBUG("[abstract_redeem_sp_ack_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "uid", 10000,MAX_INTEGER);
    m_params.readStrParam(szMsg, "cft_bank_billno", 10, 32);
    m_params.readIntParam(szMsg, "bank_type", 0,MAX_INTEGER);
    m_params.readStrParam(szMsg, "spid", 10, 15);
    m_params.readStrParam(szMsg, "sp_billno", 0, 32);
    m_params.readStrParam(szMsg, "fund_code", 0, 64);
    m_params.readIntParam(szMsg, "op_type", 1, 6);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

	m_params.readLongParam(szMsg,"redeem_total_fee",0,MAX_LONG);
	m_params.readLongParam(szMsg,"redeem2usr_fee",0,MAX_LONG);
	m_params.readLongParam(szMsg,"charge_fee",0,MAX_INTEGER);
	m_params.readStrParam(szMsg,"charge_type",0,1); // 收费方式

    m_optype = m_params.getInt("op_type");
	
    parseBizInputMsg(szMsg); // 读取业务参数

    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}
/**
  * 传入已有的商户配置,避免重复查询
  */
void AbstractRedeemSpAck::setSpConfig(FundSpConfig fundSpConfig)
{
    m_fund_sp_config = fundSpConfig;
}

/**
  * 检查参数，获取内部参数
  */
void AbstractRedeemSpAck::CheckParams() throw (CException)
{
    if(INF_REDEM_SP_ACK_SUC == m_optype)
    {
        CHECK_PARAM_EMPTY("sp_billno");   
    }
	
	// 检查收费方式是数值类型
	if(!isDigitString(m_params.getString("charge_type").c_str()))
	{
        throw CException(ERR_BAD_PARAM, string("Param is not a int value:charge_type=")+ m_params.getString("charge_type"), __FILE__, __LINE__);
	}

}

/**
  * 执行申购请求
  */
void AbstractRedeemSpAck::excute() throw (CException)
{
    try
    {
        CheckParams();

        CheckFundBind();

         /* 开启事务 */
        m_pFundCon->Begin();

        /* ckv操作放到事物之后真正提交 */
        gCkvSvrOperator->beginCkvtrans();

        /* 查询基金交易记录 */
        CheckFundTrade();

        /* 减账户余额，更新基金交易状态 */
        UpdateTradeState();

        /* 提交事务 */
        m_pFundCon->Commit();

        gCkvSvrOperator->commitCkvtrans();

        updateExauAuthLimitNoExcp();

        /* 更新各类ckv ,放在事务之后是避免事务回滚却写入ckv的问题*/
        updateCkvs();
		
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
		
		//回滚db前先回滚本地ckv
		gCkvSvrOperator->rollBackCkvtrans();

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
void AbstractRedeemSpAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUid(m_pFundCon, m_params.getInt("uid"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}


/**
  * 检查基金交易记录是否已经生成
  */
void AbstractRedeemSpAck::CheckFundTrade() throw (CException)
{
	// 没有交易记录，报错
	if(!QueryTradeFundByBankBillno(m_pFundCon, m_params.getString("cft_bank_billno").c_str(), 
		m_params.getInt("bank_type"), &m_stTradeBuy, true))
	{
		gPtrAppLog->error("buy record not exist, cft_bank_billno[%s]  ", m_params.getString("cft_bank_billno").c_str());
		throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
	}

	// 物理状态无效，报错
	if(LSTATE_INVALID == m_stTradeBuy.Flstate)
	{
		gPtrAppLog->error("fund buy pay, lstate is invalid. listid[%s], uid[%d] ", m_stTradeBuy.Flistid, m_stTradeBuy.Fuid);
		throw CException(ERR_TRADE_INVALID, "fund buy pay, lstate is invalid. ", __FILE__, __LINE__);
	}

	// 检查状态合法性
	if(m_stTradeBuy.Fstate<=0||m_stTradeBuy.Fstate>=TRADE_STATE_SIZE)
	{   
		char errMsg[128] = {0};
		snprintf(errMsg,sizeof(errMsg),"赎回订单状态异常[%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Fstate);
		throw CException(ERR_TRADE_INVALID, errMsg, __FILE__, __LINE__);
	}

/*   放到后面校验

	// 校验关键参数
	LONG checkFee=m_stTradeBuy.Ftotal_fee;
	// 只对已经扣减子账户成功的赎回单, 兼容提现金额与赎回金额不一致情况
	if((m_stTradeBuy.Fstate==REDEM_SUC||m_stTradeBuy.Fstate==REDEM_FINISH)&&m_stTradeBuy.Freal_redem_amt>0){
		checkFee=m_stTradeBuy.Freal_redem_amt;
	}
	if(checkFee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
*/

	if(m_stTradeBuy.Fuid!=m_params.getInt("uid"))
	{
		TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_stTradeBuy.Fuid, m_params.getInt("uid"));
		throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with input");
	}

	if( (0 != strcmp(m_stTradeBuy.Fspid, m_params.getString("spid").c_str())))
	{
		gPtrAppLog->error("fund trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_stTradeBuy.Fspid, m_params.getString("spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund trade exists, spid is different!", __FILE__, __LINE__);
	}

	// 得到交易相关信息
	m_params.setParam("uid", m_stTradeBuy.Fuid);
	m_params.setParam("trade_id", m_stTradeBuy.Ftrade_id);

}

/**
*根据请求类型，做相应的处理 
*/
void AbstractRedeemSpAck::UpdateTradeState()
{
	switch (m_optype)
    {       
        case INF_REDEM_SP_ACK_SUC: // 调基金公司赎回成功
			UpdateRedemTradeForSpInfoSuc();
            break;
			
		case INF_REDEM_SP_ACK_TIMEOUT: // 调基金公司赎回超时
            UpdateRedemTradeForInfoTimeout();
            break;
			
		case INF_REDEM_SP_ACK_FAIL: // 调基金公司赎回失败
            UpdateRedemTradeForInfoFail();
            break;
                    
		case INF_REDEM_BALANCE_ACK_SUC: //延迟类型使用:基金公司通知赎回份额确认成功
            UpdateRedemTradeForAckSuc();
            break;
			
		case INF_REDEM_BALANCE_ACK_FAIL: //延迟类型使用:基金公司通知赎回份额失败成功
            UpdateRedemTradeForAckFail();
            break;

		case INF_REDEM_SP_ACK_FINISH: // // 赎回单提现完成
            UpdateRedemTradeForFinish();
            break;
			
        default:
            throw CException(ERR_BAD_PARAM, "op_type invalid", __FILE__, __LINE__);
            break;

    }
}

/**
 *  实时通知基金公司成功
 */ 
void AbstractRedeemSpAck::UpdateRedemTradeForSpInfoSuc() throw (CException)
{
	//超时补单的订单直接去补单
	if(TRADE_RECORD_TIMEOUT == m_stTradeBuy.Fspe_tag)
	{
		UpdateRedemTradeForBudan();
		return;
	}
	
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		// 实时确认类型:直接更新为赎回成功
		UpdateRedemTradeForSuc();
		return;
	}else
	{
		// 延迟确认类型:仅通知成功
		UpdateRedemTradeForInfoSuc();
	}
}

void AbstractRedeemSpAck::CheckRedemTradeForInfoSuc() throw (CException)
{
	char errMsg[128]={0};
	
	// 检查重入	
	if(REDEEM_STATE_ORDER[REDEEM_INFO_SUC]<=REDEEM_STATE_ORDER[m_stTradeBuy.Fstate])
	{            
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}
	
	// 不支持合约机冻结类型
	if (PURPOSE_UNFREEZE_FOR_FETCH == m_stTradeBuy.Fpurpose)
	{
		snprintf(errMsg,sizeof(errMsg),"订单赎回通知不扣款, 不支持合约机类型[%s]",m_stTradeBuy.Flistid);
        throw CException(ERR_BAD_PARAM, errMsg, __FILE__, __LINE__);
	}

	// 赎回通知不支持垫支赎回
	if(m_stTradeBuy.Floading_type==DRAW_USE_LOADING)
	{
		snprintf(errMsg,sizeof(errMsg),"赎回通知不支持垫资[%s][%s]",m_stTradeBuy.Flistid,m_stTradeBuy.Fspid);
        throw CException(ERR_BAD_PARAM, errMsg, __FILE__, __LINE__);
	}

	// 检查份额一致性
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		snprintf(errMsg,sizeof(errMsg),"赎回通知份额不一致[%s][%s][%ld][%ld]",m_stTradeBuy.Flistid,m_stTradeBuy.Fspid,m_stTradeBuy.Ftotal_fee,m_params.getLong("total_fee"));
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// 只更新4状态的订单
	if(REDEM_ININ!=m_stTradeBuy.Fstate)
	{
		snprintf(errMsg,sizeof(errMsg),"赎回通知订单状态非法[%s][%s][%d]",m_stTradeBuy.Flistid,m_stTradeBuy.Fspid,m_stTradeBuy.Fstate);
		throw CException(ERR_BAD_PARAM, errMsg, __FILE__, __LINE__);	
	}
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_NONE);
	}
	return;
}

/**
  *  组装赎回单
  */
void AbstractRedeemSpAck::BuildRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
		if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
		{
			stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
		}else{
			stRecord.Fspe_tag = 0;
		}
        stRecord.Fstate = REDEEM_INFO_SUC;
        strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
        strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
        stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
        stRecord.Fuid = m_stTradeBuy.Fuid;
        //保存trade_id,更新交易记录时需要使用
        SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
        strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

        stRecord.Fpurpose = m_stTradeBuy.Fpurpose;
        stRecord.Floading_type = m_stTradeBuy.Floading_type;
        strncpy(stRecord.Fcharge_type, m_params.getString("charge_type").c_str(), sizeof(stRecord.Fcharge_type) - 1);
}

/**
  *  更新赎回确认数据
  */
void AbstractRedeemSpAck::RecordRedemTradeForInfoSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
    UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
 *  记录赎回相关的统计数据
 */
void AbstractRedeemSpAck::RecordSpconfigForInfoSuc() throw (CException)
{
    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    //更新赎回累计额度,t+1提现赎回不累计，其它包括普通赎回、消费、t+0赎回都要累计
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedomTotal(m_pFundCon, fundSpConfig, m_stTradeBuy.Ftotal_fee,m_stTradeBuy.Floading_type,m_stTradeBuy.Facc_time);
	
}

/**
 *  实时通知基金公司成功
 */ 
void AbstractRedeemSpAck::UpdateRedemTradeForInfoSuc() throw (CException)
{
	/**
	  *延迟确认类型:更新为赎回通知成功
         */
	// 检查状态
	CheckRedemTradeForInfoSuc();
	
	// 不冻结子账户,在赎回请求冻结
	//doDrawReq();

	// 子账户更新成功,一定要把单做成功
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	try
	{
    	// 组装交易单
    	BuildRedemTradeForInfoSuc(stRecord);
		// 记录交易单
		RecordRedemTradeForInfoSuc(stRecord);
		//记录spconfig数据
		//这里不需要检查垫资,放到最后记录,避免锁表时间过长
		RecordSpconfigForInfoSuc();
    }
    catch(CException& e)
    {
        alert(e.error(), string(stRecord.Flistid)+string("赎回子账户已经冻结成功，但单做失败:原因")+e.what());
        //子账户已经成功，需要抛差错补单
        if(ERR_TYPE_MSG)
        {
            //来自差错补单时间和赎回单创建时间超过10分钟的告警
            int inteval = (int)(time(NULL) - toUnixTime(m_stTradeBuy.Facc_time));	
            gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
            //补单10分钟仍未成功告警,告警服务本身会做压制，不再抛差错
            if(inteval >= 600 )
            {	
                char szErrMsg[256] = {0};
                snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回差错补单超过%d分钟未成功[%s]" ,inteval,stRecord.Flistid);
                alert(ERR_BUDAN_TOLONG, szErrMsg);
                throw;
            }
        }
        // 发消息给差错
        callErrorRpc(m_request, gPtrSysLog);
        
        throw;
    }
    
	//交易记录发MQ,组装变化的参数
	SyncRedemTradeForSuc(stRecord);

	//TODO : 确认是否发送MQ
	// 发送MQ消息
	sendFundBuy2MqMsg(m_stTradeBuy);
	
}

void AbstractRedeemSpAck::updateWxPrePayUserBalance()throw (CException)
{
    //非王府井用户直接返回
    if (isWxPrePayCardBusinessUser(m_pFundCon,m_params["spid"],m_fund_bind.Ftrade_id) == false)
    {
        return;
    } 
    
    ST_FUND_CONTROL_INFO controlInfo;
    memset(&controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    strncpy(controlInfo.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controlInfo.Ftrade_id)),
    controlInfo.Ftype=1;
    if (false == queryFundControlInfo(m_pFundCon,controlInfo,true)) //加锁查询
    {
        //不可能发生
        throw CException(ERR_DB_UNKNOW, "queryFundControlInfo fail", __FILE__, __LINE__);
    }

    if (m_stTradeBuy.Fpurpose == PURPOSE_UNFREEZE_FOR_FETCH) //商户赎回扣款
    {
        ST_UNFREEZE_FUND unFreezedata;
        memset(&unFreezedata,0,sizeof(ST_UNFREEZE_FUND));
        strncpy(unFreezedata.Funfreeze_id,m_stTradeBuy.Fcft_trans_id,sizeof(unFreezedata.Funfreeze_id)-1);
        if (false==queryFundUnFreezeByUnfreezeid(m_pFundCon,unFreezedata,false))
        {
            throw CException(ERR_QUERY_UNFREEZE_BILL, "query unfreeze bill fail ", __FILE__, __LINE__);
        }

        if ((string(unFreezedata.Fspid) == gPtrConfig->m_AppCfg.wx_wfj_spid)) //微信王府井预付卡商户号
        {
            if (controlInfo.Ftotal_fee  < m_params.getLong("total_fee")) //微信王府井发起扣款检查王府井受限金额
            {
                throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
            }

            subFundControlBalance(m_pFundCon, m_params.getLong("total_fee") ,m_params["systime"],m_fund_bind.Ftrade_id);

            //更新冻结单状态为冻结成功，即王府井消费赎回成功
            ST_UNFREEZE_FUND unFreezedataSet;
            memset(&unFreezedataSet,0,sizeof(ST_UNFREEZE_FUND));
            strncpy(unFreezedataSet.Funfreeze_id,m_stTradeBuy.Fcft_trans_id,sizeof(unFreezedataSet.Funfreeze_id)-1);
            unFreezedataSet.Fstate = FUND_UNFREEZE_OK;
            strncpy(unFreezedataSet.Fmodify_time,m_params["systime"],sizeof(unFreezedataSet.Fmodify_time)-1);
            strncpy(unFreezedataSet.Facc_time,m_params["systime"],sizeof(unFreezedataSet.Facc_time)-1);
            updateFundUnFreeze(m_pFundCon,unFreezedataSet, unFreezedata);
        }
        
    }
    else   //用户发起赎回不能赎回受限金额
    {
        LONG balance = querySubaccBalance(m_params.getInt("uid"),querySubaccCurtype(m_pFundCon, m_params.getString("spid")),true);
        if ( balance<m_params.getLong("total_fee")+controlInfo.Ftotal_fee) //王府井用户发起赎回金额不能赎回赎回部分的金额
        {
            throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
        }
    }
    return;
}

bool AbstractRedeemSpAck::CheckRedemTradeForSuc() throw (CException)
{	
	if(REDEM_SUC == m_stTradeBuy.Fstate || REDEM_FINISH == m_stTradeBuy.Fstate)
	{            
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	// 检查份额一致性
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// 检查状态是否可以更新:只更新4到5状态的交易单
	if( REDEM_ININ != m_stTradeBuy.Fstate)
	{
		//不为赎回初始状态，不允许再次更新成功。
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
    }
	// 设置默认手续费收取方式
	if(m_params.getString("charge_type").empty())
	{
		m_params.setParam("charge_type",TRADE_FUND_CHARGE_TYPE_NONE);
	}
	return true;
}

/**
  *  子账户赎回
  */
void AbstractRedeemSpAck::DrawSubacc() throw (CException)
{
	
	//由于子账户操作耗时都比较长，所以在垫资充足时把子账户操作放到锁基金配置表之前,提升并发能力
	if ((m_stTradeBuy.Floading_type == DRAW_USE_LOADING)
		&& false == preCheckSpLoaningEnough(m_pFundCon,m_stTradeBuy.Fspid,m_stTradeBuy.Ffund_code,m_stTradeBuy.Ftotal_fee))
	{
		//基金公司垫支等配置检查
		checkSpLoaning();
		//先减账户余额，减余额失败直接报错并回滚事务
		doDrawResult(SUBACC_FETCH_RESULT_OK);
	}
	else
	{
		//先减账户余额，减余额失败直接报错并回滚事务
		doDrawResult(SUBACC_FETCH_RESULT_OK);
		//基金公司垫支等配置检查
		try
		{
			checkSpLoaning();
		}
		catch(CException& e)
		{
			alert(e.error(), string("赎回子账户已经减成功，但检查垫资失败(可能导致垫资过度消耗):原因")+e.what());
		}
	}
}

/**
  *  组装赎回单
  */
void AbstractRedeemSpAck::BuildRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
        if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
        {
            stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
        }
        else
        {
            stRecord.Fspe_tag = 0;//超时补单成功需要将超时状态修改，否则导致不停补单
        }

        stRecord.Fstate = REDEM_SUC;
        strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
        strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
        stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
        stRecord.Fuid = m_stTradeBuy.Fuid;
        //保存trade_id,更新交易记录时需要使用
        SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
        strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);

        stRecord.Fpurpose = m_stTradeBuy.Fpurpose;
        stRecord.Floading_type = m_stTradeBuy.Floading_type;
        strncpy(stRecord.Fcharge_type, m_params.getString("charge_type").c_str(), sizeof(stRecord.Fcharge_type) - 1);
}

/**
  *  更新赎回确认数据
  */
void AbstractRedeemSpAck::RecordRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
    UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
  *  同步变更成功的参数到全局变量
  */
void AbstractRedeemSpAck::SyncRedemTradeForSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
	//交易记录发MQ,组装变化的参数
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcoding, stRecord.Fcoding, sizeof(m_stTradeBuy.Fcoding) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fpurpose = stRecord.Fpurpose;
}

/**
*	到基金公司赎回成功处理:更新为5状态
*	减基金账户余额
*/
void AbstractRedeemSpAck::UpdateRedemTradeForSuc() throw (CException)
{
	// 基金公司已经通知过的赎回再次确认:更新13到5状态的交易单
	if(REDEEM_INFO_SUC == m_stTradeBuy.Fstate)
	{
		UpdateRedemTradeForAckSuc();
		return;
	}
	// 检查参数	
	if(!CheckRedemTradeForSuc())
	{
		return;
	}

    //微信预付卡合作项目临时方案
    updateWxPrePayUserBalance();

	//减子账户
	DrawSubacc();
	
    //减子账户成功即返回业务成功，更新赎回单失败发差错补单，必须做成功
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    try
    {
    	// 组装交易单
    	BuildRedemTradeForSuc(stRecord);
		// 记录交易单
		RecordRedemTradeForSuc(stRecord);
    }
    catch(CException& e)
    {
        alert(e.error(), string("赎回子账户已经减成功，但单做失败:原因")+e.what());
        //子账户已经成功，需要抛差错补单
        if(ERR_TYPE_MSG)
        {
            //来自差错补单时间和赎回单创建时间超过10分钟的告警
            int inteval = (int)(time(NULL) - toUnixTime(m_stTradeBuy.Facc_time));	
            gPtrAppLog->warning("fund_deal_server.inteval:%ds", inteval);
			
            //补单10分钟仍未成功告警,告警服务本身会做压制，不再抛差错
            if(inteval >= 600 )
            {	
                char szErrMsg[256] = {0};
                snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回差错补单超过%d分钟未成功" ,inteval);
                alert(ERR_BUDAN_TOLONG, szErrMsg);
                throw;
            }
        }
        // 发消息给差错
        callErrorRpc(m_request, gPtrSysLog);
        
        throw;
    }
    
    // 余额发生变化
    m_balanceChanged = true;

	//交易记录发MQ,组装变化的参数
	SyncRedemTradeForSuc(stRecord);
	
	// 发送MQ消息
	sendFundBuy2MqMsg(m_stTradeBuy);
}


bool AbstractRedeemSpAck::CheckRedemTradeForAckSuc() throw (CException)
{	
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		throw CException(ERR_REDEM_DRAW_REFUSE, "实时份额确认类型不存在基金公司通知", __FILE__, __LINE__);
	}
	
	if(REDEM_SUC == m_stTradeBuy.Fstate || REDEM_FINISH == m_stTradeBuy.Fstate)
	{            
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	// 检查份额一致性
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// 检查状态是否可以更新:只更新13到5状态的交易单
	if( REDEEM_INFO_SUC!= m_stTradeBuy.Fstate)
	{
		throw CException(ERR_REDEM_DRAW_REFUSE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
    }
	// 检查金额
	CHECK_PARAM_EMPTY("redeem_total_fee"); //赎回总金额
	CHECK_PARAM_EMPTY("redeem2usr_fee"); //赎回给用户金额
	CHECK_PARAM_EMPTY("charge_fee"); //赎回给用户金额
	// 检查金额一致性
	if(m_params.getLong("charge_fee")+m_params.getLong("redeem2usr_fee")!=m_params.getLong("redeem_total_fee") )
	{
        char szErrMsg[128] = {0};
        snprintf(szErrMsg, sizeof(szErrMsg), "赎回确认金额不一致[%s][%ld][%ld][%ld]" ,m_stTradeBuy.Flistid,m_params.getLong("charge_fee"),m_params.getLong("redeem2usr_fee"),m_params.getLong("redeem_total_fee"));
		throw CException(ERR_REDEM_FEE_UNCONSISTENT, szErrMsg, __FILE__, __LINE__);
	}
	return true;
}

/**
  *  组装赎回单
  */
void AbstractRedeemSpAck::BuildRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
        strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
        stRecord.Fuid = m_stTradeBuy.Fuid;
        strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
		stRecord.Fpur_type = m_stTradeBuy.Fpur_type;

        stRecord.Fspe_tag = 0;
        stRecord.Fstate = REDEM_SUC;
        strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
		stRecord.Freal_redem_amt = m_params.getLong("redeem2usr_fee");
		stRecord.Fcharge_fee = m_params.getLong("charge_fee");		
		if(strcmp(m_stTradeBuy.Fcoding,"")==0)
		{
        	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
		}
}

/**
  *  更新赎回确认数据
  */
void AbstractRedeemSpAck::RecordRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
    UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
  *  同步变更成功的参数到全局变量
  */
void AbstractRedeemSpAck::SyncRedemTradeForAckSuc(ST_TRADE_FUND& stRecord) throw (CException)
{
	//交易记录发MQ,组装变化的参数
	m_stTradeBuy.Fstate= stRecord.Fstate;
	strncpy(m_stTradeBuy.Fcoding, stRecord.Fcoding, sizeof(m_stTradeBuy.Fcoding) - 1);
	strncpy(m_stTradeBuy.Fmemo, stRecord.Fmemo, sizeof(m_stTradeBuy.Fmemo) - 1);
	m_stTradeBuy.Fpurpose = stRecord.Fpurpose;
	m_stTradeBuy.Fspe_tag = stRecord.Fspe_tag;
	m_stTradeBuy.Freal_redem_amt = stRecord.Freal_redem_amt;
	m_stTradeBuy.Fcharge_fee = stRecord.Fcharge_fee;	
}

/**
*	到基金公司赎回成功处理
*      13 到5状态处理
*	减基金账户余额
*/
void AbstractRedeemSpAck::UpdateRedemTradeForAckSuc() throw (CException)
{
	// 检查参数	
	if(!CheckRedemTradeForAckSuc())
	{
		return;
	}

	//确认成功减子账户
	doDrawResult(SUBACC_FETCH_RESULT_OK);
	
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	// 组装交易单
    BuildRedemTradeForAckSuc(stRecord);
	// 记录交易单
	RecordRedemTradeForAckSuc(stRecord);
  
    // 余额发生变化
    m_balanceChanged = true;

	//交易记录发MQ,组装变化的参数
	SyncRedemTradeForSuc(stRecord);
	
	// 发送MQ消息
	sendFundBuy2MqMsg(m_stTradeBuy);

}

bool AbstractRedeemSpAck::CheckRedemTradeForAckFail() throw (CException)
{
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)
	{
		throw CException(ERR_REDEM_DRAW_REFUSE, "实时份额确认类型不存在基金公司通知", __FILE__, __LINE__);
	}
	// 检查份额一致性
	if(m_stTradeBuy.Ftotal_fee != m_params.getLong("total_fee"))
	{
		gPtrAppLog->error("fund buy pay, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_stTradeBuy.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_BAD_PARAM, "fund buy pay, total_fee is different!", __FILE__, __LINE__);
	}
	// 检查重入
	if(m_stTradeBuy.Fstate==REDEM_FAIL)
	{
		throw CException(ERR_REPEAT_ENTRY, "fund purchase record state has update ", __FILE__, __LINE__);
	}
	// 检查状态是否可以更新:只更新13到6状态的交易单
	if( REDEEM_INFO_SUC!= m_stTradeBuy.Fstate)
	{
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
    }
	return true;
}

void AbstractRedeemSpAck::BuildRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	stRecord.Fstate = REDEM_FAIL;
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
    strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
}

void AbstractRedeemSpAck::RecordRedemTradeForAckFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 更新数据
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}


/**
*	赎回确认失败处理
*/
void AbstractRedeemSpAck::UpdateRedemTradeForAckFail() throw (CException)
{
	// 检查参数
	if(!CheckRedemTradeForAckFail())
	{
		return;
	}
	// 确认失败解冻份额	
	doDrawResult(SUBACC_FETCH_RESULT_FAIL);
	
	// 修改赎回单状态为失败
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	BuildRedemTradeForAckFail(stRecord);
	RecordRedemTradeForAckFail(stRecord);
}


void AbstractRedeemSpAck::checkSpLoaning() throw (CException)
{
    //赎回类型为提现才累计赎回额度
    //创建记录时默认Fpurpose = PURPOSE_DRAW_T1,赎回成功才决定使用哪种类型
    //暂时决定除了T+1提现外，所有赎回都走垫资
    /*
    if(m_stTradeBuy.Fpurpose != PURPOSE_DRAW_T1 && m_stTradeBuy.Fpurpose != PURPOSE_DRAW_T0)
    {
    	return;
    }
    */

    FundSpConfig fundSpConfig;
    memset(&fundSpConfig, 0, sizeof(FundSpConfig));

    strncpy(fundSpConfig.Fspid, m_stTradeBuy.Fspid, sizeof(fundSpConfig.Fspid) - 1);
    strncpy(fundSpConfig.Ffund_code, m_stTradeBuy.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }

    if(m_stTradeBuy.Floading_type == DRAW_USE_LOADING) // 需要垫资
    {
        checkRedemOverLoading(m_pFundCon, fundSpConfig, m_params.getLong("total_fee"),true);
    }

    //更新赎回累计额度,t+1提现赎回不累计，其它包括普通赎回、消费、t+0赎回都要累计
    strncpy(fundSpConfig.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundSpConfig.Fmodify_time)-1);
    updateFundSpRedomTotal(m_pFundCon, fundSpConfig, m_stTradeBuy.Ftotal_fee,m_stTradeBuy.Floading_type,m_stTradeBuy.Facc_time);
	
}


/**
*赎回 通知超时处理
*/
void AbstractRedeemSpAck::UpdateRedemTradeForInfoTimeout() throw (CException)
{
	//根据配置决定赎回超时当成功还是失败，如果当成功处理，需要开启补单批跑程序对超时赎回单进行补单
	if("true" == gPtrConfig->m_AppCfg.redem_timeout_conf)
	{
		UpdateRedemTradeForSpInfoSuc();
	}
	else
	{
		UpdateRedemTradeForInfoFail();
	}
}
void AbstractRedeemSpAck::CheckRedemTradeForInfoFail() throw (CException)
{
	
	if(REDEM_FAIL == m_stTradeBuy.Fstate)
	{
		//重入错误
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_ININ != m_stTradeBuy.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
}

void AbstractRedeemSpAck::BuildRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	if(INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
	}
	stRecord.Fstate = REDEM_FAIL;		
	strncpy(stRecord.Fcoding, m_params.getString("sp_billno").c_str(), sizeof(stRecord.Fcoding) - 1);
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;
    //保存trade_id,更新交易记录时需要使用
    strncpy(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id,sizeof(stRecord.Ftrade_id)-1);
	strncpy(stRecord.Fmemo, m_params.getString("desc").c_str(), sizeof(stRecord.Fmemo) - 1);
}

void AbstractRedeemSpAck::RecordRedemTradeForInfoFail(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 更新数据
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

/**
*	赎回通知失败处理
*/
void AbstractRedeemSpAck::UpdateRedemTradeForInfoFail() throw (CException)
{
	// 检查参数
	CheckRedemTradeForInfoFail();
	
	// 确认失败先解冻份额	
	doDrawResult(SUBACC_FETCH_RESULT_FAIL);

	// 构建对象
	ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	BuildRedemTradeForInfoFail(stRecord);
	RecordRedemTradeForInfoFail(stRecord);
}


void AbstractRedeemSpAck::BuildRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException)
{
	// 赎回到余额直接更新到账时间和到账状态
	if(m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA||m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA_T1)
	{
		strncpy(stRecord.Ffetch_arrival_time,m_params.getString("systime").c_str(),sizeof(stRecord.Ffetch_arrival_time));
		stRecord.Ffetch_result=FETCH_RESULT_BALANCE_SUCCESS;
	}

	stRecord.Fstate = REDEM_FINISH;

	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fspe_tag = m_stTradeBuy.Fspe_tag; //不能修改超时标记
	stRecord.Fuid = m_stTradeBuy.Fuid;
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
}

void AbstractRedeemSpAck::RecordRedemTradeForFinish(ST_TRADE_FUND& stRecord) throw (CException)
{
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

void AbstractRedeemSpAck::UpdateRedemTradeForFinish() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
	
	if(REDEM_FINISH == m_stTradeBuy.Fstate)
	{
		//重入错误
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}

	if(REDEM_SUC != m_stTradeBuy.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}
	BuildRedemTradeForFinish(stRecord);
	RecordRedemTradeForFinish(stRecord);
}

/**
  * 5状态的超时补单
  */
void AbstractRedeemSpAck::UpdateRedemTradeForBudan() throw (CException)
{
	ST_TRADE_FUND  stRecord;
	memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

	if(REDEM_SUC != m_stTradeBuy.Fstate && REDEM_FINISH != m_stTradeBuy.Fstate && REDEEM_INFO_SUC!=m_stTradeBuy.Fstate)
	{
		//数据库状态不能更新
		throw CException(ERR_BUY_CANNOT_UPDATE, "fund purchase record state cannot update! ", __FILE__, __LINE__);
	}

	if(TRADE_RECORD_TIMEOUT != m_stTradeBuy.Fspe_tag || INF_REDEM_SP_ACK_TIMEOUT == m_optype)
	{
		//非超时单，当重入错误,或者超时在重入，直接返回
		throw CException(ERR_REPEAT_ENTRY, "redem ack repeat enter! ", __FILE__, __LINE__);
	}
	
	stRecord.Fspe_tag = 0;
	strncpy(stRecord.Flistid, m_stTradeBuy.Flistid, sizeof(stRecord.Flistid) - 1);
	stRecord.Fpur_type = m_stTradeBuy.Fpur_type;
	stRecord.Fuid = m_stTradeBuy.Fuid;    
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_stTradeBuy.Ftrade_id);
	
	UpdateFundTrade(m_pFundCon, stRecord, m_stTradeBuy, m_params.getString("systime"));
}

bool AbstractRedeemSpAck::payNotifyOvertime(string pay_suc_time)
{
	if(pay_suc_time.size() == 14)
	{
		//YYYYMMDDHHMMSS 转YYYY-MM-DD HH:MM:SS
		pay_suc_time = changeDatetimeFormat(pay_suc_time);
	}
	int pay_time = toUnixTime(pay_suc_time.c_str());
	if(pay_time + gPtrConfig->m_AppCfg.paycb_overtime_inteval < (int)(time(NULL)) )
	{
		return true;	
	}

	return false;
}


/**
 * 子账户核心提现
 * 子账户失败了不能差错补单，用户可以主动再发起赎回，如果补单会多赎回或造成财付通损失
 * 写在这里的，赎回到底是以基金公司成功为准，还是以子账户为准，一直在争论，前面讨论定下来的方案，后面有人忘记了就再拿出来说事。。。
 * 赎回涉及多个系统 基金公司+余额增值系统+子账户+财付通 +及其它相关操作设计的系统
 * 一个大的原则是减钱成功加基金交易单成功即认为成功，其它的失败都通过补单来完成。
 * 减钱涉及到两个系统，基金公司系统减份额，子账户减钱，到底以哪个为准? 以谁为准都有问题:
 * 个人认为以子账户成功为准更为靠谱，理由: 1)子账户是财付通可控制的，成功与否不由基金公司决定；2)子账户异常了我们可以通过实时对账发现，并及时补单；
 * 存在的问题:子账户失败未扣钱，基金公司已经减份额，用户无法再次赎回，可以通过第二天与基金公司的对账把用户资金补上，后续可以优化，对这类单实时通知基金公司赎回失败；
 *
 */
void AbstractRedeemSpAck::doDraw() throw (CException)
{
	gPtrAppLog->debug("doDraw, listid[%s]  ", m_stTradeBuy.Fsub_trans_id);
	string subAccControlList; //合约机解冻提现 子账户受控单号

   if (PURPOSE_UNFREEZE_FOR_FETCH == m_stTradeBuy.Fpurpose)
   {
       //合约机解冻扣款，需要先查询出子账户的受控单号
       ST_UNFREEZE_FUND unFreezedata;
       memset(&unFreezedata,0,sizeof(ST_UNFREEZE_FUND));
       strncpy(unFreezedata.Funfreeze_id,m_stTradeBuy.Fcft_trans_id,sizeof(unFreezedata.Funfreeze_id)-1);
       if (false==queryFundUnFreezeByUnfreezeid(m_pFundCon,unFreezedata,false))
       {
            throw CException(ERR_QUERY_UNFREEZE_BILL, "query unfreeze bill fail ", __FILE__, __LINE__);
       }
       subAccControlList = unFreezedata.Fsub_acc_control_no;
   }


	try
	{
	    SubaccDraw(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeBuy.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeBuy.Facc_time,subAccControlList);
           m_subAccDrawOk = true;
	}
	
	catch(CException& e)
	{

		//赎回要谨慎补单
		//如果赎回子账户减钱超过10分钟没成功的告警不在补单，无论是差错补单还是外部批跑补单，10分钟没成功的异常时都会触发告警
		if(payNotifyOvertime(m_stTradeBuy.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回补单超过10分钟子账户仍未成功");		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);

		throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功
		
	}
	
}

void AbstractRedeemSpAck::doDrawReq() throw (CException)
{    
	gPtrAppLog->debug("doDrawReq, listid[%s]  ", m_stTradeBuy.Fsub_trans_id);
    try
    {
        SubaccFetchReq(gPtrSubaccRpc, m_params.getString("spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeBuy.Fsub_trans_id, m_params.getLong("total_fee"), m_stTradeBuy.Facc_time,m_fund_sp_config.Fcurtype);
           m_subAccDrawOk = true;
    }
    catch(CException &e)
    {
		// 如果子账户赎回冻结超过10分钟没成功的告警不再补单，
		// 无论是差错补单还是外部批跑补单，10分钟没成功的异常时都会触发告警
		if(payNotifyOvertime(m_stTradeBuy.Facc_time))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回补单超过10分钟子账户仍未冻结成功[%s]",m_stTradeBuy.Flistid);		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);

		throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功
    }
}

void AbstractRedeemSpAck::doDrawResult(int result) throw (CException)
{
	gPtrAppLog->debug("doDrawResult, listid[%s][%d]  ", m_stTradeBuy.Fsub_trans_id,result);

	string subaccTime;
	if(m_fund_sp_config.Fbuy_confirm_type==SPCONFIG_BALANCE_PAY_CONFIRM)	
	{	
		// 直接确认金额使用acc_time进行差错补单
		subaccTime = m_stTradeBuy.Facc_time;
	}else{
		// 延时确认类型使用当前时间确认提现,等待批跑重入
		subaccTime = m_params.getString("systime");
	}

	try
	{
	    SubaccFetchResult(gPtrSubaccRpc, m_fund_sp_config.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_stTradeBuy.Fsub_trans_id, m_stTradeBuy.Ftotal_fee, subaccTime ,result,m_fund_sp_config.Fcurtype);
           m_subAccDrawOk = true;
	}
	catch(CException& e)
	{
		// 延迟确认类型不发差错直接拋错异常
		if(m_fund_sp_config.Fbuy_confirm_type!=SPCONFIG_BALANCE_PAY_CONFIRM)	
		{
			throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功
		}

		// 赎回要谨慎补单
		// 如果赎回子账户减钱超过10分钟没成功的告警不在补单，无论是差错补单还是外部批跑补单，10分钟没成功的异常时都会触发告警
		if(payNotifyOvertime(subaccTime))	
		{
			char szErrMsg[256] = {0};
			snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.赎回确认补单超过10分钟子账户仍未冻结成功[%s]",m_stTradeBuy.Flistid);		  
			alert(ERR_BUDAN_TOLONG, szErrMsg);

			throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功

		}
		
		TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

		// 发消息给差错
		callErrorRpc(m_request, gPtrSysLog);

		throw;//直接抛出异常，阻止后面继续执行，不能把赎回单做成功
		
	}
	
}

void AbstractRedeemSpAck::updateCkvs()
{
	//更新用户操作CKV, 赎回更新失败不抛异常
	if(m_balanceChanged)
	{
		//setFundTotalaccToKV(m_fundUserTotalAcc);
		// updateUserAcc(m_stTradeBuy); 首页轮播删除,不考虑赎回排序
	}
}

void AbstractRedeemSpAck::updateExauAuthLimitNoExcp()
{
	// 余额没变化,不进行更新
	if(!m_balanceChanged)
	{
		return;
	}
	// T+0  垫资赎回才限额
	if (m_stTradeBuy.Floading_type == 0)
	{
		return;
	}

	//累加exau 放在事务外，避免超时导致的事务回滚
	//赎回请求的时候检查exau限制，累加时失败不报错，避免多次累加，或赎回单无法处理成功的问题
	try
	{
		//累计用户赎回限额
		int redem_type = (m_stTradeBuy.Floading_type == 0?DRAW_ARRIVE_TYPE_T1:DRAW_ARRIVE_TYPE_T0);
		updateExauAuthLimit(gPtrExauRpc,m_fund_bind.Fuid, m_params.getLong("total_fee"),m_fund_bind.Fcre_id,redem_type);
	}
	catch(CException& e)
	{
		TRACE_ERROR("updateExauAuthLimit error.[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
	}
}


void AbstractRedeemSpAck::sendFundBuy2MqMsg(ST_TRADE_FUND fundTradeBuy)
{
	char szMsg[MAX_MSG_LEN + 1] = {0};

    // 组装关键参数
    CUrlAnalyze::setParam(szMsg, "Flistid", fundTradeBuy.Flistid, true);
    CUrlAnalyze::setParam(szMsg, "Fspid", fundTradeBuy.Fspid);
    CUrlAnalyze::setParam(szMsg, "Fcoding", fundTradeBuy.Fcoding);
	CUrlAnalyze::setParam(szMsg, "Ftrade_id", fundTradeBuy.Ftrade_id);
	CUrlAnalyze::setParam(szMsg, "Fuid", fundTradeBuy.Fuid);
	CUrlAnalyze::setParam(szMsg, "Ffund_code", fundTradeBuy.Ffund_code);
	CUrlAnalyze::setParam(szMsg, "Fpur_type", fundTradeBuy.Fpur_type);
	CUrlAnalyze::setParam(szMsg, "Ftotal_fee", fundTradeBuy.Ftotal_fee);
	CUrlAnalyze::setParam(szMsg, "Fstate", fundTradeBuy.Fstate);
	CUrlAnalyze::setParam(szMsg, "Ftrade_date", fundTradeBuy.Ftrade_date);
	CUrlAnalyze::setParam(szMsg, "Ffund_vdate", fundTradeBuy.Ffund_vdate);
	CUrlAnalyze::setParam(szMsg, "Fcreate_time", fundTradeBuy.Fcreate_time);
	CUrlAnalyze::setParam(szMsg, "Fmodify_time", fundTradeBuy.Fmodify_time);
	CUrlAnalyze::setParam(szMsg, "Fcft_trans_id", fundTradeBuy.Fcft_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_charge_ctrl_id", fundTradeBuy.Fcft_charge_ctrl_id);
	CUrlAnalyze::setParam(szMsg, "Fsp_fetch_id", fundTradeBuy.Fsp_fetch_id);
	CUrlAnalyze::setParam(szMsg, "Fcft_bank_billno", fundTradeBuy.Fcft_bank_billno);
	CUrlAnalyze::setParam(szMsg, "Fsub_trans_id", fundTradeBuy.Fsub_trans_id);
	CUrlAnalyze::setParam(szMsg, "Fcur_type", fundTradeBuy.Fcur_type);
	CUrlAnalyze::setParam(szMsg, "Fpurpose", fundTradeBuy.Fpurpose);
	CUrlAnalyze::setParam(szMsg, "Facc_time", fundTradeBuy.Facc_time);
	CUrlAnalyze::setParam(szMsg, "Fchannel_id", fundTradeBuy.Fchannel_id);
	CUrlAnalyze::setParam(szMsg, "Fmemo", fundTradeBuy.Fmemo);

	sendMsg2Mq(szMsg);
}


/**
  * 打包输出参数
  */
void AbstractRedeemSpAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());

	int loading_type = m_stTradeBuy.Floading_type;
    if (m_params.getInt("fetch_type") != 4) // t1 开关放开前用之前的临时t1方案返回该标记给itg延迟提现
    {
        if (m_stTradeBuy.Fpurpose==PURPOSE_REDEM_TO_BA)
        {
            CUrlAnalyze::setParam(rqst->odata, "fetch_type", 5); //赎回到理财通余额
        }
        else
        {
            CUrlAnalyze::setParam(rqst->odata, "fetch_type", (loading_type==DRAW_USE_LOADING?DRAW_ARRIVE_TYPE_T0:DRAW_ARRIVE_TYPE_T1));
        }
    }
    else
    {
        CUrlAnalyze::setParam(rqst->odata, "fetch_type",4);
    }
    CUrlAnalyze::setParam(rqst->odata, "loading_type", loading_type);

    rqst->olen = strlen(rqst->odata);
    return;
}


