/**
  * FileName: fund_balance_unfreeze_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-29
  * Description: 份额解冻
  */
  
#include "fund_commfunc.h"
#include "fund_balance_unfreeze_service.h"

FundBalanceUnFreeze::FundBalanceUnFreeze(CMySQL* mysql)
{
    memset(&m_freezeBill,0,sizeof(m_freezeBill));
    memset(&m_unfreezeData,0,sizeof(m_unfreezeData));
    memset(&m_fund_bind,0,sizeof(m_fund_bind));
    
    m_pFundCon = mysql;
    m_bFreezeSunaccSuc = true;
    m_subAccErrCode = 0;
}

void FundBalanceUnFreeze::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_balance_unfreeze_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "sp_billno", 0, 32); //商户解冻请求单号
    m_params.readStrParam(szMsg, "spid", 0, 15);
    m_params.readStrParam(szMsg, "freeze_id", 10, 32);  //冻结单号
    m_params.readStrParam(szMsg, "fund_spid", 10, 15);
    m_params.readStrParam(szMsg, "fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG); //解冻金额
    m_params.readLongParam(szMsg, "control_fee", 0, MAX_LONG); //解冻后受控金额
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readIntParam(szMsg, "unfreeze_type", UNFREEZE_SP_REFUND, UNFREEZE_INNER_CACEL);
    m_params.readStrParam(szMsg, "desc", 0, 128);
    m_params.readStrParam(szMsg, "token", 32, 64);
    m_params.readStrParam(szMsg, "sub_spid", 0, 16);
    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);
}

void FundBalanceUnFreeze::CheckParams()  throw (CException)
{
    checkToken();
    if (m_params.getLong("control_fee") > (m_params.getLong("total_fee")))
    {
        throw EXCEPTION(ERR_BAD_PARAM, "control_fee check  error,bigger than total_fee");    
    }
    
    if (m_params.getInt("unfreeze_type") == UNFREEZE_FOR_FETCH
        && m_params.getLong("control_fee") <=0)
    {
        throw EXCEPTION(ERR_BAD_PARAM, "control_fee check  error with unfreeze_type");    
    }
    
}

void FundBalanceUnFreeze::checkToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
       // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["sp_billno"] << "|" ;
    ss << m_params["spid"] << "|" ; 
    ss << m_params["freeze_id"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["fund_spid"] << "|" ;
    ss << m_params["unfreeze_type"] << "|" ;
    ss << gPtrConfig->m_AppCfg.freeze_service_key;

    
    //TRACE_DEBUG("check token, sourceStr=[%s]", ss.str().c_str());
    
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    if (StrUpper(m_params.getString("token")) != StrUpper(buff))
    {   
        TRACE_DEBUG("token check failed, input=%s", 
	                m_params.getString("token").c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "input token error");    
    } 
}

void FundBalanceUnFreeze::CheckBalanceUnFreeze(bool mustExist)  throw (CException)
{
    memset(&m_unfreezeData,0,sizeof(m_unfreezeData));
    strncpy(m_unfreezeData.Fspid,m_params["spid"],sizeof(m_unfreezeData.Fspid)-1);
    strncpy(m_unfreezeData.Fcoding,m_params["sp_billno"],sizeof(m_unfreezeData.Fcoding)-1);
    
    if (false == queryFundUnFreeze(m_pFundCon, m_unfreezeData, true))
    {
        if (mustExist)
        {
            throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData not exists,!!", __FILE__, __LINE__);
        }
        return;
    }

    // 检查关键参数

    if( (0 != strcmp(m_unfreezeData.Ffund_spid, m_params.getString("fund_spid").c_str())))
    {
        gPtrAppLog->error("unfreezeData exists, fund_spid is different! fund_spid in db[%s], fund_spid input[%s]", 
			m_unfreezeData.Ffund_spid, m_params.getString("fund_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, fund_spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_unfreezeData.Ffund_code, m_params.getString("fund_code").c_str()))
    {
        gPtrAppLog->error("unfreezeData exists, fund_code is different! fund_code in db[%s], fund_code input[%s] ", 
			m_unfreezeData.Ffund_code, m_params.getString("fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, fund_code is different!", __FILE__, __LINE__);
    }

    if(m_unfreezeData.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("unfreezeData exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_unfreezeData.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, total_fee is different!", __FILE__, __LINE__);
    }

    if(m_unfreezeData.Fcontrol_fee!= m_params.getLong("control_fee"))
    {
        gPtrAppLog->error("unfreezeData exists, control_fee is different! control_fee in db[%lld], control_fee input[%lld] ", 
			m_unfreezeData.Fcontrol_fee, m_params.getLong("control_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, control_fee is different!", __FILE__, __LINE__);
    }
    
    
    if(m_unfreezeData.Funfreeze_type != m_params.getInt("unfreeze_type"))
    {
        gPtrAppLog->error("unfreezeData exists, unfreeze_type is different! unfreeze_type in db[%d], unfreeze_type input[%d] ", 
			m_unfreezeData.Funfreeze_type, m_params.getInt("unfreeze_type"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, unfreeze_type is different!", __FILE__, __LINE__);
    }


    if(m_fund_bind.Fuid != m_unfreezeData.Fuid)
    {
        gPtrAppLog->error("unfreezeData exists, Fuid is different! Fuid in m_unfreezeData[%d], m_fund_bind  Fuid[%d] ", 
			m_unfreezeData.Fuid, m_fund_bind.Fuid);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, Fuid is different!", __FILE__, __LINE__);
    }
    
    
    if(0 != strcmp(m_unfreezeData.Ffreeze_id, m_params.getString("freeze_id").c_str()))
    {
        gPtrAppLog->error("unfreezeData exists, freeze_id is different! freeze_id in db[%s], freeze_id input[%s] ", 
			m_unfreezeData.Ffreeze_id, m_params.getString("freeze_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "unfreezeData exists, freeze_id is different!", __FILE__, __LINE__);
    }

    if (m_unfreezeData.Fstate == FUND_UNFREEZE_OK)
    {
        throw CException(ERR_REPEAT_ENTRY, "unfreezeData record state reentered. ", __FILE__, __LINE__);
    }

    if (m_unfreezeData.Fstate != CREATE_INIT)
    {
        gPtrAppLog->error("unfreezeData state invalid =%d",m_unfreezeData.Fstate);
        throw CException(ERR_INVALID_STATE, "unfreezeData record state invalid. ", __FILE__, __LINE__);
    }
    
}

void FundBalanceUnFreeze::CheckFreezeRecord()  throw (CException)
{
    //临时方案，无对应冻结单
    if ((m_params.getString("spid")== gPtrConfig->m_AppCfg.wx_wfj_spid)) //微信王府井预付卡商户号
    {
        ST_FUND_CONTROL_INFO controlInfo;
        memset(&controlInfo,0,sizeof(ST_FUND_CONTROL_INFO));
        strncpy(controlInfo.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(controlInfo.Ftrade_id)),
        controlInfo.Ftype=1;
        if (false == queryFundControlInfo(m_pFundCon,controlInfo,false)) 
        {
            //不可能发生
            throw CException(ERR_DB_UNKNOW, "queryFundControlInfo fail", __FILE__, __LINE__);
        }
        if (controlInfo.Ftotal_fee  < m_params.getLong("total_fee")) //微信王府井发起扣款检查王府井受限金额
        {
            throw CException(ERR_USER_BALANCE_CONTROLED, "wx wfj controled, not enough money", __FILE__, __LINE__);
        }
        return; 
    }


    strncpy(m_freezeBill.Ffreeze_id,m_params["freeze_id"],sizeof(m_freezeBill.Ffreeze_id)-1);

    if (false == queryFundFreeze(m_pFundCon, m_freezeBill, true))
    {
        gPtrAppLog->error("freeze record not exist");
        throw CException(ERR_BUYPAY_NOLIST, "transfer record not exist! ", __FILE__, __LINE__);
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

    if(m_freezeBill.Ftotal_fee < m_params.getLong("total_fee")+m_freezeBill.Ftotal_unfreeze_fee)
    {
        gPtrAppLog->error("freezeData exists, total_fee check fail! m_freezeBill.Ftotal_fee=[%lld], m_freezeBill.Ftotal_unfreeze_fee=[%lld],total_fee input[%lld] ", 
			m_freezeBill.Ftotal_fee,m_freezeBill.Ftotal_unfreeze_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, total_fee check fail!", __FILE__, __LINE__);
    }

    if (m_freezeBill.Fstate != FUND_FREEZE_OK && m_freezeBill.Fstate != FUND_FREEZE_UNFREEZEED)
    {
        gPtrAppLog->error("freezeData state invalid =%d",m_freezeBill.Fstate);
        throw CException(ERR_INVALID_STATE, "freezeData record state invalid. ", __FILE__, __LINE__);
    }
}

void FundBalanceUnFreeze::RecordBalanceUnFreeze()  throw (CException)
{
    if (m_unfreezeData.Funfreeze_id[0] != 0) //已经存在
    {
        return;
    }
    strncpy(m_unfreezeData.Fspid,m_params["spid"],sizeof(m_unfreezeData.Fspid)-1);
    strncpy(m_unfreezeData.Fcoding,m_params["sp_billno"],sizeof(m_unfreezeData.Fcoding)-1);
    strncpy(m_unfreezeData.Funfreeze_id,GenerateIdsBySpid(m_params["fund_spid"]).c_str(),sizeof(m_unfreezeData.Funfreeze_id)-1);
    strncpy(m_unfreezeData.Ffreeze_id,m_params["freeze_id"],sizeof(m_unfreezeData.Ffreeze_id)-1);
    strncpy(m_unfreezeData.Ffund_spid,m_params["fund_spid"],sizeof(m_unfreezeData.Ffund_spid)-1);
    strncpy(m_unfreezeData.Ffund_code,m_params["fund_code"],sizeof(m_unfreezeData.Ffund_code)-1);
    strncpy(m_unfreezeData.Fqqid,m_params["uin"],sizeof(m_unfreezeData.Fqqid)-1);
    m_unfreezeData.Funfreeze_type = m_params.getInt("unfreeze_type"); 
    if (m_unfreezeData.Funfreeze_type == UNFREEZE_FOR_FETCH)
    {
        if ((m_params.getString("spid")== gPtrConfig->m_AppCfg.wx_wfj_spid))
        {
            memset(m_unfreezeData.Fsub_acc_control_no,0,sizeof(m_unfreezeData.Fsub_acc_control_no));
        }
        else
        {
            strncpy(m_unfreezeData.Fsub_acc_control_no,m_unfreezeData.Funfreeze_id,sizeof(m_unfreezeData.Fsub_acc_control_no)-1);
        }
        //strncpy(m_unfreezeData.Fpay_trans_id,GenerateIdsBySpid(m_params["spid"]).c_str(),sizeof(m_unfreezeData.Fpay_trans_id)-1);
    }
    m_unfreezeData.Fstate = CREATE_INIT;
    m_unfreezeData.Flstate = LSTATE_VALID;
    m_unfreezeData.Ftotal_fee = m_params.getLong("total_fee");
    m_unfreezeData.Fcontrol_fee= m_params.getLong("control_fee");
    m_unfreezeData.Fcur_type = querySubaccCurtype(m_pFundCon, m_params.getString("fund_spid"));
    strncpy(m_unfreezeData.Fchannel_id,m_freezeBill.Fchannel_id,sizeof(m_unfreezeData.Fchannel_id)-1);
    strncpy(m_unfreezeData.Fsub_acc_unfreeze_no,m_unfreezeData.Funfreeze_id,sizeof(m_unfreezeData.Fsub_acc_unfreeze_no)-1);
    strncpy(m_unfreezeData.Fcreate_time,m_params["systime"],sizeof(m_unfreezeData.Fcreate_time)-1);
    strncpy(m_unfreezeData.Fmodify_time,m_params["systime"],sizeof(m_unfreezeData.Fmodify_time)-1);
    strncpy(m_unfreezeData.Facc_time,m_params["systime"],sizeof(m_unfreezeData.Facc_time)-1);
    m_unfreezeData.Fuid = m_fund_bind.Fuid;
    strncpy(m_unfreezeData.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(m_unfreezeData.Ftrade_id)-1);
    strncpy(m_unfreezeData.Fmemo,m_params["desc"],sizeof(m_unfreezeData.Fmemo)-1);
    strncpy(m_unfreezeData.Fstandby3,m_params["sub_spid"],sizeof(m_unfreezeData.Fstandby3)-1);
    
    insertFundUnFreeze(m_pFundCon, m_unfreezeData);
}

bool FundBalanceUnFreeze::unfreezeSubAccBalance()  throw (CException)
{

    try
    {
        SubaccUnFreeze(gPtrSubaccRpc, m_unfreezeData.Ffund_spid, m_params["uin"], m_params["client_ip"], m_unfreezeData.Fsub_acc_unfreeze_no
                            ,m_freezeBill.Fsub_acc_freeze_no, m_unfreezeData.Ftotal_fee, m_unfreezeData.Facc_time,m_unfreezeData.Fsub_acc_control_no,m_params.getLong("control_fee"),m_params["desc"]);
    }
    catch(CException &e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        m_subAccErrCode = e.error();
        m_sunAccErrInfo = e.what();
        return false;
    }

    return true;
}



//更新解冻单状态
void FundBalanceUnFreeze::updateBalanceUnFreeze()  throw (CException)
{
    ST_UNFREEZE_FUND unfreezeBill;
    memset(&unfreezeBill,0,sizeof(unfreezeBill));
    strncpy(unfreezeBill.Funfreeze_id,m_unfreezeData.Funfreeze_id,sizeof(unfreezeBill.Funfreeze_id)-1);
    unfreezeBill.Fstate = FUND_UNFREEZE_OK;
    strncpy(unfreezeBill.Fmodify_time,m_params["systime"],sizeof(unfreezeBill.Fmodify_time)-1);
    strncpy(unfreezeBill.Facc_time,m_params["systime"],sizeof(unfreezeBill.Facc_time)-1);
    updateFundUnFreeze(m_pFundCon,unfreezeBill,m_unfreezeData);
}

void FundBalanceUnFreeze::updateBalanceFreeze()  throw (CException)
{
    
    ST_FREEZE_FUND freezeBill;
    memset(&freezeBill,0,sizeof(freezeBill));
    strncpy(freezeBill.Ffreeze_id,m_freezeBill.Ffreeze_id,sizeof(m_freezeBill.Ffreeze_id)-1);
    freezeBill.Fstate = FUND_FREEZE_UNFREEZEED;
    freezeBill.Ftotal_unfreeze_fee += m_params.getLong("total_fee");
    strncpy(freezeBill.Fmodify_time,m_params["systime"],sizeof(freezeBill.Fmodify_time)-1);
    updateFundFreeze(m_pFundCon,freezeBill,m_freezeBill);
}

void FundBalanceUnFreeze::preRecordUnfreezeBill()throw (CException)
{
    // 开启事务 
    m_pFundCon->Begin();
         
    //重入检查
    CheckBalanceUnFreeze(false);

    //校验冻结单
    CheckFreezeRecord();
        
    //创建解冻单
    RecordBalanceUnFreeze();
        
    m_pFundCon->Commit();
}


void FundBalanceUnFreeze::CheckFundBind()  throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
}
 
void FundBalanceUnFreeze::excute()  throw (CException)
{
    try
    {
        CheckParams();

        CheckFundBind();

        //先创建冻结单,由于解冻接口逻辑比较简单，为了方便外部调用
        //没有把创建冻结单独成一个接口，而是合并到了冻结确认接口中
        preRecordUnfreezeBill();
        
         // 开启事务 
        m_pFundCon->Begin();

        //重入检查
        CheckBalanceUnFreeze(true);

        if (m_params.getString("spid")!= gPtrConfig->m_AppCfg.wx_wfj_spid) //非微信王府井消费扣款才有子账户操作
        {
            //校验冻结单
            CheckFreezeRecord();

            //解冻子账户余额
            m_bFreezeSunaccSuc = unfreezeSubAccBalance();
            if (true == m_bFreezeSunaccSuc)
            {
                //更新解冻单状态
                updateBalanceUnFreeze();

                //更新冻结单
                updateBalanceFreeze();
            }
        }
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

    if (false == m_bFreezeSunaccSuc)
    {
        throw  CException(m_subAccErrCode,m_sunAccErrInfo.c_str());
    }
    
}
 
void FundBalanceUnFreeze::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    
    if (m_unfreezeData.Funfreeze_type == UNFREEZE_FOR_FETCH)
    {
        CUrlAnalyze::setParam(rqst->odata, "unfreeze_id", m_unfreezeData.Funfreeze_id);
        CUrlAnalyze::setParam(rqst->odata, "redem_id", m_unfreezeData.Fredem_id);
    }

    rqst->olen = strlen(rqst->odata);
    return;
}

