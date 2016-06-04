#include "tools_comm.h"

string Tools::IntoStr(const int i)
{
	char szBuf[32] = {0};
	snprintf(szBuf, sizeof(szBuf), "%d", i);
	return szBuf;
}

//大写转为小写
string Tools::toSmallLetter(string strParaValue)
{	
	string strResult=strParaValue;
	for(unsigned int index=0;index<strResult.size();index++)
	{
		if(strResult[index]>='A' &&  strResult[index]<='Z' )
		{
			strResult[index]= strResult[index]  + 'a' -'A';
		}
	}

	return strResult;
}

int Tools::Bcd2ToAscii(const char *bcd,int len,int align,char *ascii)
{
    char *tbl="0123456789ABCDEF";
    int i,j;
    char *tmpbuf;
    tmpbuf=(char *) malloc((len+1)*sizeof(char));
    memset(tmpbuf,'0',len+1);

    if((align!=0)&&(len%2)!=0)//右对齐
    {
        memcpy(tmpbuf+1,bcd,len);
    }
    else	//左对齐
    {
        memcpy(tmpbuf,bcd,len);
    }

    for(i=0;i<((len+1)/2);i++)
    {
        for(j=0;j<16;j++)
            if(*(tbl+j)==tmpbuf[2*i])
                break;
        if(j==16)
        {
            free(tmpbuf);
            return -1;
        }
        ascii[i]=j;
        ascii[i] = ascii[i] << 4;
        for(j=0;j<16;j++)
            if(*(tbl+j)==tmpbuf[2*i+1])
                break;
        if(j==16)
        {
            free(tmpbuf);
            return -1;
        }
        ascii[i] += j;
    }	

    free(tmpbuf);
    return (len+1)/2;
}

int Tools::AsciiToBcd2(const char *ascii,int len,char *bcd,int mode)
{
    char *tbl2="0123456789ABCDEF";	
    char *tbl1="0123456789:;<=>?";
    char *tbl;

    if(mode == 0)
        tbl = tbl2;
    else
        tbl = tbl1;

    unsigned char t;
    for(int i=0;i<len;i++)
    {
        t=(ascii[i]&(0xf0));
        t=t>>4;
        bcd[2*i]=*(tbl+t);
        t=(ascii[i]&(0x0f));
        bcd[2*i+1]=*(tbl+t);		
    }	
    
    return (2*len);
};

string Tools::CharToHex(const char & c)
{
	  string result;
	  char first, second;

	  first = (c & 0xF0) / 16;
	  first += first > 9 ? 'A' - 10 : '0';
	  second = c & 0x0F;
	  second += second > 9 ? 'A' - 10 : '0';

	  result.append(1, first);
	  result.append(1, second);
	  
	  return result;	
}

string Tools::StrToHex(const unsigned char* str)
{
	  string result;
	  for(unsigned index=0;str[index] != '\0';index++)
	  {
		result+= CharToHex(str[index]);
	  }	  

	  return result;	
}

char Tools::HexToChar(char firstChar,char secondChar)
{
	  int digit;
	  
	  digit = (firstChar >= 'A' ? ((firstChar & 0xDF) - 'A') + 10 : (firstChar - '0'));
	  digit *= 16;
	  digit += (secondChar >= 'A' ? ((secondChar & 0xDF) - 'A') + 10 : (secondChar - '0'));
	  return static_cast<char>(digit);
}


// 1.encodeURIComponent 2.encodeURI 3.escape
string Tools::EncodeString(const string & oldStr,int encodeType)
{
  string result;
  string::const_iterator iter ;
  //几种编码中不用进行编码的字符
  string escapeStr 	= "*+-./@" ;
  string uriStr 		= "!#$&'()*+,-./:;=?@_~" ;
  string uriCompent 	=  "!'()*-._~" ;

  bool bIsEncode = true  ;
  
  for(iter = oldStr.begin(); iter != oldStr.end(); ++iter) 
  {
  	if(isalnum(*iter))
  	{
		result.append(1, *iter);
		continue ;
	}
  	switch(encodeType)
  	{
		 case 1:
		 		if(uriCompent.find(*iter) != string::npos)
		 		{
					bIsEncode = false ;
				}
				break;
		 case 2:
		 		if(uriStr.find(*iter) != string::npos)
		 		{
					bIsEncode = false ;
				}		 		
				break;
		 case 3:
		 		if(escapeStr.find(*iter) != string::npos)
		 		{
					bIsEncode = false ;
				}		 		
				break;		 
		  default:;
	 }
	if((unsigned char)(*iter) > 127 || !bIsEncode)
	{
		result.append(1, *iter);
	}
	else
	{
      	result.append(1, '%');
     	result.append(CharToHex(*iter));
	}
	bIsEncode = true ;
	
  }
  
  return result;
}

string Tools::DecodeString(const string & oldStr)
{
  	  string result;
      string::const_iterator iter;
 	  char c;

	  for(iter = oldStr.begin(); iter != oldStr.end(); ++iter)
	  {
	     switch(*iter) 
		 {
	   			case '%':
						if(distance(iter, oldStr.end()) >= 2
		 					  && isxdigit(*(iter + 1)) && isxdigit(*(iter + 2)))
		 				{
		  				   c = *++iter;
		  				   result.append(1, HexToChar(c, *++iter));
						}
						else 
						{
						    result.append(1, '%');
						}
						break;
		    
		   		  default:
		     		      result.append(1, *iter);
		      		      break;
	      }
	  }
  	  return result ;
}

bool Tools::StrToVector(TStrVector & strVec,const string & Str,const string& sEleSep)
{
	strVec.clear() ;
	std::size_t iEndpos = Str.find(sEleSep) ;
	std::size_t iPos = 0 ;
	while(iEndpos != string::npos)
	{
		if(iPos == iEndpos && iEndpos != 0)
		{
			return false ;
		}
		string tmpStr = Str.substr(iPos,iEndpos-iPos);
		strVec.push_back(tmpStr) ;
		iPos = iEndpos + 1;
		iEndpos = Str.find(sEleSep,iPos); 
	}
	if(iPos+1 <= Str.size()) 
	{
		strVec.push_back(Str.substr(iPos)) ;
	}
	return true ;
}

void Tools::MapToStr(TStr2StrMap& inMap,string & Str,string sEleSep,string sNvSep)
{
	  TStr2StrMap::iterator iter 		= inMap.begin();
	  TStr2StrMap::iterator iter_end  = inMap.end();

	  if(inMap.size() == 0)
	  {
			return ;
	  }
	  Str += iter->first + sNvSep + EncodeString(iter->second) ;
	  iter++ ;
	  while(iter != iter_end)
	  {
			Str += sEleSep + iter->first + sNvSep + EncodeString(iter->second);
			iter++ ;
	  }
              

}

bool Tools::StrToMap(TStr2StrMap & outMap,const string & Str,string sEleSep ,string sNvSep,int emptyFlag)
{
	  TStrVector vEle, vNv;
	  StrToVector(vEle,Str,sEleSep) ;
	 
	  for(unsigned int i = 0; i< vEle.size();i++)
	  {
	  		StrToVector(vNv,vEle[i],sNvSep) ;
	        if(vNv.size() == 1 && emptyFlag)
	        {
	           outMap[vNv[0]] = "" ;
	        }
	        else if(vNv.size() == 2)
	        {
		 	  outMap[vNv[0]] = DecodeString(vNv[1]) ;
	        }
	  }
	  if(outMap.size() == 0)
	  {
	 	return false ;
	  }  
	  return true;	
}



bool Tools::IsDigit(const char * str)
{
	int iLen = strlen(str) ;
	if( iLen == 0)
	{
		return false ;
	}
	for(int iIndex = 0 ; iIndex < iLen ; iIndex++)
	{
		if(!isdigit(*(str+iIndex)))
		{
			return false ;
		}
	}
	return true ;
}


bool Tools::isQQ(const char *str)
{
	int iLen=0;
	for(const char* p=str; *p; p++,iLen++)
	{
		if (!isdigit(*p))  return false;
	}

	return (iLen>=5 && iLen<=12)? true:false;
};

string Tools::GetDbNum(const string& uin)
{
	if(IsDigit(uin.c_str())){
		return uin;
	}

	//email用户,一般不会低于5位
	if(uin.size()<5){
		return "00000";
	}
	
	// Email：前2位为数据库序号
	int iDb = CHAR_HASH( uin.at( 0 ) ) * 10 + CHAR_HASH( uin.at( 1 ) );

	// Email：第3位的散列值为表序号
	int iTbl = CHAR_HASH( uin.at( 2 ) );

	//主机
	int iHost = CHAR_HASH( uin.at( 3 ) ) * 10 + CHAR_HASH( uin.at( 4 ) );

	return ToStr( iDb + iTbl*100 + iHost*1000 );	
		
};
//int转为字符串
/*string IntoStr(const int i)
{
	char szBuf[32] = {0};
	snprintf(szBuf, sizeof(szBuf), "%d", i);
	return szBuf;
}
//大写转为小写
string toSmallLetter(string strParaValue)
{	
	string strResult=strParaValue;
	for(unsigned int index=0;index<strResult.size();index++)
	{
		if(strResult[index]>='A' &&  strResult[index]<='Z' )
		{
			strResult[index]= strResult[index]  + 'a' -'A';
		}
	}

	return strResult;
}*/
bool Tools::CachestrToMap(CIntStr2Map & cachemap,const string & Str,string sMsgSep,string sEleSep ,string sNvSep,int emptyFlag)
{
	  TStrVector vMsg, vEle, vNv;
	  if(StrToVector(vMsg,Str,sMsgSep) != true)
	  	return false;  
	  for(unsigned int i = 0; i< vMsg.size();i++)
	  {
	  		if(StrToVector(vEle,vMsg[i],sEleSep) != true)
				return false;
			cachemap[i]["msg_id"] = vEle[0];
			cachemap[i]["msg_type"] = vEle[1];
			cachemap[i]["nvalue"] = vEle[2];
			cachemap[i]["recv_time"] = vEle[3];
			cachemap[i]["read_flag"] = vEle[4];

	  }
	  if(cachemap.size() == 0)
	  {
	 	return false ;
	  }  
	  return true;	
}

std::string Tools::UrlDecode(const std::string& src)
{
  std::string result;
  std::string::const_iterator iter;
  char c;

  for(iter = src.begin(); iter != src.end(); ++iter) {
    switch(*iter) {
    /*
    case '+':
      result.append(1, ' ');
      break;
    */
    case '%':
	// Don't assume well-formed input
	if(std::distance(iter, src.end()) >= 2
	   && std::isxdigit(*(iter + 1)) && std::isxdigit(*(iter + 2))) {
	    c = *++iter;
	    result.append(1, Tools::HexToChar(c, *++iter));
	}
	// Just pass the % through untouched
	else {
	    result.append(1, '%');
	}
	break;
    
    default:
      result.append(1, *iter);
      break;
    }
  }
  
  return result;
}

bool Tools::ChkParaRegx(TStr2StrMap &toCheck,  
    TStr2StrMap & mapParamRequiredRegx, 
    TStr2StrMap & mapParamOptionalRegx, 
    string &errMsg)
{
    TStr2StrMap::iterator regxRequiredIt = mapParamRequiredRegx.begin();
    TStr2StrMap::iterator regxOptionalIt = mapParamOptionalRegx.begin();
    string strErrmsg, param;

    if(!toCheck["tid"].empty() && !mapParamRequiredRegx["tid"].empty() && 
        regex_match(toCheck["tid"], mapParamRequiredRegx["tid"], strErrmsg) != 0 )
    {
        errMsg = "参数格式错误[" + toCheck["tid"] + "][" + mapParamRequiredRegx["tid"] + "]";
        return false;
    }
    
    //必选参数检查
    for(;regxRequiredIt != mapParamRequiredRegx.end();regxRequiredIt++)
    {
        param = regxRequiredIt->first;
        if(!(regxRequiredIt->second).empty())
        { 
            if(regex_match(toCheck[param], regxRequiredIt->second, strErrmsg) != 0 )
            {
                errMsg = "参数格式错误[" + param + "][" + regxRequiredIt->second + "]";
                return false;
            }
        }
        else if(toCheck[param].empty())
        {
            errMsg = "参数不能为空[" + param + "][" + regxRequiredIt->second + "]";
            return false;
        }
    }  

    //可选参数检查
    for(;regxOptionalIt != mapParamOptionalRegx.end();regxOptionalIt++)
    {
        param = regxOptionalIt->first;
        if(toCheck.count(param) > 0 && !toCheck[param].empty())
        {
            if(!(regxOptionalIt->second).empty())
            { 
                if(regex_match(toCheck[param], regxOptionalIt->second, strErrmsg) != 0 )
                {
                    errMsg = "参数格式错误[" + param + "][" + regxRequiredIt->second + "]";
                    return false;
                }
            }
            else if(toCheck[param].empty())
            {
                errMsg = "参数格式错误[" + param + "][" + regxRequiredIt->second + "]";
                return false;
            }
        }
    }  

    return true;
}

string Tools::MsgNoCount(pthread_mutex_t &m_msgNoMutex)
{
    static unsigned msgNum = 0;
    unsigned num;
    if(msgNum < 0xFFFFFFFE)
    {
        pthread_mutex_lock(&m_msgNoMutex);
        num = msgNum++;
        pthread_mutex_unlock(&m_msgNoMutex); 
    }
    else
    {
        pthread_mutex_lock(&m_msgNoMutex);
        msgNum = 0;
        num = msgNum;
        pthread_mutex_unlock(&m_msgNoMutex); 
    }
    
    char msgNo[11] = {0};
    snprintf(msgNo,sizeof(msgNo), "%010u", num);
    return msgNo;
}



int Tools::regex_match(const string& strSrc,const string& strRegular,string& strErrmsg,int cflags)
{
    int iRet = -1;
    regex_t      reg;  
    char		 ebuf[128] = {0} ; 
    regmatch_t   regmatch[10] ;  
    const int 	 nmatch = 10 ;
    int iSuccess = 2 ;

    iRet = regcomp(&reg, strRegular.c_str(), cflags);
    if(iRet != 0)
    {
        regerror(iRet, &reg, ebuf, sizeof(ebuf)-1);
        strErrmsg = ebuf ;
        regfree(&reg);
        return 2 ;
    }

    iRet = regexec(&reg, strSrc.c_str(), nmatch, regmatch, 0) ;
    if(iRet == 0)
    {
        iSuccess = 0 ;
    }
    else if(iRet == REG_NOMATCH)
    {
        iSuccess = 1 ;
    }
    else
    {
        regerror(iRet, &reg, ebuf, sizeof(ebuf)-1);
        strErrmsg = ebuf ;
        iSuccess = 2 ;
    }
    regfree(&reg);
    return iSuccess ;
}


int Tools::Url2IP(const string &url,string& ip,string &errMsg)
{
	ip = "" ;
	struct hostent *hostinfo;
	//struct in_addr *addp;
	if((hostinfo = gethostbyname(url.c_str())) == NULL)
	{
		//g_RuntimeGather.SaveLog(WARN,"[%s,%d] fail to convert url:%s",
		//	              __FILE__,__LINE__,url.c_str()) ;
        errMsg = "Url2IP fail to convert url:" + url;
		return -1;
	}
	char IPstr[20]={0};
	if(inet_ntop(AF_INET,*hostinfo->h_addr_list,IPstr,sizeof(IPstr))==NULL)
	{
		//g_RuntimeGather.SaveLog(WARN,"[%s,%d] fail to convert url:%s",
		//	              __FILE__,__LINE__,url.c_str()) ;
        errMsg = "Url2IP fail to convert url:" + url;
        return -1;
	}
	ip = IPstr ;
	
	return 0;
}

int Tools::ParseUrl(const char* pszUrl,string& host,string& uri,int &servicePort,int& protocol)
{
    const char* start = NULL;
    const char* end = NULL;

    if (strncasecmp(pszUrl, "http://", 7) == 0)
	{
        protocol = 0;
        servicePort = 80;
        start = pszUrl + 7;
    }
    else if (strncasecmp(pszUrl, "https://", 8) == 0) 
	{
        protocol = 1;
        servicePort = 443;
        start = pszUrl + 8;
    }
    else 
	{
        protocol = 0;
        servicePort = 80;
        start = pszUrl;
    }

    end = strchr(start, '/');
    string strHost;

    if (end == NULL)
    {
        uri = "/";
        strHost = start;
    }
    else
    {
        uri = end;
        strHost = string(start, end - start);
    }
    size_t pos;
	if((pos=strHost.find(":"))==string::npos)
    {
        host = strHost;
    }
    else
    {
        host = strHost.substr(0,pos);
        servicePort = atoi(strHost.substr(pos+1).c_str());
    }
    if (servicePort == 0)
    {
        return -1;
    }
    return 0;
}


bool Tools::CheckQQEmail(const string &emailbox) 
{
	std::size_t iPos = emailbox.find("qq.com") ;
	if(iPos == string::npos)
	{
		return false ;
	}
	string tmp = emailbox.substr(iPos) ;
	if(tmp == "qq.com")
	{
		return true ;
	}
	return false ;
}


string Tools::replace_senstive_word(const string strSRC, const string strSenW, char cRe, char cEsep, char cNsep)
{
    string strDES   =   strSRC;
    
    string strSenstiveWord  =   strSenW+cEsep;
    
    size_t  srcPos  =   strSRC.find(strSenstiveWord);
    if( string::npos == srcPos )
    {
        return strSRC;
    }
    
    size_t rPos(0), rLen(0);
    
    size_t nsepPos  =   strSRC.find(cNsep, srcPos+1);
    if( string::npos == nsepPos )
    {
        rLen    =   strSRC.size() - srcPos - strSenstiveWord.size();
    }else
    {
        rLen    =   nsepPos - srcPos - strSenstiveWord.size();
    }
    
    rPos    =   srcPos + strSenstiveWord.size();
    
    return strDES.replace(rPos, rLen, rLen, cRe);
}


