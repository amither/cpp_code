#ifndef _DB_PRE_RECORD_USER_ACC_H_
#define _DB_PRE_RECORD_USER_ACC_H_

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
 * 潜在新用户账户表
*/
typedef struct
{
    char    Facc_id[64+1];     // 账户id
    int      Facc_type;// 账户类型
    char    Fuin[64+1];
    char    Fchannel_id[64+1];  // 渠道号
    char    Fbusiness_type[32+1]; 
    char    Fcreate_time[20+1];  // 记录创建时间
    char    Fmodify_time[20+1];  // 记录最新修改时间
    
} ST_PREUSER_ACC;

bool queryPreUserAcc(CMySQL* pMysql, ST_PREUSER_ACC& data,  bool lock);

void insertPreUserAcc(CMySQL* pMysql, ST_PREUSER_ACC& data );

int updatePreUserAcc(CMySQL* pMysql, ST_PREUSER_ACC& data);
   
#endif