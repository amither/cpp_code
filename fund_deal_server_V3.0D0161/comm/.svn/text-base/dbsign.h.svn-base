#ifndef _DB_SIGN_H_
#define _DB_SIGN_H_

#include <stdint.h>
#include <sstream>

#include "decode.h"
#include "globalconfig.h"
#include "fund_commfunc.h"

extern CftLog* gPtrAppLog;

//各需要签名校验的表的字段配置
//#define SIGN_KEY    "020494b6787f58fabaae42bf544ca4c4" -- 安全考虑，启用新key
#define SIGN_KEY    "bff501d2c2454badac80dd66cd33460b"

enum eTableIndex{
	t_fund_profit = 0,
    t_fund_profit_record = 1,
    t_fund_user_total_acc = 2,
    t_fund_close_trans = 3,
    t_fund_close_profit_record = 4,
    t_fund_close_balance_rolllist = 5,
    t_freeze_fund = 6,
    t_unfreeze_fund = 7,
    t_fund_balance_order = 8,
    t_trade_fund = 9,
    t_trade_user_fund = 10,
    t_fund_bind = 11,
    t_fund_pay_card = 12,
    t_fund_trans_process = 13,
};


static const char *cTableIndex[] = {"t_fund_profit", "t_fund_profit_record", "t_fund_user_total_acc", 
	"t_fund_close_trans","t_fund_close_profit_record", "t_fund_close_balance_rolllist",
	"t_freeze_fund", "t_unfreeze_fund",  "t_fund_balance_order", "t_trade_fund","t_trade_user_fund",
	"t_fund_bind", "t_fund_pay_card", "t_fund_trans_process"};


/**
 * DB安全加固
 * DB敏感字段签名生成
 * 856380
 * 支持db数据和更新数据的merge
 * 建议更新操作使用该方法获取sign,避免增加查询以及污染待更新变量
 */
template<class T>
string genMergeSign(const char* sTableName, T const& data, T const& dbData)
{
    stringstream ss;
    char buff[128] = {0};
	
    int i = t_fund_profit;
    for(;i<=t_fund_trans_process;i++){
        if( !strcmp(sTableName, cTableIndex[i] )){            
            break;
        }
    }

    switch(i){
        case t_trade_fund:
		case t_trade_user_fund:{            
			ST_TRADE_FUND &tradeFund = (ST_TRADE_FUND &)data;
            ST_TRADE_FUND &dbTradeFund = (ST_TRADE_FUND &)dbData;
			ss << escapeString(strcmp("",tradeFund.Flistid)==0?dbTradeFund.Flistid:tradeFund.Flistid) << "|"
               << (tradeFund.Fpur_type==0?dbTradeFund.Fpur_type:tradeFund.Fpur_type) << "|"
               << escapeString(strcmp("",tradeFund.Fspid)==0?dbTradeFund.Fspid:tradeFund.Fspid) << "|"
               << escapeString(strcmp("",tradeFund.Ftrade_id)==0?dbTradeFund.Ftrade_id:tradeFund.Ftrade_id) << "|"
               << (tradeFund.Fuid==0?dbTradeFund.Fuid:tradeFund.Fuid) << "|"
               << (tradeFund.Ftotal_fee==0?dbTradeFund.Ftotal_fee:tradeFund.Ftotal_fee) << "|" 
               << (tradeFund.Fstate==0?dbTradeFund.Fstate:tradeFund.Fstate) << "|" 
               << SIGN_KEY;
            break;
		}
        case t_fund_bind:{
            ST_FUND_BIND &fundBind = (ST_FUND_BIND &)data;
            ST_FUND_BIND &dbFundBind = (ST_FUND_BIND &)dbData;
			ss << escapeString(strcmp("",fundBind.Ftrade_id)==0?dbFundBind.Ftrade_id:fundBind.Ftrade_id) << "|"
               << escapeString(strcmp("",fundBind.Fcre_id)==0?dbFundBind.Fcre_id:fundBind.Fcre_id) << "|"
               << escapeString(strcmp("",fundBind.Fqqid)==0?dbFundBind.Fqqid:fundBind.Fqqid) << "|"
               << (fundBind.Fuid==0?dbFundBind.Fuid:fundBind.Fuid) << "|"
               << escapeString(strcmp("",fundBind.Fmobile)==0?dbFundBind.Fmobile:fundBind.Fmobile) << "|" 
               << escapeString(strcmp("",fundBind.Fopenid)==0?dbFundBind.Fopenid:fundBind.Fopenid) << "|" 
               << SIGN_KEY;
            break;
        }
        case t_unfreeze_fund:{
			ST_UNFREEZE_FUND &unFreezeFund = (ST_UNFREEZE_FUND &)data;
            ST_UNFREEZE_FUND &dbUnFreezeFund = (ST_UNFREEZE_FUND &)dbData;
            ss << escapeString(strcmp("",unFreezeFund.Funfreeze_id)==0?dbUnFreezeFund.Funfreeze_id:unFreezeFund.Funfreeze_id) << "|"
               << escapeString(strcmp("",unFreezeFund.Ftrade_id)==0?dbUnFreezeFund.Ftrade_id:unFreezeFund.Ftrade_id) << "|"
               << (unFreezeFund.Ftotal_fee==0?dbUnFreezeFund.Ftotal_fee:unFreezeFund.Ftotal_fee) << "|"
               << (unFreezeFund.Fcontrol_fee==0?dbUnFreezeFund.Fcontrol_fee:unFreezeFund.Fcontrol_fee) << "|" 
               << (unFreezeFund.Fstate==0?dbUnFreezeFund.Fstate:unFreezeFund.Fstate) << "|" 
               << SIGN_KEY;
            break;
        }
        case t_freeze_fund:{
			ST_FREEZE_FUND &freezeFund = (ST_FREEZE_FUND &)data;
            ST_FREEZE_FUND &dbFreezeFund = (ST_FREEZE_FUND &)dbData;
            ss << escapeString(strcmp("",freezeFund.Ffreeze_id)==0?dbFreezeFund.Ffreeze_id:freezeFund.Ffreeze_id) << "|"
               << escapeString(strcmp("",freezeFund.Ftrade_id)==0?dbFreezeFund.Ftrade_id:freezeFund.Ftrade_id) << "|" 
               << (freezeFund.Fuid==0?dbFreezeFund.Fuid:freezeFund.Fuid) << "|" 
               << (freezeFund.Ftotal_fee==0?dbFreezeFund.Ftotal_fee:freezeFund.Ftotal_fee) << "|" 
               << (freezeFund.Fstate==0?dbFreezeFund.Fstate:freezeFund.Fstate) << "|" 
               << (freezeFund.Ftotal_unfreeze_fee==0?dbFreezeFund.Ftotal_unfreeze_fee:freezeFund.Ftotal_unfreeze_fee) << "|" 
               << SIGN_KEY;
            break;
        }
        case t_fund_trans_process:{
		    FundTransProcess &fundProcess = (FundTransProcess &)data;
		    FundTransProcess &dbProcess = (FundTransProcess &)dbData;
		    ss << (strcmp("",fundProcess.Ftrade_id)==0?dbProcess.Ftrade_id:fundProcess.Ftrade_id)<<"|";
		    ss << (strcmp("",fundProcess.Fspid)==0?dbProcess.Fspid:fundProcess.Fspid)<<"|";
		    ss << (fundProcess.Ftotal_fee==MIN_INTEGER?dbProcess.Ftotal_fee:fundProcess.Ftotal_fee)<<"|";
		    ss << (strcmp("",fundProcess.Flistid)==0?dbProcess.Flistid:fundProcess.Flistid)<<"|";
		    ss << SIGN_KEY;
            break;
        }
        default:return string("");
    }
    getMd5(ss.str().c_str(), ss.str().size(), buff);
    return string(buff);
}

/**
 * DB安全加固
 * DB敏感字段签名生成
 * 待迁移:  跟genMergeSign合并;只一个字段时,使用相同的参数表示db数据
 */
template<class T>
string genSign(const char* sTableName, T const& data)
{
    stringstream ss;
    char buff[128] = {0};

    int i = t_fund_profit;
    for(;i<=t_fund_trans_process;i++){
        if( !strcmp(sTableName, cTableIndex[i] )){            
            break;
        }
    }

    switch(i){
        case t_fund_profit:{
			FundProfit &fundProfit = (FundProfit &)data;
            ss << escapeString(fundProfit.Ftrade_id) << "|"
               << fundProfit.Fcurtype << "|"
               << fundProfit.Frecon_balance << "|"
               << fundProfit.Frecon_day << "|"
               << fundProfit.Fprofit << "|"
               << fundProfit.Freward_profit << "|"
               << fundProfit.Ftotal_profit << "|" << SIGN_KEY;
            break;
        }
        case t_fund_profit_record:{
			FundProfitRecord &fundProfitRecord = (FundProfitRecord &)data;
            ss << escapeString(fundProfitRecord.Flistid) << "|"
               << escapeString(fundProfitRecord.Fsub_trans_id) << "|"
               << escapeString(fundProfitRecord.Ftrade_id) << "|"
               << fundProfitRecord.Fcurtype << "|"
               << escapeString(fundProfitRecord.Fspid) << "|"
               << fundProfitRecord.Ftotal_profit << "|"
               << fundProfitRecord.Fprofit << "|"
               << escapeString(fundProfitRecord.Fday) << "|" 
               << fundProfitRecord.Fend_tail_fee << "|" << SIGN_KEY;
            break;
        }
        case t_fund_user_total_acc:{
			FundUserTotalAcc &fundUserTotalAcc = (FundUserTotalAcc &)data;
            ss << escapeString(fundUserTotalAcc.Ftrade_id) << "|"
               << fundUserTotalAcc.Fbusiness_type << "|"
               << fundUserTotalAcc.Fbalance << "|"
               << fundUserTotalAcc.Ffreeze << "|" << SIGN_KEY;
            break;
        }        
        case t_fund_close_trans:{
			FundCloseTrans &fundCloseTrans = (FundCloseTrans &)data;
            ss << escapeString(fundCloseTrans.Ftrade_id) << "|"
               << fundCloseTrans.Fseqno << "|"
               << escapeString(fundCloseTrans.Ffund_code) << "|"
               << fundCloseTrans.Fcurrent_total_fee << "|" 
               << fundCloseTrans.Fend_tail_fee << "|" << SIGN_KEY;
            break;
        }
        case t_fund_close_profit_record:{
			FundCloseProfitRecord &fundCloseProfitRecord = (FundCloseProfitRecord &)data;
            ss << fundCloseProfitRecord.Fid << "|"
               << escapeString(fundCloseProfitRecord.Flistid) << "|"
               << fundCloseProfitRecord.Fclose_id << "|"
               << escapeString(fundCloseProfitRecord.Ftrade_id) << "|"
               << escapeString(fundCloseProfitRecord.Ffund_code) << "|"
               << fundCloseProfitRecord.Ftotal_fee << "|"
               << fundCloseProfitRecord.Fprofit << "|"
               << escapeString(fundCloseProfitRecord.Fday) << "|" << SIGN_KEY;
            break;
        }
        case t_fund_close_balance_rolllist:{
			FundCloseBalanceRolllist &fundCloseBalanceRolllist = (FundCloseBalanceRolllist &)data;
            ss << escapeString(fundCloseBalanceRolllist.Flistid) << "|"
               << fundCloseBalanceRolllist.Fclose_id << "|"
               << escapeString(fundCloseBalanceRolllist.Ftrade_id) << "|"
               << escapeString(fundCloseBalanceRolllist.Ffund_code) << "|"
               << fundCloseBalanceRolllist.Fbiz_fee << "|"
               << fundCloseBalanceRolllist.Fsubject << "|"
               << escapeString(fundCloseBalanceRolllist.Facc_time) << "|" << SIGN_KEY;
            break;
        }
		case t_freeze_fund:{
			ST_FREEZE_FUND &freezeFund = (ST_FREEZE_FUND &)data;
            ss << escapeString(freezeFund.Ffreeze_id) << "|"
               << escapeString(freezeFund.Ftrade_id) << "|" 
               << freezeFund.Fuid << "|" 
               << freezeFund.Ftotal_fee << "|" 
               << freezeFund.Fstate << "|" 
               << freezeFund.Ftotal_unfreeze_fee << "|" << SIGN_KEY;
            break;
        }
		case t_unfreeze_fund:{
			ST_UNFREEZE_FUND &unFreezeFund = (ST_UNFREEZE_FUND &)data;
            ss << escapeString(unFreezeFund.Funfreeze_id) << "|"
               << escapeString(unFreezeFund.Ftrade_id) << "|"
               << unFreezeFund.Ftotal_fee << "|"
               << unFreezeFund.Fcontrol_fee << "|" 
               << unFreezeFund.Fstate << "|" << SIGN_KEY;
            break;
        }
        case t_fund_balance_order:{
			ST_BALANCE_ORDER &balanceOrder = (ST_BALANCE_ORDER &)data;
            ss << escapeString(balanceOrder.Flistid) << "|"
               << balanceOrder.Ftotal_fee << "|"
               << escapeString(balanceOrder.Ftrade_id) << "|"
               << balanceOrder.Ftype << "|" 
               << balanceOrder.Fuid << "|" << SIGN_KEY;
            break;
        }
		case t_trade_fund:
		case t_trade_user_fund:{
			ST_TRADE_FUND &tradeFund = (ST_TRADE_FUND &)data;
			ss << escapeString(tradeFund.Flistid) << "|"
               << tradeFund.Fpur_type << "|"
               << escapeString(tradeFund.Fspid) << "|"
               << escapeString(tradeFund.Ftrade_id) << "|"
               << tradeFund.Fuid << "|"
               << tradeFund.Ftotal_fee << "|" 
               << tradeFund.Fstate << "|" << SIGN_KEY;
            break;
		}
        case t_fund_bind:{
            ST_FUND_BIND &fundBind = (ST_FUND_BIND &)data;
			ss << escapeString(fundBind.Ftrade_id) << "|"
               << escapeString(fundBind.Fcre_id) << "|"
               << escapeString(fundBind.Fqqid) << "|"
               << fundBind.Fuid << "|"
               << escapeString(fundBind.Fmobile) << "|" 
               << escapeString(fundBind.Fopenid) << "|" << SIGN_KEY;
            break;
        }
        case t_fund_pay_card:{
            FundPayCard &fundPayCard = (FundPayCard &)data;
			ss << escapeString(fundPayCard.Ftrade_id) << "|"
               << escapeString(fundPayCard.Fqqid) << "|"
               << escapeString(fundPayCard.Fbind_serialno) << "|"
               << escapeString(fundPayCard.Fbank_id) << "|"
               << escapeString(fundPayCard.Fmobile) << "|" << SIGN_KEY;
            break;
        }
        case t_fund_trans_process:{
			return genMergeSign(sTableName,data,data);
        }
        default:return string("");
    }

    getMd5(ss.str().c_str(), ss.str().size(), buff);
    
    return string(buff);
}

/**
 * 检验签名
 * sTableName:表名 data:表对应的数据结构 
 * handleFlag:hash校验失败后处理方式 0-不做处理；1-记录错误日志 2-记录错误日志加告警 3-记录错误日志加告警并抛出异常，结束流程
 * true:当前配置无需校验或者需校验且校验通过；false:需校验但校验不通过
 */
template<class T>
bool checkSign(const char* sTableName, T const& data, int handleFlag=1)
{	
    //检验hash key
    string strGenSign = genSign( sTableName , data);
    if( !strcmp(data.Fsign, strGenSign.c_str()) ){
        return true;
    }

    char szMsg[256] = {0};
    snprintf(szMsg, sizeof(szMsg), "%s表tradeid:%s对应记录DB hash(data.Fsign:%s != genSign:%s)校验失败", 
                sTableName, data.Ftrade_id, data.Fsign, strGenSign.c_str());
    switch( handleFlag ){
        case 0:{
            break;
        }
        case 1: case 2: case 3:{
            TRACE_ERROR("check DB hash key is failed,Table[%s] Ftrade_id[%s] dbSign[%s] genSign[%s]", 
                            sTableName, data.Ftrade_id, data.Fsign, strGenSign.c_str());
            //if( handleFlag == 2 || handleFlag == 3 ){ alert(ERR_DB_SIGN_CHECK_FAILED, szMsg); }
            //if( handleFlag == 3 ){ throw CException(ERR_DB_SIGN_CHECK_FAILED, szMsg, __FILE__, __LINE__); }
            break;
        }
    }
	return false;
}

#endif

