#ifndef _DB_FUND_BALANCE_CONFIG_H_
#define _DB_FUND_BALANCE_CONFIG_H_

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

typedef struct
{
    int Fid;
    char Fbalance_spid[16+1];  //������̻���
    char Fcharge_spid[16+1];
    char Ffetch_backer_spid[16+1]; //������ֵ����̻���
    char Fbalance_spid_qqid[64+1];
    char Ffetch_backer_qqid[64+1];   //������ֵ����̻�C
    char Ffund_fetch_spid[16+1];     //�������̻���
    char Ffund_fetch_spid_qqid[64+1];
    char Ftrans_date[20+1];
    int Flstate;
    LONG Ftotal_redem_balance_old;   //��������ض��(ת��)
    LONG Ftotal_redem_balance; // ��������ض��(ת��)
    LONG Ftotal_buy_balance_old; //����������깺��
    LONG Ftotal_buy_balance;      //����������깺��
    LONG Ftotal_available_balance; //���˻� ʵ�ʿ������
    LONG Ffetch_backer_total_fee; //�ܵ��ʶ�
    LONG Ffetch_limit_offset;
    LONG Ftotal_baker_fee;           //t �������ֵ��ʶ�
    LONG Ftotal_baker_fee_old;    //���������ֵ��ʶ�
    LONG Ftotal_t1fetch_fee;       //��t+1���ֽ��
    LONG Ftotal_t1fetch_fee_old; //������t+1���ֽ��
    int Fflag;//bit 1 �Ƿ�֧�ֵ��ʣ�bit2 �Ƿ�ֹͣt+0����
    char Fsign[64+1];
    char Fcreate_time[20+1];
    char Fmodify_time[20+1];
    char Fmemo[128+1];
}FundBalanceConfig;


/**
 * ���ͨ���ز���ˮ��
*/
typedef struct
{
    char    Flistid[32+1];     // ����
    LONG    Ftotal_transfer_sup_fee;        // �ز����
    LONG    Ftotal_buy_fee;    // T��������깺���
    LONG    Ftotal_redem_fee;   // T���������ؽ��
    LONG    Ftotal_sup_backer_fee; //���ֵ��ʻز����
    char    Fpurchaser_id[64+1];     
    char    Fbargainor_id[64+1];     
    int Fcur_type;
    int     Ftype;    //1��ת����� 2�����ת�� 3�����ֻز�
    int     Fstate;            // 0����ʼ 1:ת���ز��ɹ�2:ת�������ֶ��ز��ɹ�
    int     Flstate;           // ����״̬  1 - ��Ч�� 2 - ��Ч
    char    Fcreate_time[20+1];  // ��¼����ʱ��
    char    Fmodify_time[20+1];  // ��¼�����޸�ʱ��
    char    Facc_time[20+1];	//
    char    Ftrans_date[20+1];	//
    char    Fsign[32+1];  
    char	Fmemo[128+1];
    char   Fbacker_qqid[64+1];     //���ֵ����˻�
    char   Fsup_backer_translist[32+1];     // ���ֻز�����
    char    Fsup_backer_time[20+1];  // ���ֵ��ʻز�ʱ��
} ST_BALANCE_RECOVER; 


#define DEFAULT_BALNCE_CONFIG_FID 1

#define  FETCH_LOAN_OVER 0x01   //  1 ���ʺľ�

void recordTotalBalanceAccRoll(CMySQL* pMysql, const string &listid,int listType,LONG totalFee,const string&acctime,LONG totalAccBalance,int typeAdd=0);
bool queryFundBalanceRecoverRoll(CMySQL* pMysql, ST_BALANCE_RECOVER & data,  bool lock);
void insertFundFundBalanceRecoverRoll(CMySQL* pMysql, const ST_BALANCE_RECOVER & data);
void updateFundFundBalanceRecoverRoll(CMySQL* pMysql, const ST_BALANCE_RECOVER & data);

bool isFundBalanceRecoverDelayed(CMySQL* pMysql,const string& sysTime);

bool queryFundBalanceConfig(CMySQL* pMysql, FundBalanceConfig & data,  bool lock);

bool setFundBalanceConfigToCkv(CMySQL* pMysql);

bool setFundBalanceConfigToCkv(CMySQL* pMysql,  FundBalanceConfig & data,bool NeedQuery=false);

void updateFundbalanceConfigTradeDate(CMySQL*pMysql,const string &trade_date,const string &systime);

LONG updateFundBalanceConfigForRecover(CMySQL*pMysql,const FundBalanceConfig & data,LONG total_buy,LONG total_redem,LONG total_fee);

void updateFundBalanceConfigForRecoverFetchBacker(CMySQL*pMysql,const FundBalanceConfig & data,LONG total_fee);

void updateFundBalanceConfigForBuyRefund(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForBuy(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForFetch(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date,bool &bNeedBacker);

void updateFundBalanceConfigForT1FetchReq(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);
void updateFundBalanceConfigForT1FetchCnf(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForRedem(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void updateFundBalanceConfigForCharge(CMySQL*pMysql,const FundBalanceConfig&data,LONG total_fee,const string &trade_date);

void AddQuerySqlForswitchTransDate(string &querySql,const string &trade_date,LONG totalFee,int op_type);

BalanceConfigCache getCacheBalanceConfig(CMySQL* pMysql);

#endif

