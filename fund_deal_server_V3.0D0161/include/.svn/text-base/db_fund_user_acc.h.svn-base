#ifndef _DB_FUND_USER_ACC_H_
#define _DB_FUND_USER_ACC_H_

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
#include "globalconfig.h"
#include "fund_commfunc.h"



typedef struct
{
    char Ftrade_id[32+1];
    char Ffund_code[64+1];
    int Ftype;
    LONG Fcloseid;
    int Fidx;
    LONG Ftotal_fee;
	int Fconfirm;
    char Ftime[20+1];
    int checkFlag; // 检查标记,不记录CKV
}FundUserAcc;

#define USER_ACC_NEED_CONFIRM_BUY  0x01 // 申购待确认金额
#define USER_ACC_NEED_CONFIRM_REDEEM  0x02 // 赎回待确认份额


bool setUserAcc(vector<FundUserAcc> & userAccVec);

bool setUserAccVec(vector<FundUserAcc>& userAccVec);

bool setUserAccMap(string tradeId,map<string, FundUserAcc>& userAccMap);

// bool getUserAcc(const string& trade_id, vector<FundUserAcc> & userAccVec);
bool delUserAcc(const string& trade_id);

int getUserAcc(FundUserAcc& queryUserAcc, vector<FundUserAcc> & userAccVec);

int getUserAccMap(const string& trade_id, map<string,FundUserAcc> & userAccMap);

bool addUserAcc(FundUserAcc& userAcc,bool resort);

bool removeUserAcc(FundUserAcc& userAcc);

bool minusUserAcc(FundUserAcc& userAcc);

void updateUserAcc(const ST_TRADE_FUND& stTradeBuy) throw (CException);

bool updateUserAccConfirm4Buy(const char* fundCode, const char* tradeId, int bizId, bool needConfirm) throw (CException);

bool updateUserAccConfirm4Redeem(const char* fundCode, const char* tradeId, int bizId, bool needConfirm) throw (CException);

bool setUserAccToKV(CMySQL* mysql, const string& trade_id, bool needUpdate) throw (CException);

bool addUseSubAccToKV(CMySQL* mysql,const string& trade_id,string& spid) throw (CException);

/*
bool checkUserAccKV4Del(CMySQL* mysql,ST_FUND_BIND& pstRecord,map<string,FundUserAcc>& ckvMap,vector<FundUserAcc>& userAccVec) throw (CException);

bool checkUserAccKV4Add(CMySQL* mysql,const string& tradeId,map<string,FundUserAcc>& ckvMap,vector<FundUserAcc>& userAccVec) throw (CException);
*/

bool checkOpenTransUserAcc(CMySQL* mysql,ST_FUND_BIND& pstRecord,map<string,FundUserAcc>& ckvMap,map<string,FundUserAcc>& dbMap, bool checkDelete) throw (CException);

bool checkCloseTransUserAcc(CMySQL* mysql,const string& tradeId,map<string,FundUserAcc>& ckvMap,map<string,FundUserAcc>& dbMap, bool checkDelete) throw (CException);

bool isUserAccKVAllCheck(map<string,FundUserAcc>& ckvMap) throw (CException);

#endif

