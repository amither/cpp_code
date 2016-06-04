#ifndef _DB_FUND_INFOMATION_H_
#define _DB_FUND_INFOMATION_H_

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


typedef struct
{
    char info_id[64+1];
    char title[512+1];
    char link[1024+1];
    char info_time[20+1];
    char create_time[20+1];
    int  state;
    int  state_after_audit;
    int  published;
    char source[64+1];
    char html_content[32767+1];

}FundInfomation;

bool queryInfomation(CMySQL* pMysql, FundInfomation & data,bool lock);
void insertInfomation(CMySQL* pMysql, FundInfomation& data);
void updateInfomationState(CMySQL* pMysql, FundInfomation& data);
void updateInfomationHtmlContent(CMySQL* pMysql, FundInfomation& data);
void updateInfomationTitle(CMySQL* pMysql, FundInfomation& data);
void AddToHtmlContent(CMySQL* pMysql, const string &info_id, const string & html_content);
#endif