#pragma once
#include "singleton.h"
#include "common.h"
#include "util.h"
#include "msgDef.h"
#include "opensslEx.h"


static const int MAX_IDLE_SEC = 20;
static const int MAXEVENT = 10;
static const int TCP_PORT = 2020;
static const int DATA_BUFSIZE = 2048;


#define ContentLength1		"Content-Length:"
#define ContentLength2		"content-length:"
#define ContentLength3		"Content-length:"





static const string strSPACE = " ";
static const string strCRLN = "\r\n";
static const string strCRLNCRLN = "\r\n\r\n";
#define CRLN_FLAG				strCRLN
#define HEAD_END_FLAG			strCRLNCRLN
#define HEAD_END_FLAG_ONE		"\n\n"
#define HEAD_END_FLAG_TWO		"\r\r"

static const string GET_MOTHED = "GET";
static const string POST_MOTHED = "POST";
static const int HTTP_POST = 1;
static const int HTTP_GET = 2;


enum class RESP_CONTENT_TYPE {
	ORIGIN_TYPE = 0,  
	JSON_TYPE = 1,
	HTML_TYPE = 2,
	JS_TYPE = 3,
	CSS_TYPE = 4,
	PNG_TYPE = 5,
	JPG_TYPE = 6,
	GIF_TYPE = 7,
	ICO_TYPE = 8,
	BMP_TYPE = 9,
	TXT_TYPE = 10,
};

enum class PORT_TYPE {
	PORT_TYPE_HTTP = 1,
	PORT_TYPE_SSL_TCP = 2,
};

struct FORM_DATA_INFO {
	string fileName;
	string name;
	string data;
};

struct PACKAGE_INFO {
	int commandID;
	bool bRespOk;
	string uri; //url+parameter
	string url;
	string strResp;
	string strHead;
	string strFieldFlag; //field of http head to last one   "\r\n\r\n"	
	string strBody;
	string ContentType;
	RESP_CONTENT_TYPE respType;
	string contentEncoding;
	int bodyLen;
	std::map<string, string> mOriginalParam; //not to decode parameter
	std::map<string, string> mParam; //decode urlcode of head parameter
	std::unordered_map<string, string> mField; //field of http head
	int  sock;
	string ip;
	vector<std::shared_ptr<FORM_DATA_INFO> > vecFormData;
	std::shared_ptr<HTML_INSERT_INFO> htmlTemplate;	
	std::map<string, int> mpWeight;
	std::shared_ptr<string>   localHost; //全域名+/ 如 http://www.dswd.net/	
	int pageNo; //从0开始
	std::shared_ptr<string>   prevPage;
	std::shared_ptr<string>   nextPage;

	std::shared_ptr<string>   doc_root;
	SSL					*ssl_;
	PORT_TYPE           portType;
	bool				ssl_connect;


};

typedef struct _SOCKINFO
{
	SSL_CTX				*sslCtx;
	SSL					*ssl_;
	bool				ssl_connect;
	PORT_TYPE           portType;
	int                 sock;
	int					count;
	string              ip;
	unsigned short      port;
	std::shared_ptr<UTIL_SELF::Timer> pTimer;
	unsigned int        pos;
	char data[DATA_BUFSIZE];

	int extLen;
	string extData;

	int pkgLen;
	std::shared_ptr<PACKAGE_INFO>  pPkgInfo;
	std::vector<std::shared_ptr<string> >  vecBody;
} SOCKINFO;

struct SND_MSG_INFO {
	string msg;
	int pos;
};

struct SSL_CRT {
    string ca;
    string crt;
    string key;
};


class CEpollMng : public SingletionEX <CEpollMng>
{
	SINGLETON_INIT_EX(CEpollMng);
	CEpollMng();

public:
	bool init();
	void handleSvr();
	void stop();
	void httpHtmlResp(std::shared_ptr<PACKAGE_INFO> & pPkgInfo);
	SSL_CTX * getCtx(string & key);
private:
	bool BuildSvr(int port);
	bool buildEpoll(int nPort, PORT_TYPE portType);
	int SetNonblocking(int sock);
	void DoRecv(int fd, PORT_TYPE type, SSL * ssl);
	void DoEvent();
	void DoAccept(int sock, PORT_TYPE portType);
	void sslAcept(int fd, shared_ptr<SOCKINFO> & info);
	void eraseSockInfo(int fd);
	void inputSockInfo(shared_ptr<SOCKINFO> sockInfo);
	void CloseConnect(int fd, bool Erase = true);
	void handleTimeover();
	int handleRawDataHttp(shared_ptr<SOCKINFO> & sockInfo, char *pMsg, int recvLen);	
	void handleSndMsg(shared_ptr<SOCKINFO> & sockInfo);
	bool sndMsg(int fd, string & pkg);
	bool sndRawData(int fd, std::shared_ptr<SND_MSG_INFO> & info, bool & bClose, PORT_TYPE type, SSL * ssl);
	void httpBuildResp(string & strInfo, std::shared_ptr<PACKAGE_INFO> & pPkgInfo);	
	void handleMothed(std::shared_ptr<PACKAGE_INFO> & pInfo);
	void handleField(std::shared_ptr<PACKAGE_INFO> & pInfo);
	void getLNFlag(string & flag, string & strInfo);
	bool getContentLength(int & len, string & strInfo, string flag);
	void getMothedParam(std::shared_ptr<PACKAGE_INFO> & pInfo, string & strInfo);
	bool fowardMsg(std::shared_ptr<PACKAGE_INFO> & pInfo, bool & bPost, int & len);
	void getUrlInfo(std::shared_ptr<PACKAGE_INFO> & pInfo, string & strInfo);
	void resetPkg(std::shared_ptr<PACKAGE_INFO> & pPkgInfo);
	bool existCache(std::shared_ptr<PACKAGE_INFO> & pPkgInfo);
private:
	char * headEnd(char *pMsg, string & strFlag, int & recvLen);
	void setContentType();
	void inputSndMsg(int fd, std::shared_ptr<SND_MSG_INFO> & msg, bool back=true);
	bool getSndMsg(int fd, std::shared_ptr<SND_MSG_INFO> & msg);
	void eraseSndMsg(int fd);
	PORT_TYPE getSockType(int fd);
private:
	std::atomic<bool> m_quit;
	int m_tcpSock;
	int m_epfd;
	map<int, shared_ptr<SOCKINFO> > m_mpSockInfo;	
	map<RESP_CONTENT_TYPE, string>  m_RespType;
	std::set<string>				m_setMothed;
	std::list<string>				m_liLN;

	std::mutex											m_sndLock;
	map<int, list<std::shared_ptr<SND_MSG_INFO>>>		m_sndMsg;

private:
	void initSSL(string & path);
	void handleReadSSL(int fd);
	void handleHandshake(int fd, shared_ptr<SOCKINFO> & info);
	void handleDataRead(int fd, shared_ptr<SOCKINFO> & info);
	void handleDataWrite(int fd, shared_ptr<SOCKINFO> & info);
	int  m_sslTcpSock;
private:
	BIO* errBio;
	
	SSL_CTX *g_sslTemp;
	std::map<string, SSL_CTX*>  m_mpCtxSSL;

	std::mutex	 m_lockSSL;
	map<int, std::shared_ptr<Channel> >	m_mpSSL;  //socket-->sslConnected_(false)
};

#define CEpollMngS  CEpollMng::getInstance()