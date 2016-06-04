/**
  * FileName: fund_comm_qry_chklgi.cpp
  * Author: louisjiang	
  * Version :1.0
  * Date: 2014-05-15
  * Description: 基金交易服务 查询serverice 校验登录态
  */

#include "fund_commfunc.h"
#include "fund_comm_qry_chklgi.h"
#include "tools_comm.h"
#include "dbsign.h"

#define ADD_CONDITION(SQLBUF,FIELD_NAME,PARA_NAME,MAX_VALUE_LEN,SIGN)  {string buf; \
       if (m_fieldsMap.find(PARA_NAME)!=m_fieldsMap.end()) { \
          if(m_fieldsMap[PARA_NAME].length()>MAX_VALUE_LEN){throw CException(ERR_BAD_PARAM, string("Param's length is out of range:") + PARA_NAME + "=" + m_fieldsMap[PARA_NAME], __FILE__, __LINE__);}\
          buf+=" AND "; buf+=FIELD_NAME; buf+=SIGN; buf+="'"; buf+=escapeString(m_fieldsMap[PARA_NAME]).c_str();buf+="'";strncpy(SQLBUF+strlen(SQLBUF),buf.c_str(),buf.length()+1); \
}}

#define ADD_ORDERBY(SQLBUF,FIELD_NAME) {string buf; \
        buf +=" ORDER BY ";buf +=FIELD_NAME;\
        if (m_params.getInt("order_flag")==0){buf +=" DESC ";}\
        buf +=" LIMIT  ";buf += m_params["offset"];buf +=",";buf += m_params["limit"]; \
        strncpy(SQLBUF+strlen(SQLBUF),buf.c_str(),buf.length()+1); \
}

#define ADD_IN(SQLBUF,FIELD_NAME,PARA_NAME,MAX_VALUE_LEN)  {string buf; \
       if (m_fieldsMap.find(PARA_NAME)!=m_fieldsMap.end()) { \
          if(m_fieldsMap[PARA_NAME].length()>MAX_VALUE_LEN){throw CException(ERR_BAD_PARAM, string("Param's length is out of range:") + PARA_NAME + "=" + m_fieldsMap[PARA_NAME], __FILE__, __LINE__);}\
          buf+=" AND "; buf+=FIELD_NAME; buf+=" in ("; buf+=escapeString(m_fieldsMap[PARA_NAME]).c_str();buf+=") ";strncpy(SQLBUF+strlen(SQLBUF),buf.c_str(),buf.length()+1); \
}}

FundComQryChkLgi::FundComQryChkLgi(CMySQL* mysql,int type)
{
    m_fund_conn = gPtrFundSlaveDB; //查询服务统一查询备机
    memset(&m_fund_bind,0,sizeof(m_fund_bind));
    m_servicetype = type;
}


/**
  * service step 1: 解析输入参数
  */
void FundComQryChkLgi::parseInputMsg(TRPC_SVCINFO* rqst)  throw (CException)
{
    char *pMsg = (char*)(rqst->idata);
    // 读取参数
    m_params.readStrParam(pMsg, "qlskey", 0, 64);
    m_params.readStrParam(pMsg, "qluin", 1, 64);//为兼容手Q和微信,此值长度可能小于10
    m_params.readIntParam(pMsg, "reqid", 1, 1000);
    m_params.readIntParam(pMsg, "limit", 0, 21);
    m_params.readIntParam(pMsg, "offset", 0,MAX_INTEGER);
    m_params.readIntParam(pMsg, "order_flag", 0, 1); //0 降序 1 升序
    m_params.readIntParam(pMsg, "uid", 0,MAX_INTEGER);
    m_params.readStrParam(pMsg, "fields", 0, 4096);

    Tools::StrToMap(m_fieldsMap,m_params["fields"],"|",":",true);
    Tools::MapToStr(m_fieldsMap, m_fields_nv_value);
    
    if (m_params.getInt("limit") == 0)
    {
        m_params.setParam("limit", 1);
    }
    if (m_params.getInt("offset") == 0)
    {
        m_params.setParam("offset", 0);
    }
}


/**
  * 检查参数，获取内部参数
  */
void FundComQryChkLgi::CheckParams() throw (CException)
{
    //微信王府井临时方案below
    if ((m_params.getInt("reqid")) == COMMQRY_TYPE_RPOFITLIST && m_fieldsMap.find("spid")!=m_fieldsMap.end()&&m_fieldsMap["spid"]==gPtrConfig->m_AppCfg.wx_wfj_spid)
    {
        return;
    }
    //微信王府井临时方案above

    if (m_servicetype == CHECK_LOGIN)
    {
        if (m_params.getString("qlskey").length() <= 0)
        {
            throw CException(ERR_BAD_PARAM, string("Param's length is out of range:qlskey="), __FILE__, __LINE__);
        }
        checkSession(m_params["qlskey"], m_params["qluin"], "100087");
    }
}

string  FundComQryChkLgi::buildTransQuerySql()  throw (CException)
{
    char    szSqlStr[MAX_SQL_LEN+1]={0};

    m_params.readStrParam(m_fields_nv_value.c_str(), "listid", 10, 64);
    m_params.readIntParam(m_fields_nv_value.c_str(), "pur_type", 1, 100);
    
    snprintf(szSqlStr, sizeof(szSqlStr), 
        "SELECT Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, "
                   " Fpur_type, Ftotal_fee, Fbank_type, Fcard_no, Fstate, Flstate, "
                   " Ftrade_date, Ffund_value, Ffund_vdate, Ffund_type, "
                   " Frela_listid, Fcreate_time, Fmodify_time, Ffetchid, Fcft_charge_ctrl_id, Fsp_fetch_id,Fcft_bank_billno, "
                   " Fspe_tag, Facc_time,Fpurpose,Floading_type,Fcft_trans_id,Fclose_listid,Fopt_type,Freal_redem_amt,Fend_date,Fsign  "
                   " FROM  fund_db_%02d.t_trade_fund_%d "
                   " WHERE Flistid='%s'   AND Fpur_type=%d  AND Fuid=%d ", 
        Sdb2(m_params["listid"]), Stb2(m_params["listid"]), escapeString(m_params["listid"]).c_str(),m_params.getInt("pur_type"),m_fund_bind.Fuid);

    return szSqlStr;
    
}



string  FundComQryChkLgi::buildProfitListQuerySql()  throw (CException)
{
    //微信王府井临时方案below
    if (m_fieldsMap.find("spid")!=m_fieldsMap.end() && m_fieldsMap["spid"]==gPtrConfig->m_AppCfg.wx_wfj_spid)
    {
        m_fieldsMap["trade_id"] = m_fund_bind.Ftrade_id;
        m_fieldsMap["wx_wfj_sp_id"] = gPtrConfig->m_AppCfg.wx_wfj_spid;
        m_params.setParam("trade_id", m_fund_bind.Ftrade_id);
        m_fieldsMap["spid"] = gPtrConfig->m_AppCfg.consum_fund_spid;
    }
    //微信王府井临时方案above
    else
    {
        m_params.readStrParam(m_fields_nv_value.c_str(), "trade_id", 1, 100);
    }
    if (m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        throw CException(ERR_SESSION_PARA_CHECK, "trade_id check fail with funddb", __FILE__, __LINE__);
    }
    char    szSqlStr[MAX_SQL_LEN+1]={0};
    
    snprintf(szSqlStr, sizeof(szSqlStr), 
        "SELECT Flistid,Fsub_trans_id,Ftrade_id,Fcurtype,Fspid, "
                   " Fvalid_money,Fstop_money,Fprofit,Fday,Fprofit_type, "
                   " F1day_profit_rate,F7day_profit_rate,Flogin_ip,Fsign,Fcreate_time, "
                   " Fmodify_time,Fmemo,Ftotal_profit  "
                   " FROM  fund_db_%02d.t_fund_profit_record_%d "
                   " WHERE Ftrade_id='%s'   AND Fprofit>0  ", 
        Sdb2(m_params["trade_id"]), Stb2(m_params["trade_id"]), escapeString(m_params["trade_id"]).c_str());
   
    ADD_CONDITION(szSqlStr,"Fcreate_time","begin_time",20,">=") 
    ADD_CONDITION(szSqlStr,"Fcreate_time","end_time",20,"<=") 
    ADD_CONDITION(szSqlStr,"Fspid","spid",15,"=") 
    ADD_CONDITION(szSqlStr,"Flistid","listid",32,"=") 
    ADD_CONDITION(szSqlStr,"Fstandby3","wx_wfj_sp_id",32,"=") 
    ADD_ORDERBY(szSqlStr,"Fday")
        
    return szSqlStr;
    
}

string  FundComQryChkLgi::buildWeekListQuerySql()  throw (CException)
{
    TRACE_DEBUG("buildWeekListQuerySql");
    checkUid();
    char    szSqlStr[MAX_SQL_LEN+1]={0};
    
    snprintf(szSqlStr, sizeof(szSqlStr), 
        "SELECT Fpk_id,Flistid ,Fbill_id,Fbill_type,Fbill_start_date,Fbill_end_date, "
                   " Fbill_trade_stime ,Fbill_trade_etime ,Ftrade_id ,Fuid, Fopenid, "
                   " Fspid ,Ffund_code,Ftotal_buy_amt,Ftotal_redeem_amt ,Ftotal_profit_amt ,Fbuy_num, "
                   " Fredeem_num ,Ffund_balance ,Ffund_total_profit,Fdetail_fmt_type,Fdetail,Fstandby6 "
                   " FROM  fund_db_%02d.t_fund_user_bill_%d "
                   " WHERE Fuid=%d AND Fstate=2 ", 
        Sdb1(m_params.getInt("uid")), Stb1(m_params.getInt("uid")), m_params.getInt("uid"));
   
    ADD_CONDITION(szSqlStr,"Fbill_type","bill_type",10,"=") 
    ADD_CONDITION(szSqlStr,"Fbill_id","start_bill_id",32,">=") 
    ADD_CONDITION(szSqlStr,"Fbill_id","end_bill_id",32,"<=") 
    
    ADD_ORDERBY(szSqlStr,"Fbill_id")
        
    return szSqlStr;
    
}

string  FundComQryChkLgi::buildBalanceListQuerySql()  throw (CException)
{
    TRACE_DEBUG("buildBalanceListQuerySql");
    m_params.readStrParam(m_fields_nv_value.c_str(), "trade_id", 1, 100);
    if (m_params.getString("trade_id") != m_fund_bind.Ftrade_id)
    {
        throw CException(ERR_SESSION_PARA_CHECK, "trade_id check fail with funddb", __FILE__, __LINE__);
    }
    char    szSqlStr[MAX_SQL_LEN+1]={0};


    snprintf(szSqlStr, sizeof(szSqlStr),
                    " SELECT "
                    " Flistid,Fspid,Ftrade_id,Ftotal_fee, Fuin,"
                    " Fuid,Ftype,Fstate,Fcreate_time, Fmodify_time,"
                    " Facc_time,Ftotal_acc_trans_id,Fsubacc_trans_id,Fcontrol_id, Fcur_type,"
                    " Fchannel_id,Fsign,Ft1fetch_date,Fmemo,Fflag,Fstandby1,Fstandby2,Fstandby3 "
                    " FROM fund_db_%02d.t_fund_balance_order_%d  "
                    " WHERE "
                    " Ftrade_id='%s'  AND Flstate=1 ",
                    Sdb2(m_params["trade_id"]), Stb2(m_params["trade_id"]), escapeString(m_params["trade_id"]).c_str());
   
    ADD_CONDITION(szSqlStr,"Facc_time","begin_time",20,">=") 
    ADD_CONDITION(szSqlStr,"Facc_time","end_time",20,"<=") 
    ADD_CONDITION(szSqlStr,"Ftype","type",2,"=") 
    ADD_CONDITION(szSqlStr,"Fstate","state",2,"=") 
    ADD_CONDITION(szSqlStr,"Flistid","listid",32,"=") 
    ADD_IN(szSqlStr,"Ftype","type_list",20)
    ADD_IN(szSqlStr,"Fstate","state_list",20)
    ADD_ORDERBY(szSqlStr,"Facc_time")
        
    return szSqlStr;
    
}


string  FundComQryChkLgi::buildCloseListQuerySql()  throw (CException)
{
    TRACE_DEBUG("buildCloseListQuerySql");
    m_params.readLongParam(m_fields_nv_value.c_str(), "close_id", 0, MAX_LONG);

    char    szSqlStr[MAX_SQL_LEN+1]={0};


    snprintf(szSqlStr, sizeof(szSqlStr),
                     " SELECT "
                    " Fid,Ftrade_id,Ffund_code,Fspid,Fseqno, "
                    " Fuid,Fpay_type,Flastid,Fstart_total_fee,Fcurrent_total_fee, "
                    " Fend_tail_fee,Fuser_end_type,Fend_sell_type,Fend_plan_amt,Fend_real_buy_amt, "
                    " Fend_real_sell_amt,Fpresell_listid,Fsell_listid,Fend_listid_1,Fend_listid_2, "
                    " Fend_transfer_fundcode,Fend_transfer_spid,Ftrans_date,Ffirst_profit_date,Fopen_date, "
                    " Fbook_stop_date,Fstart_date,Fend_date,Fprofit_end_date,Fchannel_id, "
                    " Fstate,Flstate,Fcreate_time,Fmodify_time,Fmemo, "
                    " Fexplain,Fsign,Facc_time, "
                    " Fprofit_recon_date,Flast_profit,Fdue_date,Ftotal_profit,Flastids "
                    " FROM fund_db_%02d.t_fund_close_trans_%d "
                    " WHERE "
                    " Fid= %ld ",
                    Sdb2(m_fund_bind.Ftrade_id),
                    Stb2(m_fund_bind.Ftrade_id),
                    m_params.getLong("close_id"));
        
    return szSqlStr;
    
}



string  FundComQryChkLgi::buildUserTransQuerySql()  throw (CException)
{
    checkUid();
    char    szSqlStr[MAX_SQL_LEN+1]={0};
    
    snprintf(szSqlStr, sizeof(szSqlStr), 
        "SELECT Flistid, Fspid, Fcoding, Ftrade_id, Fuid, Ffund_name, Ffund_code, "
                   " Fpur_type, Ftotal_fee, Fbank_type, Fcard_no, Fstate, Ftrade_date, Ffund_value, "
                   " Ffund_vdate, Ffund_type, Frela_listid, Fcreate_time, Fmodify_time,Ffetchid,Floading_type,Fcft_bank_billno,"
                   " Fcft_trans_id, Facc_time, Fmemo,Fclose_listid,Fopt_type,Freal_redem_amt,Fend_date,Fsign "
                   " FROM  fund_db_%02d.t_trade_user_fund_%d "
                   " WHERE Fuid=%d AND Flstate=1  AND Fstate in (2,3,4,5,6,8,9,10,12,13) ", 
        Sdb1(m_params.getInt("uid")), Stb1(m_params.getInt("uid")), m_params.getInt("uid"));

    ADD_CONDITION(szSqlStr,"Flistid","listid",32,"=")
    ADD_CONDITION(szSqlStr,"Fspid","spid",15,"=")    
    ADD_CONDITION(szSqlStr,"Ffund_name","fund_name",64,"=")
    ADD_CONDITION(szSqlStr,"Ffund_code","fund_code",32,"=")
    ADD_CONDITION(szSqlStr,"Fpur_type","pur_type",2,"=")    
    ADD_CONDITION(szSqlStr,"Facc_time","begin_time",20,">=") 
    ADD_CONDITION(szSqlStr,"Facc_time","end_time",20,"<=") 
    ADD_IN(szSqlStr,"Fpur_type","pur_type_list",20)
    ADD_ORDERBY(szSqlStr,"Facc_time")
        
    return szSqlStr;
    
}


string  FundComQryChkLgi::buildFetchListQuerySql()  throw (CException)
{
    char    szSqlStr[MAX_SQL_LEN+1]={0};

    m_params.readStrParam(m_fields_nv_value.c_str(), "uin", 1, 64);
    m_params.readStrParam(m_fields_nv_value.c_str(), "cft_fetch_no", 10, 32);
    m_params.readStrParam(m_fields_nv_value.c_str(), "order_time", 10, 10);
    string orderTime = m_params["order_time"];
    orderTime = orderTime.substr(0,4)+orderTime.substr(5,2);
    
    snprintf(szSqlStr, sizeof(szSqlStr), 
        " SELECT Fbank_type,Fcard_id,Fstate,Fmodify_time,Fnum,Fcard_name,Ffund_apply_id  "
        " FROM  fund_db.t_fetch_list_%s "
        " WHERE Fuin='%s'   AND Fcft_fetch_no='%s'   ", 
        orderTime.c_str(),m_params["uin"],m_params["cft_fetch_no"]);

    return szSqlStr;
    
}

void FundComQryChkLgi::buildSql()  throw (CException)
{
    switch (m_params.getInt("reqid"))
    {
        case COMMQRY_TYPE_TRANS:
        {
            m_querySql = buildTransQuerySql();
            break;
        }
        case COMMQRY_TYPE_USER_TRANS:
        {
            m_querySql = buildUserTransQuerySql();
            break;
        }
        case COMMQRY_TYPE_FETCHLIST:
        {
            m_querySql = buildFetchListQuerySql();
            break;
        }
        case COMMQRY_TYPE_RPOFITLIST:
        {
            m_querySql = buildProfitListQuerySql();
            break;
        }
        case COMMQRY_TYPE_WEEKLIST:
        {
            m_querySql = buildWeekListQuerySql();
            break;
        }
        case COMMQRY_TYPE_BALIST:
        {
            m_querySql = buildBalanceListQuerySql();
            break;
        }
        case COMMQRY_TYPE_CLOSE_LIST:
        {
            m_querySql = buildCloseListQuerySql();
            break;
        }
        default:
        {
             throw CException(ERR_BAD_PARAM, "invalid reqid", __FILE__, __LINE__);
        }
    }

}

void FundComQryChkLgi::checkUid()throw (CException)
{
    if (m_params.getInt("uid")==0)
    {
        m_params.readIntParam(m_fields_nv_value.c_str(), "uid", 0, MAX_INTEGER);
    }
    if (m_params.getInt("uid") != m_fund_bind.Fuid)
    {
        throw CException(ERR_SESSION_PARA_CHECK, "uid check fail with funddb", __FILE__, __LINE__);
    }
}

void FundComQryChkLgi::doQeury() throw (CException)
{
       doBatchSelect(m_fund_conn, m_querySql, m_result);
}

/**
  * 执行基金账户开户
  */
void FundComQryChkLgi::excute() throw (CException)
{
    CheckParams();
    CheckFundBind();
    buildSql();
    doQeury();
    switch (m_params.getInt("reqid"))
    {
        case COMMQRY_TYPE_TRANS: case COMMQRY_TYPE_USER_TRANS:
        {            
            //敏感字段hash检查            
            if( m_result.size() > 0 && m_result["Flistid"].size() > 0){
                for ( size_t i = 0; i < m_result["Flistid"].size(); i++)
                {
                    ST_TRADE_FUND tTradeFund;
                    SCPY(tTradeFund.Flistid, m_result["Flistid"][i].c_str());
                    tTradeFund.Fpur_type = atoi(m_result["Fpur_type"][i].c_str());
                    SCPY(tTradeFund.Fspid, m_result["Fspid"][i].c_str());
                    SCPY(tTradeFund.Ftrade_id, m_result["Ftrade_id"][i].c_str());
                    tTradeFund.Fuid = atoi(m_result["Fuid"][i].c_str());
                    tTradeFund.Ftotal_fee = atoi(m_result["Ftotal_fee"][i].c_str());
                    tTradeFund.Fstate = atoi(m_result["Fstate"][i].c_str());
                    SCPY(tTradeFund.Fsign, m_result["Fsign"][i].c_str());
                    checkSign("t_trade_fund", tTradeFund);
                }
            }
            break;
        }  
        case COMMQRY_TYPE_CLOSE_LIST:{

            //敏感字段hash检查            
            if( m_result.size() > 0 && m_result["Fid"].size() > 0){
                for ( size_t i = 0; i < m_result["Fid"].size(); i++)
                {
                    FundCloseTrans tfundCloseTrans;
                    SCPY(tfundCloseTrans.Ftrade_id, m_result["Ftrade_id"][i].c_str());
                    tfundCloseTrans.Fseqno = atoi(m_result["Fseqno"][i].c_str());
                    SCPY(tfundCloseTrans.Ffund_code, m_result["Ffund_code"][i].c_str());
                    tfundCloseTrans.Fcurrent_total_fee = atol(m_result["Fcurrent_total_fee"][i].c_str());
                    tfundCloseTrans.Fend_tail_fee = atol(m_result["Fend_tail_fee"][i].c_str());
                    SCPY(tfundCloseTrans.Fsign, m_result["Fsign"][i].c_str());
                    checkSign("t_fund_close_trans", tfundCloseTrans);
                }
            }            
            break;
        }case COMMQRY_TYPE_RPOFITLIST:{
            
            //敏感字段hash检查            
            if( m_result.size() > 0 && m_result["Flistid"].size() > 0){
                for ( size_t i = 0; i < m_result["Flistid"].size(); i++)
                {
                    FundProfitRecord tFundProfitRecord;
                    SCPY(tFundProfitRecord.Flistid, m_result["Flistid"][i].c_str());
                    SCPY(tFundProfitRecord.Fsub_trans_id, m_result["Fsub_trans_id"][i].c_str());
                    SCPY(tFundProfitRecord.Ftrade_id, m_result["Ftrade_id"][i].c_str());
                    tFundProfitRecord.Fcurtype= atoi(m_result["Fcurtype"][i].c_str());
                    SCPY(tFundProfitRecord.Fspid, m_result["Fspid"][i].c_str());
                    tFundProfitRecord.Ftotal_profit = atol(m_result["Ftotal_profit"][i].c_str());
                    tFundProfitRecord.Fprofit = atol(m_result["Fprofit"][i].c_str());
                    SCPY(tFundProfitRecord.Fday, m_result["Fday"][i].c_str());
                    tFundProfitRecord.Fend_tail_fee = atol(m_result["Fend_tail_fee"][i].c_str());
                    SCPY(tFundProfitRecord.Fsign, m_result["Fsign"][i].c_str());
                    checkSign("t_fund_profit_record", tFundProfitRecord);
                }
            } 
            break;
        }case COMMQRY_TYPE_BALIST:{
            //敏感字段hash检查            
            if( m_result.size() > 0 && m_result["Flistid"].size() > 0){
                for ( size_t i = 0; i < m_result["Flistid"].size(); i++)
                {
                    ST_BALANCE_ORDER stBalanceOrder;
                    SCPY(stBalanceOrder.Flistid, m_result["Flistid"][i].c_str());
                    stBalanceOrder.Ftotal_fee = atol(m_result["Ftotal_fee"][i].c_str());
                    SCPY(stBalanceOrder.Ftrade_id, m_result["Ftrade_id"][i].c_str());
                    stBalanceOrder.Ftype = atoi(m_result["Ftype"][i].c_str());
                    stBalanceOrder.Fuid = atoi(m_result["Fuid"][i].c_str());
                    SCPY(stBalanceOrder.Fsign, m_result["Fsign"][i].c_str());
                    checkSign("t_fund_balance_order", stBalanceOrder);
                }
            } 
            break;
        }
    }
}


/*
 * 查询基金账户是否存在
 */
void FundComQryChkLgi::CheckFundBind() throw (CException)
{
    if(!QueryFundBindByUin(m_fund_conn, m_params["qluin"], &m_fund_bind, false))
    {
        throw CException(ERR_FUNDBIND_NOTREG, "the fund bind record not exist", __FILE__, __LINE__);
    }
}

void FundComQryChkLgi::packXMLReturnMsg(TRPC_SVCINFO* rqst)
{
    char safeBuffer[MAX_RSP_MSG_LEN*3+100]={0};
    
    CUrlAnalyze::setParam(safeBuffer, "result", 0, true);
    CUrlAnalyze::setParam(safeBuffer, "res_info", "ok");
    
    string rec_info="<root>";
    CStr2sVec::iterator pos=m_result.begin();
   
    size_t retnum = 0;
    if (pos != m_result.end())
    {
        retnum = pos->second.size();
    }
      
    string records;
    for (size_t i=0; i < retnum; i++)
    {
        records += string("<record>");
        for(pos = m_result.begin(); pos != m_result.end(); pos++)
        {
            records += string("<")+pos->first+">"+xmlEscape(pos->second[i])+string("</")+pos->first+">";
        }
        records += "</record>";
        if (records.length() >MAX_RSP_MSG_LEN)
        {
            throw CException(ERR_RSP_MAG_LEHTN_OVERFLOW, "rsp msg overflow!", __FILE__, __LINE__);
        }
    }
    rec_info+=(string("<ret_num>")+ toString(retnum) +"</ret_num>");
    rec_info+=records;
    rec_info+="</root>";
    
    CUrlAnalyze::setParam(safeBuffer, "rec_info", rec_info.c_str());
    int rspMsgLen = strlen(safeBuffer);
    if (rspMsgLen>MAX_RSP_MSG_LEN)
    {
        throw CException(ERR_RSP_MAG_LEHTN_OVERFLOW, "rsp msg overflow!", __FILE__, __LINE__);
    }
    strncpy(rqst->odata,safeBuffer,rspMsgLen+1);
    rqst->odata[rspMsgLen]=0;
    rqst->olen = rspMsgLen;
    return;
}

void FundComQryChkLgi::packNVReturnMsg(TRPC_SVCINFO* rqst)
{
    CUrlAnalyze::setParam(rqst->odata, "result", 0, true);
    CUrlAnalyze::setParam(rqst->odata, "res_info", "ok");
    
    CStr2sVec::iterator pos=m_result.begin();
   
    int retnum = 0;
    if (pos != m_result.end())
    {
        retnum = pos->second.size();
    }
    CUrlAnalyze::setParam(rqst->odata, "ret_num", retnum);

    if (retnum == 1)
    {
        for(pos = m_result.begin(); pos != m_result.end(); pos++)
        {
            CUrlAnalyze::setParam(rqst->odata, pos->first.c_str(), pos->second[0].c_str());
        }  
    }
    else if (retnum != 0)
    {
        throw CException(ERR_RSP_MAG_LEHTN_OVERFLOW, "more than one records getted!", __FILE__, __LINE__);
    }

    rqst->olen = strlen(rqst->odata);
    return;
}


/**
  * 打包输出参数
  */
void FundComQryChkLgi::packReturnMsg(TRPC_SVCINFO* rqst)
{
    if (m_servicetype == CHECK_LOGIN)
    {
        packXMLReturnMsg(rqst);
    }
    else
    {
        packNVReturnMsg(rqst);
    }
}



