#ifndef _DB_C2CDB_USER_H_
#define _DB_C2CDB_USER_H_

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
    int Fuid;
    char Fqqid[65+1];
    int Fcurtype;
    LONG Fbalance;
	LONG Fcon;
    
}SubaccUser;




bool querySubaccUserInfo(CMySQL* pMysql, SubaccUser & data,  bool lock);

bool querySubaccUserInfo(CMySQL* pMysql, int uid, int curtype, LONG& balance,LONG&con);

LONG querySubaccBalance(int uid, int subacc_curtype, bool throwNotexistExcp = true,LONG *freezeBalance=NULL);

void getSubaccdbAndSubaccttc(int uid, string & subacc_db,string & subacc_ttc);

int querySubaccBalanceFromCKV(int uid, int subacc_curtype, int64_t *  balance, bool throwNotexistExcp = true,LONG *freezeBalance=NULL);


int querySubaccBalanceFromTTC(int uid, int subacc_curtype, int64_t *  balance, const string & subacc_ttc, bool throwNotexistExcp = true,LONG *freezeBalance=NULL);

void querySubaccBalanceListFromCKV(int uid, const vector<int> & subacc_curtype_list, vector<SubaccUser> &SubaccListUser);

void querySubaccBalanceListFromSubDB(int uid, const vector<int> & subacc_curtype_list, vector<SubaccUser> &SubaccListUser);

#endif

