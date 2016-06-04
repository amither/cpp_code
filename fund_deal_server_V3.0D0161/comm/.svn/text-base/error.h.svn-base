/**
  * FileName: error.h
  * Author: wenlonwang
  * Version :1.0
  * Date: 2010-8-19
  * Description: 基金交易服务错误码定义头文件
  */


#ifndef _ERROR4FUND_DEAL_H_
#define _ERROR4FUND_DEAL_H_

/* 系统错误分段 */
const unsigned   FUND_SYS_ERROR   = 59010000;

/**
 * 5901 1***  系统错误0:文件配置错误
 */
const unsigned ERR_LOAD_XML_CFG = FUND_SYS_ERROR + 1001; // 读取XML配置文件失败
const unsigned ERR_NULL_XML_NODE = FUND_SYS_ERROR + 1002; // 指定的XML节点不存在
const unsigned ERR_BAD_WATCH_WORD = FUND_SYS_ERROR + 1003; // 系统口令错误
const unsigned ERR_SYSTEM_UNKNOW_ERROR = FUND_SYS_ERROR + 1005; // 系统未知错误
const unsigned ERR_CONFIG_ERROR = FUND_SYS_ERROR + 1006; // 配置项内容错误


/* 5901 1***  系统错误1:  数据库错误 */
const unsigned  ERR_DB_INITIAL      = FUND_SYS_ERROR + 1101;  // 分配数据库连接失败
const unsigned  ERR_DB_CONNECT      = FUND_SYS_ERROR + 1102;  // 连接数据库失败
const unsigned  ERR_DB_LOST         = FUND_SYS_ERROR + 1103;  // 和数据库断开连接
const unsigned  ERR_DB_QUERY        = FUND_SYS_ERROR + 1104;  // 执行数据库查询出错
const unsigned  ERR_DB_NULL_RESULT  = FUND_SYS_ERROR + 1105;  // 获取到空数据库结果集
const unsigned  ERR_DB_AFFECT_ROW   = FUND_SYS_ERROR + 1006;  // 获取影响行数异常
const unsigned  ERR_DB_FETCH_ROW    = FUND_SYS_ERROR + 1107;  // 取结果行错误
const unsigned  ERR_DB_DUP_TRANS = FUND_SYS_ERROR + 1108;     // 事务没结束，又开始新事务
const unsigned  ERR_DB_LOSTCONN_COMMIT = FUND_SYS_ERROR + 1109; // 事务提交时，数据库断连
const unsigned  ERR_DB_NO_TRANS = FUND_SYS_ERROR + 1110;      // 事务提交时，没有在事务中
const int ERR_DB_MULTI_ROW             = FUND_SYS_ERROR + 1111;     // 取到结果多余一行
const int ERR_DB_UNKNOW_FAULT  = FUND_SYS_ERROR + 1112;     // 数据库错误
const int ERR_DB_SQL_NOVALUE        = FUND_SYS_ERROR + 1113;     // 拼接SQL语句错误,没有值
const unsigned  ERR_DB_UNKNOW      = FUND_SYS_ERROR + 1114; // 数据库未知错误
const unsigned  ERR_DB_AFFECTED      = FUND_SYS_ERROR + 1115; // 影响行数多于1行
const unsigned  ERR_BUDAN_TOLONG	= FUND_SYS_ERROR + 1116; // 补单时间过长仍未成功
const unsigned  ERR_DB_SIGN_CHECK_FAILED	= FUND_SYS_ERROR + 1117; // DB sign校验失败
const unsigned  ERR_DB_SIGN_GEN_FEILD_LACK	= FUND_SYS_ERROR + 1118; // 敏感字段有更新，但缺少部分敏感字段生成签名

/* 5901 2***  系统错误2: 系统错误 */
const unsigned  ERR_READ_CONFIG     = FUND_SYS_ERROR + 2001;  // 读配置文件异常
const unsigned  ERR_RPC_CALL        = FUND_SYS_ERROR + 2002; // trpc调用失败
const unsigned  ERR_FAIL_CONNECT_MIDDLE	= FUND_SYS_ERROR + 2003;
const unsigned  ERR_FAIL_CALL_MIDDLE	= FUND_SYS_ERROR + 2004;


/* 应用错误分段 */
const unsigned   FUND_APP_ERROR   = 59020000;  


/* 5902 1*** 通用的应用错误*/
const unsigned  ERR_REPEAT_ENTRY    = FUND_APP_ERROR + 1000; // 业务重入
const unsigned  ERR_BAD_PARAM       = FUND_APP_ERROR + 1001; // 调用CParams方法异常
const unsigned  ERR_FUNDBIND_NOTREG = FUND_APP_ERROR + 1002; // 用户未注册
const unsigned  ERR_FUNDBIND_INVALID = FUND_APP_ERROR + 1003; // 用户基金注册账户无效
const unsigned  ERR_TRADE_INVALID   = FUND_APP_ERROR + 1004;  // 交易表物理状态无效
const unsigned  ERR_GET_BILLNO = FUND_APP_ERROR + 1006;       // 生成单号错误
const unsigned  ERR_DB_AFFECT_MULTIROWS = FUND_APP_ERROR + 1008;  // 数据库操作影响行数异常
const unsigned  ERR_USER_INFO_ERR     = FUND_APP_ERROR + 1025; // 用户信息核对失败
const unsigned  ERR_NOT_BIND_SP_ACC     = FUND_APP_ERROR + 1026; // 用户未绑定指定基金公司交易账号
const unsigned  ERR_SP_ACC_FREEZE		= FUND_APP_ERROR + 1027; //绑定的基金账户被冻结
const unsigned  ERR_SP_ACC_NOT_MASTER_ACC		= FUND_APP_ERROR + 1028; //非余额增值的主交易帐号
const unsigned  ERR_REPEAT_ENTRY_DIFF    = FUND_APP_ERROR + 1029; // 业务重入,重入参数不一致
const unsigned  ERR_CRE_ID_INVALID    = FUND_APP_ERROR + 1030; // 证件号码错误
const unsigned  ERR_BASE_INFO_DIFF    = FUND_APP_ERROR + 1031; // 关键信息不一致
const unsigned  ERR_USER_UNBINDED    = FUND_APP_ERROR + 1032; // 用户已经注销不能再用相同uid注册
const unsigned  ERR_QUERY_FUND_CODE_CONFIG    = FUND_APP_ERROR + 1033; //查询基金信息出错
const unsigned ERR_INIT_CFG_CENTER = FUND_APP_ERROR + 1034; // 初始化配置中心失败
const unsigned ERR_READ_CFG_CENTER = FUND_APP_ERROR + 1035; // 读取配置中心失败
const unsigned ERR_RSP_MAG_LEHTN_OVERFLOW = FUND_APP_ERROR + 1036; // 响应消息超过middle消息最大长度
const unsigned ERR_BA_ORDER_NOT_EXIST = FUND_APP_ERROR + 1037; // 余额流水单不存在
const unsigned ERR_INVALID_CKV_KEYNO = FUND_APP_ERROR + 1038; // ckv key编号不合法
const unsigned ERR_UNSUPPORT_CKV_KEYNO = FUND_APP_ERROR + 1039; // 接口不支持ckv key no的更新操作
const unsigned ERR_FIND_BIND_NOT_EXIST = FUND_APP_ERROR + 1040; // 查绑定表失败
const unsigned ERR_PAY_BIND_CARD       = FUND_APP_ERROR + 1041;//支付的时候绑的卡不是理财通支持的卡
const unsigned ERR_CKV_UNCONSIST = FUND_APP_ERROR + 1042;	// CKV数据不一致
const unsigned ERR_INVALID_IP  = FUND_APP_ERROR + 1043;  // 无效的ip

/* 5902 21** 注册基金账户的应用错误*/
const unsigned  ERR_REGOK_ALREADY   = FUND_APP_ERROR + 2100; // 已经注册成功
const unsigned  ERR_UNIQUE_MULTIROWS = FUND_APP_ERROR + 2108; // 唯一键值约束查询，结果集多条记录


/* 5902 22** 购买基金的应用错误*/
const unsigned  ERR_BUYPAY_NOLIST    = FUND_APP_ERROR + 2222; // 基金购买确认时，找不到请求记录
const unsigned  ERR_BUYPAY_LOCK      = FUND_APP_ERROR + 2223; // 基金购买时，记录被锁定
const unsigned  ERR_BUY_RECORD_INVALID      = FUND_APP_ERROR + 2224; // 订单不为有效状态
const unsigned  ERR_BUY_CANNOT_UPDATE      = FUND_APP_ERROR + 2225; // 订单状态不可修改
const unsigned  ERR_BUY_REQ_SP_OK      = FUND_APP_ERROR + 2226; // 申购请求已经预申购成功
const unsigned  ERR_BUY_RECORD_NEED_REFUND     = FUND_APP_ERROR + 2227; // 基金申购记录因账户冻结或变更交易帐号导致无法继续申购而退款
const unsigned  ERR_BUY_OVER_MAX_SHARE     = FUND_APP_ERROR + 2228;//超过用户可持有的最大份额
const unsigned  ERR_BUY_BASE_INFO_DIFF     = FUND_APP_ERROR + 2229;//申购时用户基金信息不一致
const unsigned  ERR_BUY_NOT_SUC = FUND_APP_ERROR + 2230; // 子账户补单订单非成功状态
const unsigned  ERR_CAN_NOT_PURCHASE = FUND_APP_ERROR + 2231; // 基金暂时不可以申购
const unsigned  ERR_NO_PERMISSION_PURCHASE_CLOSE_FUND = FUND_APP_ERROR + 2232; //暂时不可申购封闭基金
const unsigned  ERR_SCOPE_UPPER_LIMIT = FUND_APP_ERROR + 2233; //达到基金规模上限
const unsigned  ERR_DIFF_END_DATE = FUND_APP_ERROR + 2234; //基金公司返回的封闭结束日和本地计算不一致
const unsigned  ERR_UNFOUND_TRADE_DATE = FUND_APP_ERROR + 2235; //交易日未配置
const unsigned  ERR_CHARGE_FEE_DIFF = FUND_APP_ERROR + 2236; //手续费不一致
const unsigned  ERR_SUBACC_TIME_EMPTY = FUND_APP_ERROR + 2237; //子账户时间为空


/* 5902 23**基金帐号绑定基金公司帐号错误*/
const unsigned  ERR_FUND_BIND_SPACC_OK  = FUND_APP_ERROR + 2301; // 基金账号绑定基金公司账号已成功
const unsigned  ERR_BIND_SPACC_INFO_DIFF  = FUND_APP_ERROR + 2302; // 关键信息不一致
const unsigned  ERR_FUND_BIND_SPACC_FAIL = FUND_APP_ERROR + 2303; //绑定的基金账户失败

	
/* 5902 24**分红/收益的应用错误*/
const unsigned  ERR_DIFF_BALANCE = FUND_APP_ERROR + 2401; // 基金公司提供的余额和本地计算的余额不一致
const unsigned  ERR_HAVE_RECON = FUND_APP_ERROR + 2402; // 已经对过帐
const unsigned  ERR_PROFIT_NOT_EXIST = FUND_APP_ERROR + 2403; // 收益记录不存在
const unsigned  ERR_REG_OUTER_PROFIT_RATE = FUND_APP_ERROR + 2404; // 收益率外部入账不能入账



/* 5902 25**赎回的应用错误*/
const unsigned  ERR_REDEM_REPEAT_ENTRY = FUND_APP_ERROR + 2501; // 赎回重入
const unsigned  ERR_REDEM_DRAW_REFUSE = FUND_APP_ERROR + 2502; // 不可发起提现赎回
const unsigned  ERR_REDEM_SP_SUC_REPEAT_ENTRY = FUND_APP_ERROR + 2503; // 到基金公司赎回已成功
const unsigned  ERR_CANNOT_REDEM_TO_CFT = FUND_APP_ERROR + 2504; // 不支持赎回到财付通余额
const unsigned  ERR_OVER_EXAU_LIMIT = FUND_APP_ERROR + 2505; // 赎回超过exau配置的限制
const unsigned  ERR_NOT_ENOUGH_LOADING = FUND_APP_ERROR + 2506; // 基金公司垫资额度即将耗尽
const unsigned  ERR_T0_REDEM_REFUSED = FUND_APP_ERROR + 2507; // 基金公司垫资额度不足停止T+0赎回
const unsigned  ERR_KF_ONLY_T0_REDEM = FUND_APP_ERROR + 2509; // 客服只能T+0強赎
const unsigned  ERR_KF_ONLY_TINE = FUND_APP_ERROR + 2508; // 不在強赎所要求的时间范围
const unsigned  ERR_REDEM_DRAW_SUCC = FUND_APP_ERROR + 2510; // 减子账户成功，后续失败回滚
const unsigned  ERR_REDEM_FEE_UNCONSISTENT = FUND_APP_ERROR+2511;//赎回金额不一致
const unsigned  ERR_REDEM_NET_UNCONSISTENT = FUND_APP_ERROR+2512;//赎回净值不一致
const unsigned  ERR_REDEM_REQ_SUBACC_FAIL = FUND_APP_ERROR+2513;//赎回请求已提交,子账户更新失败(等待补单)


/* 5902 26**子账户核心业务错误 */
const unsigned ERR_CORE_OTHER_ERR = FUND_APP_ERROR + 2601; // 核心失败
const unsigned ERR_FAIL_CALL_TRANS = FUND_APP_ERROR + 2602; // 调用事务管理器失败
const unsigned ERR_CORE_USER_EMPTY = FUND_APP_ERROR + 2603; // 核心用户不存在
const unsigned ERR_CORE_USER_LOCKED = FUND_APP_ERROR + 2604; // 核心用户冻结
const unsigned ERR_CORE_USER_BALANCE = FUND_APP_ERROR + 2605; // 核心用户余额不足
const unsigned ERR_USER_BALANCE_CONTROLED = FUND_APP_ERROR + 2607; // 赎回受限制余额不足
const unsigned ERR_SUBACC_TYPE_ERROR = FUND_APP_ERROR + 2608; // 子账号币种错误

/* 5902 27**非实名认证用户开户前预支付 */
const unsigned ERR_NOR_PREPAY_INIT = FUND_APP_ERROR + 2701; // 非支付初始状态
const unsigned ERR_PREPAY_NOT_EXIST = FUND_APP_ERROR + 2702; // 预支付单不存在
const unsigned ERR_USER_INFO_DIFF = FUND_APP_ERROR + 2703; // 用户信息与预支付信息不一致
const unsigned  ERR_PREPAY_CANNOT_UPDATE      = FUND_APP_ERROR + 2704; // 订单状态不可修改
const unsigned  ERR_PREPAY_CANNOT_REFUND      = FUND_APP_ERROR + 2705; // 订单不能发起申请退款

/* 5902 28**帐号注销的错误 */
const unsigned  ERR_BALANCE_CANNOT_UNBIND      = FUND_APP_ERROR + 2801; // 份额大于0不能注销
const unsigned  ERR_TPLUS_REDEM_UNBIND      = FUND_APP_ERROR + 2802; // 有在途T+1赎回
const unsigned  ERR_TRANS_NOT_COMPLETE_UNBIND      = FUND_APP_ERROR + 2803; // 有在途交易
const unsigned  ERR_NOT_UNBINDWHITE_LISTUSER      = FUND_APP_ERROR + 2804; // 非白名单用户
//用户更换安全卡
const unsigned  ERR_USR_NOT_RST_PAY_CARD      = FUND_APP_ERROR + 2805; // 用户不能更换安全卡
const unsigned  ERR_HAS_UNCONFIRM     = FUND_APP_ERROR + 2806; // 存在未确认份额


/* 5902 9999 未知错误 */
const unsigned  ERR_UNKNOWN         = FUND_APP_ERROR + 9999; //未知错误


/* 5902 28** relay调用网络错误*/
const int ERR_XYZ_RECV              = FUND_SYS_ERROR + 2801;             // 接收数据失败
const int ERR_XYZ_OPEN              = FUND_SYS_ERROR + 2802;             // 打开socket失败
const int ERR_XYZ_UNKNOWN           = FUND_SYS_ERROR + 2803;             // xyz未知异常
const int ERR_XYZ_SHUTDOWN          = FUND_SYS_ERROR + 2804;             // 读取返回0
const int ERR_LACK_RESULT           = FUND_SYS_ERROR + 2805;             // relay调用返回无result


/* 5902 29** 份额转换错误*/
const int ERR_CHANGING_EXIST              = FUND_APP_ERROR + 2901;             // 份额转换中，不可再次转换
const int ERR_CHANGE_SP_NO_BUYID              = FUND_APP_ERROR + 2902;             // 转换份额需要先预申购与账户余额一致的基金份额
const int ERR_CONNOT_CHANGESP_TIME              = FUND_APP_ERROR + 2903;             // 我们正在为您分配收益，为了确保您的收益不受损失，收益分配完成前，转换功能暂停
const int ERR_CANNOT_CHANGESP_AGAIN              = FUND_APP_ERROR + 2904;             // 今日已转换过基金公司，不可在转换
const int ERR_MUST_REDEM_ALL              = FUND_APP_ERROR + 2905;             // 份额转换，必须先赎回原基金全部份额
const int ERR_INVALID_STATE              = FUND_APP_ERROR + 2906;             // 份额转换，必须先赎回原基金全部份额
const int ERR_TRANSFER_BUY_NOT_ALLOWED              = FUND_APP_ERROR + 2907;             // 无转入权限
const int ERR_TRANSFERINT_EXIST            = FUND_APP_ERROR + 2908;             // 有转换中记录，请稍后重试


/* 5902 30** ttc错误*/
const unsigned  ERR_CALL_TTC_RELAY         = FUND_APP_ERROR + 3001; //访问ttc错误


/* 5902 31** ckv错误*/
const unsigned  ERR_INIT_CKV         = FUND_APP_ERROR + 3101; //初始化ckv错误
const unsigned  ERR_SET_CKV         = FUND_APP_ERROR + 3102; //set ckv失败
const unsigned  ERR_GET_CKV_FAILED  = FUND_APP_ERROR + 3103; //get ckv失败
const unsigned  ERR_CKV_DB_INCONSISTENT  = FUND_APP_ERROR + 3104; //ckv与DB不一致
const unsigned  ERR_CKV_NO_DB_DATA  = FUND_APP_ERROR + 3105; //ckv有数据与DB无数据
const unsigned  ERR_UNSPPORT_CKV_CHKTYPE  = FUND_APP_ERROR + 3106; //不支持的ckv对账类型
const unsigned  ERR_SUBACC_CKV_ERROR  = FUND_APP_ERROR + 3107; //访问子账户CKV返回错误



/* 5902 32** global_access_limit init错误*/
const unsigned  ERR_INIT_GLOBAL_ACCESS_LIMIT         = FUND_APP_ERROR + 3201; //初始化global_access_limit错误

/* 5902 33** 支付卡业务错误*/
const unsigned  ERR_PAY_CARD_NOT_EXIST         = FUND_APP_ERROR + 3301; //更改支付卡时，支付卡不存在
const unsigned  ERR_UKA_CONTROL_INVALID         = FUND_APP_ERROR + 3302; //王府井预付卡不支持购买

/* 5902 34** 基金总账户错误*/
const unsigned  ERR_FUND_TOTALACC_NOT_EXIST         = FUND_APP_ERROR + 3401; //总账户不存在
const unsigned  ERR_FUND_TOTALACC_BAD_PARAM         = FUND_APP_ERROR + 3402; //修改总账户参数错误
const unsigned  ERR_FUND_TOTALACC_BALANCE_ERROR        = FUND_APP_ERROR + 3403; //账户余额不足
const unsigned  ERR_TOTALACC_NOT_FOUND_SPCONFIG        = FUND_APP_ERROR + 3404; //找不到商户信息


/* 5902 35** 修改默认基金*/
const unsigned  ERR_CHANGE_DEFAULT_SP_FAIL         = FUND_APP_ERROR + 3501; //修改默认基金失败

/* 5902 36** 修改默认基金*/
const unsigned  ERR_OVER_BOOK_STOP_DATE         = FUND_APP_ERROR + 3601; //超过预约赎回时间


//子账户报出来的错误码
const int ERR_SUBACC_EXIST = 60120105; // 子账户已开户，开户重入会报这个
//const int ERR_SUBACC_NEED_BUDAN = 83621001; // 需要补单的错误码
const int ERR_SUBACC_NOT_EXIST = 1012; // 子账户未开户


//ckv相关错误码
const int ERR_KEY_NOT_EXIST = -13200; // key不存在
const int ERR_KEY_EXPIRE = -13106;	//key 过期
const int ERR_CKV_DEL_KEY_NO_EXIST = -13105; //del 操作,key 不存在




/*
 * session 服务错误5902 36**
 */
const int ERR_SESSION_SERVER_ADD = FUND_APP_ERROR + 3601; // session server add 失败
const int ERR_SESSION_DATA_GET = FUND_APP_ERROR + 3602;   // 获取 session data 失败
const int ERR_SESSION_DATA_EMPTY = FUND_APP_ERROR + 3603; // 校验参数为空
const int ERR_SESSION_CHECK = FUND_APP_ERROR + 3604;      // 登录态校验失败
const int ERR_SESSION_PARA_CHECK = FUND_APP_ERROR + 3605;      // session 参数校验失败，无权操作该用户

//冻结解冻相关错误码37**
const int ERR_UPDATE_FREEZE_BILL = FUND_APP_ERROR + 3701; 
const int ERR_QUERY_UNFREEZE_BILL = FUND_APP_ERROR + 3702; 

/* 定期到期处理错误码 */
const unsigned ERR_CLOSE_END_REDEM_OVERTIME = FUND_APP_ERROR + 3801; 
const unsigned ERR_FUND_CLOSE_END_BAD_PARA = FUND_APP_ERROR + 3802; // 定期批跑参数错误
const unsigned ERR_FUND_CLOSE_END_BAD_STATE = FUND_APP_ERROR + 3803; // 定期批跑参数错误
const unsigned ERR_FUND_CLOSE_END_BAD_REDEM = FUND_APP_ERROR + 3804; // 定期批跑相关赎回单不存在
const unsigned ERR_FUND_CLOSE_TRANS_NOT_EXIST = FUND_APP_ERROR + 3805; // 定期交易记录不存在
const unsigned ERR_FUND_CLOSE_NO_NEED_ROLL = FUND_APP_ERROR + 3806; // 定期批跑不需要滚动
const unsigned ERR_FUND_CLOSE_UNFOUND_CYCLE = FUND_APP_ERROR + 3807; //交易日未配置
const unsigned ERR_FUND_CLOSE_END_REDEM_FAIL = FUND_APP_ERROR + 3808; // 定期批跑相关赎回单不存在
const unsigned ERR_CLOSE_ROLL_OVERTIME = FUND_APP_ERROR + 3809; //滚动超时


/*客服强赎错误码从29改到39*/
const unsigned ERR_FUND_CLOSE_REQ_FAIL		= FUND_APP_ERROR + 3909; //定期基金查询失败
const unsigned ERR_FUND_CLOSE_BALANCE_NOT_EQ 		= FUND_APP_ERROR + 3910; // 定期基金強赎金额不等于本金
const unsigned ERR_FUND_CLOSE_REDEM_SUC 		= FUND_APP_ERROR + 3911; //定期基金已被成功赎回
const unsigned ERR_FUND_CLOSE_FOCRE_STATE 		= FUND_APP_ERROR + 3912; //定期基金状态不能被强制赎回
const unsigned ERR_FUND_CLOSE_FOCRE_TOO_EARLY   = FUND_APP_ERROR + 3913; //定期基金份额未确认完整
const unsigned ERR_FUND_CLOSE_FOCRE_TOO_LATE    = FUND_APP_ERROR + 3914; //定期基金已到达赎回截止日,应该由用户发起

/* 定期实时赎回错误码 */
const unsigned ERR_CLOSE_REDEM_NOT_ENOUGH_MONEY = FUND_APP_ERROR + 4001; //实时赎回金额不足,已预约部分不可以重复发起赎回
const unsigned ERR_CLOSE_REDEM_USER_END = FUND_APP_ERROR + 4002; //指定的预约赎回方式不能发起实时赎回
const unsigned ERR_CLOSE_REDEM_HAS_ALL_REDEM = FUND_APP_ERROR + 4003; //已成功发起扫尾赎回,不能再赎回
const unsigned ERR_CLOSE_REDEM_TIME = FUND_APP_ERROR + 4004; //当前时间不能发起实时赎回
const unsigned ERR_CLOSE_REDEM_STATE = FUND_APP_ERROR + 4005; //当前状态不能发起实时赎回
const unsigned ERR_CLOSE_REDEM_DRAW_SUCC = FUND_APP_ERROR + 4006; // 减子账户成功，后续失败回滚
const unsigned ERR_CLOSE_REDEM_INVALID_MONEY = FUND_APP_ERROR + 4007; //扫尾赎回金额不一致
const unsigned ERR_CLOSE_REDEM_EXISTS_INIT_REDEM = FUND_APP_ERROR + 4008; //存在未完成的赎回单,请等待赎回完成后重试
const unsigned ERR_CLOSE_REDEM_PROFIT_RECORDING = FUND_APP_ERROR + 4009; //收益入账正在进行中,请等待收益入账完成后重试
//余额相关错误码4100 到4200
const unsigned ERR_FUND_FETCH_BALANCE_LACK		= FUND_APP_ERROR + 4100;  //余额子账户提现金额不足
const unsigned ERR_FUND_QUERY_BA_CONFIG		= FUND_APP_ERROR + 4101;   //查询余额配置表失败
const unsigned ERR_FUND_QUERY_TRADE_DATE		= FUND_APP_ERROR + 4102;   //查询交易单失败
const unsigned ERR_FUND_BA_BUY_BALANCE_LACK		= FUND_APP_ERROR + 4103; //余额申购金额校验失败
const unsigned ERR_FUND_BA_FETCH_SYS_ERORR		= FUND_APP_ERROR + 4104;  //余额提现金额校验失败
const unsigned ERR_FUND_T1FETCH_BALANCE_LACK		= FUND_APP_ERROR + 4105;  // t+1余额提现金额校验失败
const unsigned ERR_FUND_QUERY_BA_ORDER		= FUND_APP_ERROR + 4106;        //查询余额流水单失败
const unsigned ERR_FUND_FETCH_OVER_FULL		= FUND_APP_ERROR + 4107; //今日快速赎回额度用完
const unsigned ERR_FUND_RECOVER_FEE_CHECK		= FUND_APP_ERROR + 4108;  //回补金额校验失败
const unsigned ERR_FUND_ASSET_OVER_LIMIT		= FUND_APP_ERROR + 4109;  //总资产超过限额

// 管理系统错误码4201到4300
const unsigned ERR_MANAGE_ERROR	= FUND_APP_ERROR + 4201;  // 操作错误
const unsigned ERR_MANAGE_NOT_FOUND_DATA = FUND_APP_ERROR + 4202;  //数据不存在

//指数级业务相关错误码4301到4400
const unsigned ERR_RISK_TYPE = FUND_APP_ERROR+4301;//用户承受能力级别错误
const unsigned ERR_REFUND_TYPE = FUND_APP_ERROR+4302;//退款类型错误
const unsigned ERR_USER_ASSESS = FUND_APP_ERROR+4303;//不存在评测结果
const unsigned ERR_UNCONFIM_LSTATE = FUND_APP_ERROR+4304;//未确认份额无效状态不正确
const unsigned ERR_REFUND_REQUEST = FUND_APP_ERROR+4305;//不合法的退款请求
const unsigned ERR_INDEX_BAD_PARAM = FUND_APP_ERROR+4306;//指数型基金参数错误
const unsigned ERR_INDEX_BUY_TRANS = FUND_APP_ERROR+4307;//指数型基金申购关联单与交易单不一致
const unsigned ERR_INDEX_BUY_REPEAT = FUND_APP_ERROR+4308;//指数型基金申购重入,关联单与交易单不一致
const unsigned ERR_INDEX_REFUND_TRANS = FUND_APP_ERROR+4309;//指数型基金申购关联单与退款单不一致
const unsigned ERR_REFUND_REASON = FUND_APP_ERROR+4310;//退款原因错误

//理财通冻结错误码 4401到4500
const unsigned ERR_ALREADY_FREEZE	= FUND_APP_ERROR + 4401;  // 账号已经被冻结
const unsigned ERR_NOT_FROZEN = FUND_APP_ERROR + 4402;  //解冻账号，单账号未被冻结


//理财通资讯错误码 4501到4600
const unsigned ERR_INFO_NOT_EXIST = FUND_APP_ERROR + 4501;  //资讯不存在
const unsigned ERR_INFO_EXIST = FUND_APP_ERROR + 4502;  //资讯已存在

//交易日相关错误码4601到4700
const unsigned ERR_TRADE_DATE_FORMAT = FUND_APP_ERROR + 4601;//交易日格式不合法
const unsigned ERR_HK_TRADE_DAY = FUND_APP_ERROR + 4602;//非沪港通交易日
const unsigned ERR_HK_TRADE_FORMAT = FUND_APP_ERROR + 4603;//沪港通交易时间格式不合法
const unsigned ERR_HK_TRADE_TIME = FUND_APP_ERROR + 4604;//不是沪港通交易时间段
const unsigned ERR_HK_OPER_TIME = FUND_APP_ERROR + 4605;//操作时间不合法

// 理财通提现回导错误吗
const unsigned ERR_FETCH_ARRIVAL_BALANCE_UNEXIST = FUND_APP_ERROR + 4701;  //提现回导余额流水不存在
const unsigned ERR_FETCH_ARRIVAL_BALANCE_INVALID = FUND_APP_ERROR + 4702;  //提现回导余额流水单异常

#endif /* _ERROR4FUND_DEAL_H_ */

