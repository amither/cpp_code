
#ifndef _QPAY_ENCRYPT_CLIENT_H_
#define _QPAY_ENCRYPT_CLIENT_H_

#define LENGTH_DIGEST_SHA1	20
#define LENGTH_DIGEST_MD5	16

typedef struct _ST_PUB_ANS_ {
	int iResult;			/* 返回值 */
	char szErrInfo[128];	/* 结果信息，iResult=0时，置为OK */
} ST_PUB_ANS;

enum QPAY_ENCRYPT_CLIENT_ERROR_TYPE {
	UnknownError = 1, 	// 末知错误
	KeyBadParity, 	// 密钥奇偶性错误
	WeakKey				// 弱密钥
};

/*
 * 信息摘要-sha-1
 *
 * 输入参数说明: 
 *
 * 输出参数说明: 
 * 		
 * 说明: 
 */
int qpay_generate_digest_sha1(const char* const szTobeDigest, unsigned char* szDest, int* iDestLen);

/*
 * 信息摘要-md5
 *
 * 输入参数说明: 
 *
 * 输出参数说明: 
 * 		
 * 说明: 
 */
int qpay_generate_digest_md5(const char* const szTobeDigest, unsigned char* szDest, int* iDestLen);

/*
 * 加密-3DES
 *
 * 输入参数说明: 
 *
 * 输出参数说明: 
 * 		
 * 说明: 使用此函数前需要前填充pstKey参数，SPID必须填准确
 */
int qpay_en_3des_v1(const char* szSpid, const unsigned char* const szTobeEncrypt, int iTElen, unsigned char* szDest, int *iDestLen, ST_PUB_ANS* pstAns);

/*
 * 解密-3DES
 *
 * 输入参数说明: 
 *
 * 输出参数说明: 
 * 		
 * 说明: 使用此函数前需要前填充pstKey参数，SPID必须填准确
 */
int qpay_de_3des_v1(const char* szSpid, const unsigned char* const szTobeEncrypt, int iTElen, unsigned char* szDest, int *iDestLen, ST_PUB_ANS* pstAns);


char *pt(unsigned char *md, int ilen, char *sDest);

#endif
