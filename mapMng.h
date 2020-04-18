#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"
#include "taskQueue.h"
#include "util.h"
#include "utf8String.h"
#include "linksMng.h"
#include "rankMng.h"

static const int MAX_SEARCH_QUEUE_LEN = 10;
static const int MAX_HTML_QUEUE_LEN = 150;
static const int MAX_LINKS_QUEUE_LEN = 10;
static const int MAX_QUEUE_WAIT_MS = 50;
static const int DEFAULT_WEIGHT = 10;
static const int DEFAULT_PAGE_NUM = 10;

struct RESULT_CACHE_INFO {
	UTIL_SELF::Timer  timer;
	string  queryResult;
};

class CMapMng : public SingletionEX<CMapMng> 
{
	SINGLETON_INIT_EX(CMapMng);

	CMapMng();
public:
	bool init();
	void join();
	void stop();

	void saveSearch(MAP_INFO_TYPE & mpTermSnapshot,  map<string, vector<std::shared_ptr<TERM_INFO>>> & mpHtml,
		std::shared_ptr<string> & keyFile);
	
	void writeDb(MAP_INFO_TYPE & mpTermSnapshot, string & url);
	void snapshotSearch(MAP_INFO_TYPE & mpSnapshot);
	void syncFileDb(std::unordered_map<string, std::shared_ptr<MAP_INFO> > & mpSnapshot, string & url);
	bool getQueryCache(string & uri, string & strInfo);
	void inputQueryCache(string & uri, string & strInfo);
	void handleTimeOver();
	void loadKeyword(string & key, vector<TERM_DB_INFO> & vecInfo, map<string, std::shared_ptr<string>> & mpFilePtr);
	
	void lanuchSpider(bool bFirst = false);

	void inputMsg(std::shared_ptr<PACKAGE_INFO> & pkg);
	void searchWord(vector<string> & vecWord, PACKAGE_INFO & info, string & keyword);
	void searchSnapshot(string & snapshotID, PACKAGE_INFO & info);
	void earseWord(string & word, string & url);	
	void buildThread();
private:
	void threadHandleSearch();
	void threadHandleHtml();
	void threadHandleDownlad();
	void threadHandleLinks();

	void mergeKerword(std::shared_ptr<DOC_INFO> & docInfo, vector<std::shared_ptr<TERM_INFO>> & vecTerm);
	void handleHtml(std::shared_ptr<HTML_INFO> & htmlInfo);
	void handleLinks(std::shared_ptr<LINKS_INFO> & links);
	
	bool queryQueueInfo(std::shared_ptr<HTML_INFO> & htmlInfo, string & prevDomain, int time_interval);
	int getOvertime(string & fristDomain);
private:
	void sortPageNum(int & pageStart, int & pageEnd, int pageNow, int pageCount);
	bool extractHtmlTemplate(int threadNum);
	void extractPartTemplate(string & strLast, map<string, string> &mpResult, vector<string> & vecFlag, string & strInfo);
	string pagesLinksView(int total, PACKAGE_INFO & info, vector<string> & vecWord);
	void links2Hmtl(CLinksMng & linkMng, HTML_INFO & htmlInfo);	
	void extractOutLink(std::set<string> & outWebsite, std::set<string> & outLinks, std::set<string> & weightLinks);
	void uniqueOutLink(std::set<string> & outWebsite, std::set<string> & fullDomainLink);
	bool existUrlInDb(const string & url);
	std::shared_ptr<string> buildFilePtr(map<string, std::shared_ptr<string>> & mpFilePtr, string & strFilename);	
	void getFieldOfPage(string & strInfo, list<std::shared_ptr<DOC_INFO>> & liDoc, PACKAGE_INFO & info);
	void getDocInfo(DOC_INFO_TYPE & mpPageInfo, vector<string> & vecWord);
	void getDocByKey(std::list<std::shared_ptr<DOC_INFO> > & docInfo, string & keyword);
	void rankDoc(WEIGHT_INFO_TYPE & mpRank, DOC_INFO_TYPE & mpPageInfo);
	int  getWeight(std::map<string, int> & mpWeight, string & key);
	bool eraseMore(std::list<std::shared_ptr<DOC_INFO> > & liTemp, std::shared_ptr<DOC_INFO> & docInfo);
	bool existValue(std::list<uint32_t> & liOffset, uint32_t value);
	bool existDomain(std::set<string> & links, string & domain);		
	void copyMapInfo(MAP_INFO_TYPE & mpInfo);	
	void putWeblinks(std::vector<std::shared_ptr<HTML_INFO>> & vecLinks);
	bool getWeblinks(std::shared_ptr<HTML_INFO> & info);
	bool getCsdn(std::shared_ptr<HTML_INFO> & info);
	void updateFreshLinks();
	bool sleepDomain(string & firstDomain, int interval, string & url);
	
private:
	std::mutex	m_lock;
	MAP_INFO_TYPE m_mpKeyword;  //分词-->文档信息
	
	std::atomic<uint32_t>  m_download;
	std::mutex	m_webSiteLock;
    int m_sitePos;
    std::unordered_map<string, std::vector<std::shared_ptr<HTML_INFO>>> m_mpSite;
	
	
	std::vector<std::thread>	 m_threadPool;	
	std::shared_ptr<SafeQueue<std::shared_ptr<PACKAGE_INFO>>>         m_QDispatch;
	std::atomic<bool> m_stop;
	std::shared_ptr<SafeQueue<std::shared_ptr<HTML_INFO>>>			m_QHtml;
	std::shared_ptr<SafeQueue<std::shared_ptr<HTML_INFO>>>			m_QDownloadUrl;
	std::shared_ptr<SafeQueue<std::shared_ptr<LINKS_INFO>>>			m_QLinksUrl;
	std::list <std::shared_ptr<HTML_INSERT_INFO>>					m_liHtmlTemplate;

	std::mutex														m_resultCacheLock;
	std::map<string, std::shared_ptr<RESULT_CACHE_INFO>>			m_mpResultCache;

	std::mutex														m_timerLock;
	map<string, std::shared_ptr<UTIL_SELF::Timer >>					m_doainTimer;
};

#define CMapMngS  CMapMng::getInstance()