#ifndef _DB_FUND_TRANSFER_H_
#define _DB_FUND_TRANSFER_H_

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


bool queryFundTransfer(CMySQL* pMysql, ST_TRANSFER_FUND& data,  bool lock);

void insertFundTransfer(CMySQL* pMysql, ST_TRANSFER_FUND& data );

void updateFundTransfer(CMySQL* pMysql, ST_TRANSFER_FUND& data );

bool checkIfExistTransferIngBill(CMySQL* pMysql, const string&tradeId);
   
#endif


