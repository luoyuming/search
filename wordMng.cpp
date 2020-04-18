#include "wordMng.h"
#include "log.h"
#include "util.h"


CWordMng::CWordMng()
{

}


void CWordMng::Quit()
{

}

bool CWordMng::initErase(string & strPath)
{
	string FILE_NAME = "dict/wordErase.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}

	string strWord;
	while (ifs >> strWord) {		
		{
			std::lock_guard<std::mutex>lck(m_lock);
			m_setErase.insert(strWord);
		}
	}

	return true;
}

bool CWordMng::init(string & strPath)
{
	string FILE_NAME = "dict/wordMng.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}

	string strWord;
	while (ifs >> strWord) {
		UTIL_SELF::trimstr(strWord);
		//LOG_INFO("--%s---", strWord.c_str());
		if (!strWord.empty())
		{
			std::lock_guard<std::mutex>lck(m_lock);
			m_setTerm.insert(strWord);
		}
	}

	return initErase(strPath);
}

bool CWordMng::existErase(const string & word)
{
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto it = m_setErase.find(word);
		if (m_setErase.end() != it) {
			return true;
		}
	}
	return false;
}

bool CWordMng::existWord(const string & word)
{
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto it = m_setTerm.find(word);
		if (m_setTerm.end() != it) {
			return true;
		}
	}
	return false;
}