#ifndef _CKV_SERVICE_OPERATOR_H_
#define _CKV_SERVICE_OPERATOR_H_

#include <stdint.h>
#include "parameter.h"
#include "common.h"
#include "globalconfig.h"
#include <sstream>
#include "cftlog.h"
#include "trmem_cache_lib.h"

//CKV�������  cmem cache �������
#define CKV_ERR_MSG_ILLEGAL           -13000    // �Ƿ���Ϣ 
#define CKV_ERR_PIPE_FULL             -13001    // �ܵ���,���� cache �ܵ� �ǲ��Ƕ��ˡ� 
#define CKV_ERR_LOCAL_PARSE_ARGS      -13002    // ƫ�Ʋ����������Ϸ�
#define CKV_ERR_OP_ADD_PARSE_ARGES    -13003    // �����ƼӲ������Ϸ� 
#define CKV_ERR_SO_PARSE_ARGS         -13004    // �в������Ϸ� ������ȡ�˷Ƿ����к� �����������ݲ����в����ġ� 
#define CKV_ERR_SO_PARSE_NOT_SUPPORT  -13005    // cache ��֧����
#define CKV_ERR_INVALID_MASTER_MSG    -13006    // �Ƿ� master, �ڲ�ʹ�� 
#define CKV_ERR_CACHEVER_NO_MATCH     -13103    // �ڲ�ʹ�� cache �汾���� 
#define CKV_ERR_KEYSTAMP_NO_MATCH     -13104    // CAS ����,ʹ�ô���� cas ֵ�޸����� �᷵���������,�����һ�� key �в�������,��ô��Ȼ��һ����ʧ�ܡ�
                                                // Cas �Ĺ����൱����ȷ����Ķ���д���ԭ�ӵġ������˴�� �����ʧ�ܡ� 
#define CKV_ERR_KEY_NO_EXIST          -13105    // del ���� key ������ 
#define CKV_ERR_KEY_EXIST             -13109    // key �Ѿ����� add �������ء� 
#define CKV_E_NO_SPACE                -13205    // ���ݴ�������Ҫ���ݲ��ܽ���� 

//��ʹ�õ�
#define CKV_E_NO_DATA                 -13200    // get �������ݲ����ڡ� 
#define CKV_ERR_KEY_EXPIRED           -13106    // get ���� key ����,��������� expire �ڹ��ڵ�ʱ�������������롣 
//�����в��� ������, API ������
#define CKV_ERR_EMPTY_REQ             -11911    //err_empty_req �� -11911 �� ����Ϊ�������Լ����
#define CKV_ERR_INVALID_COL           -11910    //err_invalid_col �� -11910 ���кŲ��Ϸ�����Ҫ�� 0~511 ֮�䣬�Լ���� 
#define CKV_ERR_PACKET_TOO_BIG        -11909    //err_packet_too_big �� -11909 �� �������� CMEM �Ե�������İ���С�����ƣ��벻Ҫ���� 1M �� 
#define CKV_ERR_LIST_TOO_LONG         -11914    //err_list_too_long �� -11914 �� ����̫���� server �ˣ��� 256 �����ơ�

using namespace ssdasn;

struct TCKVLocalCacheNode
{
    TCKVLocalCacheNode(){isv2=false;expire=0;offset=0;len=-1;writeSlaveCkv=true;}
    int keyno;
    string value;
    bool isv2;
    int expire;
    int offset ;
    int len;
    bool writeSlaveCkv;
};


/**
 * �йع�˾��ƷCKV����������
 * �ٴα��ط�װ
 */
class CkvSvrOperator
{
public:
    CkvSvrOperator();
    /**
     * ��������
     */
    virtual ~CkvSvrOperator();

    /**
    * ��ʼ������
    */
    int init();


    //Ϊ����mysql�����ύ֮����ύckv�����������������������Ƕ�����del��������Ҫ��ϸ�����Ƿ�����set��del�ĳ�����
    //������del��set�ĳ������ܿ���ckv���

    //����ckv������������ô��ckv��set����ֻ�Ỻ�浽����
    void beginCkvtrans(bool masterDelay=true);

    //�Ա��ػ����ckv���������ύ��ckv
    void commitCkvtrans();

    //mysql����ع�����ôɾ�����ػ����ckv����
    void rollBackCkvtrans();

    int init_sub_acc_ckv();

	/**
     * get������
     */
    int get(string&key, CParams& paramOrder); 
    int get(string& key, string& value);
	
    /**
     * д������
     */
    int set(int key_no, string& key, string& value,bool writeSlaveCkv=true);

	int del(int key_no, string& key) ;

	int incr_create(string &key, uint64_t value = 0);

	int incr_init(string &key, uint64_t value = 0);

	int incr_value_v2(string &key, int64_t& value);

	int set_v2(int key_no, string &key, string &data, int &cas, int expire = 0, int offset = 0, int len = -1);

	int get_v2(string &key, string &data, int &cas, int offset = 0, int len = -1, TRspExt* rsp_ext = NULL);

	int set_v2_with_get_v2(int key_no, string &key, string &data, int expire = 0, int offset = 0, int len = -1);

    /************************************
	*����: GetByCol
	*����: ��ȡ CKV ��ģʽ������
	*���:
	*    key - CKV��key
	*    col - �б�ţ�0~511.��UserBalance�з�Χ�� 0-20
	*����:
	*    value - CKV��ֵ
	*    cas - (Compare and swap)��ʽ��
	*       ���ڽ���ֲ�ʽ�����޸ĵ����⣬��ֻ֤��һ�������ǳɹ��ģ�ʧ�ܷ���
	*       ERR_KEYSTAMP_NO_MATCH��
	*       ���ڷ��ص�ǰ��casֵ,-1��ʾ����cas
	*����ֵ:
	*	0 - �ɹ�
	*    ���� ʧ��
	***********************************/
	int get_by_col(string key,unsigned int col,string &value,int &cas);
	
	/***********************************
	*����: GetByCol_MultiCol
	*����: ������ȡ CKV ����key�Ķ��������
	*���:
	*	node - ����ѯ������Ϣ
	*����:
	*����ֵ:
	*	0 - ȫ���ɹ�
	*	>0 - ����ʧ�ܵĸ���
	*	-11910 - �������ID
	*	-11911 - ����Ϊ��
	*	�����������readme_errcode.txt
	*ע��:
	*	�����ÿ���ڵ�����ķ���ֵ��v_node�ڵ�retcode�����������ο� GetByCol
	*   node.key="";
	*    TGetNode node;node.col=iCol;
	*   node.v_col_node.push(node);
	************************************/
	int get_by_multi_col(TKeyGetListNode & node);

private:
    //д��ckv��ͬ���ӿ�
    int setMasterCkv(int key_no, string& key, string& value);
    //д����ckv�������첽д��
    int setSlaveCkv(int key_no, string& key, string& value);
    //д��ckv��֧�ֳ�ʱ����
    int setSlaveCkv_v2(int key_no,string &key, string &data, int cas, int expire, int offset, int len);
    //��ʼ��ckv����
    int initCKV(trmem_client_api &stApid, CkvCfg&ckvCfgInfo,bool isMaster);

private:
    int m_bid;
	trmem_client_api m_st_api;
 	trmem_client_api m_st_api_slave;
 	bool m_bSlaveActive; //�Ƿ����ñ�ckv
       bool m_bisTrans; //����ckv�����Ƿ���������
       bool m_bMasterDelay; //��ckv�Ƿ��ӳ�д��
       map<string,TCKVLocalCacheNode>m_key2KNodeInfo; //cvk��mysql����֮���ύ����������key �Ͷ�Ӧ��value��Ϣ

};

#endif

