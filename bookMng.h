#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"



class CBookMng : public SingletionEX<CBookMng> {
	SINGLETON_INIT_EX(CBookMng);
	CBookMng();
public:
	bool init(string & strPath);
	bool existWord(string & word);
private:
	std::mutex  m_lock;
	std::set<string> m_setWord;
};
#define CBookMngS  CBookMng::getInstance()