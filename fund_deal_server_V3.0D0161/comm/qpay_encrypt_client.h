
#ifndef _QPAY_ENCRYPT_CLIENT_H_
#define _QPAY_ENCRYPT_CLIENT_H_

#define LENGTH_DIGEST_SHA1	20
#define LENGTH_DIGEST_MD5	16

typedef struct _ST_PUB_ANS_ {
	int iResult;			/* ����ֵ */
	char szErrInfo[128];	/* �����Ϣ��iResult=0ʱ����ΪOK */
} ST_PUB_ANS;

enum QPAY_ENCRYPT_CLIENT_ERROR_TYPE {
	UnknownError = 1, 	// ĩ֪����
	KeyBadParity, 	// ��Կ��ż�Դ���
	WeakKey				// ����Կ
};

/*
 * ��ϢժҪ-sha-1
 *
 * �������˵��: 
 *
 * �������˵��: 
 * 		
 * ˵��: 
 */
int qpay_generate_digest_sha1(const char* const szTobeDigest, unsigned char* szDest, int* iDestLen);

/*
 * ��ϢժҪ-md5
 *
 * �������˵��: 
 *
 * �������˵��: 
 * 		
 * ˵��: 
 */
int qpay_generate_digest_md5(const char* const szTobeDigest, unsigned char* szDest, int* iDestLen);

/*
 * ����-3DES
 *
 * �������˵��: 
 *
 * �������˵��: 
 * 		
 * ˵��: ʹ�ô˺���ǰ��Ҫǰ���pstKey������SPID������׼ȷ
 */
int qpay_en_3des_v1(const char* szSpid, const unsigned char* const szTobeEncrypt, int iTElen, unsigned char* szDest, int *iDestLen, ST_PUB_ANS* pstAns);

/*
 * ����-3DES
 *
 * �������˵��: 
 *
 * �������˵��: 
 * 		
 * ˵��: ʹ�ô˺���ǰ��Ҫǰ���pstKey������SPID������׼ȷ
 */
int qpay_de_3des_v1(const char* szSpid, const unsigned char* const szTobeEncrypt, int iTElen, unsigned char* szDest, int *iDestLen, ST_PUB_ANS* pstAns);


char *pt(unsigned char *md, int ilen, char *sDest);

#endif
