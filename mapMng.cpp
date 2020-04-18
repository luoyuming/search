#include "mapMng.h"
#include "log.h"
#include "handleXml.h"
#include "dispatch.h"
#include "html.h"
#include "iconvCode.h"
#include "storeMng.h"
#include "needMng.h"
#include "httpClient.h"
#include "md5.h"
#include "adMng.h"
#include "bussMng.h"
#include "exceptMng.h"
#include "fileMng.h"

static const int MAX_TIME_DOWNLOAD = 864000000;

CMapMng::CMapMng() 
	:m_stop(false)
{	
	m_QDispatch = std::make_shared<SafeQueue<std::shared_ptr<PACKAGE_INFO>>>(MAX_SEARCH_QUEUE_LEN);
	m_QHtml = std::make_shared<SafeQueue<std::shared_ptr<HTML_INFO>>>(MAX_HTML_QUEUE_LEN);
	m_QDownloadUrl = std::make_shared<SafeQueue<std::shared_ptr<HTML_INFO>>>(MAX_HTML_QUEUE_LEN);
	m_QLinksUrl = std::make_shared<SafeQueue<std::shared_ptr<LINKS_INFO>>>(MAX_LINKS_QUEUE_LEN);
    m_sitePos = -1;

	m_download = 0;
}


void CMapMng::Quit()
{
	
}



void CMapMng::stop()
{
	m_stop.store(true);
	CEpollMngS->stop();
	m_QDispatch->exit();
	m_QHtml->exit();
	m_QDownloadUrl->exit();
	m_QLinksUrl->exit();
}

void CMapMng::join()
{	
	for (std::thread & thread : m_threadPool) {
		if (thread.joinable()) {
			thread.join(); // 等待任务结束， 前提：线程一定会执行完			
		}
	}
	LOG_INFO("join to finish....");
}

bool CMapMng::init()
{
	string path = CHandleXmlS->getPath();
	if (!CAdMngS->init(path)) {
		return false;
	}

	int threadNum = CHandleXmlS->getThreadSearchNum();
	if (!extractHtmlTemplate(threadNum)) {
		return false;
	}	
	CFileMngS->init();
	return true;
}

void CMapMng::buildThread()
{
	m_threadPool.clear();
	int coreNum = thread::hardware_concurrency();
	LOG_INFO("hardware_concurrency %d", coreNum);

	int threadNum = CHandleXmlS->getThreadSearchNum();

	for (int i = 0; i < threadNum; i++) {
		std::thread threadHandleSearch(&CMapMng::threadHandleSearch, this);
		m_threadPool.push_back(std::move(threadHandleSearch));
	}

	if (!CHandleXmlS->onlySearchMode()) {
		threadNum = CHandleXmlS->getThreadHtmlNum();
		for (int i = 0; i < threadNum; i++) {
			std::thread threadHandleHtml(&CMapMng::threadHandleHtml, this);
			m_threadPool.push_back(std::move(threadHandleHtml));
		}
		threadNum = CHandleXmlS->getThreadDownloadNum();
		LOG_INFO("threadHandleDownlad %d", coreNum);
		for (int i = 0; i < threadNum; i++)
		{
			std::thread threadHandleDownlad(&CMapMng::threadHandleDownlad, this);
			m_threadPool.push_back(std::move(threadHandleDownlad));
		}
		if (threadNum > 0) {
			std::thread threadHandleLinks(&CMapMng::threadHandleLinks, this);
			m_threadPool.push_back(std::move(threadHandleLinks));
		}
	}
}

void CMapMng::extractPartTemplate(string & strLast, map<string, 
	string> &mpResult, vector<string> & vecFlag, string & strInfo)
{
	string strTemp;
	string::size_type pos = 0;
	string::size_type size = strInfo.size();
	int num = vecFlag.size();
	for (int i = 0; i <= num; i++) {
		if (i == num) {
			i--;
		}
		UTIL_SELF::prevSubStr(strTemp, strInfo, vecFlag[i], pos);
		if (pos >= size)
		{
			break;
		}
		mpResult[vecFlag[i]] = strTemp;
	}
	strLast = strTemp;
	return;
}

bool CMapMng::extractHtmlTemplate(int threadNum)
{
	bool bResult = true;

	auto htmlTemplate = std::make_shared<HTML_INSERT_INFO>();
	string strAllHtml;
	string strPath = CHandleXmlS->getPath();
	string file = strPath + "web/result.html";
	if (!UTIL_SELF::readFile(file, strAllHtml)) {
		LOG_ERROR("falult to get %s ", file.c_str());
		return false;
	}

    string strHost = CHandleXmlS->getSystemXml().host;
    UTIL_SELF::replaceEx(strAllHtml, QYT_HOST, strHost);

	string strTemp;
	map<string, string> mpResult;
	vector<string> vecFlag;
	vecFlag.push_back(QYT_TOTAL_PAGE);
	vecFlag.push_back(HTML_INSERT_BEGIN_FLAG);
	vecFlag.push_back(HTML_INSERT_END_FLAG);
	extractPartTemplate(strTemp, mpResult, vecFlag, strAllHtml);
	htmlTemplate->lastPart = strTemp;
	CAdMngS->fillAd(htmlTemplate->lastPart);

	htmlTemplate->total_page_prev = mpResult[QYT_TOTAL_PAGE]; 
	htmlTemplate->total_page_next = mpResult[HTML_INSERT_BEGIN_FLAG];
	string txtTemplate = mpResult[HTML_INSERT_END_FLAG];
	if (txtTemplate.empty()) {
		LOG_ERROR("error to extract result.html");
		return false;
	}

	strTemp.clear();
	mpResult.clear();
	vecFlag.clear();
	vecFlag.push_back(PAGE_BEGIN_FLAG);
	vecFlag.push_back(PAGE_END_FLAG);//此部分不使用
	extractPartTemplate(strTemp, mpResult, vecFlag, htmlTemplate->lastPart);
	htmlTemplate->lastPart = strTemp;
	htmlTemplate->pagePart = mpResult[PAGE_BEGIN_FLAG];

	strTemp.clear();
	mpResult.clear();
	vecFlag.clear();
	vecFlag.push_back(QYT_H3_MAIN_HERF);
	vecFlag.push_back(QYT_H3_VEIW_HERF);
	vecFlag.push_back(QYT_CONTENT);
	vecFlag.push_back(QYT_URL_HERF);
	vecFlag.push_back(QYT_URL_DISPLAY);
	vecFlag.push_back(QYT_URL_SNAPSHOT);
	extractPartTemplate(strTemp, mpResult, vecFlag, txtTemplate);

	htmlTemplate->url_snapshot = strTemp;
	htmlTemplate->h3_main_herf_prev = mpResult[QYT_H3_MAIN_HERF];
	htmlTemplate->h3_view__herf_prev = mpResult[QYT_H3_VEIW_HERF];
	htmlTemplate->content_prev = mpResult[QYT_CONTENT];
	htmlTemplate->url_ref_prev = mpResult[QYT_URL_HERF];
	htmlTemplate->url_display_prev = mpResult[QYT_URL_DISPLAY];
	htmlTemplate->url_snapshot_prev = mpResult[QYT_URL_SNAPSHOT];

	for (int i = 0; i < (threadNum - 1); ++i) {
		auto info = std::make_shared<HTML_INSERT_INFO>();
		*info = *htmlTemplate;
		m_liHtmlTemplate.push_back(info);
	}
	m_liHtmlTemplate.push_back(htmlTemplate);

	return bResult;

}

void CMapMng::threadHandleLinks()
{
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	string threadID = oss.str();
	LOG_INFO("threadHandleLinks begin thread_id %s", threadID.c_str());	
	auto urlInfo = std::make_shared<LINKS_INFO>();
	while (!m_stop) {
		if (m_QLinksUrl->pop_wait(&urlInfo, MAX_QUEUE_WAIT_MS)) {	
			try {
				handleLinks(urlInfo);
			}
			catch (bad_alloc &e) {
				LOG_ERROR("threadHandleLinks bad_alloc %s", urlInfo->downladInfo.url.c_str());
				std::this_thread::sleep_for(std::chrono::seconds(3));				
			}
			catch (...) {
				LOG_ERROR("threadHandleLinks try...catch.%s", urlInfo->downladInfo.url.c_str());
				std::this_thread::sleep_for(std::chrono::seconds(3));				
			}
		}
		if (m_QLinksUrl->exited()) {
			break;
		}
	}
	LOG_INFO("threadHandleLinks end thread_id %s", threadID.c_str());
	return;
}

void CMapMng::links2Hmtl(CLinksMng & linkMng, HTML_INFO & htmlInfo)
{
	htmlInfo.domainName = linkMng.getFullDomain();
	htmlInfo.firstName = linkMng.getFirstDomain();
	htmlInfo.location = linkMng.getLocation();
	htmlInfo.path = linkMng.getPath();
}

bool CMapMng::sleepDomain(string & firstDomain, int interval, string & url)
{
	bool bResult = true;
	int sec = 0;	
	{
		std::lock_guard<std::mutex> lck(m_timerLock);
		auto it = m_doainTimer.find(firstDomain);
		if (m_doainTimer.end() != it) {		
			if (it->second->elapsed_seconds() < interval) {
				sec = interval - it->second->elapsed_seconds();
			}
			else {
				it->second->reset();
			}
		}
		else {
			m_doainTimer[firstDomain] = std::make_shared<UTIL_SELF::Timer>();
		}
	}

	if (sec > 0) {
		LOG_INFO("sleep time (%d) domain(%s) url(%s)", sec, firstDomain.c_str(), url.c_str());
		for (int i = 0; i < sec; i++) {
			if (m_stop) {
				bResult = false;
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}	
	return bResult;
}



int CMapMng::getOvertime(string & fristDomain)
{
	int ret = 0;
	if ("iteye" == fristDomain) {
		ret = 10;
	}
	return ret;
}

bool CMapMng::queryQueueInfo(std::shared_ptr<HTML_INFO> & htmlInfo, string & prevDomain, int time_interval)
{
	bool bResult = false;
	auto linkMng(std::make_unique<CLinksMng>());
	string firstDomain;
	int sleepTime = time_interval;
	auto timer = std::make_shared<UTIL_SELF::Timer>();
	do {		
		if (m_QDownloadUrl->pop_wait(&htmlInfo, MAX_QUEUE_WAIT_MS)) {
			linkMng->parseUrl(htmlInfo->url);
			firstDomain = linkMng->getFirstDomain();
			sleepTime = getOvertime(firstDomain);
			sleepTime += time_interval;
			if (prevDomain != firstDomain) {
				prevDomain = firstDomain;
				//反爬虫
				sleepDomain(firstDomain, sleepTime, htmlInfo->url);
				bResult = true;
				break;
			}
			else {
				if (timer->elapsed_seconds() > sleepTime) {
					bResult = true;
					//LOG_INFO("timeove to do...");
					break;
				}
				m_QDownloadUrl->pushEx(htmlInfo);
			}
			sleepTime = time_interval;
		}
		else {
			break;
		}
		linkMng->reset();
	} while (!m_stop);

	return bResult;
}

void CMapMng::threadHandleDownlad()
{	
	std::this_thread::sleep_for(std::chrono::seconds(5));

	std::ostringstream oss;
	oss << std::this_thread::get_id();
	string threadID = oss.str();
	LOG_INFO("threadHandleDownlad begin thread_id %s", threadID.c_str());
	string strPath = CHandleXmlS->getPath();	
	int time_interval = static_cast<int>(atoi(CHandleXmlS->getSystemXml().spider_interval.c_str()));
	
	string prevDomain;	

	
	vector<string> vecHead;
	auto handleJson(std::make_unique<CHandleJson>());
	auto htmlInfo = std::make_shared<HTML_INFO>();	
	auto linkMng(std::make_unique<CLinksMng>());	
	string url;
	string strValue;
	//bool bUrlNeed = false;
	bool bUrlBuss = false;
	while (!m_stop) {	
		strValue.clear();
		linkMng->reset();
		vecHead.clear();
		url.clear();
	
		if (queryQueueInfo(htmlInfo, prevDomain, time_interval)) {
			//bUrlNeed = htmlInfo->bNeed;
			bUrlBuss = htmlInfo->bBuss;
			url = htmlInfo->url;	
		
			linkMng->reset();
			linkMng->parseUrl(htmlInfo->url, htmlInfo->domainName);
			string firstDomain = linkMng->getFirstDomain();			

			htmlInfo->bUpdate = false;
			bool bValidDate = true;
			if (CStoreMngS->getDb(VISIT_DB, htmlInfo->url, strValue)) {
				time_t now = time(nullptr);
				auto htmlInfoSave = std::make_shared<HTML_INFO>();
				if (handleJson->getHtmlByJson(*htmlInfoSave, strValue)) {
					int diff = difftime(now, htmlInfoSave->downloadTime);
					if (diff < MAX_TIME_DOWNLOAD) {
						bValidDate = false; //一天时间86400 * 10
					}
					else {
						LOG_INFO("diff(%d) now(%d) build_time(%d)", diff, now, htmlInfo->downloadTime);
					}
				}
				htmlInfo->bUpdate = true;
				LOG_WARNING("(%d) check exist url(%s)", bValidDate, htmlInfo->url.c_str());
			}
			
			auto location = std::make_shared<string>();		
			auto body = std::make_shared<string>();
			if (bValidDate) {
				if (!bUrlBuss) {
					LOG_INFO("(%s) url(%s)", htmlInfo->classValue.c_str(), htmlInfo->url.c_str());
				}
				if (CHttpClientS->getEx(htmlInfo->url, htmlInfo->head, *body, vecHead)) {					

					bool bSame = false;
					string docMd5;
					auto md5 = std::make_shared<CMD5>(*body);
					docMd5 = md5->ToString();					
					if (htmlInfo->bUpdate) {						
						if (htmlInfo->md5 == docMd5) {
							//存在相同的md5
							LOG_INFO("---same md5(%s) url(%s)", docMd5.c_str(), htmlInfo->url.c_str());
							bSame = true;
						}
						htmlInfo->oldMd5 = htmlInfo->md5;
					}
					if (!bSame) {
						links2Hmtl(*linkMng, *htmlInfo);
						if (!vecHead.empty()) {
							htmlInfo->location = vecHead[0];
						}
						htmlInfo->md5 = docMd5;
						htmlInfo->downloadTime = ::time(nullptr);
						if (htmlInfo->bUpdate) {
							UTIL_SELF::saveFile(htmlInfo->fullPath, *body);
						}
						else {
							string strDataPath = linkMng->buildMonthDay();
							htmlInfo->fullPath = UTIL_SELF::buildPath(htmlInfo->url, strDataPath, htmlInfo->snapshotUrl);
							UTIL_SELF::saveFile(htmlInfo->fullPath, *body);
							htmlInfo->txtPath = strDataPath;
						}

						strValue = handleJson->buildHtmlJson(*htmlInfo);
						CStoreMngS->putDb(VISIT_DB, htmlInfo->url, strValue);

						//LOG_INFO("*****%s ", htmlInfo->fullPath.c_str());
					
						int donwloadNum = ++m_download;
						LOG_WARNING("downladNum:%d", donwloadNum);
						
						while (!m_QHtml->push(htmlInfo)) {
							if (m_QHtml->exited()) { //退出程序
								LOG_INFO("threadHandleDownlad end thread_id %s", threadID.c_str());
								return;
							}
							std::this_thread::sleep_for(std::chrono::seconds(1));
							LOG_INFO("m_QHtml is sleeping ...");
						}						
					}
				}
				else  if (htmlInfo->bNeed) {
					CNeedMngS->inputNeed(*htmlInfo);
					LOG_INFO("need url(%s)", url.c_str());
				}
			}
			else {
				LOG_INFO("short time url(%s)", url.c_str());
			}	
		
			CStoreMngS->deleteDb(FRESH_URL_DB, url);
		}

		if (m_QDownloadUrl->exited()) {			
			break;
		}
		
		lanuchSpider();
	}

		
	LOG_INFO("threadHandleDownlad end thread_id %s", threadID.c_str());
	return;
}

void CMapMng::threadHandleHtml()
{
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	string threadID = oss.str();
	LOG_INFO("threadHandleHtml begin thread_id %s", threadID.c_str());
	int hotWordNum = static_cast<int>(atoi(CHandleXmlS->getSystemXml().hotWord.c_str()));
	auto htmlInfo = std::make_shared<HTML_INFO>();
	while (!m_stop) {
		if (m_QHtml->pop_wait(&htmlInfo, MAX_QUEUE_WAIT_MS)) {	
			htmlInfo->hotWordNum = hotWordNum;
			try {
				string strTemp = htmlInfo->classValue;
				htmlInfo->bEmptyFrame = htmlInfo->classValue.empty() ? true : false;							
				handleHtml(htmlInfo);				
			}
			catch (bad_alloc &e) {
				LOG_ERROR("threadHandleHtml bad_alloc %s", htmlInfo->url.c_str());
				std::this_thread::sleep_for(std::chrono::seconds(3));
				//m_stop.store(true);
			}
			catch (...) {
				LOG_ERROR("threadHandleHtml try...catch.%s", htmlInfo->url.c_str());
				std::this_thread::sleep_for(std::chrono::seconds(3));
				//m_stop.store(true);
			}
			UTIL_SELF::rmFile(htmlInfo->fullPath);

		}
		if (m_QHtml->exited()){		
			break;
		}		
	}
	stop();
	LOG_INFO("threadHandleHtml end thread_id %s", threadID.c_str());
}

void CMapMng::inputMsg(std::shared_ptr<PACKAGE_INFO> & pkg)
{	
	while (!m_QDispatch->push(pkg)) {
		if (m_QDispatch->exited()) {
			m_stop.store(true);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		LOG_WARNING("m_QDispatch is sleeping ...");
	}
}

void CMapMng::threadHandleSearch()
{
	std::ostringstream oss;
	oss << std::this_thread::get_id();	
	string threadID = oss.str();
	LOG_INFO("threadHandleSearch begin thread_id %s", threadID.c_str());

	std::shared_ptr<HTML_INSERT_INFO> htmlTemplate;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		htmlTemplate = m_liHtmlTemplate.front();
		m_liHtmlTemplate.pop_front();
	}	
	auto doc_root = std::make_shared<string>();
	auto prevPage = std::make_shared<string>();
	auto nextPage = std::make_shared<string>();
	auto localHost = std::make_shared<string>();
	*localHost = CHandleXmlS->getSystemXml().host;
	*prevPage = CHandleXmlS->getSystemXml().prevPage;
	*nextPage = CHandleXmlS->getSystemXml().nextPage;
	*doc_root = CHandleXmlS->getPahtEx();
	
	std::shared_ptr<PACKAGE_INFO> pPkgInfo = std::make_shared<PACKAGE_INFO>();
	auto dispatch = std::make_shared<CDispatch>();	
	while (!m_stop) {		
		if (m_QDispatch->pop_wait(&pPkgInfo, MAX_QUEUE_WAIT_MS)) {
			pPkgInfo->htmlTemplate = htmlTemplate;		
			pPkgInfo->localHost = localHost;
			pPkgInfo->prevPage = prevPage;
			pPkgInfo->nextPage = nextPage;
			pPkgInfo->bRespOk = false;
			pPkgInfo->doc_root = doc_root;
			dispatch->handleInfo(*pPkgInfo);
			CEpollMngS->httpHtmlResp(pPkgInfo);
		}
		if (m_QDispatch->exited()) {
			break;
		}		
	}
	
	LOG_INFO("threadHandleSearch end thread_id %s", threadID.c_str());
	return;
}

void CMapMng::mergeKerword(std::shared_ptr<DOC_INFO> & docInfo, vector<std::shared_ptr<TERM_INFO>> & vecTerm)
{
	docInfo->weight = 0;
	for (auto & item : vecTerm) {
		docInfo->weight += item->weight;
		docInfo->offset.push_back(item->offset);
	}
	return;
}


void CMapMng::writeDb(MAP_INFO_TYPE & mpTermSnapshot, string & url)
{
	if (mpTermSnapshot.empty()) {
		return;
	}
	syncFileDb(mpTermSnapshot, url);

}

void CMapMng::syncFileDb(std::unordered_map<string, std::shared_ptr<MAP_INFO> > & mpSnapshot, string & url)
{
	auto mpDb(std::make_unique<UMAP_TYPE>());
	vector<TERM_DB_INFO> vecTerm;	
	
	auto handleJson(UTIL_SELF::make_unique<CHandleJson>());
	string value;
	for (auto & item : mpSnapshot) {
		vecTerm.clear();
		auto & liDoc = item.second->liDoc;
		for (auto & doc : liDoc) {			
			if (doc && doc->keyFile) {
				auto termInfo(std::make_unique<TERM_DB_INFO>());
				for (auto value : doc->offset) {
					termInfo->offset.push_back(value);
				}				
				termInfo->strFilename = *(doc->keyFile);
				termInfo->weight = doc->weight;
				vecTerm.push_back(*termInfo);
			}
			else {
				LOG_ERROR("nullptr is checking(%s)(%s)",item.first.c_str(), url.c_str());
			}
		}
		if (!vecTerm.empty()) {
			(*mpDb)[item.first] = handleJson->buildTermJson(vecTerm);
		}
	}
	if (!mpDb->empty()) {
		CStoreMngS->putDb(SEARCH_DB, *mpDb);
	}
	return;
}

void CMapMng::snapshotSearch(MAP_INFO_TYPE & mpSnapshot)
{	
	{
		std::lock_guard<std::mutex> lock(m_lock);
		for (auto & item : m_mpKeyword) {
			auto mpInfo = std::make_shared<MAP_INFO>();
			auto & liDoc = item.second->liDoc;
			for (auto & doc : liDoc) {
				auto docPtr = std::make_shared<DOC_INFO>();
			    docPtr->keyFile = std::make_shared<string>();
			    *(docPtr->keyFile) = *(doc->keyFile);
				docPtr->weight = doc->weight;
				docPtr->offset = doc->offset;
				mpInfo->liDoc.push_back(docPtr);
			}
			mpSnapshot[item.first] = mpInfo;
		}
	}
	return;
}

void  CMapMng::saveSearch(MAP_INFO_TYPE & mpTermSnapshot, map<string, vector<std::shared_ptr<TERM_INFO>>> & mpHtml,
	std::shared_ptr<string> & keyFile)
{
	
	for (auto & item : mpHtml) {		
		auto docInfo = std::make_shared<DOC_INFO>();
		docInfo->keyFile = keyFile;
		mergeKerword(docInfo, item.second);	
		auto mpInfo = std::make_shared<MAP_INFO>();
		std::list<std::shared_ptr<DOC_INFO> > & liInfo = mpInfo->liDoc;
		{
			std::lock_guard<std::mutex> lock(m_lock);
			auto itFind = m_mpKeyword.find(item.first);
			if (m_mpKeyword.end() == itFind) {
				m_mpKeyword[item.first] = std::make_shared<MAP_INFO>();				
			}
		
			auto & liDoc = m_mpKeyword[item.first]->liDoc;
			liDoc.push_back(docInfo);
			for (auto & doc : liDoc) {
				auto info = std::make_shared<DOC_INFO>();
				info->keyFile = std::make_shared<string>();
				*(info->keyFile) = *(doc->keyFile);
				info->weight = doc->weight;
				for (auto & pos : doc->offset) {
					info->offset.push_back(pos);
				}
				liInfo.push_back(info);
			}			
		}		
		mpTermSnapshot[item.first] = mpInfo;
	}
	return;
}


void CMapMng::copyMapInfo(MAP_INFO_TYPE & mpInfo)
{
	mpInfo.clear();
	{
		std::lock_guard<std::mutex> lock(m_lock);
		for (auto & item : m_mpKeyword) {
			auto & liDoc = item.second->liDoc;
			for (auto & doc : liDoc) {
				auto info = std::make_shared<DOC_INFO>();
				info->keyFile = std::make_shared<string>();
				*(info->keyFile) = *(doc->keyFile);
				info->keyword = doc->keyword;
				info->offset = doc->offset;
				info->weight = doc->weight;

				auto it = mpInfo.find(item.first);
				if (mpInfo.end() == it) {
					mpInfo[item.first] = std::make_shared<MAP_INFO>();
					mpInfo[item.first]->liDoc.clear();
				}
				mpInfo[item.first]->liDoc.push_back(info);
			}
		}
	}
	return;
}


std::shared_ptr<string> CMapMng::buildFilePtr(map<string, std::shared_ptr<string>> & mpFilePtr, string & strFilename)
{
	std::shared_ptr<string> filePtr;
	auto it = mpFilePtr.find(strFilename);
	if (mpFilePtr.end() == it) {
		filePtr = std::make_shared<string>(strFilename);
		mpFilePtr[strFilename] = filePtr;
	}
	else {
		filePtr = it->second;
	}
	return filePtr;
}

void CMapMng::loadKeyword(string & key, vector<TERM_DB_INFO> & vecInfo, map<string, std::shared_ptr<string>> & mpFilePtr)
{
	if (vecInfo.empty()) {
		LOG_WARNING("empty to kekword %s", key.c_str());
		return;
	}

	auto keywordInfo = std::make_shared<MAP_INFO>();
	for (auto & item : vecInfo) {		
		auto docInfo = std::make_shared<DOC_INFO>();
		docInfo->keyFile = buildFilePtr(mpFilePtr, item.strFilename);
		docInfo->weight = item.weight;
		docInfo->offset = item.offset;	
		keywordInfo->liDoc.push_back(docInfo);
	}

	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_mpKeyword[key] = keywordInfo;
	}
	return;
}

void CMapMng::handleHtml(std::shared_ptr<HTML_INFO> & htmlInfo)
{
	string strContents;
	if (!UTIL_SELF::readFile(htmlInfo->fullPath, strContents))
	{
		LOG_WARNING("fault to read file (%s)", htmlInfo->fullPath.c_str());
		return;
	}	
	auto html(UTIL_SELF::make_unique<CHtml>());
	if (html->parseWord(strContents, htmlInfo)) {

		auto links = html->getLinks();	
		if (!links->hrefLinks.empty()) {
			while (!m_QLinksUrl->push(links)) {
				if (m_QLinksUrl->exited()) {
					this->stop();
					break;
				}
				std::this_thread::sleep_for(std::chrono::seconds(2));
				LOG_INFO("m_QLinksUrl is sleeping ...");
			}
		}
	}	
	return;
}

int  CMapMng::getWeight(std::map<string, int> & mpWeight, string & key)
{
	auto it = mpWeight.find(key);
	if (mpWeight.end() != it) {
		return it->second;
	}
	return DEFAULT_WEIGHT;
}

void CMapMng::rankDoc(WEIGHT_INFO_TYPE & mpRank, DOC_INFO_TYPE & mpPageInfo)
{
	for (auto & item : mpPageInfo) {	
		int weight = 0;
		for (auto & docIno : item.second) {
			weight += docIno->weight;
		}
		if (0 == weight) {
			LOG_ERROR("zero weight ......");
		}
		//LOG_ERROR("weight(%d)(%s)", weight, item.first.c_str());
		mpRank.insert(std::make_pair(weight, item.second));
	}
	
}

void CMapMng::getDocByKey(std::list<std::shared_ptr<DOC_INFO> > & docInfo, string & keyword)
{	
	std::lock_guard<std::mutex>lck(m_lock);
	auto it = m_mpKeyword.find(keyword);
	if (m_mpKeyword.end() != it) {	
		//LOG_ERROR("(%d)(%s)", it->second->liDoc.size(), keyword.c_str());
		for (auto & doc : it->second->liDoc) {
			auto record = std::make_shared<DOC_INFO>();	
			*record = *doc;
			record->keyFile = std::make_shared<string>();			
			*(record->keyFile) = *(doc->keyFile);		
			docInfo.push_back(record);			
		}			
	}	
	return;
}

void CMapMng::getDocInfo(DOC_INFO_TYPE & mpPageInfo, vector<string> & vecWord)
{	
	for (auto & item : vecWord) {
		//LOG_INFO("(%s)", item.c_str());
		std::list<std::shared_ptr<DOC_INFO> > liDoc;//所有关键字对应不同的网页
		getDocByKey(liDoc, item);		
		for (auto & doc : liDoc) {
			//LOG_INFO("%s(%d) url(%s)", item.c_str(), liDoc.size(), doc->keyFile->c_str());
			string & url = *(doc->keyFile);
			auto itFind = mpPageInfo.find(url);
			if (mpPageInfo.end() == itFind) {
				mpPageInfo[url] = list<std::shared_ptr<DOC_INFO>>();
			}
			if (doc->keyword.empty()) {
				doc->keyword = item;
			}
			mpPageInfo[url].push_back(doc);
		}
	}
	//LOG_INFO("(%d)", mpPageInfo.size());

	return;
}

void CMapMng::getFieldOfPage(string & strInfo, list<std::shared_ptr<DOC_INFO>> & liDoc, PACKAGE_INFO & info)
{		
	bool bInit = false;
	std::shared_ptr<CRankMng> rankMng;
	for (auto & doc : liDoc) {
		if (!bInit) {
			string strDocument;
			if (CStoreMngS->getDb(DOCUMENT_DB, *(doc->keyFile), strDocument)) {
				rankMng = std::make_shared<CRankMng>(strDocument, *(doc->keyFile));
				bInit = true;				
			}
			else {
				LOG_ERROR("empty document to get");
				return;
			}
		}
		if (bInit) {			
			rankMng->inputField(doc->offset, doc->keyword);		
		}		
	}
	if (rankMng && bInit) {
		rankMng->extractField();		
		rankMng->fillTemplate(strInfo, info);
	}
	
	return;
} 

void CMapMng::searchSnapshot(string & snapshotID, PACKAGE_INFO & info)
{
	info.strResp = "bad to call";

	if (!CStoreMngS->getDb(DOCUMENT_DB, snapshotID, info.strResp)) {
		LOG_WARNING("not to find the snapshot ID %s", snapshotID.c_str());
	}
	return;
}

void  CMapMng::sortPageNum(int & pageStart, int & pageEnd, int pageNow, int pageCount)
{	
	int pageNum = DEFAULT_PAGE_NUM;
	if (pageNow <= (pageNum / 2 + 1)) {
		pageStart = 1;
		pageEnd = pageNum;
	}
	else if (pageNow > (pageNum / 2 + 1)) {
		pageStart = pageNow - pageNum / 2;
		pageEnd = pageNow + pageNum / 2 - 1;
	}
	// 对pageEnd 进行校验，并重新赋值
	if (pageEnd > pageCount) {
		pageEnd = pageCount;
	}
	if (pageEnd <= pageNum) {
		pageStart = 1;
	}
	return;
}


string CMapMng::pagesLinksView(int total, PACKAGE_INFO & info, vector<string> & vecWord)
{	
	int pageNow = info.pageNo+1;
	int pageNum = total/ DEFAULT_PAGE_NUM + 1; //页数 ，每页为DEFAULT_PAGE_NUM个记录
	int pageStart = 0;
	int pageEnd = 0;
	sortPageNum(pageStart, pageEnd, pageNow, pageNum);

	string parm = info.mOriginalParam[QUERY_KEY_WORD_GET];	
	
	bool bFirst = pageStart > 1 ? true:false;	
	bool bLast = (pageEnd < pageNum) && (pageEnd > 1) ? true :false;

	string pageBeginNo = "/page_url?pageNo=";
	string pageEndNo = "&";
	pageEndNo += QUERY_KEY_WORD_PAD;
	pageEndNo += parm;
	

	string strRet;
	if (bFirst) 
	{
		int curPage = pageNow - 2;
		if (curPage < 0) {
			curPage = 0;
		}
		string firstUrl = pageBeginNo;
		firstUrl += std::to_string(curPage);
		firstUrl += pageEndNo;

		strRet += "<a href = \"";
		strRet += firstUrl;
		strRet += "\" class=\"n\">";
		strRet += *(info.prevPage);
		strRet += "</a>";
	}
	for (; pageStart < pageEnd; pageStart++) {
		
		if (pageStart == pageNow) {
			strRet += "<strong><span class=\"fk fk_cur\"><i class=\"c-icon c-icon-bear-p\"></i></span><span class=\"pc\">";
			strRet += std::to_string(pageStart);
			strRet += "</span></strong>";	
		}
		else {		
			string firstUrl = pageBeginNo;
			firstUrl += std::to_string(pageStart-1);
			firstUrl += pageEndNo;

			strRet += "<a href=\"";
			strRet += firstUrl;
			strRet += "\"><span class=\"fk\"><i class=\"c-icon c-icon-bear-p\"></i></span><span class=\"pc\">";			
			strRet += std::to_string(pageStart);
			strRet += "</span></a>";			
		}
	}
	if (bLast) 
	{
		int curPage = pageNow;
		if (curPage >= pageNum) {
			curPage = pageNum-1;
		}
		string firstUrl = pageBeginNo;
		firstUrl += std::to_string(curPage);
		firstUrl += pageEndNo;

		strRet += "<a href = \"";
		strRet += firstUrl;
		strRet += "\" class=\"n\">";
		strRet += *(info.nextPage);
		strRet += "</a>";
	}	
	return strRet;
}

void CMapMng::searchWord(vector<string> & vecWord, 
	PACKAGE_INFO & info, string & keyword)
{
	string & strResult = info.strResp;
	//一个网页包括的关键字映射集合
	DOC_INFO_TYPE mpPageInfo;
	getDocInfo(mpPageInfo, vecWord);
	if (mpPageInfo.empty()) {
		string strTemp ="/";
		for (auto & item : vecWord) {
			strTemp += item;
			strTemp += "/";			
		}
		LOG_WARNING("%", strTemp.c_str());
		return;
	}

	/*for (auto & item : mpPageInfo) {
		cout << item.first << "/";
		list<std::shared_ptr<DOC_INFO>> & liDoc = item.second;
		cout << " " << liDoc.size();
		for (auto & doc : liDoc) {
			cout << " " << doc->keyword;
			cout << " " << doc->weight;
			for (auto value : doc->offset) {
				cout << " " << (int)value;
			}
		}
		cout << endl;
	}*/
	
	
	auto rankPageInfo = std::make_shared<WEIGHT_INFO_TYPE>();
	rankDoc(*rankPageInfo, mpPageInfo);		
	int total = static_cast<int>(rankPageInfo->size());
	int pageCount = total / DEFAULT_PAGE_NUM + 1;
	//LOG_WARNING("total=%d pageCount=%d", total, pageCount);
	if (info.pageNo >= pageCount) {
		info.pageNo = pageCount - 1;
	}
	
	int maxNum = (info.pageNo + 1) * DEFAULT_PAGE_NUM;
	int beginNo = info.pageNo * DEFAULT_PAGE_NUM;
	std::list<std::shared_ptr<string>> liStrField;
	auto rit = rankPageInfo->rbegin();
	for (int i = 0; i < beginNo; i++) {			
		++rit;
	}	
	
	//LOG_WARNING("beginNo=%d info.pageNo=%d", beginNo, info.pageNo);

	for (; (rit != rankPageInfo->rend()) && (beginNo < maxNum); ++rit, beginNo++)
	{	
		auto strField = std::make_shared<string>();		
		getFieldOfPage(*strField, rit->second, info);
		liStrField.push_back(strField);	
		if (strField->empty()) {
			LOG_WARNING("empty information to query..");
		}		
	}

	strResult = info.htmlTemplate->total_page_prev;
	UTIL_SELF::replaceEx(strResult, QYT_KEYWORD, keyword, true);
	strResult += std::to_string(total);
	strResult += info.htmlTemplate->total_page_next;
	for (auto & item : liStrField) {
		strResult += *(item);
	}
	
	strResult += info.htmlTemplate->pagePart;
	strResult += pagesLinksView(total, info, vecWord);	
	strResult += info.htmlTemplate->lastPart;
	info.bRespOk = true;
	return;
}

bool CMapMng::eraseMore(std::list<std::shared_ptr<DOC_INFO> > & liTemp, std::shared_ptr<DOC_INFO> & docInfo)
{
	bool bResult = true;
	string strTemp;
	string strFlag;
	for (auto & item : liTemp) {
		strTemp = item->keyword;
		strFlag = docInfo->keyword;
		if (strTemp.size() < strFlag.size()) {
			strTemp = docInfo->keyword;
			strFlag = item->keyword;
		}
		string::size_type pos = strTemp.find(strFlag); //相近的 (程序员--程序)
		
		if (string::npos != pos) {
			auto & liOffset = docInfo->offset;
			auto it = liOffset.begin();
			for (; it != liOffset.end(); ) {
				if (existValue(item->offset, *it)) {
					it = liOffset.erase(it);
				}
				else {
					++it;
				}
			}
		}
	}

	if (docInfo->offset.size() > 0) {
		liTemp.push_back(docInfo);
	}
	else {
		bResult = false;
	}
	return bResult;
}

bool CMapMng::existValue(std::list<uint32_t> & liOffset, uint32_t value)
{
	for (auto & item : liOffset) {
		if (value == item) {
			return true;
		}
	}
	return false;
}

void CMapMng::lanuchSpider(bool bFirst)
{	
	if (bFirst) {
		if ((CHandleXmlS->getThreadDownloadNum() <= 0)
			|| (CHandleXmlS->onlySearchMode()))
			return;
	}

	///////////////////////////////////////////////////////////
	////https://m.wanren8.co/0/548/129447_6.html error SSL_ERROR_UNSUPPORTED_VERSION
	//static bool bSnd = false;
	//if (!bSnd) {
	//	bSnd = true;	
	//	auto htmlInfo = std::make_shared<HTML_INFO>();	
	//	htmlInfo->url = "https://blog.csdn.net/jason_wang1989/article/details/104684988";
 //       CLASS_INFO cls;
 //       if (CWeightMngS->get(htmlInfo->url, *htmlInfo)) {
 //          // LOG_WARNING("2");
 //       }
 //       else {
 //          // LOG_WARNING("1");
 //       }

	//	
	//	LOG_WARNING("------%s---%s--", htmlInfo->className.c_str(), htmlInfo->classValue.c_str());
	//	if (CNeedMngS->getNeedWebSite(*htmlInfo))
	//	{  
	//		//htmlInfo->bBuss = true;
	//		htmlInfo->url = "https://blog.csdn.net/jason_wang1989/article/details/104684988";
	//	
	//		while (!m_QDownloadUrl->push(htmlInfo)) {
	//			if (m_QDownloadUrl->exited()) {
	//				stop();
	//				break;
	//			}
	//			std::this_thread::sleep_for(std::chrono::seconds(1));
	//			LOG_WARNING("m_QDownloadUrl is sleeping ...");
	//		}
	//	}
	//}
	//return;
	/////////////////////////////////////////////////////////

	
	auto htmlInfo = std::make_shared<HTML_INFO>();	
	bool bPush = false;
	if (CNeedMngS->getNeedWebSite(*htmlInfo)) {
		bPush = true;
		htmlInfo->bBuss = false;
		htmlInfo->bNeed = true;
	}
	else if (CBussMngS->getBussNeedUrl(*htmlInfo)) {
		htmlInfo->bNeed = false;
		htmlInfo->bBuss = true;
		bPush = true;
	}
	else if (getWeblinks(htmlInfo)) {
		htmlInfo->bNeed = false;
		bPush = true;		
	}
	if (bPush) {	
		while (!m_QDownloadUrl->push(htmlInfo)) {
			if (m_QDownloadUrl->exited()) {
				stop();
				break;
			}			
			std::this_thread::sleep_for(std::chrono::seconds(1));
			LOG_INFO("m_QDownloadUrl is sleeping ...");
		}			
	}
	return;
}



void CMapMng::earseWord(string & word, string & url)
{
	{
		std::lock_guard<std::mutex> lock(m_lock); 
		auto itFind = m_mpKeyword.find(word);
		if (m_mpKeyword.end() != itFind) {
			auto & liDoc = itFind->second->liDoc;
			auto it = liDoc.begin();
			for (; it != liDoc.end(); ) {
				string & key = *((*it)->keyFile);
				if (key == url) {
					it = liDoc.erase(it);					
				}
				else {
					++it;
				}
			}
			if (liDoc.empty()) {
				m_mpKeyword.erase(itFind);
			}
		}
	}
	return;
}

bool CMapMng::existDomain(std::set<string> & links, string & domain)
{
	bool bResult = false;
	auto it = links.find(domain);
	if (links.end() != it) {
		bResult = true;
	}
	return bResult;
}

void CMapMng::extractOutLink(std::set<string> & outWebsite, std::set<string> & outLinks, std::set<string> & weightLinks)
{
	string flag = "://";
	string::size_type pos;
	for (auto & item : weightLinks) {
		for (auto & url : outLinks) {
			pos = url.find(item); 
			if (string::npos != pos) {
				pos = url.find(flag);
				if (string::npos != pos) {
					pos += flag.size();					
					string value = url.substr(0, pos);
					value += "www.";
					value += item;
					outWebsite.insert(value);
					//cout <<"11111111111111111111111 " << item << " " << url << endl;
				}
			}
		}
	}
	return;
}

void CMapMng::uniqueOutLink(std::set<string> & outWebsite, std::set<string> & fullDomainLink)
{

	string::size_type pos;
	auto it = fullDomainLink.begin();
	for (; it != fullDomainLink.end(); ) {
		bool bFind = false;
		for (auto & item : outWebsite) {
			pos = item.find((*it));
			if (string::npos != pos) {
				bFind = true;
				break;
			}
		}
		if (bFind) {
			fullDomainLink.erase(it++);
		}
		else {
			++it;
		}
	}
	return;
}

void CMapMng::updateFreshLinks()
{
    {
        std::lock_guard<std::mutex>lck(m_webSiteLock);
        if (!m_mpSite.empty()) {
            return;
        }        
    }
	std::vector<std::shared_ptr<HTML_INFO>> vecLinks;
	std::set<string> setInfo;
	if (CStoreMngS->getFreshUrl(setInfo, 5000)) {		
		auto handleJson(std::make_unique<CHandleJson>());
		
		for (auto & value : setInfo) {
			auto htmlInfo = std::make_shared<HTML_INFO>();
			if (handleJson->getHtmlByJson(*htmlInfo, value)) {
				CLASS_INFO cls;
				if (CWeightMngS->get(htmlInfo->url, cls)) {
					htmlInfo->className = cls.className;
					htmlInfo->classValue = cls.classValue;
					htmlInfo->weight = cls.weight;
					vecLinks.push_back(htmlInfo);
				}				
			}
		}
		putWeblinks(vecLinks);
	}
	return;
}

bool CMapMng::getCsdn(std::shared_ptr<HTML_INFO> & info)
{
	bool bResult = false;
	{
		std::lock_guard<std::mutex>lck(m_webSiteLock);
		auto it = m_mpSite.find("csdn");
		if (m_mpSite.end() != it) {
			auto & vecLinks = it->second;
			if (!vecLinks.empty()) {
				auto first = vecLinks.begin();
				info = *first;
				vecLinks.erase(first);
				bResult = true;
			}
		}
	}
	return bResult;
}

bool CMapMng::getWeblinks(std::shared_ptr<HTML_INFO> & info)
{
    bool bResult = false;
    updateFreshLinks();
	if (getCsdn(info)) {
		return true;
	}
    {
        std::lock_guard<std::mutex>lck(m_webSiteLock);
        if (!m_mpSite.empty()) {
            m_sitePos++;
            int size = static_cast<int>(m_mpSite.size());
            if (m_sitePos >= size) {
                m_sitePos = 0;
            }
            int i = 0;
            auto it = m_mpSite.begin();
            for (; it != m_mpSite.end();) {
                if (i == m_sitePos) {
                    auto & vecLinks = it->second;
                    if (!vecLinks.empty()) {
                        auto first = vecLinks.begin();
                        info = *first;
                        vecLinks.erase(first);
                        bResult = true;
                    }
                    if (vecLinks.empty()) {
                        m_mpSite.erase(it++);
                    }                   
                    break;
                }
                else {
                    ++it;
                }
                i++;
            }
            if (bResult && m_mpSite.empty()) {
                std::unordered_map<string, std::vector<std::shared_ptr<HTML_INFO>>>().swap(m_mpSite);
            }
        }
    }
    if (!bResult) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
        LOG_ERROR("no url to get!!!");
    }
    return bResult;
}

void CMapMng::putWeblinks(std::vector<std::shared_ptr<HTML_INFO>> & vecLinks)
{ 
    CLinksMng links;
    for (auto & item : vecLinks) {
        links.reset();
        links.parseUrl(item->url);
        string firstName = links.getFirstDomain();
        if (firstName.empty()) {
            firstName = "no_first_name";
            LOG_ERROR("%s", firstName.c_str());
        }
        std::lock_guard<std::mutex>lck(m_webSiteLock);
        auto it = m_mpSite.find(firstName);
        if (m_mpSite.end() == it) {
            m_mpSite[firstName] = std::vector<std::shared_ptr<HTML_INFO>>();
			m_mpSite[firstName].clear();
        }
		std::size_t queueLen = 100;
		if ("csdn" == firstName) {
			queueLen = 1000;
		}
		else if ("cppblog" == firstName) {
			queueLen = 500;
		}
		else if ("cnblogs" == firstName) {
			queueLen = 500;
		}
		else if ("iteye" == firstName) {
			queueLen = 50;
		}
		//cout << "--------------------"<< firstName << endl;
        if (m_mpSite[firstName].size() < queueLen) {
            m_mpSite[firstName].push_back(item);
        }
    }
    return;
}

void CMapMng::handleLinks(std::shared_ptr<LINKS_INFO> & links)
{		
	std::set<string> outLinks;
	std::set<string> inLinks;
	auto hrefMng = std::make_shared<CLinksMng>();
	
	LOG_INFO("hrefLinks(%d) firstname(%s) domain(%s) location(%s) url(%s) weight(%d)", 
		links->hrefLinks.size(),
		links->downladInfo.firstName.c_str(),
		links->downladInfo.domainName.c_str(),
		links->downladInfo.location.c_str(),
		links->downladInfo.url.c_str(),
		links->downladInfo.weight );

	string strTemp;
	string url;
	string fullDomain;
	string location;
	
	for (auto & item : links->hrefLinks) {
		hrefMng->reset();
		if (this->m_stop) {
			break;
		}
		if ("/" == item)  {		
			continue;
		}
	
		string old_url = item;
		url = item;				
		url = hrefMng->parseUrl(url, links->downladInfo.domainName);
		if (url.empty())
		{
			//LOG_ERROR("except-->%s location(%s) (%s)", links->downladInfo.domainName.c_str(), links->downladInfo.location.c_str(), old_url.c_str());
			continue;
		}
		
		string firstName = hrefMng->getFirstDomain();		
		if (firstName == links->downladInfo.firstName) {
			if (hrefMng->canVisit()) {
				inLinks.insert(url);
			}
			else {
				LOG_INFO("not html -->(%s)", old_url.c_str());
			}
			continue;
		}		
		else if (firstName.empty()) {	
			
			hrefMng->reset();			
			strTemp = hrefMng->combineUri(links->downladInfo.domainName, old_url);	
			
			if (!strTemp.empty()) {
				//LOG_INFO("%s", strTemp.c_str());
				inLinks.insert(strTemp);				
			}
			else {
				LOG_INFO("empty-->%s locate(%s) (%s)", firstName.c_str(), links->downladInfo.location.c_str(), old_url.c_str());
			}
		}
		else {			
			inLinks.insert(url);
			//LOG_INFO("outLinks-->%s locate(%s) (%s)", firstName.c_str(), links->downladInfo.location.c_str(), old_url.c_str());
		}		
	}
		
		
	HTML_INFO htmlInfo;	
	htmlInfo.downloadTime = 0;
	auto clsInfo = std::make_shared<CLASS_INFO>();
	vector<std::shared_ptr<HTML_INFO>> vecHtmlLinks;
	string strWebSite;
	auto handleJson = std::make_shared<CHandleJson>();
	UMAP_TYPE mpInfo;
    auto exceptCheck(std::make_unique<CExceptMng>());
    string execptDomain;	
	for (auto & item : inLinks) {
		
		if (m_stop) {
			return;
		}

		clsInfo->reset();
		hrefMng->reset();
		hrefMng->parseUrl(item, links->downladInfo.domainName);	
        execptDomain = hrefMng->getFullDomain();
        bool bExcept = exceptCheck->checkExcept(execptDomain, item);
		string fristDomain = hrefMng->getFirstDomain();
		if ((fristDomain == links->downladInfo.firstName) 
            && (!existUrlInDb(item)) && (!bExcept)			
            )
		{	
			htmlInfo.domainName = hrefMng->getFullDomain();
			htmlInfo.firstName = hrefMng->getFirstDomain();
			htmlInfo.location = hrefMng->getLocation();
			htmlInfo.url = item;		
		
            if (CWeightMngS->get(item, *clsInfo)) {
                //LOG_WARNING("(%s) url(%s)",clsInfo->classValue.c_str(), item.c_str());

                htmlInfo.className = clsInfo->className;
                htmlInfo.classValue = clsInfo->classValue;
                htmlInfo.weight = clsInfo->weight;

                htmlInfo.bBuss = links->downladInfo.bBuss;
                if ("http://" == htmlInfo.location) {
                    htmlInfo.location += htmlInfo.domainName;
                } 
                mpInfo[item] = handleJson->buildHtmlJson(htmlInfo);
                auto linkInfo = std::make_shared<HTML_INFO>();
                *linkInfo = htmlInfo;
                vecHtmlLinks.push_back(linkInfo);
            }
            else {
              // LOG_WARNING("%s", item.c_str());
            }
		}
		else {
			//LOG_INFO("%s", item.c_str());
		}
	}

	if (!vecHtmlLinks.empty()) {
		CStoreMngS->putDb(FRESH_URL_DB, mpInfo);      
        putWeblinks(vecHtmlLinks);
	}

	//LOG_INFO("ok to download (%s) inLinks(%d)",links->downladInfo.url.c_str(), vecHtmlLinks.size());
	
}

bool CMapMng::existUrlInDb(const string & url)
{
	bool bResult = false;
	string strValue;
	bResult = CStoreMngS->getDb(VISIT_DB, url, strValue);	
	if (!bResult) {
		bResult = CStoreMngS->getDb(FRESH_URL_DB, url, strValue);
	}
	if (bResult) {
		time_t now = time(nullptr);
		auto handleJson(std::make_unique<CHandleJson>());
		auto htmlInfo(std::make_unique<HTML_INFO>());		
		if (handleJson->getHtmlByJson(*htmlInfo, strValue)) {
			int diff = difftime(now, htmlInfo->downloadTime);
			if (diff >= MAX_TIME_DOWNLOAD) {
				bResult = false; //一天时间86400
			}
			//LOG_INFO("diff(%lf) now(%d) build_time(%d)", diff, now, htmlInfo->downloadTime);
		}
	}
	return bResult;
}


void CMapMng::inputQueryCache(string & uri, string & strInfo)
{	
	std::lock_guard<std::mutex> lck(m_resultCacheLock);
	auto it = m_mpResultCache.find(uri); 
	if (m_mpResultCache.end() == it) {
		m_mpResultCache[uri] = std::make_shared<RESULT_CACHE_INFO>();			
	}
	m_mpResultCache[uri]->timer.reset();
	m_mpResultCache[uri]->queryResult = strInfo;
	return;
}

bool CMapMng::getQueryCache(string & uri, string & strInfo)
{
	bool bResult = false;
	{
		std::lock_guard<std::mutex> lck(m_resultCacheLock);
		auto it = m_mpResultCache.find(uri);
		if (m_mpResultCache.end() != it) {
			it->second->timer.reset();
			strInfo = it->second->queryResult;
			bResult = true;
		}
	}
	return bResult;
}

void CMapMng::handleTimeOver()
{
	bool bErase = false;
	std::lock_guard<std::mutex> lck(m_resultCacheLock);
	auto it = m_mpResultCache.begin();
	for (; it != m_mpResultCache.end();) {
		if (it->second->timer.elapsed_seconds() >= 5) {
			m_mpResultCache.erase(it++);
			bErase = true;
		}
		else {
			++it;
		}
	}
	if (bErase) {
		if (m_mpResultCache.empty()) {
			std::map<string, std::shared_ptr<RESULT_CACHE_INFO>>().swap(m_mpResultCache);
		}
	}
	return;
}