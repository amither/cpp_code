/**
  * FileName: fund_transfer_ack_service.cpp
  * Author: louisjiang
  * Version :1.0
  * Date: 2014-05-16
  * Description: 份额转换确认接口
  */


#include "fund_commfunc.h"
#include "fund_transfer_ack_service.h"

FundTransferAck::FundTransferAck(CMySQL* mysql)
{
    m_pFundCon = mysql;

    memset(&m_transferOrder, 0, sizeof(m_transferOrder));
    memset(&m_fund_bind, 0, sizeof(ST_FUND_BIND));
    memset(&m_buyOrder, 0, sizeof(m_buyOrder));
    memset(&m_redemOrder, 0, sizeof(m_redemOrder));
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    memset(&m_fund_newsp_config, 0, sizeof(FundSpConfig));
    memset(&m_fund_bind_orisp_acc, 0, sizeof(FundBindSp));
    memset(&m_fund_bind_newsp_acc, 0, sizeof(FundBindSp));
    memset(&m_dynamic_info,0,sizeof(m_dynamic_info));
	memset(&m_fundCloseTrans, 0, sizeof(FundCloseTrans));
	
	m_close_fund_seqno = 0;
    m_request = NULL;
}

/**
  * service step 1: 解析输入参数
  */
void FundTransferAck::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char szMsg[MAX_MSG_LEN] = {0};
    char szSpId[MAX_SPID_LEN] = {0};
    char szTimeNow[MAX_TIME_LENGTH+1] = {0};
    TRACE_DEBUG("[fund_transfer_ack_service] parseInputMsg start: ");

    // 要保留请求数据，抛差错使用
    m_request = rqst;
    // 解密原始消息
    getDecodeMsg(rqst, szMsg, szSpId);
    
    TRACE_DEBUG("[fund_transfer_ack_service] receives: %s", szMsg);

    // 读取参数
    m_params.readIntParam(szMsg, "op_type", 1,5);
    m_params.readStrParam(szMsg, "trade_id", 10,32);
    m_params.readStrParam(szMsg, "uin", 1, 64);
    m_params.readStrParam(szMsg, "fund_exchange_id", 10, 32); //转换单号
    m_params.readStrParam(szMsg, "ori_spid", 10, 15);
    m_params.readStrParam(szMsg, "new_spid", 10, 15);
    m_params.readStrParam(szMsg, "ori_fund_code", 1, 64);
    m_params.readStrParam(szMsg, "new_fund_code", 1, 64);
    m_params.readLongParam(szMsg, "total_fee", 1, MAX_LONG);
    m_params.readStrParam(szMsg, "client_ip", 1, 16);
    m_params.readStrParam(szMsg, "token", 1, 32);   // 接口token
    m_params.readStrParam(szMsg, "memo", 0, 128);
    m_params.readStrParam(szMsg, "sp_billno_redem", 0,32);
    m_params.readStrParam(szMsg, "sp_billno_buy", 0,32);
    m_params.readStrParam(szMsg, "redem_id", 0,32);
    m_params.readStrParam(szMsg, "buy_id", 0,32);
    m_params.readStrParam(szMsg, "redem_result_sign", 0,32);

	m_params.readStrParam(szMsg, "close_end_day", 0, 8);	
	m_params.readIntParam(szMsg, "user_end_type", 0, 3);
	m_params.readIntParam(szMsg, "end_sell_type", 0, 3);
	m_params.readLongParam(szMsg,"end_plan_amt",0, MAX_LONG);
	m_params.readStrParam(szMsg, "end_transfer_spid", 0, 15);
	m_params.readStrParam(szMsg, "end_transfer_fundcode", 0, 64);

    m_optype = m_params.getInt("op_type");
    GetTimeNow(szTimeNow);
    m_params.setParam("systime", szTimeNow);

    if (m_optype == FUND_TRANSFER_REDEM_TIMEOUT
        && 	("true" == gPtrConfig->m_AppCfg.redem_timeout_conf))//根据配置决定赎回超时当成功还是失败，如果当成功处理，需要开启补单批跑程序对超时赎回单进行补单
    {
        m_optype = FUND_TRANSFER_REDEM_SUC;
    }

}

/*
 * 生成基金注册用token
 */
string FundTransferAck::GenFundToken()
{
    stringstream ss;
    char buff[128] = {0};
    
    // 按照trade_id|uin|fund_exchange_id|ori_spid|new_spid|total_fee|sp_billno_redem|sp_billno_buy|redem_result_sign|op_type|key
    // 规则生成原串
    ss << m_params["trade_id"] << "|" ;
    ss << m_params["uin"] << "|" ;
    ss << m_params["fund_exchange_id"] << "|" ;
    ss << m_params["ori_spid"] << "|" ;
    ss << m_params["new_spid"] << "|" ;
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["sp_billno_redem"] << "|" ;
    ss << m_params["sp_billno_buy"] << "|" ;
    ss << m_params["redem_result_sign"] << "|" ;
    ss << m_params["op_type"] << "|" ;
    ss << gPtrConfig->m_AppCfg.transfer_ackkey;
    //TRACE_DEBUG("FundTransferAck, sourceStr=[%s]", ss.str().c_str());

    getMd5(ss.str().c_str(), ss.str().size(), buff);

    return buff;
}

//检查前置机赎回结果签名
void FundTransferAck::checkResultSign()throw (CException)
{
    stringstream ss;
    char buff[128] = {0};
       // 规则生成原串
    ss << m_params["redem_id"] << "|" ;
    ss << m_params["ori_spid"] << "|" ;
    ss << m_params["sp_billno_redem"] << "|" ; 
    ss << m_params["total_fee"] << "|" ;
    ss << m_params["ori_fund_code"] << "|" ;
    ss << m_params["trade_id"] << "|" ;
    ss << gPtrConfig->m_AppCfg.transfer_redem_result_key;
    
    //TRACE_DEBUG("checkResultSign, sourceStr=[%s]", ss.str().c_str());
    
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    if (StrUpper(m_params.getString("redem_result_sign")) != StrUpper(buff))
    {   
        TRACE_DEBUG("fund authen token check failed, input=%s", 
	                m_params.getString("redem_result_sign").c_str());
        throw EXCEPTION(ERR_BAD_PARAM, "input redem_result_sign error");    
    } 
}

/*
 * 检验token
 */
void FundTransferAck::CheckToken() throw (CException)
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
void FundTransferAck::CheckParams() throw (CException)
{
    // 验证token
    CheckToken();

    if(OP_TYPE_TRANSFER_REDEM_SUC == m_params.getInt("op_type"))
    { 
        CHECK_PARAM_EMPTY("sp_billno_redem"); 
        checkResultSign();

		if(CLOSE_FUND_SELL_TYPE_ANOTHER_FUND == m_params.getInt("end_sell_type"))
		{
			CHECK_PARAM_EMPTY("end_transfer_spid"); 
			CHECK_PARAM_EMPTY("end_transfer_fundcode"); 
		}

		if(CLOSE_FUND_END_TYPE_PATRIAL_REDEM == m_params.getInt("user_end_type") && 0 == m_params.getLong("end_plan_amt"))
		{
			throw EXCEPTION(ERR_BAD_PARAM, "user_end_type not found, or empty"); 
		}
    }
    else if (OP_TYPE_TRANSFER_BUY_REQ == m_optype)
    {
        CHECK_PARAM_EMPTY("sp_billno_buy"); 
    }

    if (OP_TYPE_TRANSFER_REDEM_SUC == m_optype)
    {
        checkResultSign();
    }
}

void FundTransferAck::queryFundBindNewSpInfo()throw (CException)
{
    strncpy(m_fund_bind_newsp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_newsp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_newsp_acc.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_bind_newsp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_newsp_acc, false);    
}

/**
  * 检查基金交易记录是否已经生成
  */
void FundTransferAck::CheckFundTransfer() throw (CException)
{
    strncpy(m_transferOrder.Fchange_id,m_params["fund_exchange_id"],sizeof(m_transferOrder.Fchange_id)-1);

    if (false == queryFundTransfer(m_pFundCon, m_transferOrder, true))
    {
        gPtrAppLog->error("transfer record not exist, listid[%s]  ", m_params.getString("fund_exchange_id").c_str());
        throw CException(ERR_BUYPAY_NOLIST, "transfer record not exist! ", __FILE__, __LINE__);
    }

    // 检查关键参数
    if( (0 != strcmp(m_transferOrder.Fori_spid, m_params.getString("ori_spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! ori_spid in db[%s], ori_spid input[%s]", 
			m_transferOrder.Fori_spid, m_params.getString("ori_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, ori_spid is different!", __FILE__, __LINE__);
    }

    if( (0 != strcmp(m_transferOrder.Fnew_spid, m_params.getString("new_spid").c_str())))
    {
        gPtrAppLog->error("fund trade exists, spid is different! new_spid in db[%s], new_spid input[%s]", 
			m_transferOrder.Fnew_spid, m_params.getString("new_spid").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_spid is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Ftrade_id, m_params.getString("trade_id").c_str()))
    {
        gPtrAppLog->error("fund trade exists, trade_id is different! trade_id in db[%s], trade_id input[%s] ", 
			m_transferOrder.Ftrade_id, m_params.getString("trade_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, trade_id is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Fori_fund_code, m_params.getString("ori_fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! ori_fund_code in db[%s], ori_fund_code input[%s] ", 
			m_transferOrder.Fori_fund_code, m_params.getString("ori_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, ori_fund_code is different!", __FILE__, __LINE__);
    }

    if(0 != strcmp(m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str()))
    {
        gPtrAppLog->error("fund trade exists, fund_code is different! new_fund_code in db[%s], new_fund_code input[%s] ", 
			m_transferOrder.Fnew_fund_code, m_params.getString("new_fund_code").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, new_fund_code is different!", __FILE__, __LINE__);
    }

    if(m_transferOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("fund trade exists, total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_transferOrder.Ftotal_fee, m_params.getLong("total_fee"));
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, total_fee is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("buy_id").empty() && m_params.getString("buy_id") != m_transferOrder.Fbuy_id)
    {
        gPtrAppLog->error("fund trade exists, Fbuy_id is different! buy_id in db[%s], Fbuy_id input[%s] ", 
			m_transferOrder.Fbuy_id, m_params.getString("Fbuy_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, Fbuy_id is different!", __FILE__, __LINE__);
    }

    if(!m_params.getString("redem_id").empty() && m_params.getString("redem_id") != m_transferOrder.Fredem_id)
    {
        gPtrAppLog->error("fund trade exists, redem_id is different! redem_id in db[%s], redem_id input[%s] ", 
			m_transferOrder.Fredem_id, m_params.getString("redem_id").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, redem_id is different!", __FILE__, __LINE__);
    }

    //校验转换状态
    checkTransferState();

}


/**
  * 转换单状态确认
  */
void FundTransferAck::checkTransferState() throw (CException)
{
    if (m_optype != OP_TYPE_TRANSFER_BUY_REQ)
    {
        //检查赎回单状态
        checkRedemTrade();

        //检查申购单状态
        checkBuyTrade();
    }

    if (m_optype == OP_TYPE_TRANSFER_BUY_REQ)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REQ && m_transferOrder.Fstate != FUND_TRANSFER_INIT )
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        if (m_transferOrder.Fstate == FUND_TRANSFER_REQ)
        {
            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter . ", __FILE__, __LINE__);
        }
    }
    else if (m_optype == OP_TYPE_TRANSFER_REDEM_SUC)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REQ && m_transferOrder.Fstate != FUND_TRANSFER_REDEM_SUC 
           && m_transferOrder.Fstate != FUND_TRANSFER_TRANS_SUC)
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        //补单情况:
        if (m_transferOrder.Fstate == FUND_TRANSFER_REDEM_SUC || m_transferOrder.Fstate == FUND_TRANSFER_TRANS_SUC)
        {
			if(m_buyOrder.Fclose_listid > 0&&m_buyOrder.Ftrade_date[0]!=0)
			{
				// 重入，对于定期产品要查询出数据用于返回封闭开始时间
				m_fundCloseTrans.Fid = m_buyOrder.Fclose_listid;
				strncpy(m_fundCloseTrans.Ftrade_id, m_buyOrder.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
				queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);
				//用于参数返回 m_params.setParam("trans_date", m_fundCloseTrans.Ftrans_date);
				m_params.setParam("trans_date", m_buyOrder.Ftrade_date);
				
			}
            //基金公司赎回超时的补单
            if (m_params.getInt("op_type") == OP_TYPE_TRANSFER_REDEM_SUC && m_redemOrder.Fspe_tag==1)
            {
                m_optype =FUND_TRANSFER_REDEM_SPTIMEOUT_REDO_OK ;
                return;
            }
            //子账户加失败的补单
            if ((m_transferOrder.Fsubacc_state&FUND_TRANSFER_SUBACC_SAVE) == 0 
                && (m_buyOrder.Fstate == PAY_OK || m_buyOrder.Fstate == PURCHASE_SUC))
            {
                m_optype =FUND_TRANSFER_SUBACC_SAVE_REDO ;
                return;
            }

            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter . ", __FILE__, __LINE__);
        }
        
    }
    else if (m_optype == OP_TYPE_TRANSFER_REDEM_FAIL)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REQ && m_transferOrder.Fstate != FUND_TRANSFER_REDEM_FAIL )
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        if (m_transferOrder.Fstate == FUND_TRANSFER_REDEM_FAIL)
        {
            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter. ", __FILE__, __LINE__);
        }
    }
    else if (m_optype == OP_TYPE_TRANSFER_BUY_SUC)
    {
        if (m_transferOrder.Fstate != FUND_TRANSFER_REDEM_SUC && m_transferOrder.Fstate != FUND_TRANSFER_TRANS_SUC )
        {
            throw CException(ERR_INVALID_STATE, "fund transfer record state invalid. ", __FILE__, __LINE__);
        }
        if (m_transferOrder.Fstate == FUND_TRANSFER_TRANS_SUC)
        {
            throw CException(ERR_REPEAT_ENTRY, "fund transfer record state reenter. ", __FILE__, __LINE__);
        }
    }
    else
    {
        throw CException(ERR_BAD_PARAM, "optype invalid. ", __FILE__, __LINE__);
    }

}

/**
  * 执行申购请求
  */
void FundTransferAck::excute() throw (CException)
{
    try
    {
        CheckParams();

        CheckFundBind();

         // 开启事务 
        m_pFundCon->Begin();

        // 查询并校验转换单 
        CheckFundTransfer();

        //根据op_type更新转换单状态和子账户操作
        dealTransfer();

        // 提交事务 
        m_pFundCon->Commit();

        // 更新各类ckv ,放在事务之后是避免事务回滚却写入ckv的问题
        updateCkvs();
		
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


/*
 * 查询基金账户是否存在
 */
void FundTransferAck::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_pFundCon, m_params["uin"], &m_fund_bind, true))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);
    }
    if (m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        throw CException(ERR_FUNDBIND_NOTREG, "check fund bind record trade_id fail! ", __FILE__, __LINE__);
    }
}

/**
*检查用户余额
*/
void FundTransferAck::checkUserBalance() throw (CException)
{
    //TODO 冻结金额是哪个字段,子账户只用来记账，暂时不存在冻结部分
    LONG balance = querySubaccBalance(m_fund_bind.Fuid,querySubaccCurtype(m_pFundCon, m_params.getString("ori_spid")));

    if(balance < m_params.getLong("total_fee"))
    {
        throw CException(ERR_CORE_USER_BALANCE, "not enough money", __FILE__, __LINE__);
    }
}

/**
*记录申购单
*/
void FundTransferAck::RecordFundBuy() throw (CException)
{
    ST_TRADE_FUND &stRecord = m_buyOrder;
    strncpy(stRecord.Flistid, m_transferOrder.Fbuy_id, sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_transferOrder.Fnew_spid, sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno_buy").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_fund_bind.Fuid;
    strncpy(stRecord.Ffund_name, m_fund_newsp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_transferOrder.Fnew_fund_code, sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    stRecord.Fpur_type = PURTYPE_TRANSFER_PURCHASE;
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = PAY_INIT;
    stRecord.Flstate = LSTATE_VALID;

    string trade_date;
    string fund_vdate;
	string end_date;
	if(m_fund_newsp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		getTradeDate(m_pFundCon,m_params.getString("systime"), trade_date,fund_vdate);
	}
	else
	{
		// 各类日期计算
		FundCloseCycle fundCloseCycle;
		memset(&fundCloseCycle, 0, sizeof(FundCloseCycle));

		strncpy(fundCloseCycle.Fdate, calculateFundDate(m_params.getString("systime")).c_str(), sizeof(fundCloseCycle.Fdate) - 1);
		strncpy(fundCloseCycle.Ffund_code, m_transferOrder.Fnew_fund_code, sizeof(fundCloseCycle.Ffund_code) - 1);
		queryFundCloseCycle(m_pFundCon, fundCloseCycle, false);
		trade_date = fundCloseCycle.Ftrans_date;
		fund_vdate = fundCloseCycle.Ffirst_profit_date;
		end_date = fundCloseCycle.Fdue_date;
	}
	if(	trade_date.empty()){
		//无配置日期
		gPtrAppLog->error("trade_date unfound[%s], systime[%s]", stRecord.Flistid, m_params.getString("systime").c_str());
		alert(ERR_UNFOUND_TRADE_DATE,"trade_date unfound! ");
		throw CException(ERR_UNFOUND_TRADE_DATE, "trade_date unfound! ", __FILE__, __LINE__);
	}
    
    strncpy(stRecord.Ftrade_date,trade_date.c_str(), sizeof(stRecord.Ftrade_date) - 1);//交易日
    strncpy(stRecord.Fend_date,end_date.c_str(), sizeof(stRecord.Fend_date) - 1);//定期交易截至日
    strncpy(stRecord.Ffund_vdate, fund_vdate.c_str(), sizeof(stRecord.Ffund_vdate) - 1);//基金净值日期,该笔申购首次产生收益的日期
    strncpy(stRecord.Frela_listid, m_transferOrder.Fchange_id, sizeof(stRecord.Frela_listid)-1);
    strncpy(stRecord.Ffetchid, "buy_no_fetchid", sizeof(stRecord.Ffetchid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("new_spid")); // 币种类型
    strncpy(stRecord.Fsub_trans_id, m_transferOrder.Fbuy_id, sizeof(stRecord.Fsub_trans_id)-1);

    //份额转换的记录申购用途
    stRecord.Fpurpose= PURPOSE_CHANGE_SP;
    
    strncpy(stRecord.Fchannel_id, m_transferOrder.Fchannel_id, sizeof(stRecord.Fchannel_id)-1);
	
    InsertTradeFund(m_pFundCon, &stRecord);
    if(m_fund_bind.Fuid >= 10000)
    {
        InsertTradeUserFund(m_pFundCon, &stRecord);
    }
}

/**
*更新转换单状态
*/
void FundTransferAck::UpdateTransferState(int state,int sub_acc_state,const string&acc_time)throw (CException)
{
    ST_TRANSFER_FUND transOrder;
    memset(&transOrder,0,sizeof(transOrder));
    strncpy(transOrder.Fchange_id,m_transferOrder.Fchange_id,sizeof(transOrder.Fchange_id)-1);
    strncpy(transOrder.Fmodify_time,m_params["systime"],sizeof(transOrder.Fmodify_time)-1);
    strncpy(transOrder.Fmemo,m_params["memo"],sizeof(transOrder.Fmemo)-1);
    if (state)
    {
        transOrder.Fstate = state;
    }
    if (sub_acc_state)
    {
        transOrder.Fsubacc_state = sub_acc_state;
    }
    if (!acc_time.empty())
    {
        strncpy(transOrder.Facc_time,acc_time.c_str(),sizeof(transOrder.Facc_time)-1);
        strncpy(m_transferOrder.Facc_time,acc_time.c_str(),sizeof(m_transferOrder.Facc_time)-1);
    }
    
    if(OP_TYPE_TRANSFER_REDEM_TIMEOUT == m_params.getInt("op_type"))
    {
        transOrder.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
    }
    else if (OP_TYPE_TRANSFER_BUY_SUC == m_params.getInt("op_type"))
    {
        transOrder.Fspe_tag=m_transferOrder.Fspe_tag;
    }
    
    updateFundTransfer(m_pFundCon,transOrder);    
}

/**
*记录赎回单
*/
void FundTransferAck::RecordFundRedem() throw (CException)
{
    ST_TRADE_FUND  &stRecord = m_redemOrder;

    string drawid = string(m_transferOrder.Fredem_id).substr(10,18);

    strncpy(stRecord.Flistid, m_transferOrder.Fredem_id, sizeof(stRecord.Flistid)-1);
    strncpy(stRecord.Fspid, m_params.getString("ori_spid").c_str(), sizeof(stRecord.Fspid)-1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno_redem").c_str(), sizeof(stRecord.Fcoding)-1);
    strncpy(stRecord.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(stRecord.Ftrade_id)-1);
    stRecord.Fuid = m_fund_bind.Fuid;
    strncpy(stRecord.Ffund_name, m_fund_orisp_config.Ffund_name, sizeof(stRecord.Ffund_name)-1);
    strncpy(stRecord.Ffund_code, m_params.getString("ori_fund_code").c_str(), sizeof(stRecord.Ffund_code)-1);
    stRecord.Fbank_type = m_params.getInt("bank_type");
    stRecord.Fpur_type = PURTYPE_TRANSFER_REDEEM;
    stRecord.Ftotal_fee = m_params.getLong("total_fee");
    stRecord.Fstate = REDEM_ININ;
    stRecord.Flstate = LSTATE_VALID;
    strncpy(stRecord.Frela_listid, m_transferOrder.Fchange_id, sizeof(stRecord.Frela_listid)-1);
    stRecord.Fcft_timestamp = toUnixTime(m_params.getString("systime").c_str());
    strncpy(stRecord.Fcreate_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fcreate_time)-1);
    strncpy(stRecord.Fmodify_time, m_params.getString("systime").c_str(), sizeof(stRecord.Fmodify_time)-1);
    stRecord.Fcur_type= querySubaccCurtype(gPtrFundDB, m_params.getString("ori_spid"));; // 币种类型
    strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
    strncpy(stRecord.Fsub_trans_id, drawid.c_str(), sizeof(stRecord.Fsub_trans_id)-1);
    stRecord.Fpurpose = PURPOSE_CHANGE_SP;
    strncpy(stRecord.Fchannel_id, m_transferOrder.Fchannel_id, sizeof(stRecord.Fchannel_id)-1);

    stRecord.Floading_type = DRAW_NOT_USE_LOADING;

    InsertTradeFund(m_pFundCon, &stRecord);
    InsertTradeUserFund(m_pFundCon, &stRecord);

}

//检查赎回单状态
void FundTransferAck::checkRedemTrade() throw (CException)
{
    // 没有交易记录，报错
    if(!QueryTradeFund(m_pFundCon, m_transferOrder.Fredem_id, PURTYPE_TRANSFER_REDEEM,&m_redemOrder, true))
    {
        gPtrAppLog->error("redem record not exist, redem_id[%s]  ", m_transferOrder.Fredem_id);
        throw CException(ERR_BUYPAY_NOLIST, "redem record not exist! ", __FILE__, __LINE__);
    }

    // 物理状态无效，报错
    if(LSTATE_INVALID == m_redemOrder.Flstate)
    {
        gPtrAppLog->error("fund redem , lstate is invalid. listid[%s], uid[%d] ", m_redemOrder.Flistid, m_redemOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  lstate is invalid. ", __FILE__, __LINE__);
    }

    // 校验关键参数
    if(m_redemOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("redem total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_redemOrder.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "redem total_fee is different!", __FILE__, __LINE__);
    }

    if( m_redemOrder.Fuid!= m_fund_bind.Fuid)
    {
        TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_redemOrder.Fuid, m_fund_bind.Fuid);
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with fundbind");
    }

    if (0 != strcmp(m_redemOrder.Fspid, m_params.getString("ori_spid").c_str()))
    {
        gPtrAppLog->error("redem trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_redemOrder.Fspid, m_params.getString("ori_spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "redem trade exists, spid is different!", __FILE__, __LINE__);
    }

    if (m_redemOrder.Fstate != REDEM_ININ && m_redemOrder.Fstate != REDEM_FINISH)
    {
        gPtrAppLog->error("fund redem , state=%d is invalid. listid[%s], uid[%d] ", m_redemOrder.Fstate,m_redemOrder.Flistid, m_redemOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  state is invalid. ", __FILE__, __LINE__);   
    }
    
    if (!m_params.getString("sp_billno_redem").empty() && m_redemOrder.Fcoding[0] != 0
        && m_params.getString("sp_billno_redem") != m_redemOrder.Fcoding)
    {
        gPtrAppLog->error("fund trade exists, sp_billno_redem is different! sp_billno_redem in db[%s], sp_billno_redem input[%s] ", 
			m_redemOrder.Fcoding, m_params.getString("sp_billno_redem").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, sp_billno_redem is different!", __FILE__, __LINE__);
    } 
    // 得到交易相关信息
    m_params.setParam("uid", m_redemOrder.Fuid);
}

//检查申购单状态
void FundTransferAck::checkBuyTrade() throw (CException)
{
    // 没有交易记录，报错
    if(!QueryTradeFund(m_pFundCon, m_transferOrder.Fbuy_id, PURTYPE_TRANSFER_PURCHASE,&m_buyOrder, true))
    {
        gPtrAppLog->error("buy record not exist, Fbuy_id[%s]  ", m_transferOrder.Fbuy_id);
        throw CException(ERR_BUYPAY_NOLIST, "buy record not exist! ", __FILE__, __LINE__);
    }

    // 物理状态无效，报错
    if(LSTATE_INVALID == m_buyOrder.Flstate)
    {
        gPtrAppLog->error("fund buy , lstate is invalid. listid[%s], uid[%d] ", m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "buy list  lstate is invalid. ", __FILE__, __LINE__);
    }

    // 校验关键参数
    if(m_buyOrder.Ftotal_fee != m_params.getLong("total_fee"))
    {
        gPtrAppLog->error("buy total_fee is different! total_fee in db[%lld], total_fee input[%lld] ", 
			m_buyOrder.Ftotal_fee, m_params.getLong("total_fee"));
		throw CException(ERR_REPEAT_ENTRY_DIFF, "buy total_fee is different!", __FILE__, __LINE__);
    }

    if( m_buyOrder.Fuid!= m_fund_bind.Fuid)
    {
        TRACE_ERROR("uid in db=%d diff with input=%d", 
					m_buyOrder.Fuid, m_fund_bind.Fuid);
        throw EXCEPTION(ERR_REPEAT_ENTRY_DIFF, "uid in db diff with fundbind");
    }

    if (0 != strcmp(m_buyOrder.Fspid, m_params.getString("new_spid").c_str()))
    {
        gPtrAppLog->error("buy trade exists, spid is different! spid in db[%s], spid input[%s]", 
			m_buyOrder.Fspid, m_params.getString("new_spid").c_str());
		throw CException(ERR_REPEAT_ENTRY_DIFF, "buy trade exists, spid is different!", __FILE__, __LINE__);
    }

    if (m_buyOrder.Fstate != PAY_INIT && m_buyOrder.Fstate != PAY_OK && m_buyOrder.Fstate != PURCHASE_SUC)
    {
        gPtrAppLog->error("fund buy , state=%d is invalid. listid[%s], uid[%d] ", m_buyOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  state is invalid. ", __FILE__, __LINE__);   
    }
    
    // 校验商户订单号
    if (!m_params.getString("sp_billno_buy").empty() && m_buyOrder.Fcoding[0] != 0
        && m_params.getString("sp_billno_buy") != m_buyOrder.Fcoding)
    {
        gPtrAppLog->error("fund trade exists, sp_billno_buy is different! sp_billno_buy in db[%s], sp_billno_buy input[%s] ", 
			m_buyOrder.Fcoding, m_params.getString("sp_billno_buy").c_str());
        throw CException(ERR_REPEAT_ENTRY_DIFF, "fund transfer exists, sp_billno_buy is different!", __FILE__, __LINE__);
    }
    // 得到交易相关信息
    m_params.setParam("uid", m_buyOrder.Fuid);
}


/**
*累加商户单日赎回总额
*/
void FundTransferAck::UpdateFundSpRedemInfo()throw (CException)
{
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_orisp_config.Fspid, m_params["ori_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params["ori_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);
    if(!queryFundSpAndFundcodeConfig(m_pFundCon, m_fund_orisp_config, true))
    {
    	//不应该发生
    	throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
    }
    strncpy(m_fund_orisp_config.Fmodify_time, m_params["systime"], sizeof(m_fund_orisp_config.Fmodify_time) - 1);
    updateFundSpRedomTotal(m_pFundCon, m_fund_orisp_config, m_transferOrder.Ftotal_fee,DRAW_NOT_USE_LOADING,m_transferOrder.Facc_time);    
}

/**
*更新赎回单状态
*/
void FundTransferAck::UpdateFundRedem(int state)throw (CException)
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));

    stRecord.Fstate = state;
    stRecord.Fpur_type = m_redemOrder.Fpur_type;
    stRecord.Fuid = m_redemOrder.Fuid;    
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_redemOrder.Ftrade_id);
    strncpy(stRecord.Flistid, m_redemOrder.Flistid, sizeof(stRecord.Flistid) - 1);
    strncpy(stRecord.Fcoding, m_params.getString("sp_billno_redem").c_str(), sizeof(stRecord.Fcoding) - 1);
    if(OP_TYPE_TRANSFER_REDEM_TIMEOUT == m_params.getInt("op_type"))
    {
        stRecord.Fspe_tag= TRADE_RECORD_TIMEOUT;//超时标记		
    }
    
    UpdateFundTrade(m_pFundCon, stRecord, m_redemOrder, m_params.getString("systime"));
}

/**
*更新申购单状态
*/
void FundTransferAck::UpdateFundBuy(int state, LONG close_listid)throw (CException)
{
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    stRecord.Fstate = state;
    strncpy(stRecord.Flistid, m_buyOrder.Flistid, sizeof(stRecord.Flistid) - 1);
    stRecord.Fpur_type = m_buyOrder.Fpur_type;
    stRecord.Fuid = m_buyOrder.Fuid;    
    //保存trade_id,更新交易记录时需要使用
    SCPY(stRecord.Ftrade_id, m_buyOrder.Ftrade_id);
	stRecord.Fclose_listid = close_listid;
	//strncpy(stRecord.Fend_date, m_buyOrder.Fend_date, sizeof(stRecord.Fend_date) - 1);
    UpdateFundTrade(m_pFundCon, stRecord, m_redemOrder, m_params.getString("systime"));
}

/**
*累加用户今日转换次数
*/
void FundTransferAck::RecordTransferTimes() throw (CException)
{
    strncpy(m_dynamic_info.Ftrade_id,m_params["trade_id"],sizeof(m_dynamic_info.Ftrade_id)-1);
    strncpy(m_dynamic_info.Fmodify_time,m_params["systime"],sizeof(m_dynamic_info.Fmodify_time)-1);
    if (queryFundDynamic(m_pFundCon,m_dynamic_info,true))
    {
        if (m_transferOrder.Facc_time<changeDatetimeFormat(string(m_dynamic_info.Fredem_day)+"150000"))
        {
            if (m_transferOrder.Facc_time>=changeDatetimeFormat(addDays(m_dynamic_info.Fredem_day,-1)+"150000"))
            {
                m_dynamic_info.Fredem_times_day++;
                if (m_dynamic_info.Fredem_times_day >= gPtrConfig->m_AppCfg.max_transfer_times_oneday)
                {
                    m_dynamic_info.Fdyn_status_mask = (m_dynamic_info.Fdyn_status_mask|USER_STOP_TRANSFER);
                }
            }
        }
        else  
        {
            strncpy(m_dynamic_info.Fredem_day,addDays(getYYYYMMDDFromStdTime(m_transferOrder.Facc_time),1).c_str(),sizeof(m_dynamic_info.Fredem_day)-1);
            m_dynamic_info.Fredem_times_day=1;
            m_dynamic_info.Fdyn_status_mask = (m_dynamic_info.Fdyn_status_mask&(~USER_STOP_TRANSFER));
        }
        updateFundDynamic(m_pFundCon,m_dynamic_info);
    }
    else
    {
        string redemDay=getYYYYMMDDFromStdTime(m_transferOrder.Facc_time);
        if (string(m_transferOrder.Facc_time).substr(11,2)>="15")
        {
             redemDay = addDays(redemDay,1);
        }
        strncpy(m_dynamic_info.Fredem_day,redemDay.c_str(),sizeof(m_dynamic_info.Fredem_day)-1);
        m_dynamic_info.Fredem_times_day=1;
        m_dynamic_info.Fdyn_status_mask=0;
        m_dynamic_info.Flstate=1;
        strncpy(m_dynamic_info.Fcreate_time,m_params["systime"],sizeof(m_dynamic_info.Fcreate_time)-1);
        insertFundDynamic(m_pFundCon,m_dynamic_info);
    }
}


/**
*转换确认(申购成功确认)
*/
void FundTransferAck::processTransferBuyReqSuc() throw (CException)
{
    //检查用户份额
    checkUserBalance();

    //检查spid 及fund_code 是否有效并获取基金名称创建基金交易单时用到
    strncpy(m_fund_orisp_config.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params.getString("ori_fund_code").c_str(), sizeof(m_fund_orisp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_orisp_config, false);

    strncpy(m_fund_newsp_config.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_newsp_config.Fspid) - 1);
    strncpy(m_fund_newsp_config.Ffund_code, m_params.getString("new_fund_code").c_str(), sizeof(m_fund_newsp_config.Ffund_code) - 1);
    checkFundSpAndFundcode(m_pFundCon,m_fund_newsp_config, false);

    //检查商户赎回额度
    checkSpRedemRateLimit(m_pFundCon, m_fund_orisp_config,m_params["systime"],m_params.getLong("total_fee"));
    
    //更新转换单acctime和状态为申购请求成功
    UpdateTransferState(FUND_TRANSFER_REQ,0,m_params["systime"]);
    //创建申购单
    RecordFundBuy();
    //创建赎回单
    RecordFundRedem();

    //查询交易账户信息给itg做赎回
    strncpy(m_fund_bind_orisp_acc.Ftrade_id, m_params.getString("trade_id").c_str(), sizeof(m_fund_bind_orisp_acc.Ftrade_id) - 1);
    strncpy(m_fund_bind_orisp_acc.Fspid, m_params.getString("ori_spid").c_str(), sizeof(m_fund_bind_orisp_acc.Fspid) - 1);
    queryValidFundBindSp(m_pFundCon, m_fund_bind_orisp_acc, false);
}


/**
*赎回成功处理
*/
void FundTransferAck::processTransferRedemSuc() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_INIT || m_redemOrder.Fstate != REDEM_ININ)
    {
        gPtrAppLog->error("fund buy state=%d,redem state=%d. invalid for processTransferRedemSuc listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_redemOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem or buy list  state is invalid. ", __FILE__, __LINE__);      
    }

    //检查赎回额度
    memset(&m_fund_orisp_config, 0, sizeof(FundSpConfig));
    strncpy(m_fund_orisp_config.Fspid, m_params["ori_spid"], sizeof(m_fund_orisp_config.Fspid) - 1);
    strncpy(m_fund_orisp_config.Ffund_code, m_params["ori_fund_code"], sizeof(m_fund_orisp_config.Ffund_code) - 1);
    checkSpRedemRateLimit(m_pFundCon, m_fund_orisp_config,m_params["systime"],m_params.getLong("total_fee"));

	queryNewFundSpAndFundcodeInfo();

	checkUserPermissionBuyCloseFund();
    
    //查询转入交易账户给itg做申购
    queryFundBindNewSpInfo();
    
    //减子账户
    doDraw();

    //更新赎回单状态
    UpdateFundRedem(REDEM_FINISH);

	//定期产品记录
	LONG close_listid =0;
	recordCloseFund(m_buyOrder.Ftrade_date, close_listid);

    //更新申购单状态
    UpdateFundBuy(PAY_OK, close_listid);	
	
    //加子账户余额
    bool saveRet = doSave(false);
    //更新转换单状态
    int subacc_state = FUND_TRANSFER_SUBACC_DRAW;
    if (saveRet)
    {
        subacc_state = (FUND_TRANSFER_SUBACC_DRAW|FUND_TRANSFER_SUBACC_SAVE);
    }
    UpdateTransferState(FUND_TRANSFER_REDEM_SUC,subacc_state);

    //累加转换次数
    RecordTransferTimes();

    //累加赎回份额
    UpdateFundSpRedemInfo();

	//更新CKV增加记录
    ST_TRADE_FUND  stRecord;
    memset(&stRecord, 0, sizeof(ST_TRADE_FUND));
    stRecord.Fstate = PAY_OK;
    stRecord.Fpur_type = m_buyOrder.Fpur_type;
	stRecord.Fclose_listid = close_listid;
	strncpy(stRecord.Ftrade_id, m_buyOrder.Ftrade_id, sizeof(stRecord.Ftrade_id)-1);
	strncpy(stRecord.Ffund_code, m_buyOrder.Ffund_code, sizeof(stRecord.Ffund_code)-1);
	strncpy(stRecord.Facc_time, m_params.getString("systime").c_str(), sizeof(stRecord.Facc_time)-1);
	stRecord.Ftotal_fee = m_buyOrder.Ftotal_fee;
	updateUserAcc(stRecord);

    /*catch(CException &e)
    {
        gPtrAppLog->error("in processTransferRedemSuc call  RecordTransferTimes or UpdateFundSpRedemInfo throw exp:errcode=%d,errinfo=%s"
                                        ,e.error(),e.what());
        alert(e.error(), (string("转换赎回成功后累加次数和限额返回异常:%s")+e.what()).c_str());
    }*/
    
}

/**
*赎回失败处理
*/
void FundTransferAck::processTransferRedemFail() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_INIT || m_redemOrder.Fstate != REDEM_ININ)
    {
        gPtrAppLog->error("fund buy state=%d,redem state=%d. invalid for processTransferRedemFail listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_redemOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem or buy list  state is invalid. ", __FILE__, __LINE__);      
    }
    //更新赎回单状态
    UpdateFundRedem(REDEM_FAIL);

    //更新转换单状态
    UpdateTransferState(FUND_TRANSFER_REDEM_FAIL);
}

void FundTransferAck::processTransferBuySuc() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_OK || m_redemOrder.Fstate != REDEM_FINISH)
    {
        gPtrAppLog->error("fund buy state=%d,redem state=%d. invalid for processTransferBuySuc listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_redemOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem or buy list  state is invalid. ", __FILE__, __LINE__);      
    }

	checkCloseEndDate();

    //更新申购单状态
    UpdateFundBuy(PURCHASE_SUC);

    //更新转换单状态
    UpdateTransferState(FUND_TRANSFER_TRANS_SUC);
}

void FundTransferAck::processTransferSubaccSaveRedo() throw (CException)
{
    if (m_buyOrder.Fstate != PAY_OK && m_buyOrder.Fstate != PURCHASE_SUC)
    {
        gPtrAppLog->error("fund buy state=%d. invalid for processTransferSubaccSaveRedo listid[%s], uid[%d] "
                                        , m_buyOrder.Fstate,m_buyOrder.Flistid, m_buyOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, " buy list  state is invalid. ", __FILE__, __LINE__);      
    }

    //更新转换单子账户状态
    if (doSave(true))
    {
        UpdateTransferState(0,(m_transferOrder.Fsubacc_state|FUND_TRANSFER_SUBACC_SAVE));
    }
}

void FundTransferAck::processTransferSpRedemRedoSuc() throw (CException)
{
    if (m_redemOrder.Fstate != REDEM_FINISH)
    {
        gPtrAppLog->error("redem state=%d. invalid for processTransferSpRedemRedoSuc listid[%s], uid[%d] "
                                        , m_redemOrder.Fstate,m_redemOrder.Flistid, m_redemOrder.Fuid);
        throw CException(ERR_TRADE_INVALID, "redem list  state is invalid. ", __FILE__, __LINE__);      
    }
    //更新赎回单中spe_tag标记为0
    UpdateFundRedem(REDEM_FINISH);
    
    //更新转换单中spe_tag标记为0
    UpdateTransferState(m_transferOrder.Fstate);
}

void FundTransferAck::queryNewFundSpAndFundcodeInfo()
{
	strncpy(m_fund_newsp_config.Fspid, m_params.getString("new_spid").c_str(), sizeof(m_fund_newsp_config.Fspid) - 1);
	strncpy(m_fund_newsp_config.Ffund_code, m_params.getString("new_fund_code").c_str(), sizeof(m_fund_newsp_config.Ffund_code) - 1);
	queryFundSpAndFundcodeFromCkv(m_fund_newsp_config, true);
}

void FundTransferAck::checkUserPermissionBuyCloseFund()
{
	//非封闭产品不检查
	if(m_fund_newsp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}
	
	strncpy(m_fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	strncpy(m_fundCloseTrans.Ffund_code, m_buyOrder.Ffund_code, sizeof(m_fundCloseTrans.Ffund_code) - 1);

	// 以m_buyOrder的周期为准
	m_close_fund_seqno = checkPermissionBuyCloseFund(m_fundCloseTrans, m_fund_newsp_config, m_buyOrder.Ftrade_date, m_buyOrder.Fend_date, true);
    gPtrAppLog->debug("checkUserPermissionBuyCloseFund : m_close_fund_seqno=%d,m_fundCloseTrans.Fseq=%d", m_close_fund_seqno, m_fundCloseTrans.Fseqno);

	m_params.setParam("trans_date", m_buyOrder.Ftrade_date);
	
}

void FundTransferAck::recordCloseFund(string trade_date, LONG& close_listid)
{
	//非封闭产品直接返回
	if(m_fund_newsp_config.Fclose_flag == CLOSE_FLAG_NORMAL)
	{
		return;
	}

	bool sale_close_fund_seqno = m_close_fund_seqno == m_fundCloseTrans.Fseqno;
	
	// 封闭到期日计算
	FundCloseCycle fundCloseCycle;
	memset(&fundCloseCycle, 0, sizeof(FundCloseCycle));
	strncpy(fundCloseCycle.Fdate, calculateFundDate(m_buyOrder.Facc_time).c_str(), sizeof(fundCloseCycle.Fdate) - 1);
	strncpy(fundCloseCycle.Ffund_code, m_buyOrder.Ffund_code, sizeof(fundCloseCycle.Ffund_code) - 1);
	bool hasCycle = queryFundCloseCycle(m_pFundCon, fundCloseCycle, false);

	if(strcmp(m_buyOrder.Ftrade_date,fundCloseCycle.Ftrans_date)!=0||!hasCycle)
	{
		//两次计算不一致告警
		alert(ERR_DIFF_END_DATE, (string("date:") + toString(fundCloseCycle.Fdate) +string("fund_code:") + toString(fundCloseCycle.Ffund_code)
			+ "申购单计算的交易日与定期记录计算的不一致").c_str());
		throw CException(ERR_DIFF_END_DATE, "trans date diff.", __FILE__, __LINE__); 
	}
	
	FundCloseTrans fundCloseTrans;
	
	//如果相等则更新，否则创建新纪录
	if(sale_close_fund_seqno)
	{
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		fundCloseTrans.Fid= m_fundCloseTrans.Fid;
		// 如果未设置不更新
		if(m_params.getInt("user_end_type")!=0){
			fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type");
			fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		}
		if(m_params.getInt("end_sell_type")!=0){
			fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO 根据产品规则决定
			strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
			strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		}
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);
	
		fundCloseTrans.Fpay_type = m_fundCloseTrans.Fpay_type | CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_fundCloseTrans.Fstart_total_fee + m_buyOrder.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_fundCloseTrans.Fcurrent_total_fee +  m_buyOrder.Ftotal_fee;
		
		saveFundCloseTrans(fundCloseTrans,m_fundCloseTrans,m_buyOrder.Flistid,PURTYPE_PURCHASE);

		close_listid = m_fundCloseTrans.Fid;
		
	}
	else
	{
		memset(&fundCloseTrans, 0, sizeof(FundCloseTrans));
		strncpy(fundCloseTrans.Ftrade_id, m_fund_bind.Ftrade_id, sizeof(fundCloseTrans.Ftrade_id) - 1);
		strncpy(fundCloseTrans.Fspid, m_buyOrder.Fspid, sizeof(fundCloseTrans.Fspid) - 1);
		strncpy(fundCloseTrans.Ffund_code, m_buyOrder.Ffund_code, sizeof(fundCloseTrans.Ffund_code) - 1);
		fundCloseTrans.Fuid =  m_params.getInt("uid");
		fundCloseTrans.Fseqno = m_close_fund_seqno;
		fundCloseTrans.Fuser_end_type = m_params.getInt("user_end_type");
		fundCloseTrans.Fend_sell_type = m_params.getInt("end_sell_type"); //TODO 根据产品规则决定
		fundCloseTrans.Fend_plan_amt = m_params.getLong("end_plan_amt");
		strncpy(fundCloseTrans.Fend_transfer_fundcode, m_params.getString("end_transfer_fundcode").c_str(), sizeof(fundCloseTrans.Fend_transfer_fundcode) - 1);
		strncpy(fundCloseTrans.Fend_transfer_spid, m_params.getString("end_transfer_spid").c_str(), sizeof(fundCloseTrans.Fend_transfer_spid) - 1);
		strncpy(fundCloseTrans.Fmodify_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fmodify_time) - 1);
		
		fundCloseTrans.Fpay_type = CLOSE_FUND_PAY_TYPE_WX;
		fundCloseTrans.Fstart_total_fee = m_buyOrder.Ftotal_fee;
		fundCloseTrans.Fcurrent_total_fee = m_buyOrder.Ftotal_fee;
		//新创建如果未设置则使用默认值
		fundCloseTrans.Fuser_end_type = (m_params.getInt("user_end_type") != 0) ? m_params.getInt("user_end_type") : CLOSE_FUND_END_TYPE_ALL_EXTENSION; //TODO 根据产品规则决定

		//使用申请申购单的acctime，以保证三方一致
		strncpy(fundCloseTrans.Facc_time, m_buyOrder.Facc_time, sizeof(fundCloseTrans.Facc_time) - 1);	    
	    strncpy(fundCloseTrans.Ftrans_date, fundCloseCycle.Ftrans_date, sizeof(fundCloseTrans.Ftrans_date) - 1);
		strncpy(fundCloseTrans.Ffirst_profit_date, fundCloseCycle.Ffirst_profit_date, sizeof(fundCloseTrans.Ffirst_profit_date) - 1);
		strncpy(fundCloseTrans.Fopen_date, fundCloseCycle.Fopen_date, sizeof(fundCloseTrans.Fopen_date) - 1);
		strncpy(fundCloseTrans.Fbook_stop_date, fundCloseCycle.Fbook_stop_date, sizeof(fundCloseTrans.Fbook_stop_date) - 1);
		strncpy(fundCloseTrans.Fstart_date, fundCloseCycle.Fstart_date, sizeof(fundCloseTrans.Fstart_date) - 1);
		strncpy(fundCloseTrans.Fend_date, fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fend_date) - 1);
		strncpy(fundCloseTrans.Fdue_date, fundCloseCycle.Fdue_date, sizeof(fundCloseTrans.Fdue_date) - 1);
		strncpy(fundCloseTrans.Fprofit_end_date, fundCloseCycle.Fprofit_end_date, sizeof(fundCloseTrans.Fprofit_end_date) - 1);
		strncpy(fundCloseTrans.Fchannel_id, m_buyOrder.Fchannel_id, sizeof(fundCloseTrans.Fchannel_id) - 1);
		fundCloseTrans.Fstate= CLOSE_FUND_STATE_PENDING;
		fundCloseTrans.Flstate = LSTATE_VALID;
		strncpy(fundCloseTrans.Fcreate_time, m_params.getString("systime").c_str(), sizeof(fundCloseTrans.Fcreate_time) - 1);

		createFundCloseTrans(fundCloseTrans,m_buyOrder.Flistid,PURTYPE_PURCHASE);
		
		close_listid = fundCloseTrans.Fid;
	}

	//用于返回参数使用
	m_params.setParam("trans_date", fundCloseCycle.Ftrans_date);
	m_params.setParam("first_profit_date", fundCloseCycle.Ffirst_profit_date);
	m_params.setParam("open_date", fundCloseCycle.Fopen_date);
	m_params.setParam("book_stop_date", fundCloseCycle.Fbook_stop_date);
	m_params.setParam("start_date", fundCloseCycle.Fstart_date);
	m_params.setParam("end_date", fundCloseCycle.Fdue_date);
	m_params.setParam("profit_end_date", fundCloseCycle.Fprofit_end_date);
    
	FundSpConfig fundSpConfig;
	memset(&fundSpConfig, 0, sizeof(FundSpConfig));

	strncpy(fundSpConfig.Fspid, m_buyOrder.Fspid, sizeof(fundSpConfig.Fspid) - 1);
	strncpy(fundSpConfig.Ffund_code, m_buyOrder.Ffund_code, sizeof(fundSpConfig.Ffund_code) - 1);
    	
	if(!queryFundSpAndFundcodeConfig(m_pFundCon, fundSpConfig, true))
	{
		//不应该发生
		throw EXCEPTION(ERR_BAD_PARAM, "input spid or fund_code error"); 
	}
	
	//基金配置Fstat_flag=1时按自然日检查限额
	string strStatDay=( SPCONFIG_STAT_DDAY & fundSpConfig.Fstat_flag )? nowdate(m_params.getString("systime").c_str()):m_buyOrder.Ftrade_date;       

       //转换场景暂时不考虑退款
	checkAndUpdateFundScope(fundSpConfig, m_buyOrder.Ftotal_fee,strStatDay);

	//记录ckv
	setFundCloseTransToKV(m_fund_bind.Ftrade_id, m_buyOrder.Ffund_code);

	
}

void FundTransferAck::checkCloseEndDate()
{
	if(OP_TYPE_TRANSFER_BUY_SUC != m_optype)
	{
		return;
	}

	if(m_buyOrder.Fclose_listid <= 0)
	{
		//非封闭基金不检查
		return;
	}
		
	m_fundCloseTrans.Fid = m_buyOrder.Fclose_listid;
	strncpy(m_fundCloseTrans.Ftrade_id, m_buyOrder.Ftrade_id, sizeof(m_fundCloseTrans.Ftrade_id) - 1);
	queryFundCloseTrans(gPtrFundDB, m_fundCloseTrans, false);

	if(m_params.getString("close_end_day") != m_fundCloseTrans.Fend_date)
	{
		alert(ERR_DIFF_END_DATE, (string("Fid:") + toString(m_fundCloseTrans.Fid) + "基金公司返回的封闭结束日和本地计算不一致").c_str());
		throw CException(ERR_DIFF_END_DATE, "end date diff.", __FILE__, __LINE__); 
	}
}



/**
*根据请求类型，做相应的处理 
*/
void FundTransferAck::dealTransfer() throw (CException)
{
    switch (m_optype)  //这里不能改成 m_param.getInt("op_type") ,状态校验函数中会对m_optype增加内部处理类型
    {       
        case FUND_TRANSFER_REQ: //申购请求成功
            processTransferBuyReqSuc();
            break;
			
        case FUND_TRANSFER_REDEM_SUC: //赎回成功
            processTransferRedemSuc();
            break;
			
        case FUND_TRANSFER_REDEM_FAIL: //赎回失败
            processTransferRedemFail();
            break;

        case FUND_TRANSFER_TRANS_SUC: //申购确认成功
            processTransferBuySuc();
            break;
            
        case FUND_TRANSFER_REDEM_SPTIMEOUT_REDO_OK://基金公司赎回操作补单成功
            processTransferSpRedemRedoSuc();
            break;
            
        case FUND_TRANSFER_SUBACC_SAVE_REDO: //子账户加失败补单
            processTransferSubaccSaveRedo();
            break; 
            
        default:
            throw CException(ERR_BAD_PARAM, "op_type invalid", __FILE__, __LINE__);
            break;

    }
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
void FundTransferAck::doDraw() throw (CException)
{
    gPtrAppLog->debug("doDraw, listid[%s]  ", m_redemOrder.Fsub_trans_id);

    try
    {
        SubaccDraw(gPtrSubaccRpc, m_params.getString("ori_spid"), m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_redemOrder.Fsub_trans_id, m_params.getLong("total_fee"), m_redemOrder.Facc_time);
    }
    catch(CException& e)
    {
        //赎回补单由批跑发起，批跑判断补多久
        //如果赎回子账户减钱超过10分钟没成功的告警，无论是差错补单还是外部批跑补单
        if((time_t)(toUnixTime(m_redemOrder.Facc_time))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval <(time(NULL)))	
        {
            char szErrMsg[256] = {0};
            snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.转换赎回超过10分钟子账户仍未减成功");		  
            alert(ERR_BUDAN_TOLONG, szErrMsg);

            if(ERR_TYPE_MSG)
            {
                throw ; //差错补单一定时间未成功的发送告警后不再补单，避免死循环或发生雪崩,通过对账渠道进行补单
            }
        }
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

        callErrorRpc(m_request, gPtrSysLog);
        
        throw;//减子账户失败抛出异常，事物回滚
    }
}


/**
 * 子账户充值
 */
bool FundTransferAck::doSave(bool exp) throw(CException)
{
    gPtrAppLog->debug("doSave, listid[%s]  ", m_buyOrder.Flistid);
    try
    {
        //开子账户延迟到支付成功，减少活动时申购请求的压力
        createSubaccUser(m_request,m_buyOrder.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"));

        SubaccSave(gPtrSubaccRpc, m_buyOrder.Fspid, m_fund_bind.Fqqid, m_params.getString("client_ip"),
			m_buyOrder.Fsub_trans_id, m_params.getLong("total_fee"),"基金转入", m_buyOrder.Facc_time, 1);
		
    }
    catch(CException& e)
    {
        //子账户不应该失败，所有加钱失败的都发给差错
        //如果支付回调超过一定间隔时间以上，子账户还报异常，发告警
        if ((time_t)(toUnixTime(m_redemOrder.Facc_time))+ gPtrConfig->m_AppCfg.paycb_overtime_inteval < (time(NULL)))	
        {
            char szErrMsg[256] = {0};
            snprintf(szErrMsg, sizeof(szErrMsg), "fund_deal_server.转换赎回超过10分钟子账户加仍未成功");        
            alert(ERR_BUDAN_TOLONG, szErrMsg);
            if(ERR_TYPE_MSG)
            {
                return false; //差错补单一定时间未成功的发送告警后不再补单，避免死循环或发生雪崩,通过对账渠道进行补单
            }
        }
		
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());
        
        // 子账户加失败，不影响交易单状态成功，发差错，通过差错消息重试加子账户操作
        callErrorRpc(m_request, gPtrSysLog);

        if(exp)
        {
            throw;
        }
        else
        {
            return false;
        }
    }

    return true;
}

/**
  * 事物提交后把需要更新到ckv的数据提交
  */
void FundTransferAck::updateCkvs()
{
    if (m_dynamic_info.Ftrade_id[0] != 0)
    {
        setUserDynamicInfoToCkv(m_pFundCon,m_dynamic_info,false);
    }
}


/**
  * 打包输出参数
  */
void FundTransferAck::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    CUrlAnalyze::setParam(rqst->odata, "trade_id", m_params.getString("trade_id").c_str());
    CUrlAnalyze::setParam(rqst->odata, "total_fee", m_params.getString("total_fee").c_str());
	
    if (m_params.getInt("op_type") == FUND_TRANSFER_REDEM_SUC || m_params.getInt("op_type")  == FUND_TRANSFER_REDEM_TIMEOUT ) //赎回成功或者超时
    {
        if(m_fund_bind_newsp_acc.Fsp_user_id[0] ==0 && (!ERR_TYPE_MSG))
        {
            queryFundBindNewSpInfo();
        }
        CUrlAnalyze::setParam(rqst->odata, "new_sp_user", m_fund_bind_newsp_acc.Fsp_user_id);
        CUrlAnalyze::setParam(rqst->odata, "new_sp_trans_id", m_fund_bind_newsp_acc.Fsp_trans_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
        CUrlAnalyze::setParam(rqst->odata, "new_fund_code", m_params.getString("new_fund_code").c_str());
        CUrlAnalyze::setParam(rqst->odata, "buy_id", m_transferOrder.Fbuy_id);
        CUrlAnalyze::setParam(rqst->odata, "sp_billno_buy", m_buyOrder.Fcoding);
        CUrlAnalyze::setParam(rqst->odata, "acc_time", m_transferOrder.Facc_time);

		if(m_fund_newsp_config.Fclose_flag != CLOSE_FLAG_NORMAL)
		{
			//本地的trans_date为外部定义的close_start_day，前置机和基金公司协议的问题导致
			CUrlAnalyze::setParam(rqst->odata, "close_start_day", m_params.getString("trans_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "trans_date", m_params.getString("trans_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "first_profit_date", m_params.getString("first_profit_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "open_date", m_params.getString("open_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "book_stop_date", m_params.getString("book_stop_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "start_date", m_params.getString("start_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "end_date", m_params.getString("end_date").c_str());
			CUrlAnalyze::setParam(rqst->odata, "profit_end_date", m_params.getString("profit_end_date").c_str());
			//CUrlAnalyze::setParam(rqst->odata, "acc_time", m_params.getString("acc_time").c_str()); //和交易记录的冲突了
		}
    }
    else if (m_params.getInt("op_type") == FUND_TRANSFER_REQ) //申购请求成功
    {
        CUrlAnalyze::setParam(rqst->odata, "old_sp_user", m_fund_bind_orisp_acc.Fsp_user_id);
        CUrlAnalyze::setParam(rqst->odata, "old_sp_trans_id", m_fund_bind_orisp_acc.Fsp_trans_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_id", m_fund_bind.Fcre_id);
        CUrlAnalyze::setParam(rqst->odata, "cre_type", m_fund_bind.Fcre_type);
        CUrlAnalyze::setParam(rqst->odata, "old_fund_code", m_params.getString("ori_fund_code").c_str());
        CUrlAnalyze::setParam(rqst->odata, "redem_id", m_transferOrder.Fredem_id);
        CUrlAnalyze::setParam(rqst->odata, "acc_time", m_transferOrder.Facc_time);
    }

    rqst->olen = strlen(rqst->odata);
    return;
}


