#ifndef _TRANS_API_PROXY_H
#define _TRANS_API_PROXY_H

#include "UrlAnalyze.h"
#include "TransApi.h"
#include "CStringMap.h"
#include "kbase.h"

#define CORE_SUC_YET  60017000

enum OP_TYPE
{
    OP_QUERY,
    OP_INSERT
};

class CTransApiProxy
{
public:
    /**
     * �������������
     * @param strIp 
     * @param iPort 
     * @����ʧ�����쳣
     */
    void Init(const string &strIp, int iPort, int iTmout);


    /**
     * ͨ�����������������Դ������
     * @param iSourceType:��Դ����������
     * @param iSourceCmd:������
     * @param szKey:key
     * @param iMiddleNo:midder��
     * @param iParaLen ������ĳ���
     * @param szPara �����
     * @param iRespLen Ӧ�𻺳�������
     * @param szResp Ӧ�𻺳���
     * @return 0:�ɹ�;60120101:��¼������;60120105:�ظ�����;�������쳣
     */
    int Query(TRANS_OP_TYPE op_type,  int iSourceType, int iSourceCmd, const char* szKey, int iMiddleNo, int iParaLen, 
        const char* szPara, int& iRespLen, char* szResp);
    
    
    /**
     * �������������
     * @param szTransListNo ����
     * @param szSequenceNo ���к�
     * @param iCmd  ҵ��������
     * @param iReqType ��������
     * @param iParaLen ������ĳ���
     * @param szPara �����
     * @param iRespLen Ӧ�𻺳�������
     * @param szResp Ӧ�𻺳���
     * @return 0:�ɹ�60027000:�ظ�����,����������쳣
     */
    int Process(const char* szTransListNo, const char* szSequenceNo, int iCmd, int iMiddleNo, 
        int iReqType, int iParaLen, const char* szPara, int& iRespLen, char* szResp);


    /**
     * ��ѯ�˻���Ϣ
     * @param fuid in inMap:�ʻ��ڲ�ID
     * @param fcurtype in inMap:�ʻ���������
     * @param outMap:����ʻ�����Ϣ
     * @param bthrow:��¼������ʱ,�Ƿ����쳣
     * @return 0:��ѯ�ɹ�;60120101:��¼��������bThrowΪfalse
        @   ������������쳣
     *
     */
    int query_acc_info(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * ��ѯ���ֵ�
     * @param sListid:���ֵ���
     * @param sCurtype:��������
     * @param outMap:��ŷ�����Ϣ
     * @return 0:��ѯ�ɹ�;60120101:��¼��������bThrowΪfalse
        @   ������������쳣
     */
    int query_draw_list(const string& sListid, const string &sCurtype, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * ��ѯ���׵�
     * @param sListid:���׵���
     * @param sCurtype:��������
     * @param outMap:��ŷ�����Ϣ
     * @return 0:��ѯ�ɹ�;60120101:��¼��������bThrowΪfalse
        @   ������������쳣
     */
    int query_tran_list(const string& sListid, const string &sCurtype, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * ��ѯ��ֵ��
     * @param flistid :��ֵ����
     * @param sCurtype:��������
     * @param outMap:��ŷ�����Ϣ
     * @return 0:��ѯ�ɹ�;60120101:��¼��������bThrowΪfalse
        @   ������������쳣
     */
    int query_save_list(const string& sListid, const string &sCurtype, bsapi::CStringMap &outMap, bool bThrow=true);
    

    /**
     * �����˻�״̬
     * @param fuid in inMap:�ʻ��ڲ�id
     * @param fcurtpe in inMap:�ʻ���������
     * @param fmodify_time in inMap:�޸�ʱ��
     * @param outMap:��ŷ�����Ϣ
     * @param bThrow:�ʻ��������Ƿ����쳣
     */
    int set_user_state(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);
    
    /**
     * ������ͨ�Ƹ�ͨ�ʻ�
     * @param fuid in inMap:�ʻ��ڲ�ID
     * @param fcurtype in inMap:�ʻ���������
     * @param fqqid in inMap:�ʻ�ID
     * @param fcreate_time in inMap:�ʻ�����ʱ��
     * @param bThrow:�ظ�����ʱ�Ƿ����쳣
     * @return 0:�����ɹ�;60120105:�ظ�������bThrowΪfalse
     * @    ������������쳣         
     */
    int create_user(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);
    
    /**
     * �����̻��ʻ�
     * @param sp_uid in inMap:�̻�B�ʻ��ڲ�ID
     * @param fcurtype in inMap:�ʻ���������
     * @param sp_id in inMap:�̻���
     * @param sp_qquid in inMap:�̻�C�ʻ��ڲ�ID
     * @param sp_qqid in inMap:�̻�C�ʻ�ID
     * @param fcreate_time in inMap:����ʱ��
     * @param outMap:��ŷ�����Ϣ
     * @param bThrow:�ظ�����ʱ�Ƿ����쳣
     * @return 0:�����ɹ�;60120105:�ظ�������bThrowΪfalse
     * @    ������������쳣
     */
    int create_sp(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     *  �ʻ���ֵ
     * @param transaction_id in inMap:���׵���
     * @param spid in inMap:�̻���
     * @param uid in inMap:�ʻ��ڲ�id
     * @param uin in inMap:�ʻ�id
     * @param cur_type in inMap:��������
     * @param total_fee in inMap:���
     * @param client_ip in inMap:�ͻ���ip
     * @param op_time in inMap:����ʱ��
     * @param outMap:��ŷ�����Ϣ
     * @param bThrow:�ظ�����ʱ�Ƿ����쳣
     * @return 0:�ɹ�60027000:�ظ�������bThrowΪfalse
     * @    ������������쳣
     */
    int save(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * �ʻ�����
     * @param transaction_id in inMap:���׵���
     * @param spid in inMap:�̻���
     * @param uid in inMap:�ʻ��ڲ�id
     * @param uin in inMap:�ʻ�id
     * @param cur_type in inMap:��������
     * @param total_fee in inMap:���
     * @param client_ip in inMap:�ͻ���ip
     * @param op_time in inMap:����ʱ��
     * @param outMap:��ŷ�����Ϣ
     * @param bThrow:�ظ�����ʱ�Ƿ����쳣
     * @return 0:�ɹ�60027000:�ظ�������bThrowΪfalse
     * @    ������������쳣
     */
    int draw(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * b2c����,���c�ʻ�����̻�B�ʻ�
     * @param transaction_id in inMap:���׵���
     * @param spid in inMap:�̻���
     * @param purchaser_id in inMap:���id
     * @param purchaser_uid in inMap:����ڲ�id
     * @param medi_qqid in inMap:����-�н��ʻ�id
     * @param medi_uid in inMap:����-�н��˺��ڲ�id
     * @param cur_type in inMap:��������
     * @param total_fee in inMap:���
     * @param op_time in inMap:����ʱ��
     * @param desc in inMap:����˵��
     * @param client_ip in inMap:�ͻ���ip
     */
    int b2c_pay(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     *  b2c�����˿�,�̻�b�ʻ��˿���c�ʻ�
     * @param transaction_id in inMap:���׵���
     * @param spid in inMap:ƽ̨�̻���
     * @param purchaser_id in inMap:���id
     * @param purchaser_uid in inMap:����ڲ�id
     * @param medi_qqid in inMap:����-�н��ʻ�id
     * @param medi_uid in inMap:����-�н��˺��ڲ�id
     * @param cur_type in inMap:��������
     * @param refundfee in inMap:�˿���
     * @param op_time in inMap:����ʱ��
     * @param desc in inMap:����˵��
     * @param client_ip in inMap:�ͻ���ip
     */
    int b2c_refund(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     *  c2cת��,Ŀǰ�û��̻�c�ʻ�ת�˸��û�
     * @param transaction_id in inMap:���׵���
     * @param spid in inMap:�̻���
     * @param purchaser_id in inMap:���id
     * @param purchaser_uid in inMap:����ڲ�id
     * @param medi_qqid in inMap:����-�н��ʻ�id
     * @param medi_uid in inMap:����-�н��˺��ڲ�id
     * @param bargainor_id in inMap:����id,�տ
     * @param bargainor_uid in inMap:�����ڲ�id
     * @param cur_type in inMap:��������
     * @param total_fee in inMap:���
     * @param op_time in inMap:����ʱ��
     * @param desc in inMap:����˵��
     * @param client_ip in inMap:�ͻ���ip
     */
    int c2c_transfer(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);
    
private:

    /**
     * �������������ʵ��
     */
    CTransApi m_Trans;
};
#endif




