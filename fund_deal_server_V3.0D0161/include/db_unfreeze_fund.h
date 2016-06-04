
#ifndef _DB_UNFREEZE_FUND_H_
#define _DB_UNFREEZE_FUND_H_

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

/**
 * 解冻单表结构
*/
typedef struct
{
    char    Funfreeze_id[32+1];     // 解冻单号
    char    Ffreeze_id[32+1];     // 冻结单号
    LONG  Ftotal_fee;
    LONG  Fcontrol_fee;
    int      Fcur_type;// 币种类型
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    int      Fuid;
    char    Ffund_spid[15+1];       // 基金商户号
    char    Ffund_code[64+1];  // 基金代码
    int      Funfreeze_type; //解冻类型
    char    Fspid[15+1];       // 商户号
    char    Fsub_acc_unfreeze_no[32+1];  // 子账户解冻单号
    char    Fcoding[32+1]; //商户订单号
    char    Fsub_acc_draw_no[32+1]; 
    char    Fsub_acc_control_no[32+1]; 
    int     Fstate;  
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Facc_time[20+1];	//	申购/赎回时间(与基金公司发生的申购赎回全部已该字段时间为准)
    char    Fchannel_id[64+1];  
    char    Fsign[32+1];  
    char    Fredem_id[32+1]; //强制赎回单号
    char    Fpay_trans_id[32+1];//余额支付单号解冻扣款用到
    char	Fmemo[128+1];
    char Fqqid[64+1];
    char Fstandby3[32+1];
    
} ST_UNFREEZE_FUND;

bool queryFundUnFreeze(CMySQL* pMysql, ST_UNFREEZE_FUND& data,  bool lock);

bool queryFundUnFreezeByUnfreezeid(CMySQL* pMysql, ST_UNFREEZE_FUND& data,  bool lock);

void insertFundUnFreeze(CMySQL* pMysql, ST_UNFREEZE_FUND& data );

void updateFundUnFreeze(CMySQL* pMysql, ST_UNFREEZE_FUND& data, ST_UNFREEZE_FUND& dbData );
   
#endif


