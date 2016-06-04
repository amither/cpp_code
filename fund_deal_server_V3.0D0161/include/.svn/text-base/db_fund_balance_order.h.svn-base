#ifndef _DB_FUND_BALANCE_ORDER_H_
#define _DB_FUND_BALANCE_ORDER_H_

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

/**
 * 用户理财通余额流水表
*/
typedef struct
{
    char    Flistid[32+1];     // 单号
    char    Fspid[15+1];       // 商户号
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    LONG    Ftotal_fee;        // 交易金额
    char    Fuin[64+1];     
    int      Fuid;
    int     Ftype;
    int     Fstate;            // 0：初始 
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Facc_time[20+1];	//
    char	Ftotal_acc_trans_id[32+1];    // 余额总账户交易单
    char    Fsubacc_trans_id[32+1];
    char	Fcontrol_id[32+1]; //转账受控单
    int 	Fcur_type;			//  币种类型：(和核心账户币种类型一致)   1.RMB   2.基金(缺省，兼容原有基金交易记录)
    char    Fchannel_id[64+1];  
    char    Fsign[32+1];  
    char	Fmemo[128+1];
    int      Fflag;
    char    Ft1fetch_date[20+1];	//t+1提现日期
    int Fstandby1;
    int Fstandby2;
    char    Fcard_tail[8+1];     //卡尾号
    int      Fbank_type; //支付银行类型
    int     Ffetch_result; //提现结果
    char    Ffetch_arrival_time[20+1]; //提现回导时间
} ST_BALANCE_ORDER; 


/**
 * 单号和用户id关联表
*/
typedef struct
{
    char    Flistid[32+1];     // 单号
    int     Ftype;                //单类型
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
} ST_ORDER_USER_RELA; 



bool queryFundBalanceOrder(CMySQL* pMysql, ST_BALANCE_ORDER& data,  bool lock);

bool queryFundBalanceFetchByListid(CMySQL* pMysql, ST_BALANCE_ORDER& data,  bool lock);

void insertFundBalanceOrder(CMySQL* pMysql, ST_BALANCE_ORDER& data );

void updateFundBalanceOrder(CMySQL* pMysql, ST_BALANCE_ORDER& data );

void insertOrderUserRelation(CMySQL* pMysql, ST_ORDER_USER_RELA& data );

bool queryOrderUserRelation(CMySQL* pMysql, ST_ORDER_USER_RELA& data );

LONG getChargeRecordsFee(CMySQL* mysql,const string& trade_id, const string &cond);

#endif