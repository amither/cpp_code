/**
  * FileName: fund_common.h
  * Author: gloriage
  * Version :1.0
  * Date: 2010-7-12
  * Description: 基金应用服务 公共 头文件
  */

#ifndef _FUND_COMMON_H_
#define _FUND_COMMON_H_

#include "UrlAnalyze.h"

/**
 * 使用CFT::UrlAnalyze类进行解析
 */
using namespace CFT;

/* 接口是否验证登录态 */
#define NO_CHECK_LOGIN   0
#define CHECK_LOGIN   1


/**
 * Middle消息接口使用的宏和关键字
 */
const unsigned MAX_SPID_LEN = 15;
const unsigned MAX_LISTID_LENGTH = 32;
const unsigned MAX_TRADEID_LENGTH = 14;
const unsigned MAX_UID_LENGTH = 16;
const unsigned MAX_FEE_LENGTH = 16;
const unsigned MAX_MEMO_LENGTH = 96;
const unsigned MAX_TIME_LENGTH = 20;
const unsigned MAX_IP_LENGTH = 15;
const unsigned MIN_IP_LENGTH = 7;
const unsigned MAX_LOGIN_LENGTH = 31;  // 登录的用户、密码长度
const unsigned MAX_VARCHAR_LENGTH = 255; // 字符串类型的最大长度


/* 证件类型 */
#define CRE_IDENTITY   1
#define CRE_RE_CARD   5


/* 物理状态 */
#define LSTATE_VALID  1
#define LSTATE_INVALID  2
#define LSTATE_FREEZE  3


/* 开户状态 */
#define REG_INIT   1
#define REG_REVIEW   2
#define REG_OK   3


/* 实名认证状态 */
#define AUTHEN_INIT   1
#define AUTHEN_OK   2

/* CFT实名认证状态 */
#define CFT_AUTHEN_INIT   1
#define CFT_AUTHEN_OK   2
#define CFT_AUTHEN_TRUENAME   3
#define CFT_AUTHEN_IDENTY   4

/* Fund实名认证状态 */
#define FUND_AUTHEN_INIT   1
#define FUND_AUTHEN_OK   5
#define FUND_AUTHEN_WHITELIST   6

/* 绑定状态 */
#define BIND_INIT   1
#define BIND_OK   2


/* 订单状态 */
#define CREATE_INIT   0 //创建申购单
#define PAY_INIT   1 //待付款
#define PAY_OK   2 //付款成功
#define PURCHASE_SUC  3 //申购成功
#define REDEM_ININ  4  //初始赎回单
#define REDEM_SUC 5 //赎回成功
#define REDEM_FAIL 6 //赎回失败
#define PUR_REQ_FAIL 7 //申购请求失败
#define PURCHASE_APPLY_REFUND 8 //申请退款
#define PURCHASE_REFUND_SUC 9 //退款成功
#define REDEM_FINISH 10 //赎回单受理完成
//  #define SUBACC_FETCH_SUC 11 // 子账户提现请求成功
#define PAY_ACK_SUC 12 //支付通知基金公司成功
#define REDEEM_INFO_SUC 13 //赎回通知基金成功
#define TRADE_STATE_SIZE 14 // 交易单最大的状态值
// 标识申购状态流转顺序
// PURCHASE_STATE_ORDER[3]=4 表示state=3的支付单状态的最长路径是第4顺位节点
// 99表示最终状态的顺位
const int PURCHASE_STATE_ORDER[TRADE_STATE_SIZE]={0,1,2,99,0,0,0,98,4,99,0,0,3,0};
// 标识赎回状态流转顺序
const int REDEEM_STATE_ORDER[TRADE_STATE_SIZE]={0,0,0,0,1,3,99,0,0,0,99,0,0,2};


/* 用户赎回标记状态位 */
#define USER_STOP_TRANSFER   0x1

/* 基金公司赎回标记位 */
#define SP_STOP_TRANSFER   0x08
#define SP_NOT_ALLOW_TRANSFER_BUY   0x10

/* uin 登陆方式 */
#define UIN_TYPE_QQ   1
#define UIN_TYPE_EMAIL    2


/* 币种类型 */
#define CUR_RMB		1
#define CUR_FUND	2
#define CUR_FUND_SP	90 //余额增值(华夏)
#define CUR_FUND_BALANCE	89 //理财通余额


/* 基金交易类型 */
#define PURTYPE_PURCHASE   1  // 申购
#define PURTYPE_SUBSCRIBE   2  // 认购(首次募集期购买)
#define PURTYPE_PERIOD   3  // 定投
#define PURTYPE_REDEEM   4  // 赎回
#define PURTYPE_WITHDRAW   5  // 撤单
#define PURTYPE_SHARE   6  // 分红
#define PURTYPE_BUYFAIL   7  // 申购认购失败退款
#define PURTYPE_PARTFAIL   8  //  比例确认退款
#define PURTYPE_REWARD_PROFIT   9  //  赠送收益申购
#define PURTYPE_REWARD_SHARE   10  //  赠送份额申购
#define PURTYPE_TRANSFER_PURCHASE   11  //  转入
#define PURTYPE_TRANSFER_REDEEM   12  //  转出


#define PURTYPE_BUY   100 // 申购、认购、定投、赠送份额申购、赠送收益申购
#define PURTYPE_WITHDRAW_FOR_SUBSCRIBE 500  // 对认购的撤单
#define PURTYPE_ROLL_OUT   201 // 定期订单滚动
#define PURTYPE_ROLL_IN 202  // 定期订单滚入

/**基金手续费收取方式 */
#define TRADE_FUND_CHARGE_TYPE_FRONT  0 // 前端收费
#define TRADE_FUND_CHARGE_TYPE_BACK  1 // 后端收费
#define TRADE_FUND_CHARGE_TYPE_NONE  2 // 无申购费
#define TRADE_FUND_CHARGE_TYPE_BOTH  3 // 两端收费




/* 基金账户绑定基金公司账号结果 */
#define BIND_SPACC_INIT   1
#define BIND_SPACC_SUC   2
#define BIND_SPACC_FAIL   3
#define BIND_SPACC_TEMP_FAIL   4


/* 基金账户绑定基金公司账号类型*/
#define BIND_SPACC_GENERAL   0 //普通绑定
#define BIND_SPACC_MASTER   1 //主交易帐号


/* 交易记录特殊标记 */
#define TRADE_RECORD_TIMEOUT 1 //交易记录超时
#define TRADE_SPETAG_BOOK_REDEM 3 // 定期预约赎回单
#define TRADE_SPETAG_UNITS_UNUSABLE 4 //交易记录份额不可用,等待增加子账户


/* 资源管理器相关命令 */
#define SOURCE_DB_MIN 	 0                                     //  db的最大值
#define SOURCE_DB_T_USER 	 1                                  //  用户数据库
#define SOURCE_DB_T_TRAN_LIST 	 2                             //  交易单库
#define SOURCE_DB_T_TCBANKROLLLIST 	 3                        //  充值单
#define SOURCE_DB_T_TCPAY_LIST 	 4                            //  提现单
#define SOURCE_DB_T_FREEZE_LIST 	5                           //  冻结单
#define SOURCE_DB_T_MIDDLE_USER 	 6                  //  商户数据库
#define SOURCE_DB_T_BANK 	 7                                  //  银行数据库
#define SOURCE_DB_MAX 	 8                                     //  db的最大值


/* 开户前预支付单状态 */
#define PREPAY_INIT   0 //初始
#define PREPAY_OK   1 //已支付
#define PREPAY_REFUND_INIT  2 //申请退款
#define PREPAY_REFUND_SUC  3  //退款成功
#define PREPAY_INVALID 5 //作废


/* 申购赎回用途*/
#define PURPOSE_DEFAULT 0 //普通赎回，直接赎回到财付通余额（默认）
#define PURPOSE_DRAW_T1 1 //t+1 提现
#define PURPOSE_CONSUM 2 //消费
#define PURPOSE_CHANGE_SP 3 //份额转换（份额转换的申购也会用到该字段）
#define PURPOSE_FREEZE 5  //冻结购买合约机
#define PURPOSE_BALANCE_BUY 6  //余额申购
#define PURPOSE_REDEM_TO_BA 7  //赎回到余额
#define PURPOSE_UNFREEZE_FOR_FETCH  8  //合约机赎回提现给商户
#define PURPOSE_REDEM_TO_BA_T1 9  //t+1赎回到余额
#define PURPOSE_ACTION_BUY 101  //活动赠送券申购

/* 提现结果 */
#define FETCH_RESULT_BANK_SUCCESS 1  // 提现到银行卡成功
#define FETCH_RESULT_BANK_FAIL 2     // 提现到银行卡失败,资金进入余额
#define FETCH_RESULT_BALANCE_SUCCESS 3 // 赎回到余额成功
#define FETCH_RESULT_INIT 99 // 提现结果初始化(兼容历史数据)


/* 份额转换状态*/
#define CHANGE_SP_INIT 0 //转换中（初始状态）
#define CHANGE_SP_SUC 1 //转换成功（最终状态）
#define  CHANGE_SP_FAIL 2 //转换失败（最终状态）

/* 份额转换状态*/
#define FUND_TRANSFER_INIT 0 //初始
#define FUND_TRANSFER_REQ 1 //转换请求成功
#define FUND_TRANSFER_REDEM_SUC 2 //赎回成功
#define FUND_TRANSFER_REDEM_FAIL 3 //赎回失败
#define FUND_TRANSFER_TRANS_SUC 4 //转换成功
#define FUND_TRANSFER_REDEM_TIMEOUT 5 //赎回超时
#define FUND_TRANSFER_REDEM_SPTIMEOUT_REDO_OK 6 //赎回超时
#define FUND_TRANSFER_SUBACC_SAVE_REDO 7 //赎回超时

/* 份额转换子账户状态*/
#define FUND_TRANSFER_SUBACC_INIT 0 //初始
#define FUND_TRANSFER_SUBACC_DRAW 0x01 //减成功
#define FUND_TRANSFER_SUBACC_SAVE 0x02 //加成功

/* 对账类型*/
#define RECON_TYPE_BIND_SP 1 //账户绑定对账
#define RECON_TYPE_PURCHASE 2 //申购对账
#define RECON_TYPE_REDEM 3 //赎回对账
#define RECON_TYPE_PROFIT 4 //每日收益对账及入账

/* 对账状态*/
#define RECON_STATE_INIT 1 //初始状态
#define RECON_STATE_CHECK 4 //检查完成
#define RECON_STATE_FINISH 2 //对账完成

/* 提现到账类型*/
#define DRAW_ARRIVE_TYPE_T0 2 //t+0到账
#define DRAW_ARRIVE_TYPE_T1 3 //t+1到账
#define DRAW_ARRIVE_TYPE_BA 5 //到余额

/* 赎回状态 */
#define KF_FORCE_REDEM 1 //客服強赎

/* 收益类型*/
#define PROFIT_TYPE_SHARE 1 //分红产生的收益
#define PROFIT_TYPE_AWARD 2 //赠送的收益

#define DRAW_NOT_USE_LOADING 0 //	赎回无需垫资
#define DRAW_USE_LOADING 1 //	赎回需要垫资

#define FUND_TOTALACC_TYPE_SAVE 1 //	入
#define FUND_TOTALACC_TYPE_DRAW 2 //	出
#define FUND_TOTALACC_TYPE_FREEZE 3 //	冻结
#define FUND_TOTALACC_TYPE_UNFREEZE 4 //	解冻

#define DEFAULT_SP_SET_TYPE 1 //	默认基金设置类型 1:用户设置
#define SUBACC_SPLIT_BY_LAST3 1 //子账户db路由类型


#define FUND_FREEZE_INIT 0 //初始状态
#define FUND_FREEZE_PAY_BUYED 1 //已经支付申购
#define FUND_FREEZE_OK              2 // 已经冻结成功
#define FUND_FREEZE_CALCEL       3 // 取消
#define FUND_FREEZE_UNFREEZEED 4 // 解冻


#define FUND_UNFREEZE_OK 1 //解冻成功

/*赎回类型*/
#define KF_REDEM    99 //客服強赎
#define FUND_TRADE_OPT_CLOSE_FORCE_REDEM  99 //客服强赎
#define FUND_TRADE_OPT_CLOSE_PATRIAL_REDEM  1 //指定金额
#define FUND_TRADE_OPT_CLOSE_ALL_REDEM  2 //全额金额

#define FUND_USER_ACC_TYPE_BUY 1 // 申购
#define FUND_USER_ACC_TYPE_REDEM 2 //赎回
#define FUND_USER_ACC_TYPE_PROFIT 3 //收益
#define FUND_USER_ACC_TYPE_ROLL 4 //滚动

#define FUND_CHARGE_INIT  0 
#define FUND_FETCH_INT  0
#define FUND_CHARGE_PAYED_NOREFUND  1  //不可退款状态
#define FUND_CHARGE_SUC  2
#define FUND_CHARGE_REFUND  3
#define FUND_CHARGE_REFUND_OK  4
#define FUND_FETCH_SUBACC_OK  5
#define FUND_FETCH_OK  6
#define FUND_FETCH_REFUND  7

#define SUBACC_FETCH_RESULT_OK 1
#define SUBACC_FETCH_RESULT_FAIL 2

#define BALANCE_RECOVER_OK 2   //总账户和提现垫资账户都回补完成
#define BALANCE_RECOVER_TRANSFER_SUPPLYED 1  //已经回补总账户，但未回补提现垫资
#define BALANCE_RECOVER_INIT 0

#define FUND_BA_FETCH_EXAU_REQ_TYPE  51
#define FUND_BUY_PC_CHANNEL_EXAU_REQ_TYPE 58  //pc网银支付的限额

#define BA_FETCH_NOT_NOTIFY 1
#define BA_FETCH_NOTIFY 2

/*退款类型*/
#define REFUND_CARD 1
#define REFUND_BALANCE 2
#define REFUND_CFT 3

#define DB_DECODE_CRE_ID_MAX_LENGTH 23 //用户身份ID最大业务长度
#define DB_DECODE_PHONE_MAX_LENGTH  21 //电话号码最大业务长度
#define DB_DECODE_MOBLIE_MAX_LENGTH 21 //手机号码最大业务长度

/*是否是沪港通交易类型*/
#define ETF_TYPE_HK_STOCK 0
#define ETF_TYPE_PART_NOT_TRADE_TIME 1
#define ETF_TYPE_NOT_TRADE_TIME 2

/*商户子类型*/
#define SP_TYPE_HK_STOCK 1

/*退款原因*/
enum FUND_REFUND_REASON
{
	FUND_REFUND_REASON_0 = 0,//初始值
	FUND_REFUND_REASON_1 = 1,//用户已经注销
	FUND_REFUND_REASON_2 = 2,//申购请求和支付uid不一致
	FUND_REFUND_REASON_3 = 3,//申购请求和支付uin不一致
	FUND_REFUND_REASON_4 = 4,//申购请求和支付tradeId不一致
	FUND_REFUND_REASON_5 = 5,//账户被冻结
	FUND_REFUND_REASON_6 = 6,//账户开户失败
	FUND_REFUND_REASON_7 = 7,//支付超过一定时间仍未开户成功转退款
	FUND_REFUND_REASON_8 = 8,//申购记录uid和支付uid不一致
	FUND_REFUND_REASON_9 = 9,//余额冻结成功超过一定时间未申购成功退款
	FUND_REFUND_REASON_10 = 10,//申购失败转退款
	FUND_REFUND_REASON_11 = 11,//超过总资产限额
	FUND_REFUND_REASON_12 = 12,//绑定序列号为空导致退款
	FUND_REFUND_REASON_13 = 13,//非安全卡支付
	FUND_REFUND_REASON_14 = 14,//不满足购买定期产品权限
	FUND_REFUND_REASON_15 = 15,//申请申购失败退款
	FUND_REFUND_REASON_20 = 20,//证件号不合法
	FUND_REFUND_REASON_21 = 21,//基金公司要求退款
	FUND_REFUND_REASON_22 = 22,//用户撤单
};
enum FUND_FREEZE_PAY_TYPE
{
    FREEZE_FROM_BUY_SP  = 1,    // 申购后冻结
    FREEZE_FROM_BALANCE    = 2    // 已有份额冻结
};


enum FUND_UNFREEZE_TYPE
{
    UNFREEZE_SP_REFUND  = 1,    // 商户退款
    UNFREEZE_NORMAL    = 2,    // 合约到期解冻
    UNFREEZE_FOR_FETCH    = 3,    // 违约解冻提现
    UNFREEZE_INNER_CACEL    = 4    // 内部取消冻结
};


/* 余额增值账户开户操作类型 */
enum BINDSP_OP_TYPE
{
    PRE_BIND  = 1,    // 预绑定
    BIND_ACK    = 2    // 绑定确认
};


/* 申购确认接口操作类型 */
enum PUR_ACK_OP_TYPE
{
    INF_PUR_SP_REQ_SUC  = 1,	// 申购请求成功
    INF_PUR_SP_REQ_FAIL = 2,    // 申购请求失败
    INF_PAY_OK = 3,	//支付成功
    INF_PAY_FAIL =4,	//支付失败
    INF_PAY_SP_INFO_SUC = 5,	//到基金公司支付通知成功
    INF_PAY_SP_INFO_TIMEOUT = 6,		//到基金公司申购确认超时
    INF_PUR_SP_ACK_SUC = 7	//基金公司申购份额确认成功
};


/* 赎回确认接口操作类型 */
enum REDEM_ACK_OP_TYPE
{
	INF_REDEM_SP_ACK_SUC = 1,	// 调基金公司赎回成功
	INF_REDEM_SP_ACK_FAIL = 2,	// 调基金公司赎回失败
	INF_REDEM_SP_ACK_TIMEOUT = 3,	// 调基金公司赎回超时
	INF_REDEM_SP_ACK_FINISH = 4,  	// 赎回单受理完成
	INF_REDEM_BALANCE_ACK_SUC = 5,	// 调基金公司赎回份额确认成功
	INF_REDEM_BALANCE_ACK_FAIL = 6	// 调基金公司赎回份额确认失败
};

/* 赎回确认接口操作类型 */
enum FETCH_ARRIVAL_OP_TYPE
{
	INF_FETCH_ARRIVAL_BALANCE = 1,	// 余额提现回导
	INF_FETCH_ARRIVAL_REDEEM = 2	// 赎回提现回导
};

/**
 * 该定义是接口fund_update_kvcache_service中使用到
 * 除100，101，102三个值外其他值为ckv key的编号
 * 新增ckv编号是不在该类型中增加相关定义，在CKV_KEY_NO_TYPE中定义
 */
/* 更新缓存接口操作类型 */
enum UPDATE_KVCACHE_OP_TYPE
{
	UPDATE_KVCACHE_KEY_VALUE = 100,	// 更新key-value
	DELETE_KVCACHE_KEY = 101,	// 删除key-value内容
	UPDATE_KVCACHE_KEY_VALUE_V2 = 102	// 带超时时间的更新key-value

};

/**
 * 每一个ckv的key必须有一个编号且与svn中<理财通保存ckv数据格式说明.doc>中的编号相同
 * 编号定义必须连续，如果CKV_KEY_NO_TYPE定义的最后一个编号与新编号不连续，则需要
 * 将中间差的编号定义到CKV_KEY_NO_TYPE中以保证编号连续
 * 由于存在多个开发人员并行开发，ckv的key和编号必须先申请再使用。
 * 100、101、102三个编号不能使用。增加key时必须在全局数组g_ckv_key_prefix中增加key的
 * 前缀配置
 * 如果在更新ckv时如果失败写入日志不需要再自动补单则在调ckv的set接口时用如下方法在将keyno
 * 写为负数
 * gCkvSvrOperator->set(NOT_REUPDATE_CKV(CKV_KEY_FUND_SUPPORT_SP_ALL), key, szValue)
 * 作者:aixli 2014-10-29
 */
/* CKV缓存KEY编号 */
enum CKV_KEY_NO_TYPE {
    CKV_KEY_UIN = 1,//用户基本信息
    CKV_KEY_TRADE_ID = 2,//用户绑定的所有基金公司列表
    CKV_KEY_TOTAL_PROFIT = 3,//用户累计收益
    CKV_KEY_FUND_SUPPORT_SP_ALL = 4,//支持的所有基金公司
    CKV_KEY_SPID_FUNDCODE_CONF = 5,//基金代码信息
    CKV_KEY_FUND_SUPPORT_BANK = 6,//银行信息
    CKV_KEY_FUND_SUPPORT_BANK_ALL = 7,//全部银行信息
    CKV_KEY_UID = 8,//交易记录
    CKV_KEY_PROFIT_RATE = 9,//基金代码最新收益率信息
    CKV_KEY_PROFIT_RECORD = 10,//益记录流水
    CKV_KEY_PAY_CARD = 11,//首次支付卡信息
    CKV_KEY_FUND_PREPAY = 12,//预支付单信息
    CKV_KEY_FUND_TRADE = 13,//基金交易记录,单条
    CKV_KEY_BI = 14,//T日份额信息，该kEY作为后缀添加
    CKV_KEY_FUND_TOTALACC_2 = 15,//总账户份额
    CKV_KEY_MULTI_PROFIT_RECORD = 16,//基金代码最新21条收益率信息
    CKV_KEY_HIGHTEST_PROFIT_RATE_SP = 17,//收益率最高的基金公司信息
    CKV_KEY_MULTI_SP_WHITE_USER = 18,//多基金体验用户白名单
    CKV_KEY_LCT_ACTION_LIST = 19,//用户参与的活动列表信息
    CKV_KEY_LCT_ACTION = 20,//用户参与活动的具体信息
    CKV_KEY_CLOSE_FUND_CYCLE = 21,// 定期产品运作周期数据
    CKV_KEY_USER_DYNAMIC_INFO = 22,// 用户动态信息
    CKV_KEY_MOBILE_INFO = 23,// 合约机信息表
    CKV_KEY_FUND_CLOSE_TRANS = 24,//用户定期交易购买列表
    CKV_KEY_FUND_USER_ACC = 25,//用户购买有份额基金和期次列表
    CKV_KEY_USR_BILLID = 26,//用户周报信息
    CKV_KEY_CHARGE_INFO_PC = 27,//用户PC充值单信息
    CKV_KEY_FUND_TRANS_DATE = 28,//基金交易日信息
    CKV_KEY_USER_LATEST_FUND_TRADE = 29,//用户最新的21条交易列表
    CKV_KEY_MQQ_ACC_TOKEN = 30,//手Q理财通发送生活服务号消息的access_token
    CKV_KEY_FUND_BALANCE_CONFIG = 31,//余额配置信息表
    CKV_KEY_PC_CHARGE_WHITELIST = 32,//用户是否拥有理财通余额PC充值的权限
    CKV_KEY_IDXPAGE_ACTIVE = 33,//用户是否有首页活动的权限
    CKV_KEY_WHITE_LIST = 34,//白名单功能
    CKV_KEY_ALL_ONE_DAY_PROFIT_RECORD = 35,//所有基金公司最近一日的收益
    CKV_KEY_ALL_SPID_CONF = 36,//基金信息
    CKV_KEY_OPENID = 37,// openid与accid对应关系,单条
    CKV_KEY_UNCONFIRM = 38,//指数型未确认份额信息
    CKV_KEY_TDAY = 39,//基金交易日信息,key可以为非交易日
    CKV_KEY_UNFINISH_INDEX = 40,//基金交易日信息,key可以为非交易日
    CKV_KEY_CASH_IN_TRANSIT = 44,//在途资产(多条记录)
    CKV_KEY_END
};

//对代码中已确定不需要使用的key的key no使用该值。比如t_fund_config_stop_fetch_flag临时使用的key
#define CKV_KEY_INVALID_KEYNO (-9999)
//对于代码已使用的key，但不需要分配keyno的值该值。例如频率限制的ckv的key
#define CKV_KEY_UNKNOWN_KEYNO (-9998)

//是否允许从失败日志中重新补单
#define NOT_REUPDATE_CKV(keyno) ((keyno)>0?(keyno)*-1:(keyno))


/* 事物管理器操作类型 */
enum TRANS_OP_TYPE
{
    TRANS_OP_QUERY,
    TRANS_OP_INSERT
};


/* 冻结解冻接口操作类型 */
enum FREEZE_OP_TYPE
{
	INF_FREEZE = 1,	// 冻结 
	INF_UNFREEZE = 2	// 解冻
};


/* 冻结解冻接口操作类型 */
enum PREPAY_OP_TYPE
{
	OP_TYPE_AUTHEN_OK = 1,	// 实名认证成功 	
	OP_TYPE_AUTHEN_FAIL = 2 	// 实名认证失败 
	//OP_TYPE_PREPAY_SUC = 3,	// 支付成功
	//OP_TYPE_PREPAY_REFUND = 4	// 申请退款
};

/* 推荐接口操作类型 */
enum RECOMMEND_OP_TYPE
{
	OP_TYPE_CFT_WEB= 1,	// 财付通网站
	OP_TYPE_WX = 2, 	// 微信
	OP_TYPE_CFT_QQ = 3	// 手Q
};




/* 通过查询service 操作类型 */
enum COMMQRY_OP_TYPE
{
	COMMQRY_TYPE_TRANS = 1,	// 查询交易单
	COMMQRY_TYPE_USER_TRANS = 2,	// 查询提现单
	COMMQRY_TYPE_FETCHLIST = 3,  //  查询用户提现单
	COMMQRY_TYPE_RPOFITLIST = 4,  //  查询用户收益列表
	COMMQRY_TYPE_WEEKLIST = 5,  //  查询用户周账单
	COMMQRY_TYPE_BALIST = 6,  //  查询余额流水单
	COMMQRY_TYPE_CLOSE_LIST=7
};


/* 份额转换操作类型*/
enum TRANSFER_OP_TYPE
{
	OP_TYPE_TRANSFER_BUY_REQ= 1,	// 申购请求成
	OP_TYPE_TRANSFER_REDEM_SUC = 2, 	//赎回成功
	OP_TYPE_TRANSFER_REDEM_FAIL = 3, 	//赎回失败
	OP_TYPE_TRANSFER_BUY_SUC = 4,	// 申购确认成功
	OP_TYPE_TRANSFER_REDEM_TIMEOUT = 5 	//赎回失败
};
/* 份额转换操作类型*/
enum END_REDEM_OP_TYPE
{
	OP_TYPE_END_REDEM_REQ= 1,	//请求
	OP_TYPE_END_REDEM_ACK= 2,	//确认
};

/* 余额充值提现类型*/
enum FUND_BALANCE_OP_TYPE
{
	OP_TYPE_CHAEGE_PAY= 1,	// 支付充值
	OP_TYPE_CHAEGE_REDEM_T1 = 2, 	//t+1赎回到余额
	OP_TYPE_CHAEGE_REDEM_T0 = 3, 	//快速赎回到余额
	OP_TYPE_CHAEGE_FETCH_FAIL = 4,	// 提现失败
	OP_TYPE_BA_FETCH = 5,	// t+0提现
	OP_TYPE_BA_BUY = 6,	// 余额申购
	OP_TYPE_BA_FETCH_T1 = 7, //t+1提现
	OP_TYPE_BA_RECOVER = 8,
	OP_TYPE_BA_BACKER_RECOVER = 9,
    OP_TYPE_BA_ACTION_CHARGE = 10, //活动充值
    OP_TYPE_BA_TRANSFER_REFUND = 11 //余额转账退款
};

/* 余额充值渠道类型*/
enum FUND_BA_CHARGE_CHANNEL_TYPE
{
	BA_CHARGE_CHANNEL_PC= 1,	
	BA_CHARGE_CHANNEL_WX = 2, 	
	BA_CHARGE_CHANNEL_QQ = 3
};

/* 余额回补类型*/
enum FUND_BA_RECOVER_TYPE
{
	BA_RECOVER_REDEM= 1,	  //赎回大于申购
	BA_RECOVER_BUY = 2 	 //赎回小于申购
};

/* 余额回补请求类型*/
enum FUND_BA_RECOVER_OP_TYPE
{
	FUND_BA_RECOVER_REQ= 1,	  //请求
	FUND_BA_RECOVER_CNF = 2 	 //确认
};

/*理财通后台管理系统操作类型*/
enum FUND_MANAGE_CONFIG_OP_TYPE
{
	OP_TYPE_MANAGE_WHITE_LIST = 1,	//管理白名单信息
	OP_TYPE_MANAGE_SP_CONFIG = 2,	// 管理基金公司购买限额等配置
	OP_TYPE_MANAGE_ASSET_LIMIT= 3,	// 管理用户总资产限额配置
	OP_TYPE_MANAGE_PAY_CARD = 4,		// 管理安全卡信息
	OP_TYPE_MANAGE_USER_WHITELIST = 5		// 更新用户权限位
};

/*理财通后台管理系统操作子类型*/
enum FUND_MANAGE_CONFIG_OP_SUBTYPE
{
	OP_SUBTYPE_GET_WHITE_LIST = 1,	//获取白名单
	OP_SUBTYPE_SET_WHITE_LIST = 2,	//设置白名单
	OP_SUBTYPE_DEL_WHITE_LIST = 3	//删除白名单
};

/*退款接口请求类型*/
enum FUND_REFUND_OP_TYPE
{
	OP_TYPE_REFUND_REQ =1,   //发起退款
	OP_TYPE_REFUND_ACK =2   //退款完成
};

/*理财通账号冻结操作类型*/
enum FUND_ACCOUNT_FREEZE_OP_TYPE
{
	ACCOUNT_FREEZE_DO_FREEZE = 1,	//冻结理财通账号
	ACCOUNT_FREEZE_UNDO_FREEZE = 2,	//解冻理财通账号
    ACCOUNT_FREEZE_QUERY = 3    //查询冻结信息
};

/*理财通账号冻结渠道类型*/
enum FUND_ACCOUNT_FREEZE_CHANNEL_TYPE
{
	ACCOUNT_FREEZE_CUSTOM = 1,	    //客服
	ACCOUNT_FREEZE_RISK_CONTROL = 2,	//风控
	ACCOUNT_FREEZE_UNDO_INSIDE = 3	//内部
};

/*理财通资讯审核状态*/
enum FUND_INFO_STATE_AFTER_AUDIT_TYPE
{
	INFO_HANG_UP = 1,	    //挂起
	INFO_RELEASE = 2,	//发布
	INFO_RECALL = 3	//撤回
};

/*理财通资讯审核状态*/
enum FUND_INFO_UPDATE_TYPE
{
	OP_INFO_HANG_UP = 1,	    //挂起
	OP_INFO_RELEASE = 2,	//发布
	OP_INFO_RECALL = 3,	//撤回
    OP_INFO_RELEASE_REAL = 4,	//批跑脚本使用，推送到UI机器后执行此操作
    OP_INFO_UPDATE_HTML_CONTENT = 5,	// 更新资讯的html 内容   编辑时使用
    OP_INFO_UPDATE_TITLE = 6	// 更新资讯的标题
};

//订单中的支付类型
enum FUND_PAY_TYPE
{
	PAY_TYPE_CARD = 0, //微信手Q银行卡支付
	PAY_TYPE_BALANCE, //余额支付
	PAY_TYPE_WEB, //网银支付
	PAY_TYPE_END
};
#endif /* _FUND_COMMON_H_ */

