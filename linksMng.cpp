#include "linksMng.h"
#include "util.h"
#include "log.h"


CLinksMng::CLinksMng()
{
	reset();
	init();
}

void CLinksMng::init()
{
	m_twoFlag.insert("com");
	m_twoFlag.insert("edu");
	m_twoFlag.insert("org");
	m_twoFlag.insert("gov");
	m_twoFlag.insert("net");	
	m_twoFlag.insert("mil");
	m_twoFlag.insert("co");
	m_twoFlag.insert("biz");
	m_twoFlag.insert("cc");
	m_twoFlag.insert("xyz");
	m_twoFlag.insert("info");
	m_twoFlag.insert("ac");
	m_twoFlag.insert("tv");
	m_twoFlag.insert("me");
	m_twoFlag.insert("work");
	m_twoFlag.insert("in");
	m_twoFlag.insert("co");
	/*
	m_twoFlag.insert("travel");
	m_twoFlag.insert("aero");
	m_twoFlag.insert("rec");
	m_twoFlag.insert("ltd");	
	m_twoFlag.insert("top");
	m_twoFlag.insert("xin");
	m_twoFlag.insert("vip");
	m_twoFlag.insert("win");
	m_twoFlag.insert("red");
	m_twoFlag.insert("wang");
	m_twoFlag.insert("mobi");
	m_twoFlag.insert("pro");
	m_twoFlag.insert("club");
	m_twoFlag.insert("museum");
	m_twoFlag.insert("int");
	m_twoFlag.insert("post");
	m_twoFlag.insert("asia");	
	m_twoFlag.insert("idv");
	m_twoFlag.insert("name");
	m_twoFlag.insert("online");
	m_twoFlag.insert("shop");
	m_twoFlag.insert("live");
	m_twoFlag.insert("icu");
	
	m_twoFlag.insert("tech");*/


	m_OneFlag.insert("cn");	
	
	return;
}

string CLinksMng::getFullDomain()
{	
	string strRet = hostname;
	//cout << "hostname " << hostname << endl;	
	return strRet;
}


bool CLinksMng::existFlag(std::set<string> & setInfo, string & strFlag)
{	
	bool bFind = false;
	auto it = setInfo.find(strFlag);
	if (setInfo.end() != it) {
		bFind = true;
	}
	return bFind;
}


string CLinksMng::getProtocol()
{
	string httpHead = protocol;
	if (protocol.empty()) {
		httpHead = "http://";
	}
	return httpHead;
}

string CLinksMng::getUrl()
{
	string ret;
	if (protocol.empty()) {
		ret = "http://";
	}
	else {
		ret = protocol + "://";
	}
	ret += hostname_port;
	return ret;
}

string CLinksMng::getIndex()
{
	return index;
}

string CLinksMng::getSubDomain()
{
	return subDomain;
}

string CLinksMng::getRelative()
{
	return relativePath;
}

string CLinksMng::getPath()
{	
	return path;
}

string CLinksMng::buildMonthDay()
{
	string strRet;	
	time_t rawtime;
	rawtime = time(nullptr);
	struct tm * timeinfo = localtime(&rawtime);
	string year = std::to_string(timeinfo->tm_year + 1900);
	string month = std::to_string(timeinfo->tm_mon + 1);
	string day = std::to_string(timeinfo->tm_mday);
	strRet = year + "/" + month + "/" + day;
	return strRet;
}

string CLinksMng::getLocation()
{	
	string ret;
	if (protocol.empty()) {
		ret = "http://";
	}
	else {
		ret = protocol + "://";
	}	
	ret += hostname_port;
	if (!relativePath.empty()) {
		ret += relativePath;
	}
	return ret;
}

void CLinksMng::extractRelative()
{
	string::size_type pos = path.rfind("/");
	if (string::npos != pos) {
		relativePath = path.substr(0, pos);
		string::size_type next = path.size();
		pos += 1;
		if (next > pos) {
			index = path.substr(pos, next-pos);
		}
	}
}

void CLinksMng::extractSubDomain()
{
	string ret;	
	ret = hostname;
	if (ret.empty()) {
		ret = hostname_port;
	}	
	string::size_type size = ret.size();
	if (size > 0) {
		if ('/' != ret[size - 1]) {
			ret += "/";
		}
	}
	subDomain = ret;
}

string CLinksMng::parseUrl(const string & url, string domain)
{
	int nLoop = 1;
	string strRet;
	string strTemp = url;	
	std::shared_ptr<UTIL_SELF::URI> uriPtr;
	do {	
		try {
			uriPtr = std::make_shared<UTIL_SELF::URI>(strTemp);
		}
		catch (...) {
			LOG_INFO("except %s", url.c_str());
			return strRet;
		}
	
		protocol = uriPtr->getScheme();
		if (protocol.empty()) {
			if (domain.empty()) {
				LOG_INFO("empty http %s", domain.c_str());
				return strRet;
			}
			UTIL_SELF::URI uri2;
			uri2.setScheme("http");
			uri2.setAuthority(domain);
			uri2.resolve(url);
			strTemp = uri2.toString();	

		}
		else {
			hostname_port = uriPtr->getAuthority();
			hostname = uriPtr->getHost();
			port = std::to_string(uriPtr->getPort());
			path = uriPtr->getPath();			
			query = uriPtr->getQuery();
			fragment = uriPtr->getFragment();
			strRet = uriPtr->toString();
			fullPath = strRet;
			extractRelative();
			extractSubDomain();
			break;
		}
		nLoop++;
		if (nLoop > 5) {
			LOG_ERROR("%s", url.c_str());
			break;
		}
	} while (true);	
	
	return strRet;

}

void CLinksMng::extractDomain(const string & hostname_port)
{
	string strFlag = ":";
	string::size_type pos = hostname_port.find(strFlag);
	if (string::npos != pos) {
		hostname = hostname_port.substr(0, pos);
		string::size_type next = hostname_port.size();
		pos += strFlag.size();
		port = hostname_port.substr(pos, next - pos);
	}
	else {
		hostname = hostname_port;
	}
	return;
}

void CLinksMng::reset()
{
	protocol.clear();
	hostname_port.clear();
	path.clear();	
	hostname.clear();
	port.clear();
	query.clear();
	fragment.clear();
	relativePath.clear();
	fullPath.clear();
	index.clear();
}

bool CLinksMng::canVisit()
{
	bool bResult = true;
	string strFlag = ".";
	string::size_type pos = index.rfind(strFlag);
	if (string::npos != pos) {	
		pos += strFlag.size();
		string::size_type next = index.size();		
		if (next > pos) {
			string ext = index.substr(pos, next - pos);
			if (!ext.empty()) {
				if ((ext != "html") && (ext != "htm")
					&& (ext != "asp") && (ext != "php")
					&& (ext != "do") && (ext != "jsp")
					&& (ext != "aspx") && (ext != "net")
					&& (ext != "shtml") && (ext != "txt")
					) {
					bResult = false;
				}
			}
		}
	}
	if (!bResult) {
		if (!query.empty()) {
			bResult = true;
		}
	}
	
	return bResult;
}

bool CLinksMng::checkIsCN(string & strInfo)
{
	bool bResult = false;
	string::size_type pos;
	for (auto & item : m_twoFlag) {
		pos = strInfo.rfind(item);
		if (string::npos != pos) {
			bResult = true;
			break;
		}
	}
	if (!bResult) {
		pos = strInfo.rfind(".cn");
		if (string::npos != pos) {
			bResult = true;
		}
	}
	return bResult;
}

string CLinksMng::getFirstDomain()
{
	string ret;
	vector<string> vecFlag;
	vector<string> vecRerverse;
	UTIL_SELF::split(hostname, vecFlag, ".");		
	string::size_type size = vecFlag.size();	
	if (size >= 2) {
		auto it = vecFlag.rbegin();
		if (existFlag(m_OneFlag, *it)) {
			//www.test.cn;
			//www.test.com.cn
			if (existFlag(m_twoFlag, *(it+1)) ) {
				ret = size >= 3 ? *(it + 2) : ret;
			}
			else {
				ret = size >= 2 ? *(it + 1) : ret;
			}
		}	
		else {
			//www.test.com
			ret = size >= 2 ? *(it + 1) : ret;
		}
	}
	return  ret;
}

string CLinksMng::combineUri(string & domain, string & url)
{
	string ret;
	try {
		UTIL_SELF::URI uri2;
		uri2.setScheme("http");
		uri2.setAuthority(domain);
		uri2.resolve(url);
		ret = uri2.toString();
	}
	catch (...) {
		LOG_ERROR("except %s domain(%s)", url.c_str(), domain.c_str());
	}
	return ret;
}


void CLinksMng::test()
{
	//string strTest = "https://www.cnblogs.com/Albert992/";
	string strTest = "http://www.miibeian.gov.cn:2002/test/ok/index.html";
	parseUrl(strTest, "www.dswd.net");
	cout << strTest << endl;
	cout << hostname_port << endl;
	cout << getFullDomain() << endl;
	cout << getPath() << endl;
	cout << getLocation() << endl;
	cout << getRelative() << endl;
	cout << getIndex() << endl;
};

