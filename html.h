#pragma once
#include "common.h"
#include "gumbo.h"
#include "splitWord.h"
#include "iconvCode.h"
#include "handleJson.h"
#include "codeInfo.h"
#include "escapeSequence.h"

class CHtml {
public:
	CHtml();
	~CHtml();

	bool parseWord(string & strData, std::shared_ptr<HTML_INFO> & htmlInfo);
	void reset();
	std::shared_ptr<LINKS_INFO> getLinks();

	void test();
	void charset2utf8(string & strData, std::shared_ptr<HTML_INFO> & htmlInfo);
	std::string cleantext(GumboNode* node);
private:
	
	bool getValue(string & ret, string & strData, string flag);	
	void getCharset();
	void getTitleAndHead(const GumboNode* root);
	void getCleantext(GumboNode* node);
	void searchForClass(GumboNode* node, const std::string& original_text,
		const char* cls_name, const char* cls_value);
	
	bool getCharsetByHead(string & charset, string & strHead);
	
	void splitKeyword(map<string, vector<std::shared_ptr<TERM_INFO>>> & termInfo,
		int size, const string & strInfo, std::shared_ptr<HTML_INFO> & htmlInfo);
	void splitTxt(map<string, vector<std::shared_ptr<TERM_INFO>>> & termInfo, std::shared_ptr<HTML_INFO> & htmlInfo);
	string getAttibuteValue(string & strData, string bFlag, string mFlag);
	bool eraseBlockByFlag(string & strData, string bFlag, string eFlag);
	void search_for_links(GumboNode* node, string & url);
	bool isErrorLink(string & strLink);
	bool checkExtName(string strLink);	
	void eraseOldWord(vector<string> & vecOldWord, std::shared_ptr<HTML_INFO> & htmlInfo);
	void prepairTitle(string & strInfo);
	void iteyeTitle(string & strInfo);
	void blogCsdn(string & strInfo);	
	void askCsdn(string & strInfo);
	void titleMore(string & strInfo, vector<string> & vecFlag);
private:
	std::map<string, conv_type>  m_mpConv;
	GumboOutput* m_pOutput;
	GumboOptions m_options;
	string m_url;
	string m_DomainName;;
	string	m_charset;
	string  m_title;
	string  m_viewTitle;
	std::list<string>  m_liHead;
	std::list<string>  m_liText;
	std::set<string>  m_setLinks;
	std::set<string>  m_setExtName;
	std::set<string>  m_sesInvalidUrl;
	string m_document; //´¿ÎÄ±¾ÄÚÈÝ
	int m_len_offset;	
	std::shared_ptr<LINKS_INFO> m_links;
	std::list<string>  m_liInvalidLinks;
	std::shared_ptr<CCodeInfo> base64;
};