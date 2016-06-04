#ifndef _DB_FUND_PAY_CARD_H_
#define _DB_FUND_PAY_CARD_H_

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
    char Ftrade_id[32+1];
    char Fqqid[64+1];
    int Fuid;
    char Fbind_serialno[64+1];
    int Fbank_type;
	char Fcard_tail[32+1];
    char Fbank_id[32+1];
    char Fmobile[21+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    int Fstandby1;
    int Fstandby2;
    char Fsign[64+1]; //对应t_fund_pay_card表的Fstandby3字段
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundPayCard;




bool queryFundPayCard(CMySQL* pMysql, FundPayCard & data,  bool lock);



void insertFundPayCard(CMySQL* pMysql, FundPayCard& data );

    
void updateFundPayCard(CMySQL* pMysql, FundPayCard& data );

bool checkPayCard(CMySQL* pMysql, FundPayCard& fund_pay_card, string bind_serialno,TStr2StrMap &bindCareInfo);

bool checKifSafePayCardWhiteListUser(const string &uin);

bool setPayCardToKV(CMySQL* mysql, FundPayCard& fund_pay_card, bool needQuery = false);

/**
 * 组装用户安全卡ckv值
 * @param fund_pay_card 用户安卡信息
 * @param value 组装的ckv值串
 * @return 0-成功 其它-失败
 */
int packUsrPayCardCkvValue(const FundPayCard& fund_pay_card, string &value);

bool delPayCardToKV(string uin);

bool queryBindCardInfo(string uin, string bind_serialno, map<string,string> &resMap);

void saveFundPayCardLog(CMySQL* pMysql, const FundPayCard &old, const string &new_bind_serialno, const string &memo);

void disableFundPayCard(CMySQL* pMysql, const FundPayCard &data);

void recoveryFundPayCard(CMySQL* pMysql, const FundPayCard &data);

#endif

