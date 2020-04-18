#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"


class CEscapeSequence: public SingletionEX<CEscapeSequence>
{
	SINGLETON_INIT_EX(CEscapeSequence);

	CEscapeSequence();
public:
	bool init(string & strPath);
	void unquoteHtml(string & strInfo);
	void quoteHtml(string & strInfo);
private:
	void utf8Str(vector<string> & vecInfo, string & word);
	void getQuoteInfo(std::map<string, string> & mpInfo);
	void getEntityInfo(std::map<string, string> & mpInfo);
private:
	string begin_flag;
	string end_flag;
	string decimal_flag;

	std::mutex	m_lock;
	std::map<string, string> mpEntity; //&Delta;&#916;--->жд
	std::map<string, string> mpQuote; //жд--->&Delta;&#916;
};

#define CEscapeSequenceS  CEscapeSequence::getInstance()