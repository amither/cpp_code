/**
  * FileName: fund_commfunc.h
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金交易服务 公共函数 头文件
  */


#ifndef _FUND_COMMFUNC_H_
#define _FUND_COMMFUNC_H_


#include "cftlog.h"
#include "common.h"
#include "fund_common.h"
#include "UrlAnalyze.h"
#include "parameter.h"
#include "decode.h"
#include "error.h"
#include "sqlapi.h"
#include "exception.h"
#include "fund_struct_def.h"
#include "trpcwrapper.h"
//#include "serruninfo.h"
#include "runinfo.h"
#include "globalconfig.h"
#include "source_command_no.h"
#include "appcomm.h"
#include "ckv_svr_operator.h"
#include "user_ttc.h"
#include "global_access_limit.h"
#include "errorsum.h"
#include "dbconfig.h"
#include "tools_comm.h"




#include "db_fund_bind_sp.h"
#include "db_fund_profit.h"
#include "db_change_sp_record.h"
#include "db_fund_prepay.h"
#include "db_fund_recon_log.h"
#include "db_fund_config.h"
#include "db_fund_bank_config.h"
#include "db_fund_sp_config.h"
#include "db_fund_profit_rate.h"
#include "db_fund_profit_record.h"
#include "db_fund_trans_date.h"
#include "db_c2c_db_t_user.h"
#include "db_fund_fetch.h"
#include "db_fund_pay_card.h"
#include "db_fund_user_total_acc.h"
#include "db_fund_user_total_acc_rolllist.h"
#include "db_fund_transfer.h"
#include "db_fund_dynamic.h"
#include "db_fund_sp_config.h"
#include "db_freeze_fund.h"
#include "db_unfreeze_fund.h"
#include "db_fund_close_trans.h"
#include "db_fund_close_cycle.h"
#include "db_fund_balance_order.h"
#include "db_trade_fund.h"
#include "db_fund_close_profit_record.h"
#include "db_fund_user_acc.h"
#include "db_fund_balance_config.h"
#include "db_fund_close_balance_rolllist.h"
#include "db_fund_control_balance.h"
#include "db_pre_record_user_acc.h"
#include "db_fund_user_risk.h"
#include "db_fund_unconfirm.h"
#include "db_fund_account_freeze.h"
#include "db_fund_infomation.h"
#include "db_fund_trans_process.h"
#include "ckv_cash_in_transit.h"
#include <libgen.h>
#include <sstream>
#include <list>
using std::list;


extern CRpcWrapper* gPtrSubaccRpc;
extern CRpcWrapper* gPtrBindQueryRpc;
extern CRpcWrapper* gPtrQuerySubAccBanlanceRpc;
extern CftLog* gPtrAppLog;
extern CftLog* gPtrSysLog;
extern GlobalConfig* gPtrConfig;
extern CkvSvrOperator *gCkvSvrOperator;
extern CkvSvrOperator *gSubAccCkvSvrOperator;

extern CMySQL* gPtrFundDB;
extern CMySQL  *gPtrFundSlaveDB;

extern map<string,CMySQL*> subaccDB;
extern CRpcWrapper* gPtrExauRpc; 

extern UserClassify* g_user_classify; 



#define CHECK_PARAM_EMPTY(param)                                        \
do                                                                      \
{                                                                       \
    if (m_params.getString(param).empty())                              \
    {                                                                   \
        throw EXCEPTION(ERR_BAD_PARAM, param" not found, or empty");    \
    }                                                                   \
}                                                                       \
while(0)

#define SET_PARAM(MSG, PARAM, ARG, ...)      (CUrlAnalyze::setParam(MSG, PARAM, toString(ARG).c_str(), ## __VA_ARGS__))

/**
  * 赋值参数操作宏
  */
#define ASN_INT(MSG, NAME, DST)              (CUrlAnalyze::getParam(MSG, NAME, &(DST)))
#define ASN_LONG(MSG, NAME, DST)             {char ASN_TMP[1024] = {0}; (CUrlAnalyze::getParam(MSG, NAME, ASN_TMP, 1023)); DST = atoll(ASN_TMP);}
#define ASN_STR(MSG, NAME, DST)              {char ASN_TMP[1024] = {0}; (CUrlAnalyze::getParam(MSG, NAME, ASN_TMP, 1023)); DST = ASN_TMP;}
#define ASN_SZ(MSG, NAME, DST, SIZE)         (CUrlAnalyze::getParam(MSG, NAME, DST, SIZE))

//缓存用户交易记录的最大条数
#define CACHE_USR_FUND_TRADE_MAX_NUM 21

string StrUpper(const string &str);
int Sdb1(int s);
int Stb1(int s);
int Sdb2(const char *s);
int Stb2(const char *s);
int Stb2_listid(const char *s);
int Sdb2_qqid(const char *s);
int Stb2_qqid(const char *s);

bool IsTimeLimt(string lower_time, string taller_time, string cur_time = "");
string SetTodayTime(string time);
string GetDateTodayStr();


void GetTimeNow(char *str);
int GetDateToday();
string getTime_yyyymm(int monOffset=0);
string GetTimeToday();
string getLastDate();
void GetTimeColumn(int &year, int &month, int &day, int &hour, int &minute, int &second);
string toTodayTime(const char* time);
unsigned int toUnixTime(const char* time);
string toLocalTime(time_t t_time);
string getAmountStr(LONG amount);
string addDays(const string& str, int days);

string changeDatetime2Date(const char* date);
string changeDateFormat(string date ,int type =1);
string changeDatetimeFormat( string date );


string escapeString(const string& str);
void getDecodeMsg(TRPC_SVCINFO* pRequst, char* szBuf, char* szSpId) throw (CException);
char* getMd5(const char *key, int len, char *szRes);
string getMd5(const string &src_text);
unsigned int getBillno(const string& strIP, int iPort, int iAppid) throw (CException);
string genSubaccDrawid() throw (CException);

/**
 * TRPC 告警
 */

void callErrorRpc(TRPC_SVCINFO* pRequst, CftLog* pLog);
bool isTrpcErrMsg(TRPC_SVCINFO* rqst);

int createSubaccUser(TRPC_SVCINFO* m_request, const string &spid, const string &uin , const string &client_ip,int subacc_curType=-1);
int SubaccCreateUser(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip,int subacc_curType);
int SubaccSave(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string &desc, const string & acc_time,int type,int subacc_curType=-1);
int SubaccDraw(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,const string&controllist="");
int SubaccFetchReq(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,int subacc_curType=-1);
int SubaccFetchResult(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,int result_sign,int subacc_curType=-1);
int SubaccFreeze(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &transaction_id, const LONG &total_fee, const string & acc_time,const string &desc);
int SubaccUnFreeze(CRpcWrapper* pRpc, const string &spid, const string &uin , const string &client_ip, const string &unfreeze_id,const string &freeze_id, const LONG &total_fee, const string & acc_time,const string &control_list,const LONG &control_fee,const string & desc);

string TransIdentityNumber(const char* cre_id) throw (CException);
string GenerateTradeid(int uid);
string getUidFromTradeid(string tradeid);

string GetTradeDateFromListid(const char* listid);

int QueryFetchType(CMySQL* mysql, const string &Fcft_fetch_no);


bool QueryFundBindByCre(CMySQL* mysql, int cre_type, const char* cre_id, ST_FUND_BIND* pstRecord, bool islock = false);
bool QueryFundBindByUid(CMySQL* mysql, int uid, ST_FUND_BIND* pstRecord, bool islock = false);
bool QueryFundBindByUin(CMySQL* mysql, const string &uin, ST_FUND_BIND* pstRecord, bool islock = false);

bool QueryFundBindByTradeid(CMySQL* mysql, const char* trade_id, ST_FUND_BIND* pstRecord, bool islock = false,bool valid=true);
bool QueryTradeFund(CMySQL* mysql, const char* listid, int pur_type, ST_TRADE_FUND* pstRecord, bool islock = false);
bool QueryTradeFundByBankBillno(CMySQL* mysql, const char* cft_bank_billno, int bank_type, ST_TRADE_FUND* pstRecord, bool islock = false);
bool QueryBatchTradeFund(CMySQL* mysql, ST_TRADE_FUND& input, vector<ST_TRADE_FUND>& dataVec, bool islock);

bool StatTransFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, LONG& fee, string purTypeConf, bool islock = false);
bool StatTransNotAckFee(CMySQL* mysql, ST_TRADE_FUND& pstRecord, LONG& fee, string purTypeConf, bool islock = false);
bool StatHisotryNotAckFee(CMySQL* mysql,ST_TRADE_FUND& pstRecord,LONG &fee,string purTypeConf, bool islock = false);
bool StatTodayAckPurchaseFee(CMySQL* mysql,ST_TRADE_FUND& pstRecord,LONG &fee,string purTypeConf, bool islock = false);
bool StatTodayAckRedemFee(CMySQL* mysql,ST_TRADE_FUND& pstRecord,LONG &fee,string purTypeConf, bool islock = false);


void InsertFundBind(CMySQL* mysql, ST_FUND_BIND* pstRecord);
void InsertTradeFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord);
void InsertTradeFundLog(CMySQL* mysql, ST_TRADE_FUND* pstRecord, const string &systime);

void InsertTradeUserFund(CMySQL* mysql, ST_TRADE_FUND* pstRecord);
int UpdateTable(CMySQL* db_conn, const string &tb_name, const stringstream &ss_cond, const map<string, string> &kv_map);

int QueryFundTime(CMySQL *hsql, list<ST_FUND_TIME> &l_res, const char *sp_id = NULL, bool is_lock = false);

string getTradeDesc(int pur_type);

void UpdateFundTrade(CMySQL* mysql, ST_TRADE_FUND& pstRecord, ST_TRADE_FUND& pstDbRecord, const string &systime, bool updateUserTable = true);

void UpdateFundBindAssetLimit(CMySQL* mysql, ST_FUND_BIND& data);
void UpdateFundBind(CMySQL* mysql, ST_FUND_BIND& fundBind, ST_FUND_BIND& dbFundBind, const string &systime);
void UpdateFundBindFlstate(CMySQL* mysql, ST_FUND_BIND& data);
bool setFundBindToKV(CMySQL* mysql, ST_FUND_BIND& fundBind,bool needQuery = false);

/**
 * 根据db数据，将需要写入CKV的用户基本信息拼接成value串
 * @param fundBind 用户基本信息
 * @param value 返回的拼接串
 * @return 0-成功，其它-失败
 */
int packUsrFundBindCkvValue(const ST_FUND_BIND& fundBind, string &value);

bool delFundbindToKV(string uin);

bool setTradeRecordsToKV(CMySQL* mysql, ST_TRADE_FUND& trade_fund);

/**
 * 组装用户交易记录的CKV值
 * @param list 用户交易记录
 * @param value 组装后的ckv值
 * @return 0-成功，其它-失败
 */
int packTradeRecordsCkvValue(const vector<ST_TRADE_FUND> &list, string &value);

void checkExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int redem_type);

void checkBalanceFetchExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int req_type);

void updateBalanceFetchExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int req_type);

void checkExauLimit(CRpcWrapper* pRpc, const char* pMsg , const char* pspid);

void updateExauAuthLimit(CRpcWrapper* pRpc, int iUid, LONG vAmount, string creId,int redem_type);

bool checkWhitePayUser(string uin);


void notifyTradeSuccInfo(ST_TRADE_FUND& pstRecord);

//根据申购单号查询提现请求表(判断是否是提现失败重新申购)
int queryFundFetchList(CMySQL* mysql,const string &fundTransid,const string &date,const string &Fnum,string Fuin);

//判断提现失败重新申购
bool checkInnerBalancePayForReBuy(CMySQL* mysql,const string &fundTransid,const string &Fnum,string Fuin);

/**
 * 按用户统计某日的T+1赎回总份额(T-2之后到recondDay之前的总份额)
 * 返回: 查询结果
 */
LONG StatTplusRedemFee(CMySQL* mysql, const string& recondDay,const string& trade_id,int curtype);

void setStopFetchFlag(CMySQL* pMysql);

/**
 * 新增基金账户注销记录
 */
void InsertFundUnBind(CMySQL* mysql, ST_FUND_UNBIND* pstRecord);


/**
 * 基金账户注销更新绑定记录为无效，并更新qqid为tradeid
 */
void disableFundBind(CMySQL* mysql, ST_FUND_BIND* pstRecord);

/**
 * 查询用户是否注销
 */
bool checkIsUserUnbind(CMySQL* mysql, int uid);

/**
 * 恢复已经注销的用户
 */
void recoveryFundBind(CMySQL* mysql, ST_FUND_UNBIND &unbindInfo);

/**
 * 统计用户交易笔数
 */
int countTranRecords(CMySQL* mysql,int uid,const string &cond);



/**
 * 注销后重新恢复账户把注销表记录改为无效
 */
void disableFundUnBind(CMySQL* mysql, ST_FUND_UNBIND &unbindRecord);

/**
 * 查询用户最近一笔注销记录
 */
int getLastUnbindUid(CMySQL* mysql, ST_FUND_UNBIND &unbindInfo);

/**
 * 查询销户表信息
 */
bool getUnbindInfoByTradeid(CMySQL* mysql, ST_FUND_UNBIND &unbindInfo);

/**
 * 统计提现中记录
 */
int countFetchingRecords(CMySQL* mysql,const string& uin,const string &yyyymm);

 /*
 *查询成功ret >0 结果保存到result
 */
int doBatchSelect(CMySQL* mysql, const string&sql,CStr2sVec &result);

 /*
 *查询商户保有量
 */
LONG QuerySpTotalBalance(CMySQL* mysql, const string &spid);

 /*
 *校验商户今日总赎回占昨日保有量的比例，大于2%时停止转换
 */
void checkSpRedemRateLimit(CMySQL* mysql, FundSpConfig &spconfig,const string &systime,LONG transferFee);

//10位商户号+ 8位日期+ 10位随机数
string GenerateIdsBySpid(const string &spid);

//校验订单号的前缀必须是商户号
bool checkTransIdAndSpid(const string &spid,const string &transid);

//判断用户总资产是否超出限制
bool isUserAssetOverLimit(int limitLevel,LONG currentTotalAsset,LONG totalFee);

//刷新FundBind表的签名字段
void updateSignForFundBind(CMySQL* mysql, ST_FUND_BIND &bindInfo);

/**
 * 根据key no获取key前缀
 * @param key_no 
 * @return 
 */
string get_ckv_key_prefix(int key_no);
/**
 * 去除key前缀
 * @param key_no 
 * @param key 
 * @return 
 */
string adapt_ckv_key(int key_no,const string &key);

//获取用户风险评级结果 
int getAgreeRiskType(FundSpConfig& spConfig,ST_FUND_BIND& fundBind);

bool checkUserWhiteList(const string &key, const string &uid);   

//key 可以直接传uin，也可以传带前缀的真实key
string getUserWhiteListValue(string key);

//更新用户权限位
bool updateUserWhiteListValue(string key,const string &value);

/**
 * 统计用户申购交易金额
 */
LONG getBuyRecordsFee(CMySQL* mysql,int uid,const string &cond);
/**
 * 统计用户赎回交易金额
 */
LONG getRedemRecordsFee(CMySQL* mysql,int uid,const string &cond);
 /**
 * 统计用户提现中的金额
 */
LONG getFetchingRecordsFee(CMySQL* mysql,const string& uin,const string &yyyymm);


// 把string乘以10的power次方取整转换为LONG型
// 只支持15个字符数以下的正数转换
// 失败返回-1;成功返回0
int str2Long(const string &sFloat,LONG &iFee,const int power );

//隐藏证件号码
void hideCreId(const char* creId, int cretype,char* creHideId);
// 隐藏姓名:GBK
void hideName(const char* name, char* nameHide);
// 隐藏英文字符
void hideChar(const char* str, char* hide,unsigned int hideStart, unsigned int hideLenth);

bool isUserExistRedemingRecords(CMySQL* pMysql,int uid);

bool isExistsUnconfirmRedemtion(CMySQL* pMysql,int uid, const string& spid);
// 从证件号中获取性别
int getSexFromCreId(const char* creId,int creType);

//检查是否是沪港通的交易时间
void CheckTradeLimit(CMySQL* mysql, string sDay) throw (CException);

#endif /* _FUND_COMMFUNC_H_ */

