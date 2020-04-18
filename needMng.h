#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"
#include "util.h"

static const string FILE_IP_RANGE = "dict/ip.cn.utf8";
static const string FILE_NAME_NEED = "dict/need.utf8";

static const string EMPTY_FLAG = "empty";

struct NEED_INFO_EX {
	NEED_INFO need;
	UTIL_SELF::Timer timer;
};

class CNeedMng : public SingletionEX<CNeedMng>
{
	SINGLETON_INIT_EX(CNeedMng);

	CNeedMng();
public:
	bool init(string & strPath);	
	bool getNeedWebSite(DOWNLOAD_URL_INFO & info);
	void inputNeed(DOWNLOAD_URL_INFO & info);
	bool isCnDomain(string & strDomain);
	bool readIpRange(string & strPath);
	bool readNeed(string & strPath);
	void html2need(DOWNLOAD_URL_INFO & info, NEED_INFO & need);
	void need2html(NEED_INFO & need, DOWNLOAD_URL_INFO & info);	
	void getDomain(vector<string> & vecDomain);
	
private:
	bool getNeedEx(NEED_INFO & need);
	void getSplit(vector<string> & vecInfo, string & strInfo);
private:
	std::mutex  m_lock;
	std::list<IP_RANGE_INFO>	m_liIpRange; //可以搜索范围
	std::list<NEED_INFO>		m_liNeedUrl;  //开始遍历的IP地址	
	std::set<string>			m_setDomain;

	std::list<std::shared_ptr<NEED_INFO_EX>> m_liNeedBak;

	
};

#define CNeedMngS  CNeedMng::getInstance()