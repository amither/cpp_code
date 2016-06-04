#ifndef _DB_FUND_UNCONFIRM_H_
#define _DB_FUND_UNCONFIRM_H_

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
 * ���ͨ
*/
class FUND_UNCONFIRM
{

public:
	LONG    Fid;
    char    Ftrade_id[64+1];
    char    Fspid[16+1];     // �̻���
    char    Ftrade_date[20+1];
	char    Fconfirm_date[20+1];
	LONG    Ftotal_fee;          // δȷ�Ͻ��
	LONG    Fcfm_total_fee;      // ��ȷ�Ͻ��
	LONG    Fcfm_units;          //  ��ȷ�Ϸݶ�
	LONG    Funuse_units;        //  ����ʹ�÷ݶ�
	char    Ffund_net[16+1];        //  ��ֵ
	int     Fstate;
	int     Flstate;
    char    Fsign[32+1];    
    char	Fmemo[128+1];   
    char    Fcreate_time[20+1];  // ��¼����ʱ��
    char    Fmodify_time[20+1];  // ��¼�����޸�ʱ��
    
    FUND_UNCONFIRM()
		:Ftrade_id(),Fspid(),Ftrade_date(),Fconfirm_date(),Ffund_net(),Fsign(),Fmemo(),Fcreate_time(),Fmodify_time()
    {
    	Fid = MIN_INTEGER;
    	Ftotal_fee = MIN_INTEGER;
    	Fcfm_total_fee = MIN_INTEGER;
    	Fstate = MIN_INTEGER;
    	Flstate = MIN_INTEGER;
		Fcfm_units = MIN_INTEGER;
		Funuse_units = MIN_INTEGER;
    }
}; 



#define UNCONFIRM_FUND_STATE_ALL 1   // ȫ��δȷ��
#define UNCONFIRM_FUND_STATE_PART 2  // ����δȷ��
#define UNCONFIRM_FUND_STATE_UNUSABLE 3  // ��ȷ��,���ֿ���
#define UNCONFIRM_FUND_STATE_NONE 4  // ȫ����ȷ��

#define UNCONFIRM_FUND_VALID  1  //��Ч
#define UNCONFIRM_FUND_INVALID 2 // ��Ч

bool queryFundUnconfirm(CMySQL* pMysql,  FUND_UNCONFIRM &data,  bool islock = false);

int queryValidFundUnconfirmByTradeId(CMySQL* pMysql, const char* tradeId, vector<FUND_UNCONFIRM> &dataVec);

void updateFundUnconfirm(CMySQL* pMysql,  FUND_UNCONFIRM &data);

// ��ѯ�Ƿ����δȷ���ʲ��ݶ� 
bool queryFundUnconfirmExists(CMySQL* pMysql, const string& spid, const string& tradeId);

void genSign(FUND_UNCONFIRM &data);

void updateFundUnconfirmById(CMySQL* pMysql,  FUND_UNCONFIRM &data);

void insertFundUnconfirm(CMySQL* pMysql, FUND_UNCONFIRM &data );

bool setFundUnconfirm(CMySQL* mysql, const string& tradeId);

void packFundUnconfirm(vector<FUND_UNCONFIRM>& fundUnconfirmVec, string& szValue);

#endif

