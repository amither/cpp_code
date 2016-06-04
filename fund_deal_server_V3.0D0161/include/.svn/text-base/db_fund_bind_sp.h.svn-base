#ifndef _DB_FUND_BIND_SP_H_
#define _DB_FUND_BIND_SP_H_

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
    LONG Fimt_id;
    char Ftrade_id[32+1];
    char Fspid[16+1];
    char Fsp_user_id[64+1];
    char Fsp_trans_id[64+1];
    int Facct_type;
	char Fchannel_id[64+1];
    int Fstate;
    int Flstate;
    int Frecon_state;
    char Facc_time[20+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
    char Fexplain[128+1];
    int Fstandby1;
    int Fstandby2;
    char Fstandby3[64+1];
    char Fstandby4[128+1];
    char Fstandby5[20+1];
    char Fstandby6[20+1];
}FundBindSp;




bool queryFundBindSp(CMySQL* pMysql, FundBindSp & data,  bool lock);

bool queryFundBindSpSuccess(CMySQL* pMysql, FundBindSp& data,  bool lock) ;

bool queryMasterSpAcc(CMySQL* pMysql, FundBindSp& data,  bool lock);

//void queryValidMasterSpAcc(CMySQL* pMysql, FundBindSp& data, string spid,  bool lock);

void queryValidFundBindSp(CMySQL* pMysql, FundBindSp& data, bool lock);

void checkValidFundBindSp(vector<FundBindSp> & fundBindSpVec, string new_spid);

void insertFundBindSp(CMySQL* pMysql, FundBindSp& data );
    
void updateFundBindSp(CMySQL* pMysql, FundBindSp& data );

void updateFundBindSpFreeze(CMySQL* pMysql, FundBindSp& data );

void updateFundBindSpAcctType(CMySQL* pMysql, FundBindSp& data );

void updateBindSpStateAcctime(CMySQL* pMysql, FundBindSp& data);

//void deleteFundBindAllSpFromKV(const string& trade_id);

//bool setFundBindAllSpFromKV(CMySQL* pMysql,const string& trade_id, bool changeDefaultSp = false, string defaultSp = "", bool set_type = 0, string modify_time="");

bool setFundBindAllSpToKVFromDB(CMySQL* pMysql,const string& trade_id);

bool setFundBindAllSpToKV(const string& trade_id, vector<FundBindSp>& fundBindSpVec);

/**
 * 构造用户开通基金公司账户列表ckv数据
 * @param list 绑定的基金公司列表
 * @param value ckv中存数的字串值
 * @return 0-成功，其它-失败
 */
int packUsrBindSpCkvValue(const vector<FundBindSp>& list, string &value);
bool parseUsrBindSpCkvValue(const string& value, vector<FundBindSp>& ckv_bindsp_list);
bool getFundBindAllSpFromKV(const string& trade_id, vector<FundBindSp>& fundBindSpVec);

bool getDefaultSpFromKV(const string& trade_id, FundBindSp& data, vector<FundBindSp> & ckv_bindsp_list);

//void changeDefaultTradeAcc(CMySQL* pMysql,string trade_id, string new_spid, string old_spid, string systime, int set_type=0);

bool queryFundBindAllSp(CMySQL* pMysql, vector<FundBindSp>& dataList, const string& trade_id, bool lock);
   
bool getDefaultSp(FundBindSp& def_sp, const vector<FundBindSp> & bindsp_list);
#endif

