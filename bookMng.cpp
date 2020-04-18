#include "bookMng.h"
#include "log.h"
#include "util.h"


CBookMng::CBookMng()
{

}


void CBookMng::Quit()
{

}

bool CBookMng::init(string & strPath)
{
	string FILE_NAME = "dict/bookMng.utf8";
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
		{
			std::lock_guard<std::mutex>lck(m_lock);
			m_setWord.insert(strWord);
		}
	}

	return true;
}

bool CBookMng::existWord(string & word)
{
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto it = m_setWord.find(word);
		if (m_setWord.end() != it) {
			return true;
		}
	}
	return false;
}