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
     * 连接事物管理器
     * @param strIp 
     * @param iPort 
     * @连接失败抛异常
     */
    void Init(const string &strIp, int iPort, int iTmout);


    /**
     * 通过事物管理器调用资源管理器
     * @param iSourceType:资源管理器类型
     * @param iSourceCmd:命令字
     * @param szKey:key
     * @param iMiddleNo:midder号
     * @param iParaLen 请求包的长度
     * @param szPara 请求包
     * @param iRespLen 应答缓冲区长度
     * @param szResp 应答缓冲区
     * @return 0:成功;60120101:记录不存在;60120105:重复插入;其它抛异常
     */
    int Query(TRANS_OP_TYPE op_type,  int iSourceType, int iSourceCmd, const char* szKey, int iMiddleNo, int iParaLen, 
        const char* szPara, int& iRespLen, char* szResp);
    
    
    /**
     * 调用事物管理器
     * @param szTransListNo 单号
     * @param szSequenceNo 序列号
     * @param iCmd  业务命令字
     * @param iReqType 请求类型
     * @param iParaLen 请求包的长度
     * @param szPara 请求包
     * @param iRespLen 应答缓冲区长度
     * @param szResp 应答缓冲区
     * @return 0:成功60027000:重复发起,其它情况抛异常
     */
    int Process(const char* szTransListNo, const char* szSequenceNo, int iCmd, int iMiddleNo, 
        int iReqType, int iParaLen, const char* szPara, int& iRespLen, char* szResp);


    /**
     * 查询账户信息
     * @param fuid in inMap:帐户内部ID
     * @param fcurtype in inMap:帐户币种类型
     * @param outMap:存放帐户的信息
     * @param bthrow:记录不存在时,是否抛异常
     * @return 0:查询成功;60120101:记录不存在且bThrow为false
        @   其它情况均抛异常
     *
     */
    int query_acc_info(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * 查询提现单
     * @param sListid:提现单号
     * @param sCurtype:币种类型
     * @param outMap:存放返回信息
     * @return 0:查询成功;60120101:记录不存在且bThrow为false
        @   其它情况均抛异常
     */
    int query_draw_list(const string& sListid, const string &sCurtype, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * 查询交易单
     * @param sListid:交易单号
     * @param sCurtype:币种类型
     * @param outMap:存放返回信息
     * @return 0:查询成功;60120101:记录不存在且bThrow为false
        @   其它情况均抛异常
     */
    int query_tran_list(const string& sListid, const string &sCurtype, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * 查询充值单
     * @param flistid :充值单号
     * @param sCurtype:币种类型
     * @param outMap:存放返回信息
     * @return 0:查询成功;60120101:记录不存在且bThrow为false
        @   其它情况均抛异常
     */
    int query_save_list(const string& sListid, const string &sCurtype, bsapi::CStringMap &outMap, bool bThrow=true);
    

    /**
     * 更改账户状态
     * @param fuid in inMap:帐户内部id
     * @param fcurtpe in inMap:帐户币种类型
     * @param fmodify_time in inMap:修改时间
     * @param outMap:存放返回信息
     * @param bThrow:帐户不存在是否抛异常
     */
    int set_user_state(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);
    
    /**
     * 创建普通财付通帐户
     * @param fuid in inMap:帐户内部ID
     * @param fcurtype in inMap:帐户币种类型
     * @param fqqid in inMap:帐户ID
     * @param fcreate_time in inMap:帐户创建时间
     * @param bThrow:重复创建时是否抛异常
     * @return 0:创建成功;60120105:重复创建且bThrow为false
     * @    其它情况均抛异常         
     */
    int create_user(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);
    
    /**
     * 创建商户帐户
     * @param sp_uid in inMap:商户B帐户内部ID
     * @param fcurtype in inMap:帐户币种类型
     * @param sp_id in inMap:商户号
     * @param sp_qquid in inMap:商户C帐户内部ID
     * @param sp_qqid in inMap:商户C帐户ID
     * @param fcreate_time in inMap:创建时间
     * @param outMap:存放返回信息
     * @param bThrow:重复创建时是否抛异常
     * @return 0:创建成功;60120105:重复创建且bThrow为false
     * @    其它情况均抛异常
     */
    int create_sp(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     *  帐户充值
     * @param transaction_id in inMap:交易单号
     * @param spid in inMap:商户号
     * @param uid in inMap:帐户内部id
     * @param uin in inMap:帐户id
     * @param cur_type in inMap:币种类型
     * @param total_fee in inMap:金额
     * @param client_ip in inMap:客户端ip
     * @param op_time in inMap:操作时间
     * @param outMap:存放返回信息
     * @param bThrow:重复发起时是否抛异常
     * @return 0:成功60027000:重复发起且bThrow为false
     * @    其它情况均抛异常
     */
    int save(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * 帐户提现
     * @param transaction_id in inMap:交易单号
     * @param spid in inMap:商户号
     * @param uid in inMap:帐户内部id
     * @param uin in inMap:帐户id
     * @param cur_type in inMap:币种类型
     * @param total_fee in inMap:金额
     * @param client_ip in inMap:客户端ip
     * @param op_time in inMap:操作时间
     * @param outMap:存放返回信息
     * @param bThrow:重复发起时是否抛异常
     * @return 0:成功60027000:重复发起且bThrow为false
     * @    其它情况均抛异常
     */
    int draw(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     * b2c交易,买家c帐户付款到商户B帐户
     * @param transaction_id in inMap:交易单号
     * @param spid in inMap:商户号
     * @param purchaser_id in inMap:买家id
     * @param purchaser_uid in inMap:买家内部id
     * @param medi_qqid in inMap:卖家-中介帐户id
     * @param medi_uid in inMap:卖家-中介账号内部id
     * @param cur_type in inMap:币种类型
     * @param total_fee in inMap:金额
     * @param op_time in inMap:操作时间
     * @param desc in inMap:交易说明
     * @param client_ip in inMap:客户端ip
     */
    int b2c_pay(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     *  b2c交易退款,商户b帐户退款到买家c帐户
     * @param transaction_id in inMap:交易单号
     * @param spid in inMap:平台商户号
     * @param purchaser_id in inMap:买家id
     * @param purchaser_uid in inMap:买家内部id
     * @param medi_qqid in inMap:卖家-中介帐户id
     * @param medi_uid in inMap:卖家-中介账号内部id
     * @param cur_type in inMap:币种类型
     * @param refundfee in inMap:退款金额
     * @param op_time in inMap:操作时间
     * @param desc in inMap:交易说明
     * @param client_ip in inMap:客户端ip
     */
    int b2c_refund(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);

    /**
     *  c2c转账,目前用户商户c帐户转账给用户
     * @param transaction_id in inMap:交易单号
     * @param spid in inMap:商户号
     * @param purchaser_id in inMap:买家id
     * @param purchaser_uid in inMap:买家内部id
     * @param medi_qqid in inMap:卖家-中介帐户id
     * @param medi_uid in inMap:卖家-中介账号内部id
     * @param bargainor_id in inMap:卖家id,收款方
     * @param bargainor_uid in inMap:卖家内部id
     * @param cur_type in inMap:币种类型
     * @param total_fee in inMap:金额
     * @param op_time in inMap:操作时间
     * @param desc in inMap:交易说明
     * @param client_ip in inMap:客户端ip
     */
    int c2c_transfer(CStr2Map &inMap, bsapi::CStringMap &outMap, bool bThrow=true);
    
private:

    /**
     * 事物管理器对象实例
     */
    CTransApi m_Trans;
};
#endif




