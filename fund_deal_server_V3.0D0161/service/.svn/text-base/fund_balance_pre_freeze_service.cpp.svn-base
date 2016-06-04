/**
  * FileName: fund_balance_pre_freeze_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-29
  * Description: 份额预冻结
  */
  
#include "fund_commfunc.h"
#include "fund_balance_pre_freeze_service.h"

FundBalancePreFreeze::FundBalancePreFreeze(CMySQL* mysql)
{
    m_pFundCon = mysql;
}

void FundBalancePreFreeze::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_balance_pre_freeze_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "sp_billno", 10, 32); //商户冻结请求单号
    m_params.readStrParam(szMsg, "fund_spid", 10, 15);
    m_params.readStrParam(szMsg, "spid", 10, 15);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readStrParam(szMsg, "cre_id", 0, 32);
    m_params.readStrParam(szMsg, "true_name", 0, 32);
    m_params.readStrParam(szMsg, "buy_id", 0, 32);
    m_params.readIntParam(szMsg, "cre_type", 0, MAX_INTEGER);
    m_params.readStrParam(szMsg, "pay_type", FREEZE_FROM_BUY_SP, FREEZE_FROM_BALANCE);
    m_params.readStrParam(szMsg, "purpose", 0, 32);
	m_params.readStrParam(szMsg, "pre_card_no", 0, 32); // 预付卡卡号
	m_params.readStrParam(szMsg, "pre_card_partner", 0, 32);  // 预付卡门店商户号
	

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

void FundBalancePreFreeze::CheckParams()  throw (CException)
{
    //检查spid 及fund_code 是否有效
    FundSpConfig spConfig;
    memset(&spConfig,0,sizeof(spConfig));
    strncpy(spConfig.Fspid, m_params.getString("fund_spid").c_str(), sizeof(spConfig.Fspid) - 1);
    strncpy(spConfig.Ffund_code, m_params.getString("fund_code").c_str(), sizeof(spConfig.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,spConfig, false);

    if (m_params.getInt("pay_type") == FREEZE_FROM_BUY_SP && 
        m_params.getString("buy_id").find(m_params["fund_spid"])!=0)
    {
        throw CException(ERR_BAD_PARAM, "buy_id check fail!", __FILE__, __LINE__);
    }
}

void FundBalancePreFreeze::CheckBalanceFreeze()  throw (CException)
{
    ST_FREEZE_FUND freezeData;
    memset(&freezeData,0,sizeof(freezeData));
    strncpy(freezeData.Fspid,m_params["spid"],sizeof(freezeData.Fspid)-1);
    strncpy(freezeData.Fcoding,m_params["sp_billno"],sizeof(freezeData.Fcoding)-1);
    if (false == queryFundFreeze(m_pFundCon, freezeData, true))
    {
        return;
    }

    // 检查关键参数
    if (0 != strcmp(freezeData.Fqqid, m_params["uin"]))
    {
        gPtrAppLog->error("freezeData exists, Fqqid is different! Fqqid in db[%s], uin input[%s]", 
			freezeData.Fqqid, m_params.getString("uin").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, uin is different!", __FILE__, __LINE__);
    }

    if( (0 != strcmp(freezeData.Ffund_spid, m_params.getString("fund_spid").c_str())))
    {
        gPtrAppLog->error("freezeData exists, fund_spid is different! fund_spid in db[%s], fund_spid input[%s]", 
			freezeData.Ffund_spid, m_params.getString("fund_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, fund_spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(freezeData.Ffund_code, m_params.getString("fund_code").c_str()))
    {
        gPtrAppLog->error("freezeData exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			freezeData.Ffund_code, m_params.getString("fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, fund_code is different!", __FILE__, __LINE__);
    }

    if(freezeData.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("freezeData exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			freezeData.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData  exists, total_fee is different!", __FILE__, __LINE__);
    }
    
    if(freezeData.Fpay_type!= m_params.getInt("pay_type"))
    {
        gPtrAppLog->error("freezeData exists, pay_type is different! pay_type in db[%lld], pay_type input[%lld] ", 
			freezeData.Fpay_type, m_params.getInt("pay_type"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData  exists, pay_type is different!", __FILE__, __LINE__);
    }


    if (!m_params.getString("buy_id").empty() && m_params.getString("buy_id") != freezeData.Fbuy_id)
    {
        gPtrAppLog->error("freezeData exists, buy_id is different! buy_id in db[%s], buy_id input[%s] ", 
			freezeData.Fcre_id, m_params.getString("buy_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, buy_id is different!", __FILE__, __LINE__);
    }
    
    if(!m_params.getString("cre_id").empty() && m_params.getString("cre_id") != freezeData.Fcre_id)
    {
        gPtrAppLog->error("freezeData exists, cre_id is different! cre_id in db[%s], cre_id input[%s] ", 
			freezeData.Fcre_id, m_params.getString("cre_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, cre_id is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("true_name").empty() && m_params.getString("true_name") != freezeData.Ftrue_name)
    {
        gPtrAppLog->error("freezeData exists, true_name is different! true_name in db[%s], true_name input[%s] ", 
			freezeData.Ftrue_name, m_params.getString("true_name").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, true_name is different!", __FILE__, __LINE__);
    }

    if (freezeData.Fstate != FUND_FREEZE_INIT)
    {
        gPtrAppLog->error("freezeData state invalid =%d",freezeData.Fstate);
        throw CException(ERR_INVALID_STATE, "freezeData record state invalid. ", __FILE__, __LINE__);
    }
    else
    {
        throw CException(ERR_REPEAT_ENTRY, "freezeData record state reentered. ", __FILE__, __LINE__);
    }
}

void FundBalancePreFreeze::checkFreezeControl()  throw (CException)
{
	// 非王府井商户号不检查
	if(m_params.getString("spid")!=gPtrConfig->m_AppCfg.wx_wfj_spid)
	{
		return;
	}
	ST_FUND_BIND fundBind;
	bool hasFundBind = QueryFundBindByUin(m_pFundCon, m_params.getString("uin").c_str(), &fundBind, true);
	// 新用户绑定
	if(!hasFundBind)
	{
		return;
	}
	
    // 预付卡购买检查    
	ST_FUND_CONTROL_INFO controlInfo;	
    memset(&controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
    //设置微信王府井参数
	ST_FUND_CONTROL_INFO controlParams;	
    memset(&controlParams,0,sizeof(ST_FUND_CONTROL_INFO));
	strncpy(controlParams.Fspid,gPtrConfig->m_AppCfg.wx_wfj_spid.c_str(),sizeof(controlParams.Fspid)-1);
	strncpy(controlParams.Fcard_no,m_params["pre_card_no"],sizeof(controlParams.Fcard_no)-1);
	strncpy(controlParams.Ftrade_id,fundBind.Ftrade_id,sizeof(controlParams.Ftrade_id)-1);
	// 通知发现预付卡卡号冲突，不允许下单
	if(!checkWxPreCardBuy(m_pFundCon,controlParams,controlInfo,true))
	{
        throw CException(ERR_UKA_CONTROL_INVALID, "UKA预付卡卡号冲突，不允许下单", __FILE__, __LINE__);
	}
}

void FundBalancePreFreeze::RecordBalanceFreeze()  throw (CException)
{
    ST_FREEZE_FUND freezeData;
    memset(&freezeData,0,sizeof(freezeData));
    strncpy(freezeData.Fspid,m_params["spid"],sizeof(freezeData.Fspid)-1);
    strncpy(freezeData.Fcoding,m_params["sp_billno"],sizeof(freezeData.Fcoding)-1);

    if (m_params.getInt("pay_type") == FREEZE_FROM_BUY_SP)
    {
        strncpy(freezeData.Ffreeze_id,m_params["buy_id"],sizeof(freezeData.Ffreeze_id)-1);
    }
    else
    {
        strncpy(freezeData.Ffreeze_id,GenerateIdsBySpid(m_params["fund_spid"]).c_str(),sizeof(freezeData.Ffreeze_id)-1);
    }
    strncpy(freezeData.Ffund_spid,m_params["fund_spid"],sizeof(freezeData.Ffund_spid)-1);
    strncpy(freezeData.Ffund_code,m_params["fund_code"],sizeof(freezeData.Ffund_code)-1);
    strncpy(freezeData.Fbuy_id,m_params["buy_id"],sizeof(freezeData.Fbuy_id)-1);
    freezeData.Ffreeze_type = 1; //购买合约机
    freezeData.Fstate = FUND_FREEZE_INIT;
    freezeData.Flstate = LSTATE_VALID;
    freezeData.Ftotal_fee = m_params.getLong("total_fee");
    freezeData.Fcur_type = querySubaccCurtype(m_pFundCon, m_params.getString("fund_spid"));
    freezeData.Fcre_type = m_params.getInt("cre_type");
    strncpy(freezeData.Fcre_id,m_params["cre_id"],sizeof(freezeData.Fcre_id)-1);
    strncpy(freezeData.Ftrue_name,m_params["true_name"],sizeof(freezeData.Ftrue_name)-1);
    strncpy(freezeData.Fchannel_id,m_params["channel_id"],sizeof(freezeData.Fchannel_id)-1);
    strncpy(freezeData.Fqqid,m_params["uin"],sizeof(freezeData.Fqqid)-1);
    strncpy(freezeData.Fcreate_time,m_params["systime"],sizeof(freezeData.Fcreate_time)-1);
    strncpy(freezeData.Fmodify_time,m_params["systime"],sizeof(freezeData.Fmodify_time)-1);
    freezeData.Fpay_type = m_params.getInt("pay_type");
    strncpy(freezeData.Fpurpose,m_params["purpose"],sizeof(freezeData.Fpurpose)-1);
    strncpy(freezeData.Fsub_acc_freeze_no,GenerateIdsBySpid("1006").c_str(),sizeof(freezeData.Fsub_acc_freeze_no)-1);
	strncpy(freezeData.Fpre_card_no,m_params["pre_card_no"],sizeof(freezeData.Fpre_card_no)-1);
	strncpy(freezeData.Fpre_card_partner,m_params["pre_card_partner"],sizeof(freezeData.Fpre_card_partner)-1);
    insertFundFreeze(m_pFundCon, freezeData);
}

void FundBalancePreFreeze::excute()  throw (CException)
{
    try
    {
        CheckParams();

         // 开启事务 
        m_pFundCon->Begin();

        //重入检查
        CheckBalanceFreeze();

		// 检查预付卡购买
		checkFreezeControl();
        
        //创建冻结单
        RecordBalanceFreeze();

        // 提交事务 
        m_pFundCon->Commit();
    }
    catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
}
 
void FundBalancePreFreeze::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    rqst->olen = strlen(rqst->odata);
    return;
}

