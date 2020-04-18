#pragma once
#include "common.h"
#include "msgDef.h"
#include "epollMng.h"
#include "utf8String.h"


static const string DEFAULT_WWW = "web";

class CDispatch {

	typedef std::function<void(PACKAGE_INFO & info)> CallbackFun;
	typedef struct _Command {
		int              commandID;
		CallbackFun      callbackFun;
	} Command;
	typedef struct _UrlMothed {
		string           commandID;
		CallbackFun      callbackFun;
	} UrlMothed;

	
public:
	CDispatch();
	~CDispatch();
	void handleInfo(PACKAGE_INFO & info);
private:
	void initFun();
	void postMothed(PACKAGE_INFO & info);
	void getMothed(PACKAGE_INFO & info);
	void disptMothed(std::map<string, UrlMothed> & dispFun, PACKAGE_INFO & info);
	bool httpFormData(PACKAGE_INFO & info);

	bool getFormDataName(string & strName, string & strInfo);
	bool getFormFileName(string & strName, string & strInfo);
	string::size_type getFormData(FORM_DATA_INFO & formData,
		string & strInfo, string::size_type pos, string & strBoundary);
	void handleFormData(PACKAGE_INFO & info, string & strBoundary);
	void erasePadField(string & info);
	bool getHeadField(string & strSub, const string & strInfo, std::vector<string> & vecType);
	bool getSubValue(string & strSub, const string & strInfo, string begin, string end);
	bool checkCanGzip(string & extName);

	bool handlePhP(PACKAGE_INFO & info);
protected:
	void httpVersion(PACKAGE_INFO & info);
	void httpIndex(PACKAGE_INFO & info);
	void httpGetEmpty(PACKAGE_INFO & info);
	void httpGeS(PACKAGE_INFO & info);
	void httpGetSnapshot(PACKAGE_INFO & info);
	void httpGetPageUrl(PACKAGE_INFO & info);

	void httpQuery(PACKAGE_INFO & info);
	void httpVisitIP(PACKAGE_INFO & info);
private:
    void replaceTemplate(string & strInfo,string & url);
	void handleSearch(PACKAGE_INFO & info, string & strKeyword);
	void uniqueElement(vector<string> & vecWord);
	string getWebsite(std::unordered_map<string, string> & Field);
	void eraseMore(string & strInfo);
private:
	std::map<int, Command>				m_FunHttpMothed;
	std::map<string, UrlMothed>			m_FunHttpGet;
	std::map<string, UrlMothed>			m_FunHttpPost;
	std::set<string>						m_gzExt;
	std::map<string, RESP_CONTENT_TYPE>		m_mpExt2RespType;
	std::set<string>						m_setExcept; 
    std::set<string>						m_setFlag;
	std::set<string>						m_setDomain;
};