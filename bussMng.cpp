#include "bussMng.h"
#include "log.h"
#include "util.h"
#include "linksMng.h"

CBussMng::CBussMng()
{

}


void CBussMng::Quit()
{

}

bool CBussMng::init(string & strPath)
{
	if (!wordWebsite(strPath)) {
		return false;
	}
	if (!wordWeight(strPath)) {
		return false;
	}	
	if (!wordQuery(strPath)) {
		return false;
	}
	//combineKeyword(strPath);	
	return true;
}

bool CBussMng::getBussNeedUrl(HTML_INFO & info)
{
	bool bResult = false;
	std::lock_guard<std::mutex>lck(m_lock);
	if (!m_liUrlInfo.empty()) {
		info = m_liUrlInfo.front();
		m_liUrlInfo.pop_front();
		bResult = true;
	}	
	return bResult;
}

bool CBussMng::wordWebsite(string & strPath)
{
	string FILE_NAME = "dict/website_focus.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}
	
    vector<HTML_INFO> vecSite;
	std::set<string> setWebSite;
	auto links(std::make_unique<CLinksMng>()) ;
	string strUrl;	
	while (ifs >> strUrl) {
		UTIL_SELF::trimstr(strUrl);
		auto it = setWebSite.find(strUrl);
		if (setWebSite.end() == it) {
			setWebSite.insert(strUrl);
			links->reset();
			links->parseUrl(strUrl);
			HTML_INFO info;
			info.bBuss = true;
			info.url = strUrl;
			info.domainName = links->getFullDomain();
			info.firstName = links->getFirstDomain();
			info.location = links->getLocation();
            vecSite.push_back(info);			
		}
	}	
    std::random_shuffle(vecSite.begin(), vecSite.end());
    for (auto & info : vecSite)
    {
		//LOG_INFO("(%s)(%s)", info.url.c_str(), strUrl.c_str());
        std::lock_guard<std::mutex>lck(m_lock);
        m_liUrlInfo.push_back(info);
    }	
	return true;
}

bool CBussMng::wordQuery(string & strPath)
{
	string FILE_NAME = "dict/word_query.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}

	string strWord;
	while (ifs >> strWord) {
		UTIL_SELF::trimstr(strWord);
		//LOG_INFO("(%s)", strWord.c_str());
		{
			std::lock_guard<std::mutex>lck(m_lock);
			m_setQueryword.insert(strWord);
		}
	}

	//LOG_INFO("(%d)", m_setKeyword.size());

	return true;
}

bool  CBussMng::existWord(string & word)
{
	bool bResult = false;
	std::lock_guard<std::mutex>lck(m_lock);
	auto it = m_setQueryword.find(word);
	if (m_setQueryword.end() != it) {
		bResult = true;
	}
	return bResult;
}

bool CBussMng::wordWeight(string & strPath)
{
	string FILE_NAME = "dict/word_focus.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}

	string strWord;	
	while (ifs >> strWord) {
		UTIL_SELF::trimstr(strWord);
		//LOG_INFO("(%s)", strWord.c_str());
		{
			std::lock_guard<std::mutex>lck(m_lock);
			m_setKeyword.insert(strWord);
		}
	}

	//LOG_INFO("(%d)", m_setKeyword.size());

	return true;
}

bool  CBussMng::keyWord(string & word)
{
	bool bResult = false;
	std::lock_guard<std::mutex>lck(m_lock);
	auto it = m_setKeyword.find(word);
	if (m_setKeyword.end() != it) {
		bResult = true;
	}
	return bResult;
}



void CBussMng::combineKeyword(string & strPath)
{
	string FILE_NAME = "dict/word_sample.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return;
	}

	std::set<string> setTemp;
	string strWord;
	while (ifs >> strWord) {
		UTIL_SELF::trimstr(strWord);		
		if (!UTIL_SELF::is_allNum(strWord))
		{
			LOG_INFO("(%s)", strWord.c_str());
			setTemp.insert(strWord);
		}
	}

	string strData;
	for (auto & item : setTemp) {
		if (strData.empty()) {
			strData = item;
		}
		else {
			strData += "\n";
			strData += item;
		}
	}
	string file = "word_test.utf8";
	UTIL_SELF::saveFile(file, strData);
}