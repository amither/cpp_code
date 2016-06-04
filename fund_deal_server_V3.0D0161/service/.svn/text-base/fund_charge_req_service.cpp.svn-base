/**
  * FileName: fund_charge_req_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-07-23
  * Description: 理财通余额充值请求
  */

#include "fund_commfunc.h"
#include "fund_charge_req_service.h"

FundChargeReq::FundChargeReq(CMySQL* mysql)
{
    m_pFundCon = mysql;
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_fundBaCfg,0,sizeof(m_fundBaCfg));
}

void FundChargeReq::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};

    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_charge_req_service] receives: %s", szMsg);

    // 读取参数
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "channel_id", 0, 64);
    m_params.readIntParam(szMsg, "inter_channel_id", 1, 3);
    m_params.readIntParam(szMsg, "op_type", 1,MAX_INTEGER);
    m_params.readLongParam(szMsg, "total_fee", 1,MAX_LONG);
    m_params.readIntParam(szMsg, "cur_type", 1,1);
    m_params.readStrParam(szMsg, "spid", 0, 15);
    m_params.readStrParam(szMsg, "cft_trans_id", 0, 32);
    m_params.readStrParam(szMsg, "cft_transfer_id", 18, 31);
    m_params.readStrParam(szMsg, "redem_id", 0, 32);
    m_params.readStrParam(szMsg, "fetch_id", 0, 32);
    m_params.readStrParam(szMsg, "control_id", 0, 32);
    m_params.readStrParam(szMsg, "desc", 0, 255);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token

    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

}


/*
 * 检验token
 */
void FundChargeReq::CheckToken() throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
    //uin|op_type|total_fee|cft_trans_id|cft_transfer_id|key
    // 规则生成原串
    ss << m_params["uin"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["cft_trans_id"] << "|" ;
    ss << m_params["cft_transfer_id"] << "|" ;
    ss << gPtrConfig->m_AppCfg.charge_service_req_key;

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
void FundChargeReq::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();
    
    m_fundBaCfg.Fid = DEFAULT_BALNCE_CONFIG_FID;
    if (false == queryFundBalanceConfig(m_pFundCon,m_fundBaCfg,false))
    {
        throw CException(ERR_FUND_QUERY_BA_CONFIG, "query balance config fail", __FILE__, __LINE__);
    }
    
    if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_PAY)
    {
        if (checkTransIdAndSpid(m_params["spid"],m_params["cft_trans_id"]) == false)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  cft_trans_id error "); 
        } 
        if (checkTransIdAndSpid(m_params["spid"],m_params["cft_transfer_id"]) == false)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input cft_transfer_id check with  spid error "); 
        }
        if (m_params.getString("spid") != m_fundBaCfg.Fcharge_spid)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  Fcharge_spid error "); 
        }
        m_params.setParam("listid", m_params["cft_trans_id"]);
        
        if (m_params.getInt("inter_channel_id") != BA_CHARGE_CHANNEL_PC 
            && m_params.getInt("inter_channel_id") != BA_CHARGE_CHANNEL_WX
            && m_params.getInt("inter_channel_id") != BA_CHARGE_CHANNEL_QQ)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input inter_channel_id  invalid "); 
        }
    }
    else if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_REDEM_T1 
         || m_params.getInt("op_type") ==OP_TYPE_CHAEGE_REDEM_T0)
    {
        if (checkTransIdAndSpid(m_params["spid"],m_params["redem_id"]) == false)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input spid check with  redem_id error "); 
        }
        m_params.setParam("listid", m_params["redem_id"]);
    }
    else if (m_params.getInt("op_type") == OP_TYPE_CHAEGE_FETCH_FAIL)
    {
        if (m_params.getString("fetch_id").length()<18)
        {
            throw EXCEPTION(ERR_BAD_PARAM, "input fetch_id check fail"); 
        }
        m_params.setParam("listid", m_params["fetch_id"]);
    }
    else
    {
        throw EXCEPTION(ERR_BAD_PARAM, "input op_type check fail"); 
    }

    
}

/*
 * 查询基金账户是否存在
 */
void FundChargeReq::CheckFundBind() throw (CException)
{
    if (!QueryFundBindByUin(m_pFundCon, m_params.getString("uin"), &m_fund_bind, false))
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

    if (m_fund_bind.Fuid == 0) //余额充值必须已经开通理财通成功uid不能为0
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record Fuid=0! ", __FILE__, __LINE__);
    }
}

void FundChargeReq::CheckChargeOrder()throw (CException)
{
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
    data.Ftype = m_params.getInt("op_type");
    if (false == queryFundBalanceOrder(m_pFundCon, data,  true))
    {
        return;
    }

    // 检查关键参数

    if (m_params.getInt("op_type") != data.Ftype)
    {
        gPtrAppLog->error("charge data exists, op_type is different! op_type in db[%d], op_type input[%d]", 
			data.Ftype, m_params.getInt("op_type"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, op_type is different!", __FILE__, __LINE__);
    }
    
    if (m_params.getString("uin") != data.Fuin)
    {
        gPtrAppLog->error("charge data exists, uin is different! uin in db[%s], uin input[%s]", 
			data.Fuin, m_params["uin"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, uin is different!", __FILE__, __LINE__);
    }

    if (m_params.getLong("total_fee") != data.Ftotal_fee)
    {
        gPtrAppLog->error("charge data exists, total_fee is different! total_fee in db[%zd], total_fee input[%zd]", 
			data.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, total_fee is different!", __FILE__, __LINE__);
    }

    if (m_params.getString("listid") != data.Flistid)
    {
        gPtrAppLog->error("charge data exists, listid is different! listid in db[%s], listid input[%s]", 
			data.Flistid, m_params["listid"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, listid is different!", __FILE__, __LINE__);
    }

    if (m_params.getString("cft_transfer_id") != data.Ftotal_acc_trans_id)
    {
        gPtrAppLog->error("charge data exists, cft_transfer_id is different! cft_transfer_id in db[%s], cft_transfer_id input[%s]", 
			data.Ftotal_acc_trans_id, m_params["cft_transfer_id"]);
        throw CException(ERR_REPEAT_ENTRY_DIFF, "charge data exists, cft_transfer_id is different!", __FILE__, __LINE__);
    }

    if (data.Fstate != FUND_CHARGE_INIT)
    {
        gPtrAppLog->error("harge record state =%d invalid",data.Fstate);
        throw CException(ERR_INVALID_STATE, "charge record state invalid. ", __FILE__, __LINE__);
    }
    
    throw CException(ERR_REPEAT_ENTRY, "charge data exists!", __FILE__, __LINE__);
}
        
void FundChargeReq::RecordChargeOrder()throw (CException)
{
    ST_BALANCE_ORDER data;
    memset(&data,0,sizeof(ST_BALANCE_ORDER));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Fuin,m_params["uin"],sizeof(data.Fuin)-1);
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
    data.Ftotal_fee= m_params.getInt("total_fee");

    strncpy(data.Fchannel_id,m_params["channel_id"],sizeof(data.Fchannel_id)-1);
    data.Fcur_type = m_params.getInt("cur_type");
    strncpy(data.Fspid,m_params["spid"],sizeof(data.Fspid)-1);
    strncpy(data.Ftotal_acc_trans_id,m_params["cft_transfer_id"],sizeof(data.Ftotal_acc_trans_id)-1);
    strncpy(data.Fmemo,m_params["desc"],sizeof(data.Fmemo)-1);
    strncpy(data.Fcontrol_id,m_params["control_id"],sizeof(data.Fcontrol_id)-1);
    strncpy(data.Fcreate_time,m_params["systime"],sizeof(data.Fcreate_time)-1);
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);
    data.Fstandby1 = m_params.getInt("inter_channel_id");
    

    //余额总账户内部商户号做为前缀
    m_params.setParam("subacc_charge_id", GenerateIdsBySpid(m_fundBaCfg.Fbalance_spid));
    
    strncpy(data.Fsubacc_trans_id,m_params["subacc_charge_id"],sizeof(data.Fsubacc_trans_id)-1);
    data.Fstate = FUND_CHARGE_INIT;
    data.Fuid = m_fund_bind.Fuid;
    
    insertFundBalanceOrder(m_pFundCon,data);
}
        
void FundChargeReq::RecordRelationOrder()throw (CException)
{
    ST_ORDER_USER_RELA data;
    memset(&data,0,sizeof(ST_ORDER_USER_RELA));
    strncpy(data.Flistid,m_params["listid"],sizeof(data.Flistid)-1);
    data.Ftype = m_params.getInt("op_type");
    strncpy(data.Ftrade_id,m_fund_bind.Ftrade_id,sizeof(data.Ftrade_id)-1);
    strncpy(data.Fcreate_time,m_params["systime"],sizeof(data.Fcreate_time)-1);
    strncpy(data.Fmodify_time,m_params["systime"],sizeof(data.Fmodify_time)-1);
    
    insertOrderUserRelation(m_pFundCon,data);

    strncpy(data.Flistid,m_params["cft_transfer_id"],sizeof(data.Flistid)-1);
    insertOrderUserRelation(m_pFundCon,data);

    if (m_params.getString("subacc_charge_id") != m_params["cft_transfer_id"] && m_params.getString("subacc_charge_id") != m_params["listid"])
    {
        strncpy(data.Flistid,m_params["subacc_charge_id"],sizeof(data.Flistid)-1);
        insertOrderUserRelation(m_pFundCon,data);
    }
}

void FundChargeReq::checkUserTotalAsset()throw (CException)
{
    //只有微信充值和pc充值才验证总资产
    if (m_params.getInt("op_type")!= OP_TYPE_CHAEGE_PAY)
    {
        return;
    }
    
    LONG currentTotalAsset = queryUserTotalAsset(m_fund_bind.Fuid,m_fund_bind.Ftrade_id);

    //如果用户当前级别为0，那么如果单笔充值大于指定配置值需要按照等级1验证总资产限额
    int asset_limit_level = m_fund_bind.Fasset_limit_lev;
    if (asset_limit_level == 0 && m_params.getLong("total_fee") >= gPtrConfig->m_AppCfg.assert_limit_level1_chargefee)
    {
        asset_limit_level = 1;
    }
    
    if (true == isUserAssetOverLimit(asset_limit_level,currentTotalAsset, m_params.getLong("total_fee")))
    {
        TRACE_ERROR("user total Asset Over Limit!");
        throw CException(ERR_FUND_ASSET_OVER_LIMIT, "user total Asset Over Limit! ", __FILE__, __LINE__);
    }
}

void FundChargeReq::excute()  throw (CException)
{
    //参数检查
    CheckParams();

    // 检查基金账户记录 
    CheckFundBind();

    //检查用户总资产
    checkUserTotalAsset();

    try
    {
        m_pFundCon->Begin();

        //重入检查
        CheckChargeOrder();

        //记录充值请求单
        RecordChargeOrder();

        //记录单号和tradeid对应关系
        RecordRelationOrder();
        
        m_pFundCon->Commit();
    }
    catch(CException &e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        m_pFundCon->Rollback();

        if (ERR_REPEAT_ENTRY != (unsigned)e.error())
        {
            throw;
        }
    }
}

void FundChargeReq::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    rqst->olen = strlen(rqst->odata);
    return;
}

