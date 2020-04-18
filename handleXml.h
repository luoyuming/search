#pragma once
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"

#include "singleton.h"
#include "common.h"
#include "msgDef.h"
#include "util.h"
#include "log.h"

static const string  SEARCH_MODE_RUN = "1";
#define DEFAULT_THREAD_NUM  1
struct xml_system {
	string host;
	string port;
	string thread_serach; //用户搜索线程数
	string thread_html;  //html分词线程数
	string thread_download; //爬虫数
	string search_mode; // query_mode 1 仅仅搜索不启动查询
	string prevPage;
	string nextPage;
	string spider_interval;
	string hotWord;
	string test;
};

static const string  XML_NODE_CONFIG = "config";
static const string  XML_NODE_NAME = "item";
static const string  XML_FILE_SYSTEM = "system.xml";


class CHandleXml : public SingletionEX<CHandleXml>
{
	SINGLETON_INIT_EX(CHandleXml);
	CHandleXml();
public:	
    bool readDefaultXml();
	xml_system getSystemXml();
	int getPort();
	int getThreadSearchNum();
	int getThreadHtmlNum();
	int getThreadDownloadNum();
	bool onlySearchMode();
	string getPath();
	string getPahtEx();
private:
	bool readXmlFile(string & strFilename);
	bool GetNodePointerByName(TiXmlElement* pRootEle,
		const char* strNodeName, TiXmlElement* &destNode);
	bool serialSystemXml(xml_system & data, vector<string> & vecInfo);
	void checkRule();
private: 
	std::mutex                      m_lock;
	xml_system						m_xml_system;
	string                          m_strPath;
};

#define  CHandleXmlS  (CHandleXml::getInstance())