//UrlAnalyze.h

#ifndef URLANALYZE_H
#define URLANALYZE_H

extern "C" {
#include <stdlib.h>
}

#include <iostream>
#include <string>
using namespace std;
//#include "debug_new.h"

//#define PARAM_DEBUG
namespace CFT
{

/* CGI Environment Variables */
//#define SERVER_SOFTWARE getenv("SERVER_SOFTWARE")
//#define SERVER_NAME getenv("SERVER_NAME")
//#define GATEWAY_INTERFACE getenv("GATEWAY_INTERFACE")

//#define SERVER_PROTOCOL getenv("SERVER_PROTOCOL")
//#define SERVER_PORT getenv("SERVER_PORT")
//#define REQUEST_METHOD getenv("REQUEST_METHOD")
//#define PATH_INFO getenv("PATH_INFO")
//#define PATH_TRANSLATED getenv("PATH_TRANSLATED")
//#define SCRIPT_NAME getenv("SCRIPT_NAME")
//#define QUERY_STRING getenv("QUERY_STRING")
//#define REMOTE_HOST getenv("REMOTE_HOST")
//#define REMOTE_ADDR getenv("REMOTE_ADDR")
//#define AUTH_TYPE getenv("AUTH_TYPE")
//#define REMOTE_USER getenv("REMOTE_USER")
//#define REMOTE_IDENT getenv("REMOTE_IDENT")
//#define CONTENT_TYPE getenv("CONTENT_TYPE")
//#define CONTENT_LENGTH getenv("CONTENT_LENGTH")

//#define HTTP_USER_AGENT getenv("HTTP_USER_AGENT")

class CUrlAnalyze {
protected:
	CUrlAnalyze() {}
	
public:
	~CUrlAnalyze() {} //not virtual
	
	static char Hex2Char(char *what);
	static char* StringToUpper(char* src);
	static void unescape(char *url);
	static void unescape(string& url);

	static string DebugMethod();
	static string PostMethod();
	static string GetMethod();
	static string getCgiParam();


	//analyze the parameters
	static int getParam(char* param, const char* const name, int* value, const char seperator = '&');
	static int getParam(char* param, const char* const name, int* value, char** index, const char seperator = '&');
	static int getParam(char* param, const char* const name, char* value, int size, const char seperator = '&');
	static int getParam(char* param, const char* const name, char* value, int size, char** index, const char seperator = '&');

	//construct the parameter list
	static void setParam(char* param, const char* const name, int value, bool FirstParam = false, const char seperator = '&');
	static void setParam(char* param, const char* const name, long value, bool FirstParam = false, const char seperator = '&');
	static void setParam(char* param, const char* const name, const char* const value, bool FirstParam = false, const char seperator = '&');

	static int modifyParam(char* param, const char* const name, int value, const char seperator = '&');
	static int modifyParam(char* param, const char* const name, const char* const value, const char seperator = '&');
	
	static void AddNewline(char* param);
	static void DeleteNewline(char* param);

	//escape character sequence
	static void escape(const char* const src, char* dst, int size, char sep = '&');
	static char* escapeAll(const char* const src, char* dst, int size);
       static char* urlEncode(const char* const src, char* dst, int size);
	static void unescape(const char* const src, char* dst, int size);

	// check param
	static bool isDigit(const char* const str);
	static bool isQQ(const char* const str);
	static bool isEmail(const char* const str);
	static bool isValidBankAcct(const char* const str);

};

// 
class CParamList {
public:
	enum CgiEncodePair {
		_NAME = 0, _VALUE = 1
	};
	
	// 数据项
	typedef struct _EntryData{
  		char *name;
  		char *value;
	} EntryData;

	// 节点
	typedef struct _Node {
  		EntryData data;
  		struct _Node* next;
	} Node;

public:
	CParamList();
	CParamList(const char* const param, char equ = '=', char sep = '&');
	~CParamList();

	void clearList();
	int parseParams(const char *param, char ch1, char ch2);
       char* getParamVal(const char* const name);
       char* getParamVal(const char* const name, int& value);
       char* getParamVal(const char* const name, char* value, int size);

	static char* newstr(char *str);
	inline unsigned short getNum() {
		return _num;
	}
private:
       Node* appendNode(Node* node, EntryData stData);

	// 清空EntryData，包括删除分配的动态空间
	inline void clearEntry(EntryData& data) {
		if (data.name != NULL) {
			#ifdef PARAM_DEBUG
			cout << "delete name: " << data.name << endl;
			#endif
			delete []data.name;
			data.name = NULL;
		}
		if(data.value != NULL) {
			#ifdef PARAM_DEBUG
			cout << "delete value: " << data.value << endl;
			#endif
			delete []data.value;
			data.value = NULL;
		}
	}

       // 生成一个新的Node, 并初始化其中的内容
	inline Node* newNode() {
		Node* newNode = new Node;
		if (newNode == NULL)
			return NULL;
		newNode->next = NULL;
		newNode->data.name = NULL;
		newNode->data.value = NULL;

		return newNode;
	}

	inline void initNode(Node* node) {
           node->data.name = NULL;
           node->data.value = NULL;
           node->next = NULL;
	}

	inline void clearNode(Node* node) {
		clearEntry(node->data);
		node->next = NULL;
	}

private:
	Node _head;	// 列表中第一个node 不存储任何内容，EntryData的值都为NULL
	unsigned short _num;	// 节点数，不包含第一个空节点
};

class CUrlAnalyzeProtocol {
public:
	//the seperator between parameters
	static const char SEPERATOR = '&';

	//error type
	enum ErrorType {InternalError = 1, ParamNotExist};

};

}

#endif

