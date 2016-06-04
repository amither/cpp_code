/**
  * FileName: fund_struct_def.h
  * Author: wenlonwang
  * Version :1.0
  * Date: 2013-8-16
  * Description: 基金应用服务 定义关系表结构 头文件
  */

#ifndef _FUND_STRUCT_DEF_H_
#define _FUND_STRUCT_DEF_H_

/**
 * 基金账户关联表 记录结构
*/
typedef struct
{
    int     Fcre_type;         // 证件类型
    char    Fcre_id[32+1];     // 证件号码，身份证转换成18位
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    char    Fqqid[64+1];       // 基金用户的CFT账号
    int     Fuid;              // 基金用户的CFT内部ID
    char    Ftrue_name[64+1];  // 投资人真实姓名
    char    Fcre_id_orig[32+1];// 用户的输入身份证号码
    char    Fphone[21+1];      // 电话号码
    char    Fmobile[21+1];     // 手机
    int     Fstate;            // 开户状态  1 - 开户中； 2 - 审核中； 3 - 开户完成
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    int		Facct_type;			//账户类型
    char	Fchannel_id[64+1];			//渠道信息
    char	Fopenid[64+1];			//余额增值帐号在微信的id
    int     Fasset_limit_lev;                  // 用户总资产限额等级fstandby1
    int     Fcft_auth_type;  // 财付通实名标记
    int     Ffund_auth_type; // 理财通实名标记
    int     Fassess_risk_type; //风险承受能力等级fstandby4
    char    Fassess_modify_time[20+1];  //最后一次测评时间fstandby2
    char    Femail[128+1]; // 邮箱地址
    char    Faddress[128+1]; // 住宅地址
	int     Ffrozen_channle; //账号冻结渠道 1客服 2风控 3 内部
	char    Fsign[32+1]; //对应DB里面的Fstandby11字段
}ST_FUND_BIND;


/**
 * 基金账户绑定的银行卡信息 记录结构
*/
typedef struct
{
    int     Fuid;              // 基金用户的CFT内部ID
    int     Fbank_type;        // 银行代码
    char    Fcard_tail[4+1];     // 银行卡尾号4位
    char    Fcard_no[32+1];    // 银行卡号
    char    Fbank_name[64+1];  // 开户银行名称
    int     Fauthen_state;     // 银行卡的实名认证状态    1 - 实名认证中； 2 - 实名认证通过
    int     Fbind_state;       // 绑定状态   1 - 绑定中； 2 - 绑定完成
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    int     Fprimary;          // 主卡标志   1- 是主卡；提现会提到主卡中
    char    Farea[16+1];         // 开户行所在地区
    char    Fcity[16+1];         // 开户行所在城市
    int 	Fpay_channel;		 // 渠道号. 1 - 一点通
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
}ST_FUND_BIND_CARD;


/**
 * 用户的基金交易表记录结构
*/
typedef struct
{
    char    Flistid[32+1];     // 交易单号
    char    Fspid[15+1];       // 商户号
    char    Fcoding[32+1];     // 商户订单号
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    int     Fuid;              // 基金用户的CFT内部ID
    char    Ffund_name[64+1];  // 基金名称
    char    Ffund_code[64+1];  // 基金代码
    int     Fpur_type;         // 交易类型   1申购、2 认购、3 定投、4 赎回、5 撤销、6 分红 、7 认申购失败、8 比例确认退款
    LONG    Ftotal_fee;        // 交易金额
    int     Fbank_type;
    char    Fcard_no[32+1];
    int     Fstate;            // 状态：1：等待扣款（只有代扣时会出现）2：代扣成功
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Ftrade_date[20+1]; // 基金交易日期
    char    Ffund_value[16+1]; // 基金净值
    char    Ffund_vdate[20+1]; // 基金净值日期
    char    Ffund_type[32+1];  // 基金类型: 股票型、债券型等，后台直接保存字符串
    char    Fnotify_url[255+1];  // 商户通知url
    char    Frela_listid[255+1]; // 关联交易单号，可能多个，格式: |list1|list2|
    char    Fdrawid[32+1];  // 撤单的B2C退款单号
    char    Ffetchid[32+1];  // 撤单、赎回、分红、退款的提现单号
    unsigned int  Fcft_timestamp;       // 内部时间戳
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    int     Fstandby1;           // 记录是否锁定
    char	Fcft_trans_id[32+1];
	char	Fcft_charge_ctrl_id[32+1];
	char	Fsp_fetch_id[32+1];
	int 	Floading_type;
	char	Fcft_bank_billno[32+1];
	char	Fsub_trans_id[32+1];
    int 	Fcur_type;			//  币种类型：(和核心账户币种类型一致)   1.RMB   2.基金(缺省，兼容原有基金交易记录)
	int		Fspe_tag;			//  特殊标记 0：默认，无意义	1：申购/赎回超时
	char    Facc_time[20+1];	//	申购/赎回时间(与基金公司发生的申购赎回全部已该字段时间为准)
	int		Fpurpose;		//  申购/赎回用途
	char    Fchannel_id[64+1];  
	char    Fsign[32+1];  
	char	Fmemo[128+1];
	int 	Fopt_type;
	LONG 	Fclose_listid;
	LONG	Freal_redem_amt;
	char    Fend_date[16+1];
	char    Fcoupon_id[32+1];  //券消费动作中，申购单关联的券id
	int     Frefund_type;//退款类型
	LONG    Fcharge_fee; // 手续费
	char Ffetch_arrival_time[20+1];
	int    Ffetch_result;
	char    Fcharge_type[1+1]; // 收费方式
	int     Frefund_reason;//退款原因
	int 	Fpay_channel; //支付方式， 0：银行卡   1：理财通余额 2: 网银 
}ST_TRADE_FUND;

/**
 * the fund time table struct
 */
typedef struct
{
	char   Fspid[MAX_SPID_LEN + 1];
	short  Fpur_type;
	char   Fdead_line[MAX_TIME_LENGTH + 1];						// HH:MM:SS dead_line of every day
	char   Fstime[MAX_TIME_LENGTH + 1];							// datetime, become effective time
	short  Flstate;												// 1:valid 2:invalid
	char   Fdead_line_last[MAX_TIME_LENGTH + 1];				// HH:MM:SS last dead_line of every day
	char   Fcreate_time[MAX_TIME_LENGTH + 1];					// datetime
	char   Fmodify_time[MAX_TIME_LENGTH + 1];					// datetime
	int    Fstandby1;
	int    Fstandby2;
	char   Fstandby3[MAX_VARCHAR_LENGTH + 1];
	char   Fstandby4[MAX_VARCHAR_LENGTH + 1];
	char   Fstandby5[MAX_TIME_LENGTH + 1];
} ST_FUND_TIME;


/**
 * 基金账户关联表 记录结构
*/
typedef struct
{
    int     Fcre_type;         // 证件类型
    char    Fcre_id[32+1];     // 证件号码，身份证转换成18位
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    char    Fqqid[64+1];       // 基金用户的CFT账号
    int     Fuid;              // 基金用户的CFT内部ID
    char    Ftrue_name[64+1];  // 投资人真实姓名
    char    Fcre_id_orig[32+1];// 用户的输入身份证号码
    char    Fphone[21+1];      // 电话号码
    char    Fmobile[21+1];     // 手机
    int     Fstate;            // 开户状态  1 - 理财通销户成功，2基金公司销户成功
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Facc_time[20+1];  // 销户时间
    int      Facct_type;    //账户类型
    char    Fchannel_id[64+1];  //渠道信息
    char    Fopenid[64+1];  //余额增值帐号在微信的id
}ST_FUND_UNBIND;


/**
 * 用户的基金转换单表记录结构
*/
typedef struct
{
    char    Fchange_id[32+1];     // 转换单号
    char    Fori_spid[15+1];       // 商户号
    char    Fnew_spid[15+1];       // 商户号
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    char    Fori_fund_code[64+1];  // 基金代码
    char    Fnew_fund_code[64+1];  // 基金代码
    LONG    Ftotal_fee;        // 交易金额
    LONG Fbalance_fee;       //转换余额
    int     Fstate;            // 0：初始 （如果流程中断这个初始态是一种最终态）1：申购申请成功2：赎回成功3：赎回失败（最终状态）4：转换成功（最终状态）
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    int     Fsubacc_state;   //0初始1赎回减成功3申购加成功
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Facc_time[20+1];	//	申购/赎回时间(与基金公司发生的申购赎回全部已该字段时间为准)
    char	Fbuy_id[32+1];    // 申购单号
    char	Fredem_id[32+1]; //赎回单号
    int 	Fcur_type;			//  币种类型：(和核心账户币种类型一致)   1.RMB   2.基金(缺省，兼容原有基金交易记录)
    int	Fspe_tag;			//  特殊标记 0：默认，无意义	1：申购/赎回超时
    char    Fchannel_id[64+1];  
    char    Fsign[32+1];  
    char	Fmemo[128+1];
    
}ST_TRANSFER_FUND;


/**
 * 理财通账号冻结解冻流水表
*/
typedef struct
{
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    char    Fqqid[64+1];       // 基金用户的CFT账号
    int     Fuid;              // 基金用户的CFT内部ID
    int     Fop_type;          // 开户状态  1 - 冻结 2 - 解冻
    char    Fcreate_time[20+1];// 记录创建时间
    int     Fchannel_type;     // 渠道类型  1 - 客服 2 - 风控 3 - 内部
    char    Fop_name[128+1];   // 操作员名称
}ST_FUND_ACCOUNT_FREEZE_LOG;

#endif /* _FUND_STRUCT_DEF_H_ */

