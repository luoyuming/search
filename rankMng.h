#pragma once
#include "common.h"
#include "msgDef.h"
#include "utf8String.h"
#include "epollMng.h"

static const int MAX_FIELD_LEN = 110;
class CRankMng {
		
public:	
	CRankMng(std::string & str, string & webSite);
	~CRankMng();
	void inputField(std::list<uint32_t> & liOffset, string & keyword);
	void extractField();
	void fillTemplate(string & strInfo, PACKAGE_INFO & info);
private:
	void delMoreSpace();
	void checkFrontStr();
	void quoteSeqHtml();
	void combineField();
	void strongField(string & str, const string & flag);
public:
	int	 pos;
	bool bInit;
	
	std::set<int> setQueryWord;
	std::set<string>  setKeyword;
	string url;
	string field;
	string head;
	string snapshot_url;	
	std::shared_ptr<iutf8string> utfString;
	std::set<string>  setInvalid;
};