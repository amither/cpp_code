
#include "fund_commfunc.h"
#include "db_fund_balance_config.h"

extern CftLog* gPtrAppLog;
extern GlobalConfig* gPtrConfig;

bool queryFundBalanceRecoverRoll(CMySQL* pMysql, ST_BALANCE_RECOVER & data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Flistid,Ftype,Fpurchaser_id,Fbargainor_id, Fcur_type,"
                    " Ftotal_transfer_sup_fee,Ftotal_buy_fee,Ftotal_redem_fee,Fstate,Fcreate_time,"
                    " Fmodify_time,Facc_time,Ftotal_sup_backer_fee,Fsup_backer_translist,Fbacker_qqid,"
                    " Fsup_backer_time"
                    " FROM fund_db.t_fund_balance_recover_roll  "
                    " WHERE "
                    " Flistid='%s' AND  Flstate=1 " 
                    " %s ",
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow > 1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for (int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            strncpy(data.Flistid,row[0] ? row[0] : "", sizeof(data.Flistid) - 1);
            data.Ftype = row[1] ? atoi(row[1]) : 0;
            strncpy(data.Fpurchaser_id,row[2] ? row[2] : "", sizeof(data.Fpurchaser_id) - 1);
            strncpy(data.Fbargainor_id,row[3] ? row[3] : "", sizeof(data.Fbargainor_id) - 1);
            data.Fcur_type = row[4] ? atoi(row[4]) : 0;
            data.Ftotal_transfer_sup_fee = row[5] ? atoll(row[5]) : 0;
            data.Ftotal_buy_fee = row[6] ? atoll(row[6]) : 0;
            data.Ftotal_redem_fee = row[7] ? atoll(row[7]) : 0;
            data.Fstate = row[8] ? atoi(row[8]) : 0;
            strncpy(data.Fcreate_time,row[9] ? row[9] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[10] ? row[10] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Facc_time,row[11] ? row[11] : "", sizeof(data.Facc_time) - 1);
            data.Ftotal_sup_backer_fee = row[12] ? atoll(row[12]) : 0;
            strncpy(data.Fsup_backer_translist,row[13] ? row[13] : "", sizeof(data.Fsup_backer_translist) - 1);
            strncpy(data.Fbacker_qqid,row[14] ? row[14] : "", sizeof(data.Fbacker_qqid) - 1);
            strncpy(data.Fsup_backer_time,row[15] ? row[15] : "", sizeof(data.Fsup_backer_time) - 1);
        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return iRow == 1;
}

void recordTotalBalanceAccRoll(CMySQL* pMysql, const string &listid,int listType,LONG totalFee,const string&acctime,LONG totalAccBalance,int typeAdd)
{
    if (totalFee == 0) //金额没有变化不用记录流水表
    {
        return;
    }
    string tableName = string("fund_db.t_fund_balance_acc_roll_")+getTime_yyyymm();
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_balance_acc_roll_%s("
                    " Flistid,Ftype,Fbill_type,Fbalance, Ftotal_fee,"
                    " Flstate,Facc_time)"
                    " VALUES("
                    " '%s',%d,%d,%ld,%ld, "
                    " %d,'%s')",
                    getTime_yyyymm().c_str(),
                    pMysql->EscapeStr(listid).c_str(),
                    typeAdd,
                    listType,
                    totalAccBalance,
                    totalFee,
                    1,
                    acctime.c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

void insertFundFundBalanceRecoverRoll(CMySQL* pMysql, const ST_BALANCE_RECOVER & data)
{
    char szSql[MAX_SQL_LEN + 1]={0};
    // 构造SQL
    int iLen = snprintf(szSql, sizeof(szSql),
                    " INSERT INTO fund_db.t_fund_balance_recover_roll("
                    " Flistid,Ftype,Fpurchaser_id,Fbargainor_id, Fcur_type,"
                    " Ftotal_transfer_sup_fee,Ftotal_buy_fee,Ftotal_redem_fee,Fstate,Ftotal_sup_backer_fee,"
                    " Fcreate_time,Fmodify_time,Facc_time, Flstate,Fsup_backer_translist,"
                    " Fbacker_qqid,Ftrans_date)"
                    " VALUES("
                    " '%s',%d,'%s','%s',%d, "
                    " %ld,%ld,%ld,%d,%ld,"
                    " '%s','%s','%s',%d, '%s',"
                    " '%s','%s')",
                    pMysql->EscapeStr(data.Flistid).c_str(),
                    data.Ftype,
                    pMysql->EscapeStr(data.Fpurchaser_id).c_str(),
                    pMysql->EscapeStr(data.Fbargainor_id).c_str(),
                    data.Fcur_type,
                    data.Ftotal_transfer_sup_fee,
                    data.Ftotal_buy_fee,
                    data.Ftotal_redem_fee,
                    data.Fstate,
                    data.Ftotal_sup_backer_fee,
                    pMysql->EscapeStr(data.Fcreate_time).c_str(),
                    pMysql->EscapeStr(data.Fmodify_time).c_str(),
                    pMysql->EscapeStr(data.Facc_time).c_str(),
                    1,
                    pMysql->EscapeStr(data.Fsup_backer_translist).c_str(),
                    pMysql->EscapeStr(data.Fbacker_qqid).c_str(),
                    pMysql->EscapeStr(data.Ftrans_date).c_str()
                    );
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
    // 执行SQL
    pMysql->Query(szSql, iLen);
}

void updateFundFundBalanceRecoverRoll(CMySQL* pMysql, const ST_BALANCE_RECOVER & data)
{
    stringstream ss_cond;
    map<string, string> kv_map;

    // 设置需要更新的字段

    if(data.Fstate!= 0)
    {
        kv_map["Fstate"] = toString(data.Fstate);
    }

    if(data.Facc_time[0]!= 0)
    {
        kv_map["Facc_time"] = data.Facc_time;
    }
    
    if(data.Fmodify_time[0]!= 0)
    {
        kv_map["Fmodify_time"] = data.Fmodify_time;
    }

    if(data.Fsup_backer_time[0]!= 0)
    {
        kv_map["Fsup_backer_time"] = data.Fsup_backer_time;
    }
    
    ss_cond << "Flistid='" << escapeString(data.Flistid)<< "'";
    int affect_row =0;

    // 执行更新数据表操作
    affect_row = UpdateTable(pMysql, "fund_db.t_fund_balance_recover_roll", ss_cond, kv_map);
    if (affect_row != 1)
    {
        throw EXCEPTION(ERR_DB_AFFECT_MULTIROWS, "update affect rows error");
    }
}

bool isFundBalanceRecoverDelayed(CMySQL* pMysql,const string& sysTime)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    string lastRecoveAccTime;
    string lastRecoveTday;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Facc_time,Ftrans_date  "
                    " FROM fund_db.t_fund_balance_recover_roll "
                    " order by Ftrans_date DESC limit 1 "
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow >1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);
            lastRecoveAccTime = (row[0] ? row[0] : "");
            lastRecoveTday = (row[1] ? row[1] : "");
        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }

    if (lastRecoveAccTime.length()>13 && lastRecoveAccTime.substr(11,2)>= "15")
    {
        gPtrAppLog->debug("[%s][%d]recover delayed!!! lastRecoveAccTime=%s",__FILE__,__LINE__,lastRecoveAccTime.c_str());
        alert(ERR_FUND_FETCH_BALANCE_LACK, "基金公司结算回补延迟，停止余额t+0提现");
        return true;
    }

    //如果当前时间小于最近一笔入账日+2那么一定没有延迟
    if (changeDatetimeFormat(addDays(lastRecoveTday, 2)+"145900") > sysTime)
    {
        return false;
    }

    FundTransDate data;
    memset(&data, 0, sizeof(FundTransDate));
    strncpy(data.Fdate, addDays(lastRecoveTday, 1).c_str(), sizeof(data.Fdate)-1);
    queryFundProfitDate(pMysql, data, false);

    //1分钟的偏移
    if (changeDatetimeFormat(string(data.Fdate)+"145900") <= sysTime)
    {
        gPtrAppLog->debug("[%s][%d]recover delayed!!! ",__FILE__,__LINE__);
        alert(ERR_FUND_FETCH_BALANCE_LACK, "基金公司结算回补延迟，停止余额t+0提现");
        return true;
    }

    return false;
}

bool queryFundBalanceConfig(CMySQL* pMysql, FundBalanceConfig & data,  bool lock)
{
    MYSQL_RES* pRes = NULL;
    char szSql[MAX_SQL_LEN] = {0};
    int iLen = 0, iRow = 0;
    try
    {
        iLen = snprintf(szSql, sizeof(szSql),
                    " SELECT "
                    " Fid,Fbalance_spid,Fcharge_spid,Ffetch_backer_spid,Fbalance_spid_qqid, "
                    " Ffetch_backer_qqid,Ftrans_date,Ftotal_redem_balance_old,Ftotal_redem_balance,Ftotal_available_balance, "
                    " Ffetch_backer_total_fee,Ffetch_limit_offset,Fflag,Fsign,Fcreate_time, "
                    " Fmodify_time,Fmemo, Ffund_fetch_spid,Ffund_fetch_spid_qqid,Ftotal_buy_balance_old, "
                    " Ftotal_buy_balance,Ftotal_baker_fee,Ftotal_baker_fee_old,Ftotal_t1fetch_fee,Ftotal_t1fetch_fee_old  "
                    " FROM fund_db.t_fund_balance_config "
                    " WHERE Fid = %d AND "
                    " Flstate = %d " 
                    " %s ",
                    data.Fid,
                    LSTATE_VALID,
                    lock ? "FOR UPDATE" : ""
                    );
        gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,szSql );
        // 执行查询
        pMysql->Query(szSql, iLen);
        // 取结果集
        pRes = pMysql->FetchResult();
        // 获取结果行
        iRow = mysql_num_rows(pRes);
        if(iRow <0 || iRow >1)
        {
            throw CException(ERR_DB_UNKNOW, "Unknown record set!", __FILE__, __LINE__);
        }
        for(int i=0; i<iRow; i++) 
        {
            MYSQL_ROW row = mysql_fetch_row(pRes);

            data.Fid = row[0] ? atoi(row[0]) : 0;
            strncpy(data.Fbalance_spid,row[1] ? row[1] : "", sizeof(data.Fbalance_spid) - 1);
            strncpy(data.Fcharge_spid,row[2] ? row[2] : "", sizeof(data.Fcharge_spid) - 1);
            strncpy(data.Ffetch_backer_spid,row[3] ? row[3] : "", sizeof(data.Ffetch_backer_spid) - 1);
            strncpy(data.Fbalance_spid_qqid,row[4] ? row[4] : "", sizeof(data.Fbalance_spid_qqid) - 1);
            strncpy(data.Ffetch_backer_qqid,row[5] ? row[5] : "", sizeof(data.Ffetch_backer_qqid) - 1);
            strncpy(data.Ftrans_date,row[6] ? row[6] : "", sizeof(data.Ftrans_date) - 1);
            data.Ftotal_redem_balance_old = row[7] ? atoll(row[7]) : 0;
            data.Ftotal_redem_balance = row[8] ? atoll(row[8]) : 0;
            data.Ftotal_available_balance = row[9] ? atoll(row[9]) : 0;
            data.Ffetch_backer_total_fee= row[10] ? atoll(row[10]) : 0;
            data.Ffetch_limit_offset = row[11] ? atoll(row[11]) : 0;
            data.Fflag = row[12] ? atoi(row[12]) : 0;
            strncpy(data.Fsign,row[13] ? row[13] : "", sizeof(data.Fsign) - 1);
            strncpy(data.Fcreate_time,row[14] ? row[14] : "", sizeof(data.Fcreate_time) - 1);
            strncpy(data.Fmodify_time,row[15] ? row[15] : "", sizeof(data.Fmodify_time) - 1);
            strncpy(data.Fmemo,row[16] ? row[16] : "", sizeof(data.Fmemo) - 1);
            strncpy(data.Ffund_fetch_spid,row[17] ? row[17] : "", sizeof(data.Ffund_fetch_spid) - 1);
            strncpy(data.Ffund_fetch_spid_qqid,row[18] ? row[18] : "", sizeof(data.Ffund_fetch_spid_qqid) - 1);
            data.Ftotal_buy_balance_old = row[19] ? atoll(row[19]) : 0;
            data.Ftotal_buy_balance = row[20] ? atoll(row[20]) : 0;
            data.Ftotal_baker_fee = row[21] ? atoll(row[21]) : 0;
            data.Ftotal_baker_fee_old = row[22] ? atoll(row[22]) : 0;
            data.Ftotal_t1fetch_fee = row[23] ? atoll(row[23]) : 0;
            data.Ftotal_t1fetch_fee_old= row[24] ? atoll(row[24]) : 0;
        }
        mysql_free_result(pRes);
    }
    catch(CException& e)
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    catch( ... )
    {
        if(pRes)    mysql_free_result(pRes);
        throw;
    }
    return (iRow >= 1);
}

void AddQuerySqlForswitchTransDate(string &querySql,const string &trade_date,LONG totalFee,int op_type)
{
    querySql += string(" Ftotal_buy_balance_old = Ftotal_buy_balance_old + Ftotal_buy_balance")+" ,"; 
    querySql += string(" Ftotal_redem_balance_old = Ftotal_redem_balance_old + Ftotal_redem_balance")+" ,"; 
    querySql += string(" Ftotal_t1fetch_fee_old = Ftotal_t1fetch_fee_old + Ftotal_t1fetch_fee")+" ,"; 
    querySql += string(" Ftotal_baker_fee_old = Ftotal_baker_fee_old + Ftotal_baker_fee")+" ,"; 
    querySql += string(" Ftrans_date = '") + trade_date +"' ,"; 
    
    LONG total_buy_balance=0;
    LONG total_redem_balance=0;
    LONG total_t1fetch_fee=0;
    LONG total_baker_fee=0;
    
    if (op_type == OP_TYPE_BA_FETCH)
    {
        total_baker_fee = totalFee;
    }
    else if (op_type == OP_TYPE_BA_BUY)
    {
        total_buy_balance = totalFee;
    }
    else if (op_type == OP_TYPE_CHAEGE_REDEM_T0)
    {
        total_redem_balance = totalFee;
    }
    else  if (op_type == OP_TYPE_BA_FETCH_T1)
    {
        total_t1fetch_fee = totalFee;
    }
    else
    {
        throw CException(ERR_FUND_BA_FETCH_SYS_ERORR, "AddQuerySqlForswitchTransDate invalid op_type!", __FILE__, __LINE__);
    }
    
    querySql += string(" Ftotal_buy_balance = ")+toString(total_buy_balance)+" ,"; 
    querySql += string(" Ftotal_redem_balance = ")+toString(total_redem_balance)+" ,"; 
    querySql += string(" Ftotal_t1fetch_fee = ")+toString(total_t1fetch_fee)+" ,"; 
    querySql += string(" Ftotal_baker_fee = ")+toString(total_baker_fee)+" ,"; 
    
}

/*
转换回补时如果回补所在交易日期大于配置表中的交易日志则先调用该函数更新交易日期
正常情况不会发生这种场景，只有长时间无申购赎回提现操作的时候才会出现。
*/
void updateFundbalanceConfigTradeDate(CMySQL*pMysql,const string &trade_date,const string &systime)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";
    AddQuerySqlForswitchTransDate(querySql,trade_date,0,OP_TYPE_BA_FETCH);

    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(systime.c_str())+"' ";   
    querySql += string(" WHERE Fid=")+ toString(DEFAULT_BALNCE_CONFIG_FID) +" AND Flstate=1 ";   
   
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


/*
t+0提现调用该函数减总余额
*/
void updateFundBalanceConfigForFetch(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date,bool &bNeedBacker)
{
    bool needUpdateCkv = false;
    
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";
    LONG total_available_balance = data.Ftotal_available_balance;
    LONG total_baker_fee_old = data.Ftotal_baker_fee_old;
    LONG total_baker_fee = data.Ftotal_baker_fee;
    bool lackBlance=false;

    if (data.Ftotal_t1fetch_fee_old>0 && true==isFundBalanceRecoverDelayed(pMysql, data.Fmodify_time))
    {
        lackBlance = true;
    }
    else
    {
        /*判断是否可以从余额账户垫资必须满足如下条件:
        1 当前余额大等于提现金额
        2 当前金额-今日要出金额大等于提现金额
        3 当前金额-今日要出金额-明日要出金额大等于提现金额*/
        if (data.Ftotal_available_balance >= total_fee
            && data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old>= total_fee
            && data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance >= total_fee)
        {
            querySql += string(" Ftotal_available_balance = Ftotal_available_balance - ")+toString(total_fee)+" ,";
            bNeedBacker = false;
            total_available_balance -= total_fee;
        }
        else if (data.Ffetch_backer_total_fee-data.Ftotal_baker_fee_old-data.Ftotal_baker_fee >= total_fee)
        {
            if (trade_date < data.Ftrans_date)
            {
                querySql += string(" Ftotal_baker_fee_old = Ftotal_baker_fee_old + ")+toString(total_fee)+" ,"; 
                total_baker_fee_old += total_fee;
            }
            else if (trade_date == data.Ftrans_date)
            {
                querySql += string(" Ftotal_baker_fee = Ftotal_baker_fee + ")+toString(total_fee)+" ,"; 
                total_baker_fee += total_fee;
            }
            else
            {
                AddQuerySqlForswitchTransDate(querySql,trade_date,total_fee,OP_TYPE_BA_FETCH);
            }
            bNeedBacker = true;
        }
        else
        {
            //余额不足先停止t+0提现
            lackBlance = true;
            alert(ERR_FUND_FETCH_BALANCE_LACK, "余额总账户表t+0提现金额垫资剩下不足");
        }
    }

    /*停止t+0提现的条件之一:
    1:可用余额+剩余垫资额度-今日要出金额小于提现缓充剩余值
    2:可用余额+剩余垫资额度-今日要出金额-明日要出金额小于提现缓充剩余值
    3:可用余额+剩余垫资额度小于提现缓充剩余值
    4:基金公司回补延迟
    */
    //垫资剩余值
    LONG backer_balance=data.Ffetch_backer_total_fee-total_baker_fee_old-total_baker_fee;
    if (lackBlance || (total_available_balance+backer_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old<= data.Ffetch_limit_offset
        || total_available_balance+backer_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance <= data.Ffetch_limit_offset
        || total_available_balance+backer_balance <= data.Ffetch_limit_offset)
        )
    {
    
        if ((data.Fflag&FETCH_LOAN_OVER))
        {
            throw CException(ERR_FUND_FETCH_BALANCE_LACK, "updateFundBalanceConfigForFetch unexpeted balance!", __FILE__, __LINE__);
        }
    
        //停止t+0提现
        int flag = (data.Fflag|FETCH_LOAN_OVER);
        querySql += string(" Fflag = ")+toString(flag)+" ,"; 
        needUpdateCkv = true;
    }

    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   
    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }

    if (needUpdateCkv)
    {
        setFundBalanceConfigToCkv(pMysql);
    }
    if (lackBlance)
    {
        throw CException(ERR_FUND_FETCH_BALANCE_LACK, "updateFundBalanceConfigForFetch unexpeted balance!", __FILE__, __LINE__);
    }
}

/*
t+1提现请求调用该函数累加提现额度
*/
void updateFundBalanceConfigForT1FetchReq(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    if (trade_date < data.Ftrans_date)
    {
        /*T-1日申购必须满足:
          1: 当前总余额-t-1日总共要从总余额中出的钱大等于提现额
          2: 当前总余额-t日总共要从总余额中出的钱大等于提现额*/
        if (data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old>= total_fee
            && data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance >= total_fee)
        {
            querySql += string(" Ftotal_t1fetch_fee_old = Ftotal_t1fetch_fee_old + ")+toString(total_fee)+" ,"; 
        }
        else
        {
            throw CException(ERR_FUND_T1FETCH_BALANCE_LACK, "updateFundBalanceConfigForBuy unexpeted balance!", __FILE__, __LINE__);
        }
        
    }
    else if (trade_date >= data.Ftrans_date)
    {
          /*T 日申购必须满足:
          1: 当前总余额-t-1日总共要从总余额中出的钱大等于0
          2: 当前总余额-t日总共要从总余额中出的钱大等于提现额*/
        if (data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old>= 0
            && data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance >= total_fee)
        {
            if (trade_date == data.Ftrans_date)
            {
                querySql += string(" Ftotal_t1fetch_fee = Ftotal_t1fetch_fee + ")+toString(total_fee)+" ,"; 
            }
            else
            {
                AddQuerySqlForswitchTransDate(querySql,trade_date,total_fee,OP_TYPE_BA_FETCH_T1);
            }
        }
        else
        {
            throw CException(ERR_FUND_T1FETCH_BALANCE_LACK, "updateFundBalanceConfigForBuy unexpeted balance!", __FILE__, __LINE__);
        }
    }


    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   
    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


/*
t+1提现确认调用该函数减待提现确认额度
*/
void updateFundBalanceConfigForT1FetchCnf(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";
    
    if (trade_date >= data.Ftrans_date) //t+1确认时流水单中的acctime对应的t日必然小于配置表中的交易日
    {
        throw CException(ERR_FUND_BA_FETCH_SYS_ERORR, "check trade_date and data.Ftrans_date fail!", __FILE__, __LINE__);
    }

    if (data.Ftotal_t1fetch_fee_old>= total_fee && data.Ftotal_available_balance >=total_fee)
    {
        querySql += string(" Ftotal_t1fetch_fee_old = Ftotal_t1fetch_fee_old - ")+toString(total_fee)+" ,"; 
        querySql += string(" Ftotal_available_balance = Ftotal_available_balance - ")+toString(total_fee)+" ,"; 
    }
    else
    {
        alert(ERR_FUND_BA_BUY_BALANCE_LACK, "updateFundBalanceConfigForT1FetchCnf unexpeted Ftotal_t1fetch_fee_old!");
        throw CException(ERR_FUND_BA_BUY_BALANCE_LACK, "updateFundBalanceConfigForT1FetchCnf unexpeted Ftotal_t1fetch_fee_old!", __FILE__, __LINE__);
    }

    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   

    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

/*
余额申购退款调用该函数恢复申购额度
*/
void updateFundBalanceConfigForBuyRefund(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    if (trade_date < data.Ftrans_date)
    {

        if (data.Ftotal_buy_balance_old>= total_fee)
        {
            querySql += string(" Ftotal_buy_balance_old = Ftotal_buy_balance_old - ")+toString(total_fee)+" ,"; 
        }
        else
        {
            throw CException(ERR_FUND_BA_BUY_BALANCE_LACK, "updateFundBalanceConfigForBuyRefund unexpeted balance!", __FILE__, __LINE__);
        }
        
    }
    else if (trade_date == data.Ftrans_date && (data.Ftotal_buy_balance>= total_fee))
    {
        querySql += string(" Ftotal_buy_balance = Ftotal_buy_balance - ")+toString(total_fee)+" ,"; 
    }
    else
    {
        throw CException(ERR_FUND_BA_BUY_BALANCE_LACK, "updateFundBalanceConfigForBuyRefund unexpeted balance!", __FILE__, __LINE__);
    }

   
    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   

    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}


/*
余额申购调用该函数累加申购额度
*/
void updateFundBalanceConfigForBuy(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    if (trade_date < data.Ftrans_date)
    {
        /*T-1日申购必须满足:
          1: 当前总余额-t-1日总共要从总余额中出的钱大等于申购额
          2: 当前总余额-t日总共要从总余额中出的钱大等于申购额*/
        if (data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old>= total_fee
            && data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance >= total_fee)
        {
            querySql += string(" Ftotal_buy_balance_old = Ftotal_buy_balance_old + ")+toString(total_fee)+" ,"; 
        }
        else
        {
            throw CException(ERR_FUND_BA_BUY_BALANCE_LACK, "updateFundBalanceConfigForBuy unexpeted balance!", __FILE__, __LINE__);
        }
        
    }
    else if (trade_date >= data.Ftrans_date)
    {
          /*T 日申购必须满足:
          1: 当前总余额-t-1日总共要从总余额中出的钱大等于0
          2: 当前总余额-t日总共要从总余额中出的钱大等于申购额*/
        if (data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old>= 0
            && data.Ftotal_available_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance >= total_fee)
        {
            if (trade_date == data.Ftrans_date)
            {
                querySql += string(" Ftotal_buy_balance = Ftotal_buy_balance + ")+toString(total_fee)+" ,"; 
            }
            else
            {
                AddQuerySqlForswitchTransDate(querySql,trade_date,total_fee,OP_TYPE_BA_BUY);
            } 
        }
        else
        {
            throw CException(ERR_FUND_BA_BUY_BALANCE_LACK, "updateFundBalanceConfigForBuy unexpeted balance!", __FILE__, __LINE__);
        }

    }


    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   

    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

/*
充值调用该函数更新余额配置表中的当前总余额
*/
void updateFundBalanceConfigForCharge(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    querySql += string(" Ftotal_available_balance = Ftotal_available_balance + ")+toString(total_fee)+" ,"; 
    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   
    
    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
}

/*
赎回到余额调用该函数更新配置表中的赎回额度
*/
void updateFundBalanceConfigForRedem(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date)
{
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    if (trade_date < data.Ftrans_date)
    {
        querySql += string(" Ftotal_redem_balance_old = Ftotal_redem_balance_old + ")+toString(total_fee)+" ,"; 
    }
    else if (trade_date == data.Ftrans_date)
    {
        querySql += string(" Ftotal_redem_balance = Ftotal_redem_balance + ")+toString(total_fee)+" ,"; 
    }
    else
    {
        AddQuerySqlForswitchTransDate(querySql,trade_date,total_fee,OP_TYPE_CHAEGE_REDEM_T0);
    }

    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   
    
    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
    
}

/*
转换回补时调用
返回可回补提现垫资账户额度
*/
LONG updateFundBalanceConfigForRecover(CMySQL*pMysql,const FundBalanceConfig & data,LONG total_buy,LONG total_redem,LONG total_fee)
{
    LONG total_available_balance=data.Ftotal_available_balance;
    LONG total_redem_balance_old=data.Ftotal_redem_balance_old-total_redem;
    LONG total_buy_balance_old=data.Ftotal_buy_balance_old-total_buy;
    
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    querySql += string(" Ftotal_redem_balance_old = Ftotal_redem_balance_old - ")+toString(total_redem)+" ,"; 

    querySql += string(" Ftotal_buy_balance_old = Ftotal_buy_balance_old - ")+toString(total_buy)+" ,"; 

    if (total_buy > total_redem)
    { 
        total_available_balance -= total_fee;
        querySql += string(" Ftotal_available_balance = Ftotal_available_balance - ")+toString(total_fee)+" ,"; 
    }
    else
    {
        total_available_balance += total_fee;
        querySql += string(" Ftotal_available_balance = Ftotal_available_balance + ")+toString(total_fee)+" ,"; 
    }

    //垫资剩余值
    LONG backer_balance=data.Ffetch_backer_total_fee-data.Ftotal_baker_fee_old-data.Ftotal_baker_fee;

    /*可回补提现垫资金额为以下金额最小者":
      1 : 昨日总垫资额
      2 : 当前可用总金额- 今日要出金额
      3:  当前可用总金额- 今日要出金额- 明日要出金额
      4:  当前可用余额
      5:  提现账户总垫资额*/

    LONG total_sup_backer_fee = data.Ftotal_baker_fee_old;
    LONG todayBalance = total_available_balance-data.Ftotal_t1fetch_fee_old+total_redem_balance_old-total_buy_balance_old;
    LONG tomorrowBalance=total_available_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+total_redem_balance_old-total_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance;

    if (total_sup_backer_fee >todayBalance)
    {
        total_sup_backer_fee = todayBalance;
    }
    if (total_sup_backer_fee >tomorrowBalance)
    {
        total_sup_backer_fee = tomorrowBalance;
    }
    if (total_sup_backer_fee > total_available_balance)
    {
        total_sup_backer_fee = total_available_balance;
    }
    if (total_sup_backer_fee > total_available_balance-data.Ftotal_t1fetch_fee_old)
    {
        total_sup_backer_fee = total_available_balance-data.Ftotal_t1fetch_fee_old;
    }

    if (total_sup_backer_fee <0)
    {
        total_sup_backer_fee = 0;
    }


    TRACE_DEBUG("updateFundBalanceConfigForRecover backer_balance=%ld,total_sup_backer_fee=%ld,todayBalance=%ld,tomorrowBalance=%ld,total_available_balance=%ld,total_sup_backer_fee=%ld,Ftotal_t1fetch_fee_old=%ld",
                          backer_balance,total_sup_backer_fee,todayBalance,tomorrowBalance,total_available_balance,total_sup_backer_fee,data.Ftotal_t1fetch_fee_old);

    total_available_balance -= total_sup_backer_fee;

    querySql += string(" Ftotal_available_balance = Ftotal_available_balance - ")+toString(total_sup_backer_fee)+" ,";


    bool needUpdateCkv = false;

    /*开启 t+0提现的条件:
    1:可用余额+剩余垫资额度-今日要出金额大于提现缓充剩余值
    2:可用余额+剩余垫资额度-今日要出金额-明日要出金额大于提现缓充剩余值
    3:可用余额+剩余垫资额度大于提现缓充剩余值
    */
    
    if ((data.Fflag&FETCH_LOAN_OVER)&&(total_available_balance+backer_balance-data.Ftotal_t1fetch_fee_old+total_redem_balance_old-total_buy_balance_old> data.Ffetch_limit_offset
        && total_available_balance+backer_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+total_redem_balance_old-total_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance > data.Ffetch_limit_offset
        && total_available_balance+backer_balance > data.Ffetch_limit_offset
        && (data.Ffetch_limit_offset>0)))
    {
        //启动t+0提现
        int flag = (data.Fflag&(~FETCH_LOAN_OVER));
        querySql += string(" Fflag = ")+toString(flag)+" ,"; 
        needUpdateCkv = true;
    }
    

    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   
    
    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() > 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
    
    if (needUpdateCkv)
    {
        setFundBalanceConfigToCkv(pMysql);
    }

    return total_sup_backer_fee;
}

/*
更新余额配置表提现垫资回补金额
*/
void updateFundBalanceConfigForRecoverFetchBacker(CMySQL*pMysql,const FundBalanceConfig & data,LONG total_fee)
{
    if (data.Ftotal_baker_fee_old < total_fee)
    {
        throw CException(ERR_FUND_BA_FETCH_SYS_ERORR, "updateFundBalanceConfigForRecoverFetchBacker unexpeted Ftotal_baker_fee_old!", __FILE__, __LINE__);
    }
    
    string querySql = "UPDATE  fund_db.t_fund_balance_config  SET ";

    querySql += string(" Ftotal_baker_fee_old = Ftotal_baker_fee_old - ")+toString(total_fee)+" ,"; 

    //垫资剩余值
    LONG backer_balance=data.Ffetch_backer_total_fee-data.Ftotal_baker_fee_old-data.Ftotal_baker_fee +total_fee ;

    bool needUpdateCkv = false;

    /*开启 t+0提现的条件:
    1:可用余额+剩余垫资额度-今日要出金额大于提现缓充剩余值
    2:可用余额+剩余垫资额度-今日要出金额-明日要出金额大于提现缓充剩余值
    3:可用余额+剩余垫资额度大于提现缓充剩余值
    */
    
    if ((data.Fflag&FETCH_LOAN_OVER)&&(data.Ftotal_available_balance+backer_balance-data.Ftotal_t1fetch_fee_old+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old> data.Ffetch_limit_offset
        && data.Ftotal_available_balance+backer_balance-data.Ftotal_t1fetch_fee_old-data.Ftotal_t1fetch_fee+data.Ftotal_redem_balance_old-data.Ftotal_buy_balance_old+data.Ftotal_redem_balance-data.Ftotal_buy_balance > data.Ffetch_limit_offset
        && data.Ftotal_available_balance+backer_balance > data.Ffetch_limit_offset
        && (data.Ffetch_limit_offset>0)))
    {
        //启动t+0提现
        int flag = (data.Fflag&(~FETCH_LOAN_OVER));
        querySql += string(" Fflag = ")+toString(flag)+" ,"; 
        needUpdateCkv = true;
    }
    

    querySql += string(" Fmodify_time = '")+pMysql->EscapeStr(data.Fmodify_time)+"' ";   
    
    querySql += string(" WHERE Fid=")+toString(data.Fid)+" AND Flstate=1 ";   
   
    
    gPtrAppLog->debug("[%s][%d]%s",__FILE__,__LINE__,querySql.c_str());
    // 执行SQL
    pMysql->Query(querySql.c_str(), querySql.length());
    // 判断影响行数是否唯一
    if (pMysql->AffectedRows() != 1)
    {
        throw CException(ERR_DB_AFFECTED, "affected row more than 1!", __FILE__, __LINE__);
    }
    
    if (needUpdateCkv)
    {
        setFundBalanceConfigToCkv(pMysql);
    }
    
}


/*
更新余额配置表信息到ckv
*/
bool setFundBalanceConfigToCkv(CMySQL* pMysql)
{
    FundBalanceConfig data;
    memset(&data,0,sizeof(data));
    data.Fid = DEFAULT_BALNCE_CONFIG_FID;
    return setFundBalanceConfigToCkv(pMysql,data,true);
}

bool setFundBalanceConfigToCkv(CMySQL* pMysql,  FundBalanceConfig & data,bool NeedQuery)
{
    if (NeedQuery)
    {
        data.Fid = DEFAULT_BALNCE_CONFIG_FID;
        if (queryFundBalanceConfig(pMysql, data,  false) == false)
        {
            throw CException(ERR_DB_UNKNOW, "record not exist ", __FILE__, __LINE__);
        }
    }

    string key = "fund_balance_config_"+ toString(data.Fid) ;

    string szValue;

    CParams kvReqSet;
    //设置要修改的数据szValue
    kvReqSet.setParam("Fid",data.Fid);
    kvReqSet.setParam("Fbalance_spid",data.Fbalance_spid);
    kvReqSet.setParam("Fcharge_spid",data.Fcharge_spid);
    kvReqSet.setParam("Ffetch_backer_spid",data.Ffetch_backer_spid);
    kvReqSet.setParam("Fbalance_spid_qqid",data.Fbalance_spid_qqid);
    kvReqSet.setParam("Ffetch_backer_qqid",data.Ffetch_backer_qqid);
    kvReqSet.setParam("Ftrans_date",data.Ftrans_date);
    kvReqSet.setParam("Ftotal_redem_balance_old",data.Ftotal_redem_balance_old);
    kvReqSet.setParam("Ftotal_redem_balance",data.Ftotal_redem_balance);
    kvReqSet.setParam("Ftotal_available_balance",data.Ftotal_available_balance);
    kvReqSet.setParam("Ffetch_backer_total_fee",data.Ffetch_backer_total_fee);
    kvReqSet.setParam("Ffetch_limit_offset",data.Ffetch_limit_offset);
    kvReqSet.setParam("Fflag",data.Fflag);
    kvReqSet.setParam("Fsign",data.Fsign);

    szValue = kvReqSet.pack();

    //将szValue写入ckv
    if(gCkvSvrOperator->set(CKV_KEY_FUND_BALANCE_CONFIG, key, szValue))
    {
        return false;
    }
    else
    {
        return true;
    }

}


BalanceConfigCache getCacheBalanceConfig(CMySQL* pMysql)
{
	time_t tt = time(NULL);
	BalanceConfigCache* balanceConfig = &gPtrConfig->m_balanceConfigCache;
	// 使用缓存净值
	if(balanceConfig->timeout>tt){
		return gPtrConfig->m_balanceConfigCache;
	}
    // 取DB最新净值
    FundBalanceConfig data;
	memset(&data,0,sizeof(data));
	data.Fid = DEFAULT_BALNCE_CONFIG_FID;
    queryFundBalanceConfig(pMysql, data,  false);
	
	balanceConfig->spid=string(data.Fbalance_spid);
	balanceConfig->id=data.Fid;
	balanceConfig->curtype=CUR_FUND_BALANCE;
	balanceConfig->timeout=tt + 3600 * 12;// 本地缓存12小时;
	
	return gPtrConfig->m_balanceConfigCache;
}



