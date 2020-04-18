#pragma once
#include "singleton.h"
#include "common.h"
#include "msgDef.h"
#include "linksMng.h"

struct CookieValue {
	string key;
	string value;
	string path;
	CookieValue() {
		path = "/";
	}
};

class CCookieMng : public SingletionEX<CCookieMng> {
	SINGLETON_INIT_EX(CCookieMng);
	CCookieMng();
public:
	void extractCookie(const string & url, string & strData);
    bool getCookie(const string & url, string & cookie);
private:
	void setCookie(string & strDomain, shared_ptr<CookieValue> & cookie);
private:
	


	std::mutex	m_lock;
	//domain->cookie-pair
	std::map<string, std::set<std::pair<string, string>>> m_mpCookie;
};


#define CCookieMngS  CCookieMng::getInstance()