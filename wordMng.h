#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"



class CWordMng : public SingletionEX<CWordMng> {
	SINGLETON_INIT_EX(CWordMng);
	CWordMng();
public:
	bool init(string & strPath);
	bool existWord(const string & word);
	bool existErase(const string & word);
private:
	bool initErase(string & strPath);
private:
	std::mutex  m_lock;
	std::set<string> m_setTerm;
	std::set<string> m_setErase;
		
};
#define CWordMngS  CWordMng::getInstance()