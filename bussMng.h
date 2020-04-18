#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"

class CBussMng : public SingletionEX<CBussMng> {
	SINGLETON_INIT_EX(CBussMng);
	CBussMng();
public:
	bool init(string & strPath);
	bool existWord(string & word);
	bool keyWord(string & word);
	bool getBussNeedUrl(HTML_INFO & info);
private:
	bool wordWeight(string & strPath);
	bool wordQuery(string & strPath);
	bool wordWebsite(string & strPath);
	void combineKeyword(string & strPath);
private:
	std::mutex  m_lock;	
	std::set<string> m_setKeyword;
	std::set<string> m_setQueryword;	
	std::list<HTML_INFO>  m_liUrlInfo;
};
#define CBussMngS  CBussMng::getInstance()