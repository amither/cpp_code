#ifndef _CKV_SERVICE_OPERATOR_H_
#define _CKV_SERVICE_OPERATOR_H_

#include <stdint.h>
#include "parameter.h"
#include "common.h"
#include "globalconfig.h"
#include <sstream>
#include "cftlog.h"
#include "trmem_cache_lib.h"

//CKV错误代码  cmem cache 错误详解
#define CKV_ERR_MSG_ILLEGAL           -13000    // 非法消息 
#define CKV_ERR_PIPE_FULL             -13001    // 管道满,请检查 cache 管道 是不是堵了。 
#define CKV_ERR_LOCAL_PARSE_ARGS      -13002    // 偏移操作参数不合法
#define CKV_ERR_OP_ADD_PARSE_ARGES    -13003    // 二进制加操作不合法 
#define CKV_ERR_SO_PARSE_ARGS         -13004    // 列操作不合法 可能是取了非法的列号 或者数据内容不是列操作的。 
#define CKV_ERR_SO_PARSE_NOT_SUPPORT  -13005    // cache 不支持列
#define CKV_ERR_INVALID_MASTER_MSG    -13006    // 非法 master, 内部使用 
#define CKV_ERR_CACHEVER_NO_MATCH     -13103    // 内部使用 cache 版本错误 
#define CKV_ERR_KEYSTAMP_NO_MATCH     -13104    // CAS 错误,使用错误的 cas 值修改数据 会返回这个错误,如果对一个 key 有并发操作,那么必然有一个会失败。
                                                // Cas 的功能相当于能确保你的读和写变成原子的。不被人打断 打断则失败。 
#define CKV_ERR_KEY_NO_EXIST          -13105    // del 操作 key 不存在 
#define CKV_ERR_KEY_EXIST             -13109    // key 已经存在 add 操作返回。 
#define CKV_E_NO_SPACE                -13205    // 数据存满，需要扩容才能解决。 

//常使用的
#define CKV_E_NO_DATA                 -13200    // get 发现数据不存在。 
#define CKV_ERR_KEY_EXPIRED           -13106    // get 操作 key 过期,如果设置了 expire 在过期的时候会有这个返回码。 
//批量列操作 错误码, API 错误码
#define CKV_ERR_EMPTY_REQ             -11911    //err_empty_req ： -11911 ， 批量为空请求，自己检查
#define CKV_ERR_INVALID_COL           -11910    //err_invalid_col ： -11910 ，列号不合法，需要在 0~511 之间，自己检查 
#define CKV_ERR_PACKET_TOO_BIG        -11909    //err_packet_too_big ： -11909 ， 发包过大， CMEM 对单次请求的包大小有限制，请不要大于 1M 。 
#define CKV_ERR_LIST_TOO_LONG         -11914    //err_list_too_long ： -11914 ， 批量太长， server 端，有 256 个限制。

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
 * 有关公司产品CKV操作工具类
 * 再次本地封装
 */
class CkvSvrOperator
{
public:
    CkvSvrOperator();
    /**
     * 析构函数
     */
    virtual ~CkvSvrOperator();

    /**
    * 初始化函数
    */
    int init();


    //为了在mysql事物提交之后才提交ckv，增加下面三个函数，但是对于有del操作的需要仔细评估是否有先set再del的场景。
    //对于先del再set的场景不能开启ckv事物。

    //设置ckv事物启动，那么对ckv的set操作只会缓存到本地
    void beginCkvtrans(bool masterDelay=true);

    //对本地缓存的ckv数据批量提交到ckv
    void commitCkvtrans();

    //mysql事物回滚，那么删除本地缓存的ckv数据
    void rollBackCkvtrans();

    int init_sub_acc_ckv();

	/**
     * get操作。
     */
    int get(string&key, CParams& paramOrder); 
    int get(string& key, string& value);
	
    /**
     * 写操作。
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
	*函数: GetByCol
	*功能: 获取 CKV 列模式的数据
	*入参:
	*    key - CKV的key
	*    col - 列编号，0~511.在UserBalance中范围是 0-20
	*出参:
	*    value - CKV的值
	*    cas - (Compare and swap)方式，
	*       用于解决分布式数据修改的问题，保证只有一个操作是成功的，失败返回
	*       ERR_KEYSTAMP_NO_MATCH。
	*       用于返回当前的cas值,-1表示无需cas
	*返回值:
	*	0 - 成功
	*    其他 失败
	***********************************/
	int get_by_col(string key,unsigned int col,string &value,int &cas);
	
	/***********************************
	*函数: GetByCol_MultiCol
	*功能: 批量获取 CKV 单个key的多个列数据
	*入参:
	*	node - 待查询的列信息
	*出参:
	*返回值:
	*	0 - 全部成功
	*	>0 - 操作失败的个数
	*	-11910 - 错误的列ID
	*	-11911 - 批量为空
	*	其它错误详见readme_errcode.txt
	*注意:
	*	具体的每个节点操作的返回值见v_node内的retcode，其具体意义参考 GetByCol
	*   node.key="";
	*    TGetNode node;node.col=iCol;
	*   node.v_col_node.push(node);
	************************************/
	int get_by_multi_col(TKeyGetListNode & node);

private:
    //写主ckv，同步接口
    int setMasterCkv(int key_no, string& key, string& value);
    //写备份ckv操作，异步写入
    int setSlaveCkv(int key_no, string& key, string& value);
    //写备ckv，支持超时设置
    int setSlaveCkv_v2(int key_no,string &key, string &data, int cas, int expire, int offset, int len);
    //初始化ckv链接
    int initCKV(trmem_client_api &stApid, CkvCfg&ckvCfgInfo,bool isMaster);

private:
    int m_bid;
	trmem_client_api m_st_api;
 	trmem_client_api m_st_api_slave;
 	bool m_bSlaveActive; //是否配置备ckv
       bool m_bisTrans; //本次ckv操作是否启动事物
       bool m_bMasterDelay; //主ckv是否延迟写入
       map<string,TCKVLocalCacheNode>m_key2KNodeInfo; //cvk在mysql事物之后提交，用来保存key 和对应的value信息

};

#endif

