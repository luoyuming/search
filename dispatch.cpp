#include "dispatch.h"
#include "log.h"
#include "splitWord.h"
#include "handleXml.h"
#include "epollMng.h"
#include "gzipCode.h"
#include "mapMng.h"
#include "storeMng.h"
#include "fileMng.h"
#include "handleJson.h"
#include "faceCGI_C.h"

using namespace std::placeholders;

CDispatch::CDispatch()
{
	initFun();

	m_mpExt2RespType["txt"] = RESP_CONTENT_TYPE::TXT_TYPE;
	m_mpExt2RespType["js"] = RESP_CONTENT_TYPE::JS_TYPE;
	m_mpExt2RespType["css"] = RESP_CONTENT_TYPE::CSS_TYPE;
	m_mpExt2RespType["html"] = RESP_CONTENT_TYPE::HTML_TYPE;
	m_mpExt2RespType["png"] = RESP_CONTENT_TYPE::PNG_TYPE;
	m_mpExt2RespType["jpg"] = RESP_CONTENT_TYPE::JPG_TYPE;
	m_mpExt2RespType["jpeg"] = RESP_CONTENT_TYPE::JPG_TYPE;
	m_mpExt2RespType["gif"] = RESP_CONTENT_TYPE::GIF_TYPE;
	m_mpExt2RespType["ico"] = RESP_CONTENT_TYPE::ICO_TYPE;
	m_mpExt2RespType["bmp"] = RESP_CONTENT_TYPE::BMP_TYPE;

	m_gzExt.insert("js");
	m_gzExt.insert("css");
	m_gzExt.insert("html");
	m_gzExt.insert("ico");
	m_gzExt.insert("htm");

	
	m_setExcept.insert("\\");
	m_setExcept.insert("\n");
	m_setExcept.insert("\t");
	m_setExcept.insert("\r");
	

    m_setFlag.insert("/disclaimer.html");

	m_setDomain.insert("www.dswd.net");
}

CDispatch::~CDispatch()
{

}

void CDispatch::initFun()
{
	ubyte4 i = 0;
	Command httpMothed[] = {
		{ HTTP_POST, std::bind(&CDispatch::postMothed, this, _1) },
		{ HTTP_GET, std::bind(&CDispatch::getMothed, this, _1) },
	};
	for (i = 0; i < sizeof(httpMothed) / sizeof(httpMothed[0]); ++i)
	{
		m_FunHttpMothed[httpMothed[i].commandID] = httpMothed[i];
	}

	//////////////////////////////////////////////////////
	UrlMothed httpPost[] = {
		{ "/", std::bind(&CDispatch::httpVersion, this, _1) },
		{"/query", std::bind(&CDispatch::httpQuery, this, _1) },
		{"/visit_ip", std::bind(&CDispatch::httpVisitIP, this, _1) },
	};

	for (i = 0; i < sizeof(httpPost) / sizeof(httpPost[0]); ++i)
	{
		m_FunHttpPost[httpPost[i].commandID] = httpPost[i];
	}

	////////////////////////////////////////////////////////////////
	UrlMothed httpGet[] = {
		{"/", std::bind(&CDispatch::httpIndex, this, _1)},
		{"/query", std::bind(&CDispatch::httpGetEmpty, this, _1)},
		{"/s", std::bind(&CDispatch::httpGeS, this, _1) },
		{"/snapshot", std::bind(&CDispatch::httpGetSnapshot, this, _1) },
		{"/page_url", std::bind(&CDispatch::httpGetPageUrl, this, _1) },
	
	};

	for (i = 0; i < sizeof(httpGet) / sizeof(httpGet[0]); ++i)
	{
		m_FunHttpGet[httpGet[i].commandID] = httpGet[i];
	}
}

void CDispatch::handleInfo(PACKAGE_INFO & info)
{	
	info.pageNo = 0;
	auto it = m_FunHttpMothed.find(info.commandID);
	if (it != m_FunHttpMothed.end())
	{
		auto & cmd = it->second;
		cmd.callbackFun(info);
	}
	//if (info.strResp.empty()) {
		//info.strResp = "bad to call";		
	//}
	return;
}

void CDispatch::getMothed(PACKAGE_INFO & info)
{	
	disptMothed(m_FunHttpGet, info);
	return;
}

void CDispatch::postMothed(PACKAGE_INFO & info)
{	
	disptMothed(m_FunHttpPost, info);
	return;
}

void CDispatch::disptMothed(std::map<string, UrlMothed> & dispFun, PACKAGE_INFO & info)
{
	//LOG_INFO("%s ", info.strHead.c_str());
	//string test1 = "head.txt", test2 = "body.txt";
	//UTIL_SELF::saveFile(test1, info.strHead);
	//UTIL_SELF::saveFile(test2, info.strBody);
	//LOG_INFO("%s", info.strHead.c_str());
	info.respType = RESP_CONTENT_TYPE::HTML_TYPE;
	if (HTTP_POST == info.commandID) {
		//check the mothed  form-data
		httpFormData(info);
	}
	if (handlePhP(info)) {
		return;
	}

	bool bFind = false;
	auto it = dispFun.find(info.url);
	if (it != dispFun.end())
	{
		bFind = true;
	}
	if (bFind) {
		auto & cmd = it->second;
		cmd.callbackFun(info);
	}
	else if (HTTP_GET == info.commandID) {	
		
		info.strResp.clear();
		string strPath = CHandleXmlS->getPath();
		string strWebSite = getWebsite(info.mField);
		string file = strPath;
		file += strWebSite;
		file += info.url;
		LOG_INFO("%s", file.c_str());
		//string file = strPath + "web"+ info.url;		
		if (!UTIL_SELF::readFile(file, info.strResp)) {			
			LOG_ERROR("falult to get %s ", file.c_str());			
		}
		else {            
			string extName = UTIL_SELF::getExtName(info.url);
			if (checkCanGzip(extName)) {   
                replaceTemplate(info.strResp, info.url);

				string strGZip;
				auto gzip(UTIL_SELF::make_unique<CGzip>());
				if (gzip->codeGzip(strGZip, info.strResp)) {
					info.strResp = strGZip;
					info.contentEncoding = "content-encoding: gzip";
				}
			}		
			
			auto it = m_mpExt2RespType.find(extName);
			if (m_mpExt2RespType.end() != it) {
				info.respType = it->second;
			}
			else {
				LOG_ERROR("falult to find the Ext2RespType");
			}
		}
	}
	return;
}

string CDispatch::getWebsite(std::unordered_map<string, string> & Field)
{
	string strRet = DEFAULT_WWW;
	auto it = Field.find("host");
	if (Field.end() != it) {
		auto find = m_setDomain.find(it->second);
		if (m_setDomain.end() != find) {
			strRet = it->second;
		}
	}
	return strRet;
}

bool CDispatch::checkCanGzip(string & extName)
{
	bool bResult = false;
	auto it = m_gzExt.find(extName);
	if (m_gzExt.end() != it) {
		bResult = true;
	}
	return bResult;
}

bool CDispatch::httpFormData(PACKAGE_INFO & info)
{
	bool bResult = false;
	std::vector<string> vecCT;
	vecCT.push_back("\nContent-Type:");
	vecCT.push_back("\ncontent-type:");
	vecCT.push_back("\nContent-type:");
	vecCT.push_back("\ncontent-Type:");

	string strSubInfo;
	if (!getHeadField(strSubInfo, info.strHead, vecCT)) {
		LOG_INFO("%s", info.strHead.c_str());
		return bResult;
	}
	vecCT.clear();

	string strFormDataFlag = "multipart/form-data;";
	string strBoundaryFlag = "boundary=";
	string::size_type pos = strSubInfo.find(strFormDataFlag);
	if (string::npos == pos)
	{
		string JsonDataFlag = "application/json";
		pos = strSubInfo.find(JsonDataFlag);
		if (string::npos != pos) {
			return true;
			//return httpJsonData(info);
		}		
		return bResult;
	}
	pos = strSubInfo.find(strBoundaryFlag);
	if (string::npos == pos)
	{		
		return bResult;
	}
	string strBoundary;
	bResult = getSubValue(strBoundary, strSubInfo, strBoundaryFlag, strCRLN);
	if (!bResult)
	{
		return bResult;
	}

	erasePadField(strBoundary);
	//LOG_INFO("strBoundary : %s", strBoundary.c_str())

	string padFlag = "--";
	string BoundaryBegin = padFlag + strBoundary;
	string BoundaryEnd = BoundaryBegin + padFlag;
	pos = info.strBody.rfind(BoundaryEnd);
	if (string::npos == pos)
	{
		return bResult;
	}
	string::size_type next = info.strBody.size();
	if (next > pos)
	{
		info.strBody.erase(pos, next - pos);
	}
	handleFormData(info, BoundaryBegin);
	return bResult;
}

void CDispatch::handleFormData(PACKAGE_INFO & info, string & strBoundary)
{
	string::size_type pos = 0, next = 0;
	do {
		auto filedInfo = std::make_shared<FORM_DATA_INFO>();
		next = getFormData(*filedInfo, info.strBody, pos, strBoundary);
		if (string::npos == next)
		{
			//LOG_INFO("exit form");
			break;
		}
		//LOG_INFO("%s", filedInfo->name.c_str());
		info.vecFormData.push_back(filedInfo);
		pos = next;
	} while (true);
	info.strBody.clear();
	return;
}

string::size_type CDispatch::getFormData(FORM_DATA_INFO & formData,
	string & strInfo, string::size_type pos,
	string & strBoundary)
{
	string::size_type next = string::npos;
	pos = strInfo.find(strBoundary, pos);
	if (string::npos == pos)
	{
		return string::npos;
	}
	pos += strBoundary.size();
	next = strInfo.find(strBoundary, pos);
	if (string::npos == next)
	{
		next = strInfo.size();
		if (next <= pos)
		{
			return string::npos;
		}
	}
	string dataFlag = strCRLN + strCRLN;
	string::size_type mid = strInfo.find(dataFlag, pos);
	if (string::npos == mid)
	{
		return string::npos;
	}
	if (mid >= next)
	{
		return string::npos;
	}
	string strField = strInfo.substr(pos, mid - pos);
	if (!getFormDataName(formData.name, strField))
	{
		return string::npos;
	}
	getFormFileName(formData.fileName, strField);

	mid += dataFlag.size();
	string::size_type len = next - mid - strCRLN.size();
	formData.data = strInfo.substr(mid, len);
	return next;
}


bool CDispatch::getFormDataName(string & strName, string & strInfo)
{
	//LOG_INFO("\n%s\n", strInfo.c_str());
	string strFlag = "name=\"";
	string::size_type pos = strInfo.find(strFlag);
	if (string::npos == pos)
	{
		return false;
	}
	pos += strFlag.size();
	strFlag = "\"";
	string::size_type next = strInfo.find(strFlag, pos);
	if (string::npos == next)
	{
		return false;
	}
	strName = strInfo.substr(pos, next - pos);
	return true;
}

bool CDispatch::getFormFileName(string & strName, string & strInfo)
{
	//LOG_INFO("\n%s\n", strInfo.c_str());
	strName.clear();
	string strFlag = "filename=\"";
	string::size_type pos = strInfo.find(strFlag);
	if (string::npos == pos)
	{
		return false;
	}
	pos += strFlag.size();
	strFlag = "\"";
	string::size_type next = strInfo.find(strFlag, pos);
	if (string::npos == next)
	{
		return false;
	}
	strName = strInfo.substr(pos, next - pos);
	return true;
}



void CDispatch::erasePadField(string & info)
{
	string strFlag = ";";
	string::size_type pos = info.find(strFlag);
	if (string::npos == pos)
	{
		return;
	}
	string strSub = info.substr(0, pos);
	info = strSub;
	return;
}

bool CDispatch::getSubValue(string & strSub, const string & strInfo, string begin, string end)
{
	bool bResult = false;
	string::size_type pos, next;
	pos = strInfo.find(begin);
	if (string::npos == pos)
	{
		return bResult;
	}
	pos += begin.size();
	next = strInfo.find(end, pos);
	if (string::npos == next)
	{
		next = strInfo.size();
		if (next <= pos)
		{
			return bResult;
		}
	}
	strSub = strInfo.substr(pos, next - pos);
	bResult = true;
	return bResult;
}

bool CDispatch::getHeadField(string & strSub, const string & strInfo, std::vector<string> & vecType)
{
	bool bResult = false;
	for (auto & strCTFlag : vecType) {
		bResult = getSubValue(strSub, strInfo, strCTFlag, strCRLN);
		if (bResult) {
			break;
		}
	}
	return bResult;
}

void CDispatch::httpVersion(PACKAGE_INFO & info)
{
	info.strResp = "bad to call";
	//LOG_INFO("\n%s\n", info.strHead.c_str());
	//LOG_INFO("\n%s\n", info.strBody.c_str());
}


void CDispatch::httpGetEmpty(PACKAGE_INFO & info)
{
	//LOG_INFO("\n%s\n", info.strHead.c_str());
	//LOG_INFO("\n%s\n", info.strBody.c_str());

	info.strResp = "empty";
}

void CDispatch::httpIndex(PACKAGE_INFO & info)
{
	//LOG_INFO("\n%s\n", info.strHead.c_str());
	//LOG_INFO("\n%s\n", info.strBody.c_str());

	info.strResp.clear();
	string strPath = CHandleXmlS->getPath();

	//string file = strPath + "web/index.html";
	string strWebSite = getWebsite(info.mField);
	string file = strPath;
	file += strWebSite;
	file += "/index.html";

	if (!UTIL_SELF::readFile(file, info.strResp)) {		
		LOG_ERROR("%s", file.c_str());
		info.strResp = "falult to get index.html";
		return;
	}
	
    string strHost = CHandleXmlS->getSystemXml().host;
    UTIL_SELF::replaceEx(info.strResp, QYT_HOST, strHost);
	

	return;
}


void CDispatch::httpGetSnapshot(PACKAGE_INFO & info)
{	
	auto it = info.mParam.find(QUERY_KEY_WORD_GET);
	if (info.mParam.end() == it) {
		info.strResp = "not keyword to search...";
		return;
	}
	string snapshotID = it->second;

	CMapMngS->searchSnapshot(snapshotID, info);
	info.respType = RESP_CONTENT_TYPE::ORIGIN_TYPE;

	string strGZip;
	auto gzip(UTIL_SELF::make_unique<CGzip>());
	if (gzip->codeGzip(strGZip, info.strResp)) {
		info.strResp = strGZip;
		info.contentEncoding = "content-encoding: gzip";
	}
	return;
}

void CDispatch::httpGetPageUrl(PACKAGE_INFO & info)
{
	//LOG_INFO("\n%s\n", info.strHead.c_str());
	//LOG_INFO("\n%s\n", info.strBody.c_str());
	//for (auto & item : info.mParam) {
		//LOG_INFO("\n%s=%s\n", item.first.c_str(), item.second.c_str());
	//}

	auto it = info.mParam.find("pageNo");
	if (info.mParam.end() != it) {
		info.pageNo = std::atoi(it->second.c_str());
	}
	string strKeyword;
	it = info.mParam.find(QUERY_KEY_WORD_GET);
	if (info.mParam.end() != it) {
		strKeyword = it->second;
	}

	LOG_WARNING("pageNo %d keyword(%s)", info.pageNo, strKeyword.c_str());

	handleSearch(info, strKeyword);
	return;
}

void CDispatch::httpGeS(PACKAGE_INFO & info)
{
	//LOG_INFO("\n%s\n", info.strHead.c_str());
	//LOG_INFO("\n%s\n", info.strBody.c_str());
	
	auto it = info.mParam.find(QUERY_KEY_WORD_GET);
	if (info.mParam.end() == it) {
		info.strResp = "not keyword to search...";
		return;
	}
	string strKeyword = it->second;

	LOG_INFO("%s", strKeyword.c_str());
	//info.strResp = "test";
	//string strFile = "test.html";
	//UTIL_SELF::readFile(strFile, info.strResp);
	handleSearch(info, strKeyword);
	return;
}


void CDispatch::httpQuery(PACKAGE_INFO & info)
{
	string strKeyword = UTIL_SELF::UrlDecode(info.strBody);
	string padFlag = QUERY_KEY_WORD_PAD;
	string::size_type pos = strKeyword.find(padFlag);
	if (string::npos != pos) {
		pos += padFlag.size();
		strKeyword.erase(0, pos);
	}
	UTIL_SELF::trimstr(strKeyword);
	string strValue = UTIL_SELF::UrlEncode(strKeyword);
	info.mOriginalParam[QUERY_KEY_WORD_GET] = strValue;
	LOG_INFO("%s", strKeyword.c_str());
	handleSearch(info, strKeyword);
	return;
}

void CDispatch::eraseMore(string & strInfo)
{	
	for (auto & value : strInfo) {
		if ((9 == value)
			|| (10 == value)
			|| (11 == value)
			|| (13 == value)
			){
			value = 0x20;
		}
	}
	return;
}

void CDispatch::handleSearch(PACKAGE_INFO & info, string & strKeyword)
{
	info.strResp = "nothing to search ";
	string strTemp;	
	info.strResp += strKeyword;

	eraseMore(strKeyword);
	vector<string> vecWord;
	UTIL_SELF::split(strKeyword, vecWord, " ");
	
	for (auto it = vecWord.begin(); it != vecWord.end();) {

		string & key = *it;	
		UTIL_SELF::trim(key);
		if (UTIL_SELF::is_engAndNum(key)) {
			transform(key.begin(), key.end(), key.begin(), (int(*)(int))tolower);			
		}		
		++it;		
	}
	if (vecWord.empty()) {
		
		return;
	}
	
	vector<string> vecKeyword = vecWord;	
	CSplitWordS->getKeyWord(vecKeyword);	
	std::copy(vecKeyword.begin(), vecKeyword.end(), std::back_inserter(vecWord));	
	uniqueElement(vecWord);	

	CMapMngS->searchWord(vecWord, info, strKeyword);

	string strGZip;
	auto gzip(UTIL_SELF::make_unique<CGzip>());
	if (gzip->codeGzip(strGZip, info.strResp)) {
		info.strResp = strGZip;
		info.contentEncoding = "content-encoding: gzip";
	}

	return;
}

void CDispatch::uniqueElement(vector<string> & vecWord)
{
	std::set<string> mpTemp;
	auto it = vecWord.begin();
	for (; it != vecWord.end(); ) {
		auto ifFind = mpTemp.find(*it);
		if (mpTemp.end() == ifFind) {
			mpTemp.insert(*it);
			++it;
		}
		else {
			it = vecWord.erase(it);
		}
	}
	return;
}

void CDispatch::replaceTemplate(string & strInfo, string & url)
{
    auto it = m_setFlag.find(url);
    if (m_setFlag.end() == it) {
        return;
    }
    string strHost = CHandleXmlS->getSystemXml().host;
    UTIL_SELF::replaceEx(strInfo, QYT_HOST, strHost, true);
    return;
}

void CDispatch::httpVisitIP(PACKAGE_INFO & info)
{
	info.strResp = "{\"code\":200,\"msg\":\"ok\"}";
	/*
	{
		"cmd":1  1为查询， 2为删除信息
	}
	*/

	auto handleJson(std::make_unique<CHandleJson>());
	int cmd = 0;
	if (handleJson->getVisitIP(cmd, info.strBody)); {
		if (ERASE_IP_INFO == cmd) {
			CFileMngS->reset();
		}
		else if (GET_IP_INFO == cmd) {
			vector<VISIT_INFO> vecInfo;
			CFileMngS->get(vecInfo);
			info.strResp = handleJson->buildVisitIP(vecInfo);
		}
	}
	return;
}

bool CDispatch::handlePhP(PACKAGE_INFO & info)
{
	auto pos = info.url.rfind(".php");
	if (string::npos == pos) {
		return false;
	}
	info.strResp = "404 not found";
	auto fcgi(std::make_shared<CFCGI_C>());
	if (fcgi->forward(info))
	{	
		string strGZip;
		auto gzip(UTIL_SELF::make_unique<CGzip>());
		if (gzip->codeGzip(strGZip, info.strResp)) {
			info.strResp = strGZip;
			info.contentEncoding = "content-encoding: gzip";
		}	
	}
	return true;
}