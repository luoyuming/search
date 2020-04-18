#include "escapeSequence.h"
#include "log.h"
#include "util.h"


CEscapeSequence::CEscapeSequence()
{
	begin_flag = "&";
	end_flag = ";";
	decimal_flag = "#";
	mpEntity.insert(std::make_pair("&nbsp;"," "));
	mpEntity.insert(std::make_pair("&#160;", " "));
	mpEntity.insert(std::make_pair("&lt;", "<"));
	mpEntity.insert(std::make_pair("&#60;", "<"));
	mpEntity.insert(std::make_pair("&gt;", ">"));
	mpEntity.insert(std::make_pair("&#62;", ">"));
	mpEntity.insert(std::make_pair("&amp;", "&"));
	mpEntity.insert(std::make_pair("&#38;", "&"));
	mpEntity.insert(std::make_pair("&quot;", "\""));
	mpEntity.insert(std::make_pair("&#34;", "\""));
	mpEntity.insert(std::make_pair("&apos;", "\'"));
	mpEntity.insert(std::make_pair("&#39;", "\'"));

	for (auto & item : mpEntity) {
		mpQuote[item.second] = item.first;
	}
}

void CEscapeSequence::Quit()
{

}

bool CEscapeSequence::init(string & strPath)
{
	/*string FILE_NAME = "dict/escapeHtml.utf8";
	string strFilename = strPath + FILE_NAME;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}

	string symbol;
	string strOne;
	string strTwo;
	while (ifs >> symbol >> strOne >> strTwo) {
		UTIL_SELF::trim(symbol);
		UTIL_SELF::trim(strOne);
		UTIL_SELF::trim(strTwo);
		LOG_INFO("((%d)%s)--%s--%s",symbol.size(), symbol.c_str(), strOne.c_str(), strTwo.c_str());
		{
			std::lock_guard<std::mutex>lck(m_lock);
			mpEntity.insert(std::make_pair(strOne, symbol));
			mpEntity.insert(std::make_pair(strTwo, symbol));
		}
	}*/	
	return true;
}

void CEscapeSequence::utf8Str(vector<string> & vecInfo, string & word)
{
	vecInfo.clear();
	int num = word.size();
	int i = 0;
	int size = 0;
	while (i < num)
	{
		size = 1;
		if (word[i] & 0x80)
		{
			char temp = word[i];
			temp <<= 1;
			do {
				temp <<= 1;
				++size;
			} while (temp & 0x80);
		}		
		vecInfo.push_back(word.substr(i, size));
		i += size;
	}
	return;
}

void CEscapeSequence::quoteHtml(string & strInfo)
{
	if (strInfo.empty()) {
		return;
	}

	bool bFind = false;
	std::map<string, string> mpSeq;
	getQuoteInfo(mpSeq);
	vector<string> vecInfo;
	utf8Str(vecInfo, strInfo);
	for (auto & str : vecInfo) {
		auto it = mpSeq.find(str);
		if (mpSeq.end() != it) {
			str = it->second;
			bFind = true;
		}
	}
	if (bFind) {
		UTIL_SELF::joinStr(strInfo, vecInfo);
	}

	return;
}

void CEscapeSequence::getQuoteInfo(std::map<string, string> & mpInfo)
{	
	{
		std::lock_guard<std::mutex>lck(m_lock);
		mpInfo = mpQuote;
	}
	return;
}

void CEscapeSequence::getEntityInfo(std::map<string, string> & mpInfo)
{
	{
		std::lock_guard<std::mutex>lck(m_lock);
		mpInfo = mpEntity;
	}
	return;
}

void CEscapeSequence::unquoteHtml(string & strInfo)
{
	if (strInfo.empty())
		return;
	std::map<string, string> mpSeq;
	getEntityInfo(mpSeq);

	string::size_type size = strInfo.size();
	string::size_type pos = 0;
	string::size_type next = 0;
	bool bFind = false;
	string strTemp;
	for (;;) {
		next = strInfo.find(begin_flag, pos);
		if (string::npos == next) {
			//LOG_ERROR("1");
			break;
		}		
		pos = next;
		next = strInfo.find(end_flag, pos);
		if (string::npos == next) {
			//LOG_ERROR("2");
			break;
		}
		next += end_flag.size();		
		strTemp = strInfo.substr(pos, next - pos);

		LOG_INFO("%s", strTemp.c_str());

		bFind = false;			
		auto it = mpSeq.find(strTemp);
		if (mpSeq.end() != it) {
			strTemp = it->second;
			bFind = true;
		}
		if (bFind) {
			strInfo.replace(pos, next - pos, strTemp);
		}
		else {
			pos = next;
			LOG_ERROR("not to find the escape (%s)", strTemp.c_str());
		}
		
	}
	return;
}