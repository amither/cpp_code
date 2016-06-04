#ifndef _DB_FREEZE_FUND_H_
#define _DB_FREEZE_FUND_H_

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
 * 冻结单表结构
*/
typedef struct
{
    char    Ffreeze_id[32+1];     // 冻结单号
    LONG  Ftotal_fee;
    int      Fcur_type;// 币种类型
    char    Ftrade_id[32+1];   // 基金交易账户对应ID
    int      Fuid;
    char    Fqqid[64+1];
    char    Ffund_spid[15+1];       // 基金商户号
    char    Ffund_code[64+1];  // 基金代码
    int      Ffreeze_type; //冻结类型
    char    Fspid[15+1];       // 商户号
    char    Fsub_acc_freeze_no[32+1];  // 子账户解冻单号
    char    Fcoding[32+1]; //商户订单号
    char    Fbuy_id[32+1];
    int     Fstate;  
    int     Flstate;           // 物理状态  1 - 有效； 2 - 无效
    int      Fsep_tag;
    LONG  Ftotal_unfreeze_fee;
    char    Fcre_id[32+1]; //证件号md5
    int      Fcre_type;
    char    Ftrue_name[32+1]; //姓名md5
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    char    Facc_time[20+1];	//	申购/赎回时间(与基金公司发生的申购赎回全部已该字段时间为准)
    char    Fchannel_id[64+1];  
    char    Fsign[32+1];  
    char	Fmemo[128+1];
    int     Fpay_type; //冻结金额来源1:申购后冻结2:原有份额冻结
    char   Fpurpose[32+1]; 
    char   Fpre_card_no[16+1]; // 预付卡卡号 
    char   Fpre_card_partner[32+1]; // 预付卡商户号
    
} ST_FREEZE_FUND;

bool queryFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data,  bool lock);

void insertFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data );

int updateFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data, ST_FREEZE_FUND& dbData);
   
#endif

