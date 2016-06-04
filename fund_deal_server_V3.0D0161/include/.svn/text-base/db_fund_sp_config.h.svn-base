#ifndef _DB_FUND_SP_CONFIG_H_
#define _DB_FUND_SP_CONFIG_H_

#include "parameter.h"
#include "common.h"
#include "cftlog.h"
#include "sqlapi.h"
#include "trpcwrapper.h"
#include "runinfo.h"
#include <sstream>
#include  "tinystr.h"
#include  "tinyxml.h"
#include "decode.h"
#include "sqlapi.h"
#include "globalconfig.h"

typedef struct FundSpConfig
{
    char Fspid[16+1];                      // 商户号
    char Fsp_name[64+1];                   // 商户简称
    char Ffund_code[64+1];                 // 基金代码
    char Ffund_name[64+1];                 // 基金全称
    char Fsp_full_name[64+1];              // 商户全称
    char Ffund_brief_name[64+1];           // 基金简称
    
    int Flstate;                           // 物理状态：1:有效;2:无效(所有用户不可用);3:有限可用（已经绑定的用户可使用
    int Fpurpose;                          // 基金发行用途: 1：余额增值
    int Fbind_valid;                       // 是否可绑定状态: 1：可绑定;2：不可绑定
    int Fbuy_valid;                        // 是否可申购状态: 0x1：支持申购;0x2：支持认购
    
    //1：可赎回;2：不可赎回;3：只可做T+1赎回
    //以上3个状态值的判断需要通过Fredem_valid&0x07做与计算后判断。
    //扩展如下bit位：
    //Fredem_valid&0x08=1 标识Fredem_day不能再做转换
    //Fredem_valid&0x10=1 标识不能转入改基金公司
    int Fredem_valid;                      // 是否可赎回状态
    LONG Fredem_total;                     // 累计赎回额度（循环额度，用于计算基金公司基金代码可用垫资额度，结算后该值会相应减少)
    char Fcreate_time[20+1];               // 创建时间
    char Fmodify_time[20+1];               // 修改时间
    char Fmemo[128+1];                     // 
    int Fstandby1;                         // 
    int Fstandby2;                         // 
    char Fstandby3[64+1];                  // 
    char Fstandby4[128+1];                 // 
    char Fstandby5[20+1];                  // 
    char Fstandby6[20+1];                  // 
    char Fdebt_charge_bankid[128+1];       // 净赎回充值银行号
    int Fcurtype;                          // 币种类型：(和核心账户币种类型一致)
    LONG Fsp_chargenum;                    // 基金公司垫子金额
    LONG Fcft_chargenum;                   // 腾讯充值垫资金额
    char Fcharge_bankid[128+1];            // 基金公司转账充值卡号
    char Ftplus_redem_spid[32+1];          // T+1赎回出资账户
    char Frealtime_redem_spid[32+1];       // T+0赎回垫资账户
    char Ftotal_charge_spid[32+1];         // 请款总账户
    char Fredem_day[20+1];                 // 快速赎回累加日期
    LONG Fredem_total_day;                 // Fredem_day 日累加快速赎回金额（包括转换,用于计算垫资）
    int Fredem_exflag;                     // 赎回标志
    char Fchange_charge_spid[32+1];        // 转换商户号
    int Ftype;                             // 基金类型：1 货币型基金;2 定期型基金;3 保险型基金;4 指数型基金
    int Fclose_flag;                       // 是否封闭标志:1 不封闭，可以任意购买赎回;2 封闭，指定时间开放申购、赎回;3 半封闭，任何时间都可以买入，只能指定时间赎回
    int Ftransfer_flag;                    // 是否可以进行转换交易
    int Ffirst_settlement_date;            // 认购结算周期:【未用到】
    int Fnormal_settlement_date;           // 申购结算周期:【未用到】
    int Fsave_sett_time;                   // 申购结算时间:申购结算时间：1，那就是T+1，2就是t+2,0就是D日
    int Ffetch_sett_time;                  // 赎回结算时间：1，那就是T+1，2就是t+2,0就是D日
    int Fsett_type;                        // 结算方式:0表示轧差，1表示不咋差
    int Fend_type;                         // 定期到期动作:1 到期自动滚动到下一期; 2 到期自动普通赎回
    char Fstart_date[20+1];                // 该基金开始日期【未用到】清盘使用
    char Fend_date[20+1];                  // 该基金开始日期【未用到】清盘使用
    int Fduration_type;                    // 运行周期单位:0 无周期限制;1 以自然日为单位;2 以月为单位
    int Fduration;                         // 基金的运行周期长度,0表示无周期
    // 0 不限制
    // 0x1 限制购买次数(同一交易日内的所有交易计算一次)
    // 0x2 限制赎回次数(同一交易日内的所有交易计算一次)
    int Frestrict_mode;                    // 基金运行周期内的交易限制模式【定期使用】
    int Frestrict_num;                     // 限制交易频次，与Frestrict_mode结合使用
    int Fstate;                            // 绑定状态:1：初始状态;2：认购期;3：已成立
    LONG Fscope_upper_limit;               // 基金规模上限.单位:分。0表示无限制
    LONG Fscope_lower_limit;               // 基金规模下限.单位:分。0表示无限制。否则表示基金最小允许成立的规模（需要通过认购募集的才会用到）
    LONG Fscope;                           // 基金规模当前值
    LONG Fscope_upper_limit_offset;        // 总规模上限buffer
    LONG Fbuy_first_lower_limit;           // 首次购买最低限额，默认1（1分钱）
    LONG Fbuy_lower_limit;                 // 非首次购买最低限额，默认1（1分钱）
    LONG Fbuy_add_limit;                   // 申购追加金额限制，默认1（1分钱）
    char Fstat_buy_tdate[20+1];            // 标识当前统计的申购额度所在的日期yyyyMMdd
    LONG Ftotal_buyfee_tday;               // 每日购买额度
    LONG Fbuyfee_tday_limit;               // 每日购买限额
    LONG Fbuyfee_tday_limit_offset;        // 每日购买限额buffer
    char Fstat_redeem_tdate[20+1];         // 标识当前统计的赎回额度所在的日期yyyyMMdd
    LONG Ftotal_redeem_tday;               // 每日赎回额度,用于统计净申购
    // 0x1 按照D日24点统计额度计算限额.(默认T日)
    // 0x2 净申购按照净申购计算限额(默认限额纯粹申购)
    int Fstat_flag;                        // 额度统计标识
    int Frisk_ass_flag;                    // 是否需要风险测评标记:0不需要风险测评;1必须进行风险测评
    int Frisk_type;                        // 产品风险类型:1-保守型;2-稳健型;3-进取型;
    int Fbuy_confirm_type;                 // 申购份额确认类型:0 实时确认（默认值）;1 T+1确认
}FundSpConfig;

#define FUNDCODE_PURCHASE_VALID 0x1 //	基金可以申购
#define FUNDCODE_SUBSCRIBE_VALID 0x2 //	基金可以认购
#define FUNDCODE_BUY_DAY_LIMIT 0x4 //	今日申购额满
#define RESTRICT_MODE_BUY 0x1 //	限制购买次数
#define RESTRICT_MODE_REDEM 0x2 //	限制赎回次数

#define CLOSE_FLAG_NORMAL 1 //	不封闭，可以任意购买赎回
#define CLOSE_FLAG_ALL_CLOSE 2 //	封闭，指定时间开放申购、赎回
#define CLOSE_FLAG_SEMI_CLOSE 3 //	半封闭，任何时间都可以买入，只能指定时间赎回


#define SPCONFIG_TYPE_BALANCE 0 //  理财通余额
#define SPCONFIG_TYPE_DEMAND 1 //  活期基金
#define SPCONFIG_TYPE_CLOSE 2 //  定期基金
#define SPCONFIG_TYPE_INSURE 3 //  保险基金
#define SPCONFIG_TYPE_ETF 4 //	指数型基金

#define SPCONFIG_BALANCE_PAY_CONFIRM  0 // 支付确认份额
#define SPCONFIG_BALANCE_T1_CONFIRM  1 // T+1确认份额

#define SPCONFIG_STAT_TDAY 0 //	交易日标识(默认)
#define SPCONFIG_STAT_DDAY 0X1 //	自然日标识
#define SPCONFIG_STAT_NORMAL 0 // 单独统计申购限额(默认)
#define SPCONFIG_STAT_NET 0X2 // 统计净申购限额

#define SPCONFIG_RISK_NONE 0 // 不需要进行风险评测
#define SPCONFIG_RISK_NEED 1 // 需要进行风险评测 

bool sumRedomTotal(CMySQL* pMysql, LONG& allRedomTotal);

bool queryFundSpAndFundcodeConfig(CMySQL* pMysql, FundSpConfig& data,  bool lock);

bool queryFundSpConfig(CMySQL* pMysql, vector<FundSpConfig>& dataVec,const string& spid, bool lock);

bool queryFundSpAndFundcodeFromCkv(FundSpConfig& data, bool throwExp = false);

bool queryFundSpAllConfig(CMySQL* pMysql, vector<FundSpConfig>& dataVec,  bool lock);

void updateFundSpRedomTotal(CMySQL* pMysql, FundSpConfig& data, LONG& total_fee ,int userLoading,const string& acc_time);

void updateFundSpRedom(CMySQL* pMysql, FundSpConfig& data, LONG& redemfee , LONG& redemScope,int userLoading,const string& acc_time);

void checkSpidAndFundcode(CMySQL* pMysql, FundSpConfig& data);

void checkFundSpAndFundcode(CMySQL* pMysql, FundSpConfig& data, bool spMustValid);

bool setFundSpInfo(FundSpConfig& data);

bool setSupportFundSpAndFundcode(FundSpConfig& data);

bool setAllSupportSpConfig(CMySQL* pMysql);

void checkRedemOverLoading(CMySQL* pMysql, FundSpConfig& fundSpConfig ,LONG total_fee,bool isLocked);

bool preCheckSpLoaningEnough(CMySQL* pMysql,const string&spid,const string&fund_code,LONG total_fee);

void querySpConfigCache(CMySQL* pMysql, string spid, SpConfigCache& sp_config);

int querySubaccCurtype(CMySQL* pMysql, string spid);

void updateSpRedemFlag(CMySQL* pMysql, FundSpConfig& data);

void updateSpBuyfeeTday(CMySQL* pMysql, FundSpConfig& data);
void checkFundcodePurchaseValid(int buy_valid_flag);

void checkFundcodeToScopeUpperLimit(int uid,const string &sys_time,LONG total_fee, FundSpConfig& data, bool needQueryDB);

void updateFundSpBuyValid(FundSpConfig& data);

void checkAndUpdateFundScope(FundSpConfig& data, LONG total_fee,const string &trade_date,bool *pNeedRefund=NULL,string *refunddesc=NULL,bool bPresentBuy=false,double redeemRate=1);

void addFundSpScope(FundSpConfig &data, const string &acc_time, LONG fee);

#endif

