#pragma once
#include "common.h"
#include "uri.h"



class CLinksMng {

public:
	CLinksMng();
	string getFullDomain();
	string getFirstDomain();
	string getProtocol();
	string getLocation();
	string getUrl();
	string getPath();
	string buildMonthDay();	
	string getIndex();
	string getRelative();
	string getSubDomain(); //   www.iteye.com/blog/;
	void reset();	
	bool checkIsCN(string & strInfo);
	string combineUri(string & domain, string & url);
	string  parseUrl(const string & url, string domain = "");
	bool canVisit();
	void test();
private:
	void extractRelative();
	void extractSubDomain();
	void init();
	void extractDomain(const string & hostname_port);	
	bool existFlag(std::set<string> & setInfo, string & strFlag);
private:
	std::set<string> m_setNoVisit;
	std::set<string> m_twoFlag;
	std::set<string> m_OneFlag;
	std::set<string> m_otherFlag;	
	string protocol;
	string hostname_port;
	string path;
	string fragment;	
	string hostname;
	string port;
	string query;
	string relativePath;
	string index;
	string fullPath;
	string subDomain;
};