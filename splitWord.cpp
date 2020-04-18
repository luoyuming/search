#include "splitWord.h"
#include "util.h"
#include "storeMng.h"
#include "handleXml.h"
#include "log.h"
#include "weight.h"
#include "needMng.h"
#include "mapMng.h"
#include "escapeSequence.h"
#include "wordMng.h"
#include "bookMng.h"
#include "bussMng.h"

CSplitWord::CSplitWord()
{
	m_userDict = std::make_shared<CKeywordDict>();
}


void CSplitWord::Quit()
{
	
}

bool CSplitWord::init()
{	
	bool bResult = false;
	string DICT_PATH = "./dict/jieba.dict.utf8";
	string HMM_PATH = "./dict/hmm_model.utf8";
	string USER_DICT_PATH = "./dict/user.dict.utf8";
	string IDF_PATH = "./dict/idf.utf8";
	string STOP_WORD_PATH = "./dict/stop_words.utf8";	

	m_jieba = std::make_shared<cppjieba::Jieba>(DICT_PATH,
		HMM_PATH,
		USER_DICT_PATH,
		IDF_PATH,
		STOP_WORD_PATH);
	
	string strPath = CHandleXmlS->getPath();
	LOG_INFO("%s", strPath.c_str());	
	bResult = CEscapeSequenceS->init(strPath);

	if (!CHandleXmlS->onlySearchMode()) {
		m_userDict->initUserDict(strPath);
		if (bResult)
			bResult = CWeightMngS->init(strPath);
		if (bResult)
			bResult = CNeedMngS->init(strPath);
		if (bResult)
			bResult = CWordMngS->init(strPath);
		if (bResult)
			bResult = CBookMngS->init(strPath);
		if (bResult)
			bResult = CBussMngS->init(strPath);
	}	
	return bResult;
}

bool CSplitWord::existStopWord(string & word)
{
	bool bResult = false;	
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto & stopWord = m_jieba->extractor.stopWords_;
		auto it = stopWord.find(word);
		if (stopWord.end() != it) {
			bResult = true;			
		}

	}
	
	return bResult;
}

void CSplitWord::eraseStopWord(vector<cppjieba::Word> & vecWord)
{
	auto it = vecWord.begin();
	for (; it != vecWord.end(); ) {
		if (existStopWord(it->word)) {
			it = vecWord.erase(it);
		}
		else {
			++it;
		}
	}
}

void CSplitWord::eraseStopWord(vector<string> & vecWord)
{
	auto it = vecWord.begin();
	for (; it != vecWord.end(); ) {
		if (existStopWord(*it)) {
			it = vecWord.erase(it);
		}
		else {
			++it;
		}
	}
}

void CSplitWord::getKeyWord(vector<string> & vecKeyword)
{
	vector<string> vecData = vecKeyword;
	vecKeyword.clear();
	for (auto & strData : vecData) {
		vector<cppjieba::Word>  userWords;
		m_userDict->getUserKeyword(userWords, strData);

		vector<string> words;
		{
			std::lock_guard<std::mutex>lck(m_lock);
			m_jieba->Cut(strData, words);
		}
		eraseStopWord(words);

		for (auto & item : userWords) {
			vecKeyword.push_back(item.word);
		}
		std::copy(words.begin(), words.end(), std::back_inserter(vecKeyword));
	}
	return;
}

void CSplitWord::uniqueElement(vector<cppjieba::Word> & vecWord)
{
	std::set<string> mpTemp;
	auto it = vecWord.begin();
	for (; it != vecWord.end(); ) {
		auto ifFind = mpTemp.find(it->word);
		if (mpTemp.end() == ifFind) {
			mpTemp.insert(it->word);
			++it;
		}
		else {
			it = vecWord.erase(it);
		}
	}
	return;
}

void CSplitWord::eraseMorePad(string & strInfo)
{
	string strTemp = strInfo;
	string pattern = "-";
	vector<string> vecInfo;
	UTIL_SELF::split(strTemp, vecInfo, pattern);
	int size = static_cast<int>(vecInfo.size());
	if (vecInfo.size() <= 1)
	{
		return;
	}

	strTemp.clear();
	for (int i = 0; i < (size - 1); i++) {
		if (strTemp.empty()) {
			strTemp = vecInfo[i];
		}
		else {
			strTemp += " ";
			strTemp += vecInfo[i];
		}
	}
	

	vecInfo.clear();	
	pattern = "_";
	UTIL_SELF::split(strTemp, vecInfo, pattern);
	if (vecInfo.size() <= 1)
	{
		strInfo = strTemp;
		return;
	}
	
	size = static_cast<int>(vecInfo.size());
	strTemp.clear();
	for (int i = 0; i < (size - 1); i++) {
		if (strTemp.empty()) {
			strTemp = vecInfo[i];
		}
		else {
			strTemp += " ";
			strTemp += vecInfo[i];
		}
	}
	strInfo = strTemp;
	
	return;
}

void CSplitWord::parseTitleKeyWord(map<string, uint32_t> & mpKeyord, const string & strData, int weight, string & url)
{	
	LOG_INFO("(weight=%d)(title:%s)(%s)", weight, strData.c_str(), url.c_str());
	string strTemp = strData;
	
	vector<cppjieba::Word>  jiebawords;	
	{
		std::lock_guard<std::mutex>lck(m_lock);
		m_jieba->CutForSearch(strTemp, jiebawords);
	}

	uniqueElement(jiebawords);
	for (auto & item : jiebawords) {
		string & key = item.word;	
		UTIL_SELF::trim(key);
		if (!UTIL_SELF::is_chinese(key)) {
			transform(key.begin(), key.end(), key.begin(), (int(*)(int))tolower);
		}	
		if (!CWordMngS->existErase(item.word) &&  (!key.empty()) ) {
			mpKeyord[key] = (uint32_t)weight;
		}
	}	
	return;
}


void CSplitWord::parseKeyWord(multimap<string, uint32_t> & mpKeyord, const string & strData, int & weight, std::shared_ptr<HTML_INFO> & htmlInfo)
{
	if (strData.empty())
		return;

	vector<cppjieba::Word>  jiebawords;
	{
		std::lock_guard<std::mutex>lck(m_lock);		
		m_jieba->CutForSearch(strData, jiebawords);
	}	
	if (htmlInfo->bBuss) {
		
		bussWord(jiebawords, weight, htmlInfo->hotWordNum);
		for (auto & item : jiebawords) {
			mpKeyord.insert(std::make_pair(item.word, item.offset));
		}
		return;
	}	
	
	vector<cppjieba::Word>  & userWords = jiebawords;
	for (auto & item : userWords) {	
		if (!UTIL_SELF::is_chinese(item.word)) {
			transform(item.word.begin(), item.word.end(), item.word.begin(), (int(*)(int))tolower);
		}  
		if (!CWordMngS->existErase(item.word)) {			
			mpKeyord.insert(std::make_pair(item.word, item.offset));			
		}
	}	
	return;
}

void CSplitWord::bussWord(vector<cppjieba::Word> & vecWord, int & num, int hotWords)
{
	//LOG_ERROR("total(%d)", vecWord.size());
	vector<string> vecKeyword;
	auto it = vecWord.begin();
	for (; it != vecWord.end();) {
		if (CBussMngS->existWord(it->word)) {
			++it;
		}
		else {
			it = vecWord.erase(it);
		}
	}	
	for (auto & item : vecWord) {
		if (CBussMngS->keyWord(item.word)) {
			vecKeyword.push_back(item.word);
		}
	}

	//test(vecWord);
	num = static_cast<int>(vecKeyword.size());
	//LOG_INFO("(%d) (%d)", num, hotWords);
	if (num < hotWords) {
		vecWord.clear();
	}
	
	
	return;
}

void CSplitWord::test(vector<cppjieba::Word> & vecWord)
{
	std::set<string> setTemp;
	for (auto & item : vecWord) {
		setTemp.insert(item.word);
	}
	string strData;
	/*for (auto & item : setTemp) {
	if (strData.empty()) {
	strData = item;
	}
	else {
	strData += "\n";
	strData += item;
	}
	}*/
	for (auto & item : setTemp) {
		if (strData.empty()) {
			strData = "/";
			strData += item;
			strData += "/";
		}
		else {
			strData += item;
			strData += "/";
		}
	}
	cout << strData << endl;
	LOG_ERROR("total(%d) (%d)", vecWord.size(), setTemp.size());

	//string file = "word_sample.utf8";
	//UTIL_SELF::saveFile(file, strData);

}
