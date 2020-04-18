#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"



static const int ERASE_IP_INFO = 2;
static const int GET_IP_INFO = 1;

class CFileMng : public SingletionEX<CFileMng> {
	SINGLETON_INIT_EX(CFileMng);
	CFileMng();
public:
	void init();
	void put(string & ip);
	void get(vector<VISIT_INFO> & vecInfo);
	void reset();
private:
	std::mutex  m_lock;
	std::map<string, std::shared_ptr<VISIT_INFO>> m_mpVisitInfo;
};
#define CFileMngS  CFileMng::getInstance()