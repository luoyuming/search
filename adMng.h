#pragma once
#include "singleton.h"
#include "common.h"
#include "util.h"
#include "msgDef.h"


class CAdMng : public SingletionEX<CAdMng> {
	SINGLETON_INIT_EX(CAdMng);
	CAdMng();
public:
	bool init(string & strPath);
	void fillAd(string & strInfo);
private:
	string createStr(string prev, string mid, string last);
private:
	std::mutex  m_lock;
	std::vector<AD_INFO> m_vecAdInfo;
};


#define CAdMngS  CAdMng::getInstance()