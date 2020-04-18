#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"

enum class WEIGHT_HTML {	
	WEIGHT_TXT = 1,
};



class CWeightMng : public SingletionEX<CWeightMng> {
	SINGLETON_INIT_EX(CWeightMng);
	CWeightMng();
public:
	bool init(string & strPath);
	void put(string & domain, CLASS_INFO & info);
	bool get(const string & url, CLASS_INFO & info);
	bool exist(const string & url);
private:
	std::mutex  m_lock;
	map<string, CLASS_INFO>  m_mpDomanScore;
	
};


#define CWeightMngS  CWeightMng::getInstance()