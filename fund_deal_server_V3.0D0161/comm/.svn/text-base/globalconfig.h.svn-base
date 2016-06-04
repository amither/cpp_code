/**
  * FileName: globalconfig.h
  * Author: Hawkliu
  * Version :1.0
  * Date: 2007-07-31
  * Description: 本文件用于保存项目中共用的全局配置项
  */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "exception.h"
#include "common.h"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>

#include "fund_deal_service.h"

using std::map;
using std::set;
using std::string;
using std::vector;
using std::ostream;

struct BalanceConfigCache
{
	int id;
	int curtype;
	string spid;
	time_t timeout;

	BalanceConfigCache()
	{
		id=1;
		curtype=89;
		timeout = 0;
	}
};


struct SpSubaccCurtype
{
	int curtype;
	time_t timeout;

	SpSubaccCurtype()
	{
		curtype = 90;
		timeout = 0;
	}
};

/*
 * session服务器配置
 */
struct SessionHostInfo
{
    string strHost;
    int iPort;
    int iTimeout;
};


struct TradeDateCache
{
	string tradeDate;
	string t1Date;
	time_t timeout;

	TradeDateCache()
	{
		timeout = 0;
	}
};

struct HKTradeDateCache
{
	string tradeDate;
	string sTData;
	string sHKData;
	time_t timeout;

	HKTradeDateCache()
	{
		timeout = 0;
	}
};

struct SpConfigCache
{
	string spid;
	string fund_code;
	int curtype;
	int close_flag;
	int buy_confirm_type;
	int type;
	time_t timeout;

	SpConfigCache()
	{
		timeout = 0;
		curtype = 90;
		close_flag = 1;
	}
};


/**
* 公司CKV配置
*/
struct CkvCfg
{
	int m_use_l5; //是否使用l5做负载均衡，0:不使用 ，其它为使用
	int l5_modid; //l5的modid
	int l5_cmdid; //l5的cmdid
	int bid;//申请ckv时分配的业务bid
	string passwd; //BID密码
	int timeout;//设置TCP连接CKV建立超时时间,单位毫秒
	int port;
	vector<string>  host_vec;//ip 列表

};

struct ETFNetCache
{
    LONG fundNet;
	string date;
	time_t timeout;

	ETFNetCache()
	{
		timeout = 0;
		fundNet = 1;
	}
};


/**
 * 存放全局配置的类
 */
struct GlobalConfig
{

    /**
     * 远程机器配置信息
     */
    struct HostCfg
    {
        string  host; // IP地址
        string  user; //用户名
        string  pswd; // 密码
        int     port; // 端口
        int     overtime; // 超时时长
        string  charset; // 字符集
        int		concurrent;//最大并发数

		HostCfg()
		{
			host = "";
			user = "";
			pswd = "";
			port = 0;
			overtime = 0;
			charset = "latin1";
		}
    };

	/**
     * 数据库别名配置
     */
    struct PassconfDB
    {
        string  db_conf_key;
        string  db_conf_role;

		PassconfDB()
		{
			db_conf_key = "";
			db_conf_role = "1";
		}
    };

	/**
     * BILL SERVER配置信息
     */
    struct BillCfg
    {
        string  host; // IP地址
        int port; // 端口
        int appid; // 应用号
    };


    /**
    * 推荐商户配置
    */

    struct RecmmendSpCfg
    {
        map<string, string> rec_sp_white_list;//推荐商户白名单配置，以商户号开始，后面跟该商户号的白名单，商户号间以'|'分割 例2000000507:32917605,616691693;200000505:1563686969
        map<string, string> rec_sp_uin_tail_config;//推荐商户按用户尾号配置，以商户号开始，后跟uin尾号，商户号间以'|'分割
    };


    /**
     * 业务配置信息
     */
    struct AppCfg
    {
        string pre_regkey; //前置机请求生成token所使用key
        int subacc_cur_type; // 子账户cur_type
        string redem_timeout_conf; //赎回超时处理方式配置
        int trans_recon_type; //交易对账类型 1.  15:00-14:59:59模式  2.  0:00-23:59:59 模式
        string change_sp_stop_time; //份额转换暂停转换时间 ，例 14:50:00
        int paycb_overtime_inteval; //支付回调超时间隔,单位为秒
        LONG sp_loading_credit; //商户垫资额度，以分为单位
        LONG sp_loading_warning_limit; //商户垫资额度，以分为单位
        LONG sp_loading_stop_redem_limit; //总垫资小于该值的时候停止提现
        LONG sp_loading_last_allowed_redem_limit; //总垫资小于该值的时候停止提现
        LONG total_loading_credit; //总垫资额度，以分为单位
        int over_loading_draw_type; //超出垫资额度提现的处理方式 3:t+1提现，其它为拒绝提现
        int stop_fetch_when_overloading;// 1 垫资不足时停止提现
        int over_loading_use_common; //超出基金公司垫资额度是否可以继续使用总垫资池 1:可以 0:不可以
        int support_redem_to_cft; //是否支持赎回到财付通余额 1:支持，其它不支持
        LONG reward_profit_rate; //赠送收益率，百分比，如配置7，则代表7%收益率
        int ui_ttc_max_err; //ttc 最大错误数
        int ui_ttc_stop_time; //ttc 暂停访问时长，秒为单位
        string exau_sp_id;
        LONG user_max_share; //用户最大持有份额限制，以分为单位
        LONG outer_vip_max_share; //公司外白名单最大持有份额，以分为单位
		LONG tencent_vip_max_share; //公司内白名单最大持有份额，以分为单位
		LONG rebuy_buf; //赎回失败再次申购多加的上限，现在是10w
        LONG seven_day_profit_rate_max; //七日年化收益最大值，初始配置范围是1%-12%之间可接受的七日年化收益率
        LONG seven_day_profit_rate_min; //七日年化收益最小值，真实值乘10的八次方
        int sentMqMsg; //是否开启发送mq消息，1为开启，其它不开启
        LONG update_pay_card_limit; //更新支付卡信息限额，低于该限额的可以直接更新
        int check_same_pay_card; //是否检查相同支付卡 0:不检查，其它都检查
        vector<string> payCardWhiteVec; //原卡进程白名单
        string nopass_reset_paycard_key;//无密重置安全卡接口调用密钥
        int check_tpulsbalance_for_reg_profit; //是否校验 T+1 份额
        int multi_sp_config; //多基金版本配置 0:有限不开启 1:开启写入，不开启查询 2:全量开启
        string default_sp; //默认基金公司
        int tplus_redem_switch;
        string tplus_redem_sps_white_list;
        string key_unbind; //注销接口key
        int subacc_split_by_last3; //子账户使用后3为路由db和ttc, 1为使用，其它不使用
        int unbind_no_trans_days; // 用户必须最近unbind_no_trans_days天内无理财通交易记录才能注销
        string unbindwhitelist;//注销接口灰度白名单配置

        string coupon_key;//调用券服务所需的key
		string transfer_ackkey;
       string transfer_reqkey;
       string transfer_redem_result_key;
       int max_transfer_times_oneday;
       int sp_redem_rate;
       string transfer_limit_white_list; //日转换次数限制白名单
       LONG sp_redem_tranfer_limit_offset;//当总赎回量达到最大可转换量减去该偏移值时设置商户停止转出标记
       string freeze_service_key;
       LONG max_transfer_fee_one_time;//单笔最大转换份额
       int close_buy_date_upper_limit; //定期产品购买期次

	   int switch_conf_center;// 配置中心开关 0:本地配置加载  1:配置中心加载
	   string outer_vip_list;// 公司外白名单
	   string tencent_vip_list; //公司内白名单
       int update_profit_ckv_switch;//更新CKV开关,1-使用ckv数据更新,其它-从DB查数据更新
	   string kf_start_time; //客服强制赎回开始时间
	   string kf_end_time; //强制赎回结束时间
	   int check_end_redem_time; // 检查到期赎回时间配置
	   string change_end_redem_time; // 调整到期赎回交易单时间配置
	   int close_end_change_sp_config; // 配置到期处理是否修改商户余额配置
	   string close_redem_req_stop_time; //定期赎回请求停止时间
	   string close_redem_ack_stop_time; //定期赎回确认停止时间

        string charge_service_req_key;//充值接口key
        string charge_service_cnf_key;//充值接口key
        string fetch_service_key;//提现接口key
        LONG assert_limit_level1_chargefee;//升级用户最大资产限额到1的最小充值额度
        LONG sp_loading_enough_check_befor_subacc; //垫资剩余额大于该值表明垫资充足可以在子账户操作之后再校验垫资份额
        string consum_fund_spid; //用于做消费的基金商户号
        string wx_wfj_spid; //微信预付卡临时方案 王府井商户号配置
        int   multycardbuy_allow_all_switch; //多卡进，单卡出是否全面放开1是，0否
		LONG usr_rst_paycard_fee_limit;//用户重置安全卡金额上限
		int undone_trans_timespan;//用户未完成的申购单或充值单时间,以分钟为单位
        string risk_assess_key;//风险评测接口所需key
        vector<int>  AssessRiskTypeVec;//用户承受能力等级
        string refund_service_key;//退款接口key
        string update_fund_bind_key;//更新用户信息接口key
        int refund_for_check_bind_serailno_diff;//校验非安全卡非白名单用户是否退款
		string account_freeze_key;// 理财通账号冻结接口key
        string account_freeze_white_ip;//理财通冻结接口白名单ip
        int fund_index_trans_grey;//指数型基金关联表灰度
        int check_exau_auth_limit; //赎回时 是否检查用户限额
        string insert_trans_date_key;
        string update_trans_date_key;
		int db_encode_switch; // Db敏感字段是否写入加密后数据开关，0不加密，1加密
    };


	struct SubaccUserSeg
	{
		int start_uid;
		int end_uid;
		string subacc_db;
		string subacc_ttc;
	};

    /**
     * 系统配置信息
     */
    struct SysCfg
    {
        int mid_no; // middle机器编号
        string log_path; // 日志文件路径
        int log_num; // 日志文件数量
        int log_size; // 日志文件大小
        string encrypt_sp_id; // 用于加密的spid 配置
        string balance_sp_id;//余额总账户spid
        int db_idle_time;
    };

    struct SessionCfg
    {
        int bid;
        int shm_id;
        int conn_timeout;
        int rw_timeout;
        int ischk_shm;
        int chk_shm_num;
        vector<SessionHostInfo> sess_hosts;
    };

public:

    /**
     * 读取配置文件
     */
    GlobalConfig(const string& strCfgFile) throw(CException);      // 读取配置文件

    /**
     * 打印配置内容
     */
    ostream& dump(ostream& os) const;

public:

	/**
	* 基金交易服务访问db
	*/
	PassconfDB m_fund_deal_db;

	/**
	* 基金交易服务访问备机db
	*/
	PassconfDB m_fund_deal_slave_db;

	/**
     * 连接子账户配置
     */
    HostCfg  m_SubaccCfg;

	/**
     * 连接exau配置
     */
    HostCfg  m_ExauCfg;

	/**
     * 绑定查询服务配置
     */
    HostCfg  m_BindQueryCfg;
    
    /**
     * 查询子账户余额
     */
    HostCfg  m_QuerySubAccBanCfg;

	/**
     * BILL SERVER配置
     */
    BillCfg m_BillnoCfg;


    /**
     * 连接fund_fetch_server配置
     */
    HostCfg  m_FundFetchCfg;

    /*
     * session服务器配置
     */
    SessionCfg m_SessionSvrCfg;


    /**
    * 公司CKV cache 配置信息
    */
    CkvCfg m_CkvCfg;
    CkvCfg m_SubAccCkvCfg;
    CkvCfg m_CkvCfg_slave;

    RecmmendSpCfg m_WxRecommendSpCfg;

    RecmmendSpCfg m_CftqqRecommendSpCfg;

	/**
	* 子账户分段信息记录
	*/
	vector<SubaccUserSeg> m_subaccUserSegVec;

	/**
     * 子账户ttc配置
     */
    map<string,HostCfg>  m_subaccTtcCfgVec;

	/**
     * 子账户数据库配置
     */
    map<string,PassconfDB>  m_subaccDbCfgVec;

	/**
	* 记录基金公司与子账户的对应关系，现在基金公司开户是按商户号进行的，无fund_code信息，所以暂时先用spid做主键
	* 改配置来自数据库初始化
	*/
	map<string,SpSubaccCurtype> m_sp_subaccCurtype;

    /**
       * 记录交易日缓存
       */
	map<string,TradeDateCache> m_fundTradeDate;

    /**
       * 记录定期最大交易日缓存
       * 相同到期日的可合并最大交易日
       */
	map<string,TradeDateCache> m_closeMaxTradeDate;

    /**
       * 记录定期最大交易日缓存
       * 相同到期日的可合并最小自然日
       */
	map<string,TradeDateCache> m_closeMinNatureDate;

	/**
	* 缓存商户配置信息
	* 根据商户号一一对应到基金代码和curtype
	* 不支持一个商户号对应多个基金代码
	*/
	map<string,SpConfigCache> m_spConfigCache;

    /**
       * 记录定期最大交易日缓存
       * 相同到期日的可合并最大交易日
       */
	map<string,ETFNetCache> m_ETFNetCache;

	/**
     * 商户转换比例配置
     */
    map<string,int>  m_spTransferRate;

    /**
     * 商户停止快赎额度
     */
    map<string,LONG>  m_spLastAllowFastRedem_Dianzi;

    /**
     * 商户垫资告警额度
     */
    map<string,LONG>  m_spWarn_Dianzi;

	/**
	* 安全域内mq配置
	*/
	HostCfg  m_MqCfg;

    /**
     * 业务配置信息
     */
    AppCfg m_AppCfg;

    /**
     * 系统配置信息
     */
    SysCfg m_SysCfg;

    //用户总资金限制等级同对应的限额
    map<int,LONG>m_assetLimitLev2Value;
   /**
     * 记录交易日缓存
     */
    map<string,HKTradeDateCache> m_fTradeDate;
	
	BalanceConfigCache m_balanceConfigCache;

};

/**
 * 打印配置内容
 */
ostream& operator<<(ostream& os, const GlobalConfig& Cfg);

/**
 * 相关宏定义
 */
#define MID_NO   gPtrConfig->m_SysCfg.mid_no


#endif

