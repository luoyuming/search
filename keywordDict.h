#pragma once
#include "common.h"
#include "Jieba.hpp"

static const string FLAG_ASTERISK = "*";
//static const string FLAG_POUND = "#";
static const string FLAG_COMMA = ",";
static const string KEYWORD_DICT_FILE = "dict/kekword.dict.utf8";
class CKeywordDict {
public:
	void initUserDict(string & strCurPath);

	void getUserKeyword(vector<cppjieba::Word> & vccWord, const string & strData);
private:
	void insertDict(std::list<string> & liDict, string & word);
	void eraseFlag(string & line, string flag);
	
private:
	std::mutex         m_lock;
	std::list<string>  m_liUserDict; //全字匹配
	std::list<string>  m_liDigitDict; //*数字或带小数点
	
};