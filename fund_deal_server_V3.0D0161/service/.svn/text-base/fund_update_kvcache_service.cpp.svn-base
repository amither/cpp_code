/**
  * FileName: fund_update_kvcache_service.cpp
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-11-10
  * Description: 基金交易服务 查询是否可进行份额转换接口
  */

#include "fund_commfunc.h"
#include "fund_update_kvcache_service.h"

FundUpdateKvcache::FundUpdateKvcache(CMySQL* mysql, int type)
{
    m_pFundCon = mysql;      
    m_servicetype = type;
}

/**
  * service step 1: 解析输入参数
  */
void FundUpdateKvcache::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
	char *pMsg = (char*)(rqst->idata);

	TRACE_DEBUG("[fund_update_kvcache_service] receives: %s", pMsg);

    // 读取参数
    m_params.readIntParam(pMsg, "op_type", 1,500);
    m_params.readStrParam(pMsg, "key", 0,500);
	m_params.readStrParam(pMsg, "value", 0,50000);
	m_params.readStrParam(pMsg, "watch_word", 0,64);
	m_params.readIntParam(pMsg, "expire", 0, MAX_INTEGER); //写入ckv过期时间
	
    m_params.readStrParam(pMsg, "qlskey", 0, 64);
    m_params.readStrParam(pMsg, "qluin", 0, 64);
}


/**
  * 执行申购请求
  */
void FundUpdateKvcache::excute() throw (CException)
{
	CheckParams();
	
	bool ret = false;
    
    int op_type = m_params.getInt("op_type");
    if (UPDATE_KVCACHE_KEY_VALUE != op_type
        && DELETE_KVCACHE_KEY != op_type
        && UPDATE_KVCACHE_KEY_VALUE_V2 != op_type) {
        string no_prefix_key = adapt_ckv_key(op_type, m_params["key"]);
        m_params.setParam("key", no_prefix_key);
        TRACE_DEBUG("no_prefix_key=%s", no_prefix_key.c_str())
    }
    
	switch (m_params.getInt("op_type"))
    {
		case CKV_KEY_UIN:
            ST_FUND_BIND stFundAcc;
    		memset(&stFundAcc, 0, sizeof(ST_FUND_BIND));
			strncpy(stFundAcc.Fqqid, m_params.getString("key").c_str(), sizeof(stFundAcc.Fqqid) - 1);
			ret = setFundBindToKV(m_pFundCon, stFundAcc, true);
            break;
			
		case CKV_KEY_TRADE_ID:
            ret = setFundBindAllSpToKVFromDB(m_pFundCon,m_params.getString("key"));
            break;

        //更新用户总收益记录
        case CKV_KEY_TOTAL_PROFIT:
            ret = setUserTotalProfitToKV();
            break;
            
        case CKV_KEY_FUND_SUPPORT_SP_ALL:
            ret = setAllSupportSpConfig(m_pFundCon);
            break;

        case CKV_KEY_FUND_SUPPORT_BANK_ALL:
            ret = setAllSupportBankToKV(m_pFundCon);
            break;

		case CKV_KEY_PROFIT_RATE:
            ret = setProfitRateToKV();
            break;

        //更新用户收益流水
        case CKV_KEY_PROFIT_RECORD:
            ret = setUserProfitRecordsToKV();
            break;
        
		case CKV_KEY_PAY_CARD:
			ret = setPayCardInfoToKV();
			break;

		case CKV_KEY_FUND_TOTALACC_2:
			ret = setFundTotalaccInfoToKV();
			break;

		case CKV_KEY_CLOSE_FUND_CYCLE:
			ret = setAllCloseFundCycleToKV();
			break;

		case CKV_KEY_FUND_USER_ACC:
			ret = setUserSubAccToKV();
			break;
			
		case UPDATE_KVCACHE_KEY_VALUE:
            ret = setKeyValue();
            break;
			
		case DELETE_KVCACHE_KEY:
            ret = delKeyValue();
            break;

		case UPDATE_KVCACHE_KEY_VALUE_V2:
            ret = setV2KeyValue();
            break;
		// 更新用户定期交易流水
		case CKV_KEY_FUND_CLOSE_TRANS:
            ret = setUserCloseTransToKV();
            break;

        //更新基金交易日信息
        case CKV_KEY_FUND_TRANS_DATE:
        {    string cur_day = toString(GetDateToday());
            ret = setFundTransDay2Ckv(gPtrFundSlaveDB, cur_day);
            break;
        }
            
        case CKV_KEY_FUND_BALANCE_CONFIG:
            ret = setFundBalanceConfigToCkv(m_pFundCon);
		    break;
        case CKV_KEY_USER_LATEST_FUND_TRADE:
            ret = setUserFundTradeToKV();
            break;
        //更新用户白名单权限位
        case CKV_KEY_PC_CHARGE_WHITELIST:
        {  
                ret = updateUserWhiteList();
                break;
        }
        case CKV_KEY_UNCONFIRM:
        {  
                ret = setFundUnconfirmCKV();
                break;
        }
		case CKV_KEY_TDAY:
            ret = setTDayToKV();
            break; 
		case CKV_KEY_UNFINISH_INDEX:
			ret = setUnfinishTransCKV();
			break;
		case CKV_KEY_CASH_IN_TRANSIT:
			ret = setFundCashInTransitCKV();
			break;
        default:
			TRACE_WARN("unexcept op_type=[%s][%d]", m_params.getString("op_type").c_str(),m_params.getInt("op_type"));
            throw EXCEPTION(ERR_UNSUPPORT_CKV_KEYNO, "op_type invalid");
            break;

    }   
	if(!ret)
	{
		throw EXCEPTION(ERR_SET_CKV, "写入ckv 失败");
	}
	
}

/**
  * 检查参数
  */
void FundUpdateKvcache::CheckParams() throw (CException)
{
	int op_type = m_params.getInt("op_type");
	if( CKV_KEY_UIN == op_type || CKV_KEY_TRADE_ID  == op_type || CKV_KEY_FUND_SUPPORT_SP_ALL ==  op_type || 
		CKV_KEY_FUND_SUPPORT_BANK_ALL ==  op_type || CKV_KEY_PROFIT_RATE ==  op_type || CKV_KEY_PAY_CARD ==  op_type || 
		UPDATE_KVCACHE_KEY_VALUE ==  op_type)
	{
		return;
	}
	
	//新增的类型必须验证watch_word，历史的先不验，等调用方增加了在验证  
    if (m_servicetype == CHECK_LOGIN)
    {
    	if(m_params.getString("key").empty() )
		{
			throw EXCEPTION(ERR_BAD_PARAM, "empty key is not allow. ");
		}
		if( CKV_KEY_FUND_USER_ACC !=  op_type && CKV_KEY_UIN != op_type && CKV_KEY_TDAY != op_type&& CKV_KEY_UNFINISH_INDEX!= op_type)
		{
			throw EXCEPTION(ERR_BAD_PARAM, "only op_type=1,25,38,39,40 is allow.");
		}
        checkSession(m_params["qlskey"], m_params["qluin"], "100581");  //第三个参数是附带参数，这里填写request_type
    }
	else if("e10adc3949ba59abbe56e057f20f883e" != m_params.getString("watch_word"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "watch_word error"); 
	}
	
}


bool FundUpdateKvcache::setProfitRateToKV()
{
	vector<FundSpConfig> fundSpConfigVec;
	if( !queryFundSpAllConfig(m_pFundCon, fundSpConfigVec, false))
	{
		TRACE_DEBUG("no fund sp config");
		return true;
	}

	//更新全部基金代码的收益率
	for(vector<FundSpConfig>::size_type i= 0; i != fundSpConfigVec.size(); ++i)
	{
		FundSpConfig fundSpConfig = fundSpConfigVec[i];

		if(fundSpConfig.Flstate != LSTATE_VALID)
		{
			continue;
		}

		FundProfitRate data;
		memset(&data, 0,sizeof(FundProfitRate));

		strncpy(data.Fspid,fundSpConfig.Fspid, sizeof(data.Fspid) - 1);
		strncpy(data.Ffund_code,fundSpConfig.Ffund_code, sizeof(data.Ffund_code) - 1);

		if(!setSpProfitRateToKV(m_pFundCon,data))
		{
			return false;
		}
		if(!setMultiSpProfitRateToKV(m_pFundCon,data))
		{
			return false;
		}

	}
	
	if(!setAllLastSpProfitRateToKV(m_pFundCon))
	{
		return false;
	}

	//更新最高收益率记录信息
	FundProfitRate data;
	memset(&data, 0,sizeof(FundProfitRate));
	
	strncpy(data.Fdate,toString(GetDateToday()).c_str(), sizeof(data.Fdate) - 1);
	
	if(!setHighestProfitRateSpToKV(m_pFundCon,data))
	{
		return false;
	}

	return true;
}

bool FundUpdateKvcache::setKeyValue()
{
	string key = m_params.getString("key");
	string value = m_params.getString("value");
    //更新CKV接口传入key-value更新调ckv接口更新,此处key no记为op_type的负数值
	if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_UNKNOWN_KEYNO), key, value))
	{
		return false;
	}
	
	return true;
}

bool FundUpdateKvcache::setV2KeyValue()
{
	string key = m_params.getString("key");
	string value = m_params.getString("value");
	int expire = m_params.getInt("expire");
	if(gCkvSvrOperator->set_v2_with_get_v2(NOT_REUPDATE_CKV(CKV_KEY_UNKNOWN_KEYNO), key, value, expire))
	{
		return false;
	}
	
	return true;
}


bool FundUpdateKvcache::delKeyValue()
{
	string key = m_params.getString("key");
	TRACE_NORMAL("FundUpdateKvcache delete ckv key .key=[%s]", key.c_str());//记录每一笔通过该接口删除的操作
	
	if(gCkvSvrOperator->del(NOT_REUPDATE_CKV(CKV_KEY_UNKNOWN_KEYNO), key))
	{
		return false;
	}
	
	return true;
}

bool FundUpdateKvcache::setPayCardInfoToKV()
{
	if(m_params.getString("key").empty())
	{
		return false;
	}
	
	FundPayCard fund_pay_card;
	memset(&fund_pay_card, 0, sizeof(FundPayCard));
	
	strncpy(fund_pay_card.Fqqid, m_params.getString("key").c_str(), sizeof(fund_pay_card.Fqqid) - 1);

	if(!setPayCardToKV(m_pFundCon, fund_pay_card, true))
	{
		return false;
	}

	return true;
}

bool FundUpdateKvcache::setFundTotalaccInfoToKV()
{
 //   if(m_params.getString("key").empty())
	//{
	//	return false;
	//}
	//
	//FundUserTotalAcc fundUserTotalAcc;
	//memset(&fundUserTotalAcc, 0 ,sizeof(FundUserTotalAcc));

	//strncpy(fundUserTotalAcc.Ftrade_id, m_params.getString("key").c_str(), sizeof(fundUserTotalAcc.Ftrade_id)-1);
	//fundUserTotalAcc.Fbusiness_type = 2; // 2：余额增值业务(默认)

	//if(!setFundTotalaccToKV(fundUserTotalAcc,true))
	//{
	//	return false;
	//}

	return true;
}


/**
 * 更新用户总收益ckv
 * @return 
 */
bool FundUpdateKvcache::setUserTotalProfitToKV()
{
	if(m_params.getString("key").empty())
	{
		return false;
	}

    int uid = m_params.getInt("key");

    //根据uid查询trade_id
    ST_FUND_BIND userFundBinRec;
    memset(&userFundBinRec, 0 , sizeof(userFundBinRec));
    bool ret = QueryFundBindByUid(m_pFundCon, uid, &userFundBinRec, false);
    if (!ret)
        return false;
    
    FundProfit fundProfitRecord;
    memset(&fundProfitRecord, 0 , sizeof(fundProfitRecord));
    SCPY(fundProfitRecord.Ftrade_id, userFundBinRec.Ftrade_id);

    vector<FundProfit> fundProfitVec;
	ret = setTotalProfit(fundProfitRecord, uid, DEF_USR_TOTAL_PROFIT_CKV_TIMEOUT, fundProfitVec);

    return ret;
}

/**
 * 更新用户资产ckv记录
 * @return 
 */
bool FundUpdateKvcache::setUserProfitRecordsToKV()
{
	if(m_params.getString("key").empty())
	{
		return false;
	}

    FundProfitRecord fundProfitRecord;
    memset(&fundProfitRecord, 0 , sizeof(fundProfitRecord));
    SCPY(fundProfitRecord.Ftrade_id, m_params.getString("key").c_str());
    
	if(!setFundProfitRecordToKV(m_pFundCon, fundProfitRecord))
	{
		return false;
	}

	return true;
}

/**
 * 更新用户收益流水ckv记录
 * @return 
 */
bool FundUpdateKvcache::setUserSubAccToKV()
{
	vector<string> keyVec = split(m_params.getString("key"),"_");
	bool needUpdate=true;
	// 临时代码, 提供不更新CKV只检查数据的渠道,后续可以删除
	string value = m_params.getString("value");
	if(value=="query"){
		needUpdate=false;
	}
	if(keyVec.size()>1){
		// key为tradeId_SPID 更新trade_id,SPID下的所有交易
		return addUseSubAccToKV(m_pFundCon,keyVec[0],keyVec[1]);
	}else{			
		// key为trade_id, 更新trade_id下的所有交易
		return setUserAccToKV(m_pFundCon, keyVec[0],needUpdate);
	}
	return false;
}

/**
 * 更新用户定期交易ckv记录
 * @return 
 */
bool FundUpdateKvcache::setUserCloseTransToKV()
{
	vector<string> keyVec = split(m_params.getString("key"),"_");
	// key为tradeId_fundCode 
	if(keyVec.size()!=2){
		gPtrAppLog->debug("setUserCloseTransToKV:keyVec.size()=%zd, not equal to 2.usage:tradeId_fundCode",keyVec.size());
		return false;
	}
	return setFundCloseTransToKV(keyVec[0],keyVec[1]);
}

/**
 * 更新用户新最21条交易记录,前缀+trade_id
 * @return 
 */
bool FundUpdateKvcache::setUserFundTradeToKV()
{
    //如果key是user_latest_fund_trade_前缀，则去掉前缀
    string key = m_params.getString("key");
    if (!key.empty()) {
        if (0 == key.find("user_latest_fund_trade_")) {
            key = key.substr(strnlen("user_latest_fund_trade_", 24));
            m_params.setParam("key", key);
        }
    }

    string trade_id = m_params.getString("key");
    
    if(trade_id.empty()) {
		return false;
	}

    //根据trade_id查询uid
    ST_FUND_BIND userFundBinRec;
    memset(&userFundBinRec, 0 , sizeof(userFundBinRec));
    bool ret = QueryFundBindByTradeid(m_pFundCon, trade_id.c_str(), &userFundBinRec, false);
    if (!ret)
        return false;

    ST_TRADE_FUND trade_fund;
    memset(&trade_fund, 0, sizeof(trade_fund));
    trade_fund.Fstate = 3;
    trade_fund.Fuid = userFundBinRec.Fuid;
    SCPY(trade_fund.Ftrade_id, trade_id.c_str());
    
    ret = setTradeRecordsToKV(m_pFundCon, trade_fund);
    
    return ret;
}

bool FundUpdateKvcache::updateUserWhiteList()
{
    return updateUserWhiteListValue(m_params.getString("key"),m_params.getString("value"));
}


bool FundUpdateKvcache::setFundUnconfirmCKV()
{
    return setFundUnconfirm(m_pFundCon, m_params.getString("key"));
}


bool FundUpdateKvcache::setUnfinishTransCKV()
{
    return setFundUnfinishTransCKV(m_pFundCon, m_params.getString("key"));
}
bool FundUpdateKvcache::setFundCashInTransitCKV()
{
    return setCashInTransitCKV(m_pFundCon, m_params.getString("key"));
}


/**
 * 将基金交易日信息写入ckv
 * @param pMysql 
 * @param cur_day 
 * @return 
 */
bool FundUpdateKvcache::setTDayToKV()
{
	string cur_day=m_params.getString("key");
	if(cur_day.empty())
	{
		cur_day = toString(GetDateToday());
	}

	string sTransDate, sNextTransDate, sLastTransDate, sMemo, sBanTradeTime;
    vector<FundLastAndNextTransDate> list;
    queryLastAndNextDate(gPtrFundSlaveDB, cur_day, list, true);

    TRACE_DEBUG("list size=%zd", list.size());
    
    vector<FundLastAndNextTransDate>::const_iterator cite;
    for (cite = list.begin(); cite != list.end(); ++cite) {
        if (0 == strlen(cite->Fdate)
            || 0 == strlen(cite->Flast_date)
            || 0 == strlen(cite->Fnext_date)) {
            TRACE_DEBUG("queryLastAndNextDate Fdate=%s Flast_date=%s Fnext_date=%s", cite->Fdate, 
				cite->Flast_date, cite->Fnext_date);
            
    		continue;
        }
    	
    	sTransDate = cite->Fdate;
	sNextTransDate = cite->Fnext_date;
	sLastTransDate = cite->Flast_date;
	sMemo = cite->Fmemo;
	sBanTradeTime = cite->Fstandby3;
	break;
    }

	if(cite == list.end())
	{
		return false;
	}

	vector<FundLastAndNextTransDate> hklist;
	queryHKLastAndNextDate(gPtrFundSlaveDB, cur_day, hklist, true);

    	vector<FundLastAndNextTransDate>::const_iterator hkcite;
      for (hkcite = hklist.begin(); hkcite != hklist.end(); ++hkcite) 
     {
        if (0 == strlen(hkcite->Fdate)|| 0 == strlen(hkcite->Fnext_date))
	 {   
	    TRACE_DEBUG("queryHKLastAndNextDate Fdate=%s Fnext_date=%s", hkcite->Fdate, hkcite->Fnext_date);
    	     continue;
        }
        
        CParams kvReqSet;
	
        kvReqSet.setParam("Ftrans_date", sTransDate);
	 kvReqSet.setParam("Fnext_trans_date", sNextTransDate);
	 kvReqSet.setParam("Flast_trans_date", sLastTransDate);
	 kvReqSet.setParam("Fhk_trans_date", hkcite->Fdate);
	 kvReqSet.setParam("Fhk_next_trans_date", hkcite->Fnext_date);
	 kvReqSet.setParam("Fban_trade_time", "");
	 kvReqSet.setParam("Fmemo", sMemo);

        string key = "tday_" + cur_day;
        string value = kvReqSet.pack();
	TRACE_DEBUG("setTDayToKV key=%s value=%s", key.c_str(), value.c_str());
        if(gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_TDAY), key, value))
    	 { 
    		return false;
    	 }    	
    	}

    return true;
}


/**
  * 打包输出参数
  */
void FundUpdateKvcache::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
    return;
}


