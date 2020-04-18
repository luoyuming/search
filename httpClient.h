#pragma once
#include "singleton.h"
#include "common.h"
#include "./curl/curl.h"

class CHttpClient : public SingletionEX<CHttpClient>
{
    SINGLETON_INIT_EX(CHttpClient);

    CHttpClient();
public:
	void init();
	bool PostEx(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
	bool getEx(string & strUrl, string & head, string & body, vector<string> & vecHttpHeader);
	void test();
private:
	bool getHttpMothed(string & strUrl, string & head, string & body, vector<string> & vecHttpHeader);
	bool httpGetEx(const string & strUrl,
		std::vector<string> & vecHttpHeader,
		string & strResponse,
		bool bReserveHeaders = true,
		bool location = true,
		const char * pCaPath = nullptr);
	int postForm(string & strUrl, string & strImage,
		string & json, string & strResponse);
    int Get(const std::string & strUrl, std::string & strResponse);
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
	void getLocation(string & strInfo, vector<string> & vecHead);
private:
	bool m_bDebug;
    //std::mutex	m_lock;
};

#define CHttpClientS  CHttpClient::getInstance()


