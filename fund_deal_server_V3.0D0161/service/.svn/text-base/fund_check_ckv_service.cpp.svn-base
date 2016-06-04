#include "fund_check_ckv_service.h"

/* ==========收益流水对账内部函数============= */

/**
 * 判断记录是在列表中
 */
template<typename T>
bool for_each_chk(const T& rec, const vector<T> &list)
{
   typedef typename vector<T>::const_iterator CIteType;
     
    for (CIteType cite = list.begin(); cite != list.end(); ++cite) {
        if (chk_record(rec, *cite))
            return true;
    }

    return false;
}

/**
 * 比较两条收益流水记录是否相同
 * @param rec1 
 * @param rec2 
 * @return true-相同,false-不相同,记录唯一索引相同，但字段不同抛异常
 */
bool chk_record(const FundProfitRecord& rec1, const FundProfitRecord& rec2)
{
    /* 只比较关键字段是否相同 */
    //单号不同则认为记录不同
    if (0 != strcmp(rec1.Flistid, rec2.Flistid))
        return false;
        
    //单号相同，其他字段不相同则认为不一致，抛异常    
    if (rec1.Fprofit != rec2.Fprofit)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record Fprofit info inconsistent in ckv and db");

    if (rec1.Ftotal_profit != rec2.Ftotal_profit)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record Ftotal_profit info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Fspid, rec2.Fspid))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record Fspid info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Ftrade_id, rec2.Ftrade_id))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record Ftrade_id info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Fday, rec2.Fday))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record Fday info inconsistent in ckv and db");
    
    return true;
}

/**
 * 比较两条定期记录是否相同
 * @param rec1 
 * @param rec2 
 * @return true-相同,false-不相同。记录唯一索引相同，但字段不同抛异常
 */
bool chk_record(const FundCloseTrans& rec1, const FundCloseTrans& rec2)
{
    /* 只比较关键字段和会变更的字段是否相同 */
    //Fid不相同则认为是不同记录
    if (rec1.Fid != rec2.Fid)
        return false;
	
		//Fid相同，其他字段不相同则认为不一致，抛异常    
    if (rec1.Fstate != rec2.Fstate)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fstate info inconsistent in ckv and db");

    if (rec1.Fuser_end_type != rec2.Fuser_end_type)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fuser_end_type info inconsistent in ckv and db");   
    
    if (rec1.Fstart_total_fee != rec2.Fstart_total_fee)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fstart_total_fee info inconsistent in ckv and db");

    if (rec1.Fcurrent_total_fee != rec2.Fcurrent_total_fee)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fcurrent_total_fee info inconsistent in ckv and db");

    if (rec1.Fend_tail_fee != rec2.Fend_tail_fee)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fend_tail_fee info inconsistent in ckv and db");

    if (rec1.Fend_real_sell_amt != rec2.Fend_real_sell_amt)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fend_real_sell_amt info inconsistent in ckv and db");

    if (rec1.Flast_profit != rec2.Flast_profit)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Flast_profit info inconsistent in ckv and db");

    if (rec1.Ftotal_profit != rec2.Ftotal_profit)
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Ftotal_profit info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Ftrade_id, rec2.Ftrade_id))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Ftrade_id info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Fspid, rec2.Fspid))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fspid info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Fprofit_end_date, rec2.Fprofit_end_date))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fprofit_end_date info inconsistent in ckv and db");

    if (0 != strcmp(rec1.Fend_date, rec2.Fend_date))
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans record Fend_date info inconsistent in ckv and db");

    return true;
}

/* ==========收益流水对账内部函数============= */


FundCheckCkv::FundCheckCkv(CMySQL* mysql)
    :m_pSlaveFundDb(mysql)
{
}

FundCheckCkv::~FundCheckCkv()
{
}

void FundCheckCkv::parseInputMsg(TRPC_SVCINFO* rqst) throw (CException)
{
    char *pMsg = (char*)(rqst->idata);
    
    // 读取参数
    m_params.readStrParam(pMsg, "ckv_key", 1, 255);
    m_params.readIntParam(pMsg, "ckv_key_no", 1,MAX_INTEGER);
    m_params.readIntParam(pMsg, "bid", 1,MAX_INTEGER);
    m_params.readIntParam(pMsg, "check_type", 1,MAX_INTEGER);

    if (FUND_CKV_CHK_TYPE_INC_CHK == m_params.getInt("check_type")) {
        m_params.readStrParam(pMsg, "start_time", 1, 20);
        m_params.readStrParam(pMsg, "end_time", 1, 20);
    }
    
    m_params.readStrParam(pMsg, "watch_word", 1,64);
    m_params.readStrParam(pMsg, "client_ip", 1,20);
    m_params.readStrParam(pMsg, "token", 32,64);
}

void FundCheckCkv::checkParams() throw (CException)
{
    if("af9b62fdb2098b1fd22a4668287e4ba3" != m_params.getString("watch_word"))
	{
		throw EXCEPTION(ERR_BAD_PARAM, "watch_word error"); 
	}

    checkToken();
}

void FundCheckCkv::checkToken() throw (CException)
{
    //ckv_key|ckv_key_no|bid|check_type|start_time|end_time|key
    stringstream ss;
    ss << m_params["ckv_key"] << "|" ;
    ss << m_params["ckv_key_no"] << "|" ;
    ss << m_params["bid"] << "|" ; 
    ss << m_params["check_type"] << "|" ;
    ss << m_params["start_time"] << "|" ;
    ss << m_params["end_time"] << "|" ;
    ss << "2fa0aaa0a0a9e1279b7a952eb97e021e";

    string strmd5 = getMd5(ss.str());
    string token = m_params.getString("token");
    toUpper(strmd5);
    toUpper(token);

    if (strmd5 != token) {
        throw EXCEPTION(ERR_BAD_PARAM, "invalid token"); 
    }
}

void FundCheckCkv::excute()  throw (CException)
{
	checkParams();
    
    int key_no = m_params.getInt("ckv_key_no");
    int check_type = m_params.getInt("check_type");

    string no_prefix_key = adapt_ckv_key(key_no, m_params["ckv_key"]);
    m_params.setParam("no_prefix_key", no_prefix_key);
    
    TRACE_DEBUG("no_prefix_key=%s", no_prefix_key.c_str())

    ckv_result_t ckv_res;
    ckv_res.key = m_params["ckv_key"];
    ckv_res.result = gCkvSvrOperator->get(ckv_res.key, ckv_res.value);
    if (0 != ckv_res.result && ERR_KEY_NOT_EXIST != ckv_res.result) {
        string errinfo = cxx_printf("get ckv failed:%d", ckv_res.result);
        throw CException(ERR_GET_CKV_FAILED, errinfo, __FILE__, __LINE__);
    }
    
	switch (check_type)
	{
        case FUND_CKV_CHK_TYPE_OVERALL:
            overAllCheck(key_no, ckv_res, no_prefix_key);
            break;
        case FUND_CKV_CHK_TYPE_INC_CHK:
            incCheck(key_no, ckv_res, no_prefix_key);
            break;
        default:
            throw CException(ERR_UNSPPORT_CKV_CHKTYPE, "unsupport ckv check type", __FILE__, __LINE__);
            break;
	}
}

void FundCheckCkv::incCheck(int key_no, const ckv_result_t &ckv_res, const string &no_prefix_key)
{
    switch (key_no)
    {
		case CKV_KEY_PROFIT_RECORD:
            incChkUsrProfitRecord(ckv_res, no_prefix_key);
            break;
            
        default:
            throw CException(ERR_UNSUPPORT_CKV_KEYNO, "unsupport increment ckv key no", __FILE__, __LINE__);
            break;
	}
}
void FundCheckCkv::overAllCheck(int key_no, const ckv_result_t &ckv_res, const string &no_prefix_key)
{
    switch (key_no)
    {
		case CKV_KEY_UIN:
            allChkUsrFundBind(ckv_res, no_prefix_key);
            break;
            
        case CKV_KEY_TRADE_ID:
            allChkUsrBindSp(ckv_res, no_prefix_key);
            break;

        case CKV_KEY_PAY_CARD:
            allChkUsrPayCard(ckv_res, no_prefix_key);
            break;

        case CKV_KEY_FUND_CLOSE_TRANS:
            allChkUsrCloseTrans(ckv_res, no_prefix_key);
            break;

        case CKV_KEY_USER_LATEST_FUND_TRADE:
            allChkUsrFundTrade(ckv_res, no_prefix_key);
            break;

        case CKV_KEY_UNCONFIRM:
            allChkUnconfirm(ckv_res, no_prefix_key);
            break;
        case CKV_KEY_UNFINISH_INDEX:
            allChkUnfinishTrans(ckv_res, no_prefix_key);			
            break;
		case CKV_KEY_CASH_IN_TRANSIT:
			allChkCashInTransit(ckv_res, no_prefix_key);
			break;
        default:
            throw CException(ERR_UNSUPPORT_CKV_KEYNO, "unsupport over all cehck ckv key no", __FILE__, __LINE__);
            break;
	}
}

void FundCheckCkv::allChkUsrFundBind(const ckv_result_t &ckv_res, const string &uin)
{
    ST_FUND_BIND rec;
    memset(&rec, 0, sizeof(rec));
    
    string szValue;
    bool ret = QueryFundBindByUin(m_pSlaveFundDb, uin, &rec, false);
    if (true == ret) {
        packUsrFundBindCkvValue(rec, szValue);
    }

    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user fund bind info inconsistent in ckv and db");
    }
}

void FundCheckCkv::allChkUsrBindSp(const ckv_result_t &ckv_res, const string &trade_id)
{
    string szValue;
    vector<FundBindSp> list;
    bool ret = queryFundBindAllSp(m_pSlaveFundDb, list, trade_id, false);
    if (true == ret) { 
        packUsrBindSpCkvValue(list, szValue);
    }

    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user bind sp info inconsistent in ckv and db");
    }
}

void FundCheckCkv::allChkUsrPayCard(const ckv_result_t &ckv_res, const string &uin)
{
    string szValue;
    FundPayCard data;
    memset(&data, 0, sizeof(data));	
	SCPY(data.Fqqid, uin.c_str());
    
    bool ret = queryFundPayCard(m_pSlaveFundDb, data, false);
    if (true == ret) {
        /**
         * 如果安全卡的key不存在，且db中绑定序列号为空则认为数据一致
         * 考虑到用户的安全卡清空的场景
         **/
        string bind_serialno = data.Fbind_serialno;
        if (ckv_res.value.empty() && bind_serialno.empty())
            return;
        
        packUsrPayCardCkvValue(data, szValue);
    }
    
    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user pay card info inconsistent in ckv and db");
    }
}

void FundCheckCkv::allChkUsrCloseTrans(const ckv_result_t &ckv_res, const string &tradeid_fundcode)
{
    vector<string> keyVec = split(tradeid_fundcode, "_");
    if(keyVec.size() != 2){
		TRACE_ERROR("user close trans key invalid:keyVec.size()=%zd, not equal to 2.usage:tradeId_fundCode",keyVec.size());
		throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user close trans key invalid");
	}
    
    FundCloseTrans fundCloseTrans;
    memset(&fundCloseTrans, 0, sizeof(FundCloseTrans));
        
    SCPY(fundCloseTrans.Ftrade_id, keyVec[0].c_str());
    SCPY(fundCloseTrans.Ffund_code, keyVec[1].c_str());
    SCPY(fundCloseTrans.Fprofit_end_date, toString(GetDateToday()).c_str());

    //查询DB数据
    vector<FundCloseTrans> db_trans_list;
    bool ret = queryFundCloseTransWithProfitEndDate(m_pSlaveFundDb, 0, 
                    CACHE_CLOSE_TRANS_MAX_NUM, fundCloseTrans, db_trans_list, false);
    //如查db结果数据为空
    if (false == ret) {
        if (!ckv_res.value.empty()) {
            //db数据为空，ckv数据为空,认为不一致
            TRACE_DEBUG("ckv[%s],db[empty]", ckv_res.value.c_str());
            throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user cose trans info inconsistent in ckv and db");
        } else {
            return;
        }
    }

    //解析CKV数据
    vector<FundCloseTrans> ckvRecList;
    int retcode = parseFundCloseTransCkvValue(ckv_res.value, ckvRecList);    
    if (0 != retcode) {        
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user cose trans info in ckv format incorrect");
    }

    //如果CKV中的记录数与db中的记录数不同则认不一致
    if (ckvRecList.size() != db_trans_list.size()) {
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user cose trans size not eq in ckv and db");
    }

    //循环db中的数据，判断是否在ckv中
    for (vector<FundCloseTrans>::const_iterator cite = db_trans_list.begin(); cite != db_trans_list.end(); ++cite) {
        if (!for_each_chk(*cite, ckvRecList)) {
            TRACE_DEBUG("db close trans[Fid=%ld] not in ckv", cite->Fid);
            throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user cose trans info inconsistent in ckv and db");
        }
    }
}

void FundCheckCkv::allChkUsrFundTrade(const ckv_result_t &ckv_res, const string &tradeid)
{
    //根据trade_id查询uid
    ST_FUND_BIND userFundBinRec;
    memset(&userFundBinRec, 0 , sizeof(userFundBinRec));
    bool ret = QueryFundBindByTradeid(m_pSlaveFundDb, tradeid.c_str(), &userFundBinRec, false);
    if (!ret)
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist! ", __FILE__, __LINE__);;
    
    ST_TRADE_FUND trade_fund;
    memset(&trade_fund, 0, sizeof(trade_fund));        
    SCPY(trade_fund.Ftrade_id, tradeid.c_str());
    trade_fund.Fuid = userFundBinRec.Fuid;
    
    string szValue;
    vector<ST_TRADE_FUND> tradeFundVec;
    ret = QueryBatchTradeFund(m_pSlaveFundDb, trade_fund, tradeFundVec, false);
    
    if (true == ret) {
        packTradeRecordsCkvValue(tradeFundVec, szValue);;
    }

    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user fund trade info inconsistent in ckv and db");
    }
}

void FundCheckCkv::allChkUnconfirm(const ckv_result_t &ckv_res, const string &tradeid)
{
    string szValue;
    vector<FUND_UNCONFIRM> dataVec;
    queryValidFundUnconfirmByTradeId(m_pSlaveFundDb,tradeid.c_str(),dataVec);
    
    packFundUnconfirm(dataVec, szValue);;

    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "unconfirm info inconsistent in ckv and db");
    }
}


void FundCheckCkv::allChkUnfinishTrans(const ckv_result_t &ckv_res, const string &tradeid)
{
    string szValue;
    vector<FundTransProcess> dataVec;
    queryUnfinishTransByTradeId4CKV(m_pSlaveFundDb,tradeid.c_str(),dataVec);
    
    packFundUnfinishTransCKV(dataVec, szValue);;

    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "unconfirm info inconsistent in ckv and db");
    }
}


void FundCheckCkv::allChkCashInTransit(const ckv_result_t &ckv_res, const string &tradeid)
{
    string szValue;
    vector<CashInTransit> dataVec;
    queryCashInTransitBAFetch4CKV(m_pSlaveFundDb,tradeid.c_str(),dataVec);
    
    packCashInTransitCKV(dataVec, szValue);;

    if (ckv_res.value != szValue) {
        TRACE_DEBUG("ckv[%s],db[%s]", ckv_res.value.c_str(), szValue.c_str());
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "CashInTransit info inconsistent in ckv and db");
    }
}

void FundCheckCkv::incChkUsrProfitRecord(const ckv_result_t &ckv_res, const string &trade_id)
{   
    /* 查询db数据 */
    vector<FundProfitRecord> dblist;
    bool ret = queryFundProfitRecordByTime(m_pSlaveFundDb, trade_id, m_params["start_time"],
                    m_params["end_time"], dblist);

    /* 如果db中未查询到数据，直接返回 */
    if (ret != true)
        return;

    /* 解析ckv数据 */
    vector<FundProfitRecord> ckvRecList;
    if (!ckv_res.value.empty()) {
        parseProfitRecordCkvValue(ckv_res.value, ckvRecList);
    } else {
        throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record info inconsistent in ckv and db");
    }

    /* 获取ckv中Fday的最小值 */
    string min_day = "99999999";
    for (vector<FundProfitRecord>::const_iterator cite = ckvRecList.begin(); cite != ckvRecList.end(); ++cite) {
        if (min_day > cite->Fday)
            min_day = cite->Fday;
    }

    /* 如果ckv中数据列表为空，则将min_day置为默认的最小值 */
    if ("99999999" == min_day)
        min_day = "00000000";

    /* 将db中的每条数据在ckv去对比是否存在 */    
    for (vector<FundProfitRecord>::const_iterator cite = dblist.begin(); cite != dblist.end(); ++cite) {
        if (!for_each_chk(*cite, ckvRecList)) {
            /**
             * 如果db数据在ckv中不存在，且Fday比ckv最小值还大,则说明db中的数据比ckv新但不在ckv中，报错 
             * 如果db数据不在ckv中,但ckv中的记录数未达到最大,则表示db中的数据应该在ckv中，报错
             */
            if (cite->Fday > min_day
                || ckvRecList.size() < CACHE_USR_PROFIT_REC_MAX_NUM)
                throw EXCEPTION(ERR_CKV_DB_INCONSISTENT, "user profit record info inconsistent in ckv and db");
        }

        TRACE_DEBUG("[incChk]profit record list(%s) in ckv", cite->Flistid);
    }
}

void FundCheckCkv::packReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");

    rqst->olen = strlen(rqst->odata);
}

