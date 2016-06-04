#ifndef _DB_FREEZE_FUND_H_
#define _DB_FREEZE_FUND_H_

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
 * ���ᵥ��ṹ
*/
typedef struct
{
    char    Ffreeze_id[32+1];     // ���ᵥ��
    LONG  Ftotal_fee;
    int      Fcur_type;// ��������
    char    Ftrade_id[32+1];   // �������˻���ӦID
    int      Fuid;
    char    Fqqid[64+1];
    char    Ffund_spid[15+1];       // �����̻���
    char    Ffund_code[64+1];  // �������
    int      Ffreeze_type; //��������
    char    Fspid[15+1];       // �̻���
    char    Fsub_acc_freeze_no[32+1];  // ���˻��ⶳ����
    char    Fcoding[32+1]; //�̻�������
    char    Fbuy_id[32+1];
    int     Fstate;  
    int     Flstate;           // ����״̬  1 - ��Ч�� 2 - ��Ч
    int      Fsep_tag;
    LONG  Ftotal_unfreeze_fee;
    char    Fcre_id[32+1]; //֤����md5
    int      Fcre_type;
    char    Ftrue_name[32+1]; //����md5
    char    Fcreate_time[20+1];  // ��¼����ʱ��
    char    Fmodify_time[20+1];  // ��¼�����޸�ʱ��
    char    Facc_time[20+1];	//	�깺/���ʱ��(�����˾�������깺���ȫ���Ѹ��ֶ�ʱ��Ϊ׼)
    char    Fchannel_id[64+1];  
    char    Fsign[32+1];  
    char	Fmemo[128+1];
    int     Fpay_type; //��������Դ1:�깺�󶳽�2:ԭ�зݶ��
    char   Fpurpose[32+1]; 
    char   Fpre_card_no[16+1]; // Ԥ�������� 
    char   Fpre_card_partner[32+1]; // Ԥ�����̻���
    
} ST_FREEZE_FUND;

bool queryFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data,  bool lock);

void insertFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data );

int updateFundFreeze(CMySQL* pMysql, ST_FREEZE_FUND& data, ST_FREEZE_FUND& dbData);
   
#endif

