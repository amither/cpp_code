#include "globalconfig.h"
#include "runinfo.h"
#include "appcomm.h"
#include "decode.h"
#include "error.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "socket.h"
extern CSessionApi* gPtrSession;
extern CftLog*   gPtrCheckLog;


string getDefSpid()
{
        return gPtrConfig->m_SysCfg.encrypt_sp_id;
}


string common_tcp_send(const string &ip, int port, int timeout, const string &req_str)
{

    xyz::CTcpSocket* pSocket = NULL;

    char buf[MAX_MSG_LEN + 1] = {0};

    try
    {
        pSocket = new xyz::CTcpSocket(timeout, ip.c_str(), port);

        if (pSocket == NULL)
        {
            TRACE_ERROR("socket create fail, ip=%s, port=%d, timeout=%d", ip.c_str(), port, timeout);
            throw CException(ERR_XYZ_OPEN, "create socket error", __FILE__, __LINE__);
        }

        int buf_len = req_str.length();

        memcpy(buf, (char*)&buf_len, sizeof(int));
        snprintf(buf + sizeof(int), sizeof(buf) - sizeof(int), "%s", req_str.c_str());

        TRACE_NORMAL("tcp send request:%s", req_str.c_str());

        /* send */
        pSocket->Write(buf, buf_len + sizeof(int));

        /* recv */

        /* recv data length */
        buf_len = 0;
        if(pSocket->Read((char*)&buf_len, sizeof(int), timeout) <= 0)
        {
            TRACE_ERROR("get recv data len fail");
            throw CException(ERR_XYZ_RECV, "get recv length fail", __FILE__, __LINE__);
        }

        /* read data */
        memset(buf, 0, sizeof(buf));
        int total_byte = 0;
        for(total_byte = 0; total_byte < buf_len; )
        {
            int byte_read = pSocket->Read(buf + total_byte, buf_len - total_byte, timeout);
            if (byte_read >= 0)
            {
                total_byte += byte_read;
            }
            else
            {
                TRACE_ERROR("Socket->Read fail, ip=%s, port=%d, timeout=%d", ip.c_str(), port, timeout);
                throw CException(ERR_XYZ_RECV, "recv data fail", __FILE__, __LINE__);
            }

            if(byte_read == 0)
                throw CException(ERR_XYZ_SHUTDOWN, "网络错误", __FILE__, __LINE__);
        }

    }
    catch(...)
    {
        if(pSocket)
        {
            pSocket->Close();
            delete pSocket;
            pSocket = NULL;
        }

        throw;
    }

    if(pSocket)
    {
        pSocket->Close();
        delete pSocket;
    }

    TRACE_NORMAL("tcp response:%s", buf);

    return buf;
}

void sendMsg2Mq(char* szMsg)
{
	if(1 != gPtrConfig->m_AppCfg.sentMqMsg)
	{
		return ;
	}
	try
	{
        char TmpFchannel_id[1024];
        int iRet = CUrlAnalyze::getParam(szMsg, "Fchannel_id", TmpFchannel_id, 1024);
        if(iRet == 0)
        {
            string StrFchannelIdFrom(TmpFchannel_id);
            string StrFchannelIdTo = str_replace(StrFchannelIdFrom,"|","_");
            CUrlAnalyze::modifyParam(szMsg,"Fchannel_id",StrFchannelIdTo.c_str());
        }

		CUrlAnalyze::setParam(szMsg, "sp_id", getDefSpid().c_str());
		CUrlAnalyze::setParam(szMsg, "ver", 1);
	    CUrlAnalyze::setParam(szMsg, "head_u", "");
	    CUrlAnalyze::setParam(szMsg, "request_type", 532221);

		TRACE_DEBUG("sendMsg2Mq message=[%s]", szMsg);

	    string szReturnPkg = common_tcp_send(gPtrConfig->m_MqCfg.host, gPtrConfig->m_MqCfg.port, gPtrConfig->m_MqCfg.overtime, szMsg);
	}
	catch (CException& e)
    {
        TRACE_ERROR("***[%s][%d]%d,%s", e.file(), e.line(), e.error(), e.what());

    }
	catch(...)
	{
		//发送MQ失败不抛出错误
		TRACE_ERROR("sendMsg2Mq error message=[%s]", szMsg);
	}
}

void sendCouponMsg2Mq(const string& strRequest)
{
	try
	{
		char szMsg[MAX_MSG_LEN + 1] = {0};
		CUrlAnalyze::setParam(szMsg, "sp_id", getDefSpid().c_str(), true);
		CUrlAnalyze::setParam(szMsg, "ver", 1);
		CUrlAnalyze::setParam(szMsg, "head_u", "");
		CUrlAnalyze::setParam(szMsg, "request_type", 532229);

		//加密正文内容
	    char	szBuf[MAX_MSG_LEN + 1];
	    char    szResInfo[256] = {0};
	    //int		iResult = -1, oLen = sizeof(szBuf);

	    memset(szBuf, 0x00, sizeof(szBuf));
	    memset(szResInfo, 0x00, sizeof(szResInfo));

	    ST_PUB_ANS stAns; // 结果信息
	    memset(&stAns, 0, sizeof(ST_PUB_ANS));

	    char szEncode[MAX_MSG_LEN + 1] = {0};
	    memset(szEncode, 0x00, sizeof(szEncode));

	    encode(getDefSpid().c_str() , strRequest.c_str(), szEncode, stAns);

	    if (stAns.iResult != 0)
	    {
	        throw CException(stAns.iResult, stAns.szErrInfo, __FILE__, __LINE__);
	    }

		CUrlAnalyze::setParam(szMsg, "request_text", szEncode);

		//string szReturnPkg = common_tcp_send(gPtrConfig->m_MqCfg.host, gPtrConfig->m_MqCfg.port, gPtrConfig->m_MqCfg.overtime, szMsg);
		gPtrRelayClient->write_req(szMsg,strlen(szMsg));

		TRACE_NORMAL("RelayClient send OK！");
	}
	catch(...)
	{
		//发送MQ失败不抛出错误
		TRACE_ERROR("sendCouponMsg2Mq error message=[%s]", strRequest.c_str());
	}
}

/**
 * TRPC 告警
 */
void alert(int e, const string & message)
{
    char szErrCode[16] = {0};
    char szErrMsg[256] = {0};

    snprintf(szErrCode, sizeof(szErrCode), "%d", e);
    snprintf(szErrMsg, sizeof(szErrMsg), "%s", message.c_str());

    /*将字串fund_deal_server改为fudealsvr,字符超过10个则发不出告警*/
    trpc_warning("fudealsvr", szErrCode, szErrMsg);

    // 记录报警
    gPtrAppLog->warning("ALERT:%d,%s", e, message.c_str());
}


//操作鉴权
void checkSession(const string &qlskey,const string &qluin,const string &request_type) throw(CException)
{
    // 校验登录态
    map<string, string> exKeys;
    exKeys["from"] = "";
    exKeys["self"] = request_type;
    exKeys["msgno"] = MSG_NO;
    exKeys["qlskey"] = qlskey;
    exKeys["qluin"] = qluin;

    TRACE_DEBUG("check session:from=%s&self=%s&msgno=%s&qlskey=%s&qluin=%s",
        exKeys["from"].c_str(),
        exKeys["self"].c_str(),
        exKeys["msgno"].c_str(),
        qlskey.c_str(),
        qluin.c_str());

    gPtrSession->checkLogin(qlskey, qluin, exKeys);
}

