/**
  * FileName: fund_balance_freeze_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-29
  * Description: 份额冻结
  */

#include "fund_commfunc.h"
#include "fund_balance_freeze_service.h"

FundBalanceFreeze::FundBalanceFreeze(CMySQL* mysql)
{
    memset(&m_FundSpConfig,0,sizeof(m_FundSpConfig));
    memset(&m_freezeBill,0,sizeof(m_freezeBill));
    memset(&m_fund_bind,0,sizeof(m_fund_bind));
    m_pFundCon = mysql;
}

void FundBalanceFreeze::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_balance_freeze_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "sp_billno", 0, 32); //商户冻结请求单号
    m_params.readStrParam(szMsg, "spid", 0, 15);
    m_params.readStrParam(szMsg, "freeze_id", 0, 32);  //冻结单号
    m_params.readStrParam(szMsg, "fund_spid", 10, 15);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "buy_id", 0, 32);
    m_params.readIntParam(szMsg, "pay_type", FREEZE_FROM_BUY_SP, FREEZE_FROM_BALANCE);
    m_params.readStrParam(szMsg, "token", 32, 64);

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

void FundBalanceFreeze::CheckParams()  throw (CException)
{
    //检查spid 及fund_code 是否有效
    if (m_params.getString("freeze_id").empty() 
        && (m_params.getString("sp_billno").empty() ||m_params.getString("spid").empty()))
    {
        throw CException(ERR_BAD_PARAM, "freeze_id,sp_billno,spid must not all be empty!", __FILE__, __LINE__);
    }

    checkToken() ;
}

void FundBalanceFreeze::CheckBalanceFreeze()  throw (CException)
{
    if (m_params.getString("freeze_id").empty())
    {
        strncpy(m_freezeBill.Fspid,m_params["spid"],sizeof(m_freezeBill.Fspid)-1);
        strncpy(m_freezeBill.Fcoding,m_params["sp_billno"],sizeof(m_freezeBill.Fcoding)-1);
    }
    else
    {
        strncpy(m_freezeBill.Ffreeze_id,m_params["freeze_id"],sizeof(m_freezeBill.Ffreeze_id)-1);
    }
    if (false == queryFundFreeze(m_pFundCon, m_freezeBill, true))
    {
        gPtrAppLog->error("freeze record not exist");
        throw CException(ERR_BUYPAY_NOLIST, "transfer record not exist! ", __FILE__, __LINE__);
    }

    if (m_params.getInt("pay_type") != m_freezeBill.Fpay_type)
    {
        gPtrAppLog->error("freezeData exists, pay_type is different! pay_type in db[%d], pay_type input[%d]", 
			m_freezeBill.Fpay_type, m_params.getInt("pay_type"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, pay_type is different!", __FILE__, __LINE__); 
    }

    if (!m_params.getString("sp_billno").empty() && m_params.getString("sp_billno") != m_freezeBill.Fcoding)
    {
        gPtrAppLog->error("freezeData exists, sp_billno is different! sp_billno in db[%s], sp_billno input[%s]", 
			m_freezeBill.Fcoding, m_params.getString("sp_billno").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, sp_billno is different!", __FILE__, __LINE__);   
    }
    if (!m_params.getString("spid").empty() && m_params.getString("spid") != m_freezeBill.Fspid)
    {
        gPtrAppLog->error("freezeData exists, spid is different! spid in db[%s], spid input[%s]", 
			m_freezeBill.Fspid, m_params.getString("spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, spid is different!", __FILE__, __LINE__);   
    }
    if (!m_params.getString("freeze_id").empty() && m_params.getString("freeze_id") != m_freezeBill.Ffreeze_id)
    {
        gPtrAppLog->error("freezeData exists, freeze_id is different! freeze_id in db[%s], freeze_id input[%s]", 
			m_freezeBill.Ffreeze_id, m_params.getString("freeze_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, freeze_id is different!", __FILE__, __LINE__);   
    }
    
    // 检查关键参数
    if (0 != strcmp(m_freezeBill.Fqqid, m_params["uin"]))
    {
        gPtrAppLog->error("freezeData exists, Fqqid is different! Fqqid in db[%s], uin input[%s]", 
			m_freezeBill.Fqqid, m_params.getString("uin").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, uin is different!", __FILE__, __LINE__);
    }

    if( (0 != strcmp(m_freezeBill.Ffund_spid, m_params.getString("fund_spid").c_str())))
    {
        gPtrAppLog->error("freezeData exists, fund_spid is different! fund_spid in db[%s], fund_spid input[%s]", 
			m_freezeBill.Ffund_spid, m_params.getString("fund_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "freezeData exists, fund_spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_freezeBill.Ffund_code, m_params.getString("fund_code").c_str()))
    {
        gPtrAppLog->error("freezeData exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_freezeBill.Ffund_code, m_params.getString("fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, fund_code is different!", __FILE__, __LINE__);
    }

    if(m_freezeBill.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("freezeData exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_freezeBill.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, total_fee is different!", __FILE__, __LINE__);
    }
    if (m_freezeBill.Fpay_type == FREEZE_FROM_BUY_SP && m_params.getString("buy_id") != m_freezeBill.Fbuy_id)
    {
        gPtrAppLog->error("freezeData exists, buy_id is different! buy_id in db[%s], buy_id input[%s] ", 
			m_freezeBill.Fbuy_id, m_params.getString("buy_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, buy_id is different!", __FILE__, __LINE__); 
    }
    

    if (m_freezeBill.Fstate == FUND_FREEZE_OK)
    {
        throw CException(ERR_REPEAT_ENTRY, "freezeData record state already freeze ok. ", __FILE__, __LINE__);
    }

    if ((m_freezeBill.Fpay_type==FREEZE_FROM_BUY_SP && m_freezeBill.Fstate != FUND_FREEZE_PAY_BUYED)
        || (m_freezeBill.Fpay_type==FREEZE_FROM_BALANCE && m_freezeBill.Fstate != FUND_FREEZE_INIT))
    {
        gPtrAppLog->error("freezeData state invalid =%d,pay_type=%d",m_freezeBill.Fstate,m_freezeBill.Fpay_type);
        throw CException(ERR_INVALID_STATE, "freezeData record state invalid. ", __FILE__, __LINE__);
    }
   

}

/*
 * 查询基金账户是否存在
 */
void FundBalanceFreeze::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}

/*
*检查是否绑定基金公司帐号，并且可交易
*/
void FundBalanceFreeze::CheckFundBindSpAcc() throw (CException)
{
    FundBindSp fund_bind_sp_acc;
    memset(&fund_bind_sp_acc,0,sizeof(FundBindSp));
    strncpy(fund_bind_sp_acc.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fund_bind_sp_acc.Ftrade_id) - 1);
    strncpy(fund_bind_sp_acc.Fspid, m_params.getString("fund_spid").c_str(), sizeof(fund_bind_sp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, fund_bind_sp_acc, false);
}

void FundBalanceFreeze::updateBalanceFreeze()throw (CException)
{
    ST_FREEZE_FUND freezeBill;
    memset(&freezeBill,0,sizeof(freezeBill));
    strncpy(freezeBill.Ffreeze_id,m_freezeBill.Ffreeze_id,sizeof(m_freezeBill.Ffreeze_id)-1);
    freezeBill.Fstate = FUND_FREEZE_OK;
    freezeBill.Fuid = m_fund_bind.Fuid;
    strncpy(freezeBill.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(freezeBill.Ftrade_id)-1);
    strncpy(freezeBill.Fsub_acc_freeze_no,m_freezeBill.Fsub_acc_freeze_no,sizeof(freezeBill.Fsub_acc_freeze_no)-1);
    strncpy(freezeBill.Facc_time,m_params["systime"],sizeof(freezeBill.Facc_time)-1);
    strncpy(freezeBill.Fmodify_time,m_params["systime"],sizeof(freezeBill.Fmodify_time)-1);
    updateFundFreeze(m_pFundCon,freezeBill,m_freezeBill);
}


void FundBalanceFreeze::CheckBuyRecord()throw (CException)
{ 
    if (FREEZE_FROM_BUY_SP != m_freezeBill.Fpay_type) //冻结余额的方式，没有申购单
    {
        return;
    }
    ST_TRADE_FUND buyOrder;
    memset(&buyOrder,0,sizeof(buyOrder));
    
    // 没有交易记录，报错
    if(!QueryTradeFund(m_pFundCon, m_freezeBill.Fbuy_id, PURTYPE_PURCHASE,&buyOrder, false))
    {
        gPtrAppLog->error("buy record not exist, Fbuy_id[%s]  ", m_freezeBill.Fbuy_id);
        throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
    }

    // 物理状态无效，报错
    if(LSTATE_VALID != buyOrder.Flstate)
    {
        gPtrAppLog->error("fund buy , lstate is invalid. listid[%s], uid[%d] ", buyOrder.Flistid, buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "buy list  lstate is invalid. ", __FILE__, __LINE__);
    }

    // 校验关键参数
    if(buyOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("buy total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			buyOrder.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "buy total_fee is different!", __FILE__, __LINE__);
    }

    if( buyOrder.Fuid!= m_fund_bind.Fuid)
    {
        TRACE_ERROR("uid in db=%d diff with input=%d", 
					buyOrder.Fuid, m_fund_bind.Fuid);
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with fundbind");
    }

    if (0 != strcmp(buyOrder.Fspid, m_params.getString("fund_spid").c_str()))
    {
        gPtrAppLog->error("buy trade exists, spid is different! spid in db[%s], spid input[%s]", 
			buyOrder.Fspid, m_params.getString("fund_spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "buy trade exists, spid is different!", __FILE__, __LINE__);
    }

    if (buyOrder.Fstate != PAY_OK && buyOrder.Fstate != PURCHASE_SUC)
    {
        gPtrAppLog->error("fund buy , state=%d is invalid. listid[%s], uid[%d] ", buyOrder.Fstate,buyOrder.Flistid, buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "buyOrder list  state is invalid. ", __FILE__, __LINE__);   
    }

    //目前申购单中通过purpose 字段已经申购单号和冻结单号一致保证申购单和冻结单匹配
    // 校验冻结单号
    if (string(m_freezeBill.Ffreeze_id) != buyOrder.Flistid)
    {
        gPtrAppLog->error("fund trade exists, freeze_id is different! rela_listid in db[%s], freeze_id input[%s] ", 
			buyOrder.Flistid, m_freezeBill.Ffreeze_id);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, rela_listid and  freeze_id is different!", __FILE__, __LINE__);
    }
    
    if (buyOrder.Fpurpose!= PURPOSE_FREEZE)
    {
        gPtrAppLog->error("fund buy , Fpurpose=%d is invalid. listid[%s], uid[%d] ", buyOrder.Fpurpose,buyOrder.Flistid, buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "buyOrder list  Fpurpose is invalid. ", __FILE__, __LINE__);   
    }
    
}

void FundBalanceFreeze::freezeSubAccBalance() throw (CException)
{
    if ((string(m_freezeBill.Fspid) == gPtrConfig->m_AppCfg.wx_wfj_spid)) //微信王府井预付卡商户号
    {
        return; //临时方案
    }
    
    try
    {
        SubaccFreeze(gPtrSubaccRpc, m_params["fund_spid"], m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_freezeBill.Fsub_acc_freeze_no, m_params.getLong("total_fee"), m_params["systime"],"购买合约机冻结");
    }
    catch(CException &e)
    {
        //超过一定时间然后没有冻结成功的发送告警，需要人工查看原因
        if ((time_t)(toUnixTime(m_freezeBill.Fmodify_time))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval < (time(NULL)))
        {
            alert(e.error(), (string("用户")+m_freezeBill.Fqqid+string("购买合约机子账户冻结失败单号:")+m_freezeBill.Ffreeze_id+"错误码原因"+e.what()).c_str());
        }
        throw;
    }
}

void FundBalanceFreeze::checkToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
       // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["spid"] << "|" ; 
    ss << m_params["freeze_id"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["buy_id"] << "|" ;
    ss << m_params["fund_spid"] << "|" ;
    ss << gPtrConfig->m_AppCfg.freeze_service_key;

    
    TRACE_DEBUG("check token, sourceStr=[%s]", ss.str().c_str());
    
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    if (StrUpper(m_params.getString("token")) != StrUpper(buff))
    {   
        TRACE_DEBUG("token check failed, input=%s", 
	                m_params.getString("token").c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    } 
}
 
void FundBalanceFreeze::excute()  throw (CException)
{
    try
    {
        CheckParams();

        // 检查基金账户记录 
        CheckFundBind();

        //检查基金账户绑定基金公司交易账户记录 
        CheckFundBindSpAcc();
        
         // 开启事务 
        m_pFundCon->Begin();

        //重入检查
        CheckBalanceFreeze();

        //校验申购单
        CheckBuyRecord();

        //冻结子账户余额
        freezeSubAccBalance();
        
        //冻结单
        updateBalanceFreeze();

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
 
void FundBalanceFreeze::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "spid", m_freezeBill.Fspid);
    CUrlAnalyze::setParam(rqst->odata, "sp_billno", m_freezeBill.Fcoding);
    CUrlAnalyze::setParam(rqst->odata, "freeze_id", m_freezeBill.Ffreeze_id);
    rqst->olen = strlen(rqst->odata);
    return;
}

 