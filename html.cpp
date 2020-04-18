#include "html.h"
#include "log.h"
#include "util.h"
#include "mapMng.h"
#include "storeMng.h"
#include "iconvCode.h"
#include "needMng.h"
#include "md5.h"
#include "handleXml.h"
#include "wordMng.h"


CHtml::CHtml():m_options(kGumboDefaultOptions)
{	
	m_links = std::make_shared<LINKS_INFO>();

	m_pOutput = nullptr;
	m_len_offset = 0;

	m_mpConv["gbk"] = conv_type::gbk_to_utf8;
	m_mpConv["gb2312"] = conv_type::gbk_to_utf8;
	m_mpConv["gb18030"] = conv_type::gb18030_to_utf8;

	m_setExtName.insert(".gif");
	m_setExtName.insert(".jpg");
	m_setExtName.insert(".jpeg");
	m_setExtName.insert(".png");
	m_setExtName.insert(".ttf");
	m_setExtName.insert(".tif");
	m_setExtName.insert(".pdf");
	m_setExtName.insert(".doc");
	m_setExtName.insert(".docx");
	m_setExtName.insert(".zip");
	m_setExtName.insert(".iso");
	m_setExtName.insert(".rar");
	m_setExtName.insert(".tar");
	m_setExtName.insert(".gz");
	m_setExtName.insert(".bz");
	m_setExtName.insert(".tar");
	m_setExtName.insert(".war");
	m_setExtName.insert(".zip");
	m_setExtName.insert(".z");
	m_setExtName.insert(".arj");
	m_setExtName.insert(".jar");
	m_setExtName.insert(".war");
	m_setExtName.insert(".mmf");
	m_setExtName.insert(".gzip");
	m_setExtName.insert(".bmp");
	m_setExtName.insert(".xlsx");
	m_setExtName.insert(".xls");
	m_setExtName.insert(".txt");
	m_setExtName.insert(".ppt");
	m_setExtName.insert(".pptx");
	m_setExtName.insert(".apk");	
	m_setExtName.insert(".exe");
	m_setExtName.insert(".dll");
	m_setExtName.insert(".lib");
	m_setExtName.insert(".a");
	m_setExtName.insert(".so");
	m_setExtName.insert(".bin");
	m_setExtName.insert(".wps");
	m_setExtName.insert(".ddb");
	m_setExtName.insert(".dwg");
	m_setExtName.insert(".swf");
	m_setExtName.insert(".mp3");
	m_setExtName.insert(".ram");
	m_setExtName.insert(".aac");
	m_setExtName.insert(".flac");
	m_setExtName.insert(".wma");
	m_setExtName.insert(".mp4");
	m_setExtName.insert(".avi");
	m_setExtName.insert(".swf");
	m_setExtName.insert(".dat");
	m_setExtName.insert(".mpeg");
	m_setExtName.insert(".mpg");
	m_setExtName.insert(".mov");
	m_setExtName.insert(".rmvb");
	m_setExtName.insert(".rm");
	m_setExtName.insert(".3gp");
	m_setExtName.insert(".flv");
	m_setExtName.insert(".ts");
	m_setExtName.insert(".asf");
	m_setExtName.insert(".wav");
	m_setExtName.insert(".au");
	m_setExtName.insert(".vob");
	m_setExtName.insert(".mid");
	m_setExtName.insert(".mdf");	
	m_setExtName.insert(".bak");
	m_setExtName.insert(".css");
	m_setExtName.insert(".js");
	m_setExtName.insert(".c");
	m_setExtName.insert(".java");

	m_liInvalidLinks.push_back("javascrip:");
	m_liInvalidLinks.push_back("javascript:");
	m_liInvalidLinks.push_back("vbscript:");
	m_liInvalidLinks.push_back("tel:");
	m_liInvalidLinks.push_back("mailto:");
	m_liInvalidLinks.push_back("javaacript:void(0)");
	
	base64 = std::make_shared<CCodeInfo>();
}

CHtml::~CHtml()
{
	reset();
}

void CHtml::reset()
{
	m_len_offset = 0;
	m_charset.clear();
	m_title.clear();
	m_liHead.clear();
	m_liText.clear();
	m_setLinks.clear();
	m_document.clear();

	if (m_pOutput != nullptr)
	{
		gumbo_destroy_output(&m_options, m_pOutput);
		m_pOutput = nullptr;
	}
}

bool CHtml::getValue(string & ret, string & strData, string flag)
{
	bool bResult = false;
	string::size_type size = strData.size();
	string::size_type pos = strData.find(flag);
	if (string::npos != pos) {
		pos += flag.size();
		int len = size - pos;
		if (len > 0) {
			ret = strData.substr(pos, len);
			ret.erase(ret.find_last_not_of(" \n\r\t") + 1);
			ret.erase(0, ret.find_first_not_of(" \n\r\t"));
			bResult = true;
		}
	}
	return bResult;
}

void CHtml::getCharset()
{
	const GumboNode* root = m_pOutput->root;
	assert(root->type == GUMBO_NODE_ELEMENT);
	assert(root->v.element.children.length >= 2);

	const GumboVector* root_children = (GumboVector*)&root->v.element.children;
	GumboNode* head = NULL;
	for (unsigned int i = 0; i < root_children->length; ++i) {
		GumboNode* child = (GumboNode*)root_children->data[i];
		if (child->type == GUMBO_NODE_ELEMENT &&
			child->v.element.tag == GUMBO_TAG_HEAD) {
			head = child;
			break;
		}
	}
	assert(head != NULL);

	GumboVector* head_children = &head->v.element.children;
	for (unsigned int i = 0; i < head_children->length; ++i) {
		GumboNode* child = (GumboNode*)head_children->data[i];
		if (child->type == GUMBO_NODE_ELEMENT &&
			child->v.element.tag == GUMBO_TAG_META) {
			GumboAttribute* charset = gumbo_get_attribute(&child->v.element.attributes, "charset");
			if (charset) {
				m_charset = charset->value;	
				transform(m_charset.begin(), m_charset.end(), m_charset.begin(), (int(*)(int))tolower);
				break;
			}
			else {
			  auto cont = gumbo_get_attribute(&child->v.element.attributes, "content");
			  if (cont) {
				  string strA = cont->value;
				  transform(strA.begin(), strA.end(), strA.begin(), (int(*)(int))tolower);
				  if (getValue(m_charset, strA, "charset=")){					 
					  break;
				  }
			  }
			}
		}
	}
	if (m_charset.empty()) {
		LOG_WARNING("not to find charset");
	}
	else {		
		LOG_INFO("charset=%s", m_charset.c_str());
	}
	return;
}

bool CHtml::isErrorLink(string & strLink)
{
	bool bResult = false;
	for (auto & strErr : m_liInvalidLinks) {
		string::size_type pos = strLink.find(strErr);
		if (string::npos != pos) {
			bResult = true;
			break;
		}
	}

	if (bResult) {
		try {
			auto uriPtr = std::make_shared<UTIL_SELF::URI>(strLink);
		}
		catch (...) {
			//LOG_ERROR("ErrorLink %s", strLink.c_str());
			bResult = false;
		}
	}

	return bResult;
}



bool CHtml::checkExtName(string strLink)
{	
	transform(strLink.begin(), strLink.end(), strLink.begin(), (int(*)(int))tolower);
	string::size_type pos = strLink.rfind(".");
	if (string::npos != pos) {
		string::size_type next = strLink.size();
		string strExtName = strLink.substr(pos, next - pos);
		auto it = m_setExtName.find(strExtName);
		if (m_setExtName.end() != it) {
			return false;
		}
	}
	return true;
}

void CHtml::search_for_links(GumboNode* node, string & url)
{
	if (node->type != GUMBO_NODE_ELEMENT) {
		return;
	}
	GumboAttribute* href;
	if (node->v.element.tag == GUMBO_TAG_A &&
		(href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {		
		string strLink = href->value;
		UTIL_SELF::extratSubStr(strLink, "#");
		UTIL_SELF::trimstr(strLink);
		if (!isErrorLink(strLink)) {
			if (checkExtName(strLink)) {
				if (!strLink.empty()) {					
					if ((strLink != "#") || (strLink != "/")
						|| (strLink != "http://") || (strLink != "https://")
						|| (strLink != "style=\"color:")
						) {
						m_setLinks.insert(strLink);
					}
					else {
						LOG_ERROR("%s<--->(%s)", strLink.c_str(), url.c_str())
					}
				}
			}			
		}
	}

	GumboVector* children = &node->v.element.children;
	for (unsigned int i = 0; i < children->length; ++i) {
		search_for_links(static_cast<GumboNode*>(children->data[i]), url);
	}
	return;
}

void CHtml::iteyeTitle(string & strInfo)
{
	//LOG_INFO("%s", strInfo.c_str());
	string::size_type pos = strInfo.find("-");
	if (string::npos != pos) {
		string::size_type next = strInfo.size();
		strInfo.erase(pos, next - pos);
	}
	//LOG_INFO("%s", strInfo.c_str());
}

void CHtml::blogCsdn(string & strInfo)
{
	//LOG_INFO("%s", strInfo.c_str());
	string::size_type pos = strInfo.find("_");
	if (string::npos != pos) {
		string::size_type next = strInfo.size();
		strInfo.erase(pos, next - pos);
		string pattern = "_";
		string space = " ";
		UTIL_SELF::replaceEx(strInfo, pattern, space);
	}
	else {
		pos = strInfo.find("-");
		string::size_type next = strInfo.size();
		strInfo.erase(pos, next - pos);
	}
	//LOG_INFO("%s", strInfo.c_str());
}


void CHtml::askCsdn(string & strInfo)
{
	string strSpace = "-";
	string pattern;
	pattern.resize(3);
	pattern[0] = 0xe2;
	pattern[1] = 0x80;
	pattern[2] = 0x94;
	UTIL_SELF::replaceEx(strInfo, pattern, strSpace);
	UTIL_SELF::eraseMoreStr(strInfo, '-');
	UTIL_SELF::replaceUtfSpace(strInfo, '-');
	UTIL_SELF::eraseMoreSpace(strInfo);
	return;
}

void CHtml::titleMore(string & strInfo, vector<string> & vecFlag)
{
	for (auto & flag : vecFlag) {		
		string::size_type pos = strInfo.rfind(flag);
		if (string::npos == pos) {
			break;
		}
		string::size_type next = strInfo.size();
		strInfo.erase(pos, next - pos);
	}
	return;
}

void CHtml::prepairTitle(string & strInfo)
{
	vector<string> vecFlag;
	if ("www.iteye.com" == m_DomainName) {
		iteyeTitle(strInfo);
	}
	else if ("blog.csdn.net" == m_DomainName) {
		blogCsdn(strInfo);
	}
	else if (("www.cnblogs.com" == m_DomainName) 
		|| ("www.cppblog.com" == m_DomainName)
		|| ("cloud.tencent.com" == m_DomainName)
		|| ("blog.chinaunix.net" == m_DomainName)
		|| ("segmentfault.com" == m_DomainName)
		){
		vecFlag.push_back("-");
		vecFlag.push_back("-");
	    titleMore(strInfo, vecFlag);
	}
	else if ("www.debian.cn" == m_DomainName) {
		vecFlag.push_back("-");
		titleMore(strInfo, vecFlag);
	}
	else if ("ifeve.com" == m_DomainName) {
		vecFlag.push_back("-");
		vecFlag.push_back("|");
		titleMore(strInfo, vecFlag);
	}
	else if (("www.easck.com" == m_DomainName) 
		|| ("c.biancheng.net" == m_DomainName) 
		){
		vecFlag.push_back("_");
		titleMore(strInfo, vecFlag);
	}
	else if ("ruby-china.org" == m_DomainName) {
		vecFlag.push_back("¡¤");
		titleMore(strInfo, vecFlag);
	}
	else if (("www.xuebuyuan.com" == m_DomainName) 
		|| ("www.runoob.com" == m_DomainName)
		){
		vecFlag.push_back("|");
		titleMore(strInfo, vecFlag);
	}
	else {
		iteyeTitle(strInfo);
	}
	UTIL_SELF::trim(strInfo);
}

void  CHtml::getTitleAndHead(const GumboNode* root)
{
	assert(root->type == GUMBO_NODE_ELEMENT);
	assert(root->v.element.children.length >= 2);

	const GumboVector* root_children = &root->v.element.children;
	GumboNode* head = nullptr;
	for (unsigned int i = 0; i < root_children->length; ++i) {
		GumboNode* child = (GumboNode*)root_children->data[i];
		if (child->type == GUMBO_NODE_ELEMENT &&
			child->v.element.tag == GUMBO_TAG_HEAD) {
			head = child;
			break;
		}
	}
	assert(head != nullptr);
	bool bFind = false;
	GumboVector* head_children = &head->v.element.children;	
	for (unsigned int i = 0; i < head_children->length; ++i) {
		GumboNode* child = (GumboNode*)head_children->data[i];
		if (child->type == GUMBO_NODE_ELEMENT &&
			child->v.element.tag == GUMBO_TAG_TITLE) {
			bFind = true;
			if (child->v.element.children.length != 1) {
				return;
			}
			GumboNode* title_text = (GumboNode*)child->v.element.children.data[0];
			//assert(title_text->type == GUMBO_NODE_TEXT || title_text->type == GUMBO_NODE_WHITESPACE);
			m_title = title_text->v.text.text;

			
			UTIL_SELF::trim(m_title);		  
			prepairTitle(m_title);
			UTIL_SELF::eraseUtfSpace(m_title);
			UTIL_SELF::eraseMoreSpace(m_title);
			m_viewTitle = m_title;
			if (m_title.size() > 30) {
				iutf8string utf(m_title);
				m_viewTitle = utf.substrEx(0, 20);
				//cout << m_viewTitle << endl;
			}
			
		}
		if (bFind) {
			if (child->type == GUMBO_NODE_ELEMENT &&
				child->v.element.tag == GUMBO_TAG_META) {
				auto cont = gumbo_get_attribute(&child->v.element.attributes, "content");
				if (cont) {
					string strA = cont->value;
					//m_liHead.push_back(strA);
				}
			}
		}
	}
	return;
}


std::string CHtml::cleantext(GumboNode* node)
{
	if (node->type == GUMBO_NODE_TEXT) {
		return std::string(node->v.text.text);
	}
	else if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) {
		std::string contents = "";
		GumboVector* children = &node->v.element.children;
		for (unsigned int i = 0; i < children->length; ++i) {
			std::string text = cleantext((GumboNode*)children->data[i]);
			if (i != 0 && !text.empty()) {
				contents.append(" ");
			}			
			contents.append(text);
		}
		return contents;
	}
	else {
		return "";
	}
}


void CHtml::searchForClass(GumboNode* node, const std::string& original_text,
	const char* cls_name, const char* cls_value)
{	
	if (node->type != GUMBO_NODE_ELEMENT) {
		return;
	}
    GumboAttribute* cls_attr = gumbo_get_attribute(&node->v.element.attributes, cls_name);    
    if (cls_attr){     
        string strValue = cls_attr->value;
        UTIL_SELF::trimNoSpace(strValue);
        transform(strValue.begin(), strValue.end(), strValue.begin(), (int(*)(int))tolower);
        if (strValue == cls_value) 
        {
            //LOG_ERROR("(%s)=(%s)******(%s)", cls_name, cls_value, strValue.c_str());
            getCleantext(node);
            
        }       
	}
	GumboVector* children = &node->v.element.children;
	for (int i = 0; i < (int)children->length; ++i) {
		searchForClass(	static_cast<GumboNode*>(children->data[i]), original_text, cls_name, cls_value);
	}
}

void CHtml::getCleantext(GumboNode* node)
{
	m_document += cleantext(node);
	UTIL_SELF::eraseUtfSpace(m_document);
	UTIL_SELF::eraseMoreSpace(m_document);
	
	return;
}

void CHtml::splitKeyword(map<string, vector<std::shared_ptr<TERM_INFO>>> & termInfo,
	int size, const string & strInfo, std::shared_ptr<HTML_INFO> & htmlInfo)
{
	map<string, uint32_t> mpTitle;
	if (!htmlInfo->bBuss) {
		CSplitWordS->parseTitleKeyWord(mpTitle, m_title, htmlInfo->weight, m_url);
	}
	map<string, uint32_t> mpBak = mpTitle;
	int num = 0;
	multimap<string, uint32_t> mpDst;
	CSplitWordS->parseKeyWord(mpDst, strInfo, num, htmlInfo);		
		
	for (auto & item : mpDst) {
		auto term = std::make_shared<TERM_INFO>();
		term->offset = (item.second + size);
		term->weight = int(WEIGHT_HTML::WEIGHT_TXT);
		if (htmlInfo->bBuss) {

			term->weight = num;
			termInfo[item.first].push_back(term);
		}
		else {	
			auto itFind = mpBak.find(item.first); 
			if (mpBak.end() != itFind)
			{
				term->weight += (itFind->second);
				mpBak.erase(itFind);
			}

			auto pos = mpTitle.find(item.first);
			if (mpTitle.end() != pos)
			{
				auto it = termInfo.find(item.first);
				if (termInfo.end() == it) {					
					termInfo[item.first] = vector<std::shared_ptr<TERM_INFO>>();
					termInfo[item.first].clear();
				}				
				termInfo[item.first].push_back(term);
				
			}
			else if (CWordMngS->existWord(item.first)) {
				termInfo[item.first].push_back(term);
			}
		}
	}
	
	for (auto & item : mpBak) {
		auto term = std::make_shared<TERM_INFO>();
		term->offset = 0;
		term->weight = item.second;
		auto it = termInfo.find(item.first);
		if (termInfo.end() == it) {
			termInfo[item.first] = vector<std::shared_ptr<TERM_INFO>>();
			termInfo[item.first].clear();
		}
		termInfo[item.first].push_back(term);
		
	}

	if (!htmlInfo->bBuss) {
		string strTemp;
		for (auto & item : termInfo) {
			if (strTemp.empty()) {
				strTemp = "/";
			}
			strTemp += item.first;
			strTemp += "/";
		}
		if (!strTemp.empty()) {
			LOG_INFO("(%s) url(%s)", strTemp.c_str(), m_url.c_str());
		}

		strTemp.clear();
		for (auto & item : mpTitle) {
			if (strTemp.empty()) {
				strTemp = "/";
			}
			strTemp += item.first;
			strTemp += "/";
		}
		if (!strTemp.empty()) {
			LOG_INFO("(%s) url(%s)", strTemp.c_str(), m_url.c_str());
		}
	}
	
	return;
}

void CHtml::splitTxt(map<string, vector<std::shared_ptr<TERM_INFO>>> & termInfo, std::shared_ptr<HTML_INFO> & htmlInfo)
{		
	splitKeyword(termInfo, m_len_offset, m_document, htmlInfo);
	m_len_offset += static_cast<int>(m_document.size());

	//LOG_INFO("%s", m_document.c_str());
	return;
} 


string CHtml::getAttibuteValue(string & strData, string bFlag, string mFlag)
{
	string::size_type size = strData.size();
	string eFlag;
	string ret;
	string::size_type next;
	string::size_type pos = strData.find(bFlag);
	if (string::npos == pos) {		
		return ret;
	}

	pos += bFlag.size();
	pos = strData.find(mFlag, pos);
	if (string::npos == pos) {		
		return ret;
	}

	pos += mFlag.size();
	next = strData.find_first_not_of(" \t\r", pos);
	if (string::npos == next) {
		return ret;
	}
	if (next >= size) {
		return ret;
	}
	pos = next;
	eFlag = strData.at(next);
	if (("\'" == eFlag) || ("\"" == eFlag)) {		
		pos += eFlag.size();
		next = strData.find(eFlag, pos);
		if (string::npos == next) {
			return ret;
		}
	}
	else {		
		next = strData.find_first_of("\'\"", pos);
		if (string::npos == next) {
			return ret;
		}
	}
	ret = strData.substr(pos, next - pos);
	UTIL_SELF::trimstr(ret);
	
	return ret;
}

bool CHtml::eraseBlockByFlag(string & strData, string bFlag, string eFlag)
{	
	string::size_type pos = strData.find(bFlag);
	if (string::npos == pos) {
		return false;
	}	
	string::size_type next = strData.find(eFlag, pos);
	if (string::npos == next) {
		return false;
	}
	next += eFlag.size();
	strData.erase(pos, next - pos);
	return true;
}

void CHtml::charset2utf8(string & strData, std::shared_ptr<HTML_INFO> & htmlInfo)
{
	//cout << strData.size() << endl;
	std::set<string> setHead;
	setHead.insert("<head>");
	setHead.insert("<head ");
	setHead.insert("<HEAD>");
	setHead.insert("<HEAD ");	
	
	string::size_type pos = 0;
	string::size_type next = 0;
	bool bFind = false;
	for (auto & item : setHead) {
		pos = strData.find(item);
		if (string::npos != pos) {
			bFind = true;
			break;
		}
	}
	if (!bFind) {
		LOG_ERROR("not to find <head> url(%s)", htmlInfo->url.c_str());
		//return;
	}

	bFind = false;

	
	setHead.clear();	
	setHead.insert("</head>");
	setHead.insert("</HEAD>");	
	for (auto & item : setHead) {
		next = strData.find(item, pos);
		if (string::npos != next) {
			bFind = true;
			break;
		}
	}

	if (!bFind) {
		next = strData.find("<body>");
		if (string::npos == next) {
			LOG_ERROR("not to find </head> url(%s)", htmlInfo->url.c_str());
			//return;
		}
		pos = 0;
	}
    if (string::npos == next) {
        next = 0;
    }
    if (string::npos == pos) {
        pos = 0;
    }

    string strHead;
    if (next > pos) {
        strHead = strData.substr(pos, next - pos);
    }
    else {
        strHead = strData;
    }
	transform(strHead.begin(), strHead.end(), strHead.begin(), (int(*)(int))tolower);
	bFind = false;
	setHead.clear();
	vector<string> vecFlag;
	vecFlag.push_back(" charset");
	vecFlag.push_back(";charset");
	vecFlag.push_back("charset");	
	vecFlag.push_back("charset ");
	for (auto & flag : vecFlag) {
		m_charset = getAttibuteValue(strHead, flag, "=");
		UTIL_SELF::trim(m_charset);
		
		if (!m_charset.empty()) {
			bFind = true;
			break;
		}
	}
	
	if (m_charset.empty()) {
		bFind = getCharsetByHead(m_charset, htmlInfo->head);
	}
	if (!bFind) {
		LOG_ERROR("empty charset url(%s)", htmlInfo->url.c_str());
	}
	//cout << "m_charset=" << m_charset << endl;

	auto it = m_mpConv.find(m_charset);
	if (m_mpConv.end() != it) {	
		try {
			auto iconv = std::make_shared<CConvertStr>(it->second);
			if (!iconv->codeConvert(strData)){
				LOG_ERROR("codeConvert...");
			}			
		}
		catch (...) {
			LOG_WARNING("except...");
		}
		LOG_INFO("charset: %s --> utf8", m_charset.c_str());
	}
	LOG_INFO("charset: %s  url(%s)", m_charset.c_str(), htmlInfo->url.c_str());
	return;
}

bool CHtml::getCharsetByHead(string & charset, string & strHead)
{	
	string flag = "content-type:";
	string::size_type pos = strHead.find(flag);
if (string::npos == pos) {
	return false;
}
pos += flag.size();
string::size_type next = strHead.find_first_of("\n\r", pos);
if (string::npos == next) {
	return false;
}
flag = "charset";
pos = strHead.find(flag, pos);
if (string::npos == pos)
{
	return false;
}
flag = "=";
pos = strHead.find(flag, pos);
if (string::npos == pos)
{
	return false;
}
pos += flag.size();
if (pos < next) {
	charset = strHead.substr(pos, next - pos);
	UTIL_SELF::trim(charset);
}

return true;
}

void CHtml::test()
{


	const char* filename = "chinaunix.html";
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (!in) {
		std::cout << "File " << filename << " not found!\n";
		exit(EXIT_FAILURE);
	}
	std::string contents;
	string & strData = contents;
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	auto htmlInfo = std::make_shared<HTML_INFO>();
	////htmlInfo->url = "http://c.biancheng.net/";
	htmlInfo->url = "http://bbs.chinaunix.net/";
	charset2utf8(strData, htmlInfo);
	m_pOutput = gumbo_parse_with_options(&m_options, strData.data(), strData.size());
	if (nullptr == m_pOutput) {
		LOG_WARNING("fault to parse ...");
		return;
	}
	GumboOutput* output = m_pOutput;
	getTitleAndHead(output->root);
	//getCleantext(output->root);
	search_for_links(output->root, htmlInfo->url);
	m_document = cleantext(output->root);

	CHandleXmlS->readDefaultXml();
	string strPath = CHandleXmlS->getPath();
	CEscapeSequenceS->init(strPath);

	//m_document = "\'hello\'   i    am    comeing     <kkkd> one		\r1\n2		3";
	//CEscapeSequenceS->quoteHtml(m_document);
	//cout << m_document << endl;
	//eraseMoreSpace(m_document);	
	//cout << m_title << endl;

	cout << "finish" << endl;
	return;

}

bool CHtml::parseWord(string & strData, std::shared_ptr<HTML_INFO> & htmlInfo)
{
	bool bResult = false;
	string snapshotHtml = strData;
	charset2utf8(strData, htmlInfo);
	m_pOutput = gumbo_parse_with_options(&m_options, strData.data(), strData.size());
	if (nullptr == m_pOutput) {
		LOG_WARNING("fault to parse ...");
		return bResult;
	}
	m_url = htmlInfo->url;
	m_DomainName = htmlInfo->domainName;
	GumboOutput* output = m_pOutput;

	getTitleAndHead(output->root);	
	if (!htmlInfo->bBuss) {
		search_for_links(output->root, htmlInfo->url);
        searchForClass(output->root, strData, htmlInfo->className.c_str(), htmlInfo->classValue.c_str());
	}
    else {
        getCleantext(output->root);
    }

	if (m_document.empty()) {
		LOG_INFO("(%s=%s)title(%s)(%s) url(%s)", htmlInfo->className.c_str(), htmlInfo->classValue.c_str(),
			htmlInfo->subUrl.c_str(),m_title.c_str(), m_url.c_str());
	}	
	else {
		map<string, vector<std::shared_ptr<TERM_INFO> > > mpHtml; //¹Ø¼ü×Ö-->TERM_INFO
		splitTxt(mpHtml, htmlInfo);
		if (mpHtml.size() > 0) {
			string strKeyword;
			if (htmlInfo->bUpdate) {
				LOG_INFO("update document md5(%s)", htmlInfo->url.c_str());
				if (CStoreMngS->getDb(TERM_DB, htmlInfo->url, strKeyword)) {
					vector<string> vecOldWord;
					UTIL_SELF::split(strKeyword, vecOldWord, SPLIIT_FLAG_COMMA);
					eraseOldWord(vecOldWord, htmlInfo);
					CStoreMngS->deleteDb(TERM_DB, htmlInfo->url);
				}
			}
			string strB64;
			base64->GetCodeInfo(strB64, m_viewTitle);
			string strDocDb;
			strDocDb = strB64;
			strDocDb += DOC_SPLIT_FLAG;
			strDocDb += htmlInfo->snapshotUrl;
			strDocDb += DOC_SPLIT_FLAG;
			strDocDb += m_document;
			if (!CStoreMngS->putDb(DOCUMENT_DB, htmlInfo->url, strDocDb)) {
				LOG_ERROR("fault to save txt document %s", htmlInfo->url.c_str());
				return bResult;
			}
			if (!CStoreMngS->putDb(DOCUMENT_DB, htmlInfo->snapshotUrl, snapshotHtml)) {
				LOG_ERROR("fault to save html document %s", htmlInfo->url.c_str());
				return bResult;
			}

			auto mpTermSnapshot(std::make_unique<MAP_INFO_TYPE>());
			auto fileKey = std::make_shared<string>(htmlInfo->url);
			CMapMngS->saveSearch(*mpTermSnapshot, mpHtml, fileKey); //Ð´ÄÚ´æ	
			CMapMngS->writeDb(*mpTermSnapshot, htmlInfo->url); ////Ð´Íâ´æ		

			strKeyword.clear();
			for (auto & item : mpHtml) {
				strKeyword += item.first;
				strKeyword += SPLIIT_FLAG_COMMA;
			}
			if (!CStoreMngS->putDb(TERM_DB, htmlInfo->url, strKeyword)) {

				LOG_ERROR("fault to save keyword");
			}
			if (!htmlInfo->bBuss) {
				LOG_WARNING("url(%s) %s", htmlInfo->url.c_str(), strKeyword.c_str());
			}
			
		}
		else {
			LOG_ERROR("not to find keyword!!!(%s)", m_url.c_str());
		}
	}	

	LOG_INFO("url(%s)links(%d)", m_url.c_str(), m_setLinks.size());
	if (!htmlInfo->bBuss) {
		m_links->downladInfo.firstName = htmlInfo->firstName;
		m_links->downladInfo.url = htmlInfo->url;
		m_links->downladInfo.domainName = htmlInfo->domainName;
		m_links->downladInfo.liCookie = htmlInfo->liCookie;
		m_links->hrefLinks = m_setLinks;
		m_links->downladInfo.head = htmlInfo->head;
		m_links->downladInfo.location = htmlInfo->location;
		m_links->downladInfo.weight = htmlInfo->weight;
		m_links->downladInfo.className = htmlInfo->className;
		m_links->downladInfo.classValue = htmlInfo->classValue;
		m_links->downladInfo.bBuss = htmlInfo->bBuss;
	}
	bResult = true;
	//bResult = false;
	return bResult;
}

void CHtml::eraseOldWord(vector<string> & vecOldWord, std::shared_ptr<HTML_INFO> & htmlInfo)
{	
	for (auto & word : vecOldWord) {
		CMapMngS->earseWord(word, htmlInfo->url);
	}
	CStoreMngS->deleteDb(SEARCH_DB, vecOldWord);
}

std::shared_ptr<LINKS_INFO> CHtml::getLinks()
{
	return m_links;
}

