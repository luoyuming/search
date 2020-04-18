#pragma once
#include "singleton.h"
#include "common.h"
#include "Jieba.hpp"
#include "weight.h"
#include "msgDef.h"
#include "keywordDict.h"



class CSplitWord : public SingletionEX<CSplitWord> {
	SINGLETON_INIT_EX(CSplitWord);
	CSplitWord();
public:
	bool init();
	void parseKeyWord(multimap<string, uint32_t> & mpKeyord, const string & strData, int & weight, std::shared_ptr<HTML_INFO> & htmlInfo);
	void parseTitleKeyWord(map<string, uint32_t> & mpKeyord, const string & strData,int weight, string & url);
	void getKeyWord(vector<string> & vecKeyword);
	
private:
	void test(vector<cppjieba::Word> & vecWord);
	void eraseMorePad(string & strInfo);
	void uniqueElement(vector<cppjieba::Word> & vecWord);
	void eraseStopWord(vector<cppjieba::Word> & vecWord);
	void eraseStopWord(vector<string> & vecWord);
	bool existStopWord(string & word);
	void bussWord(vector<cppjieba::Word> & vecWord, int & num, int hotWords);
private:
	std::shared_ptr<CKeywordDict>  m_userDict;
	std::shared_ptr<cppjieba::Jieba>  m_jieba;
	std::mutex	m_lock;
};

#define CSplitWordS  CSplitWord::getInstance()

