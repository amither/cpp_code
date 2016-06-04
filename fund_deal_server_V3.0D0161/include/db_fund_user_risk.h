#ifndef _DB_FUND_USER_RISK_H_
#define _DB_FUND_USER_RISK_H_

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
#include "fund_commfunc.h"


/**
 * 理财通用户测评结果表
*/
typedef struct
{
    char    Fqqid[64+1];     // 单号
    char    Fspid[32+1];     // 商户号
    int      Frisk_score;    //测评分数
    char    Fsubject_no[128+1];     // 测评题目编号
    char    Fanswer[2048+1];     // 测评答案
    char Fclient_ip[16+1];
    int    Frisk_type;            // 测评等级1-安益型;2-保守型;3-稳健型;4-积极型;5-激进型;
    char	Fmemo[128+1];   //备注
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
} ST_USER_RISK; 



void saveFundUserRisk(CMySQL* pMysql,  ST_USER_RISK &data);
bool queryFundUserRisk(CMySQL* pMysql,  ST_USER_RISK &data);


#endif

