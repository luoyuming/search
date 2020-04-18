#include "needMng.h"
#include "log.h"
#include "util.h"
#include "linksMng.h"
#include "weight.h"

CNeedMng::CNeedMng()
{

}


void CNeedMng::Quit()
{

}

bool CNeedMng::init(string & strPath)
{
	bool bResult = readIpRange(strPath);
	if (bResult) {
		bResult = readNeed(strPath);
	}
	return bResult;
}

void CNeedMng::getDomain(vector<string> & vecDomain)
{
	std::lock_guard<std::mutex> lck(m_lock);
	for (auto & item : m_setDomain) {
		vecDomain.push_back(item);
	}
	m_setDomain.clear();
	std::set<string>().swap(m_setDomain);
	return;
}

bool CNeedMng::readNeed(string & strPath)
{
	string strFilename = strPath + FILE_NAME_NEED;
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}
	CLinksMng links;	
	NEED_INFO info;	
	string strTemp;
	vector<string> vecLine;
	while(ifs)
	{
		
		info.weight = 0;
		info.className.clear();
		info.classValue.clear();

		strTemp.clear();
		vecLine.clear();
		getline(ifs, strTemp);		
		getSplit(vecLine, strTemp);
		if (vecLine.size() >= 3) {
			UTIL_SELF::trimstr(vecLine[0]);			
			info.url = vecLine[0];	
			if (!info.url.empty())
			{		
                CLASS_INFO clsInfo;
				links.reset();
				string subUrl = vecLine[1];
                vector<string> vecSub;
                UTIL_SELF::split(subUrl, vecSub, "|");  
                if (vecSub.size() >= 2) {
                    subUrl = vecSub[0];                    
                }
				clsInfo.subUrl = subUrl;

				string url = links.parseUrl(info.url);
				info.domainName = links.getFullDomain();
				info.firstName = links.getFirstDomain();
				info.location = links.getLocation();	
				info.path = links.getPath();

				m_setDomain.insert(info.firstName);

				strTemp = vecLine[2];
				//cout << vecLine[2] << endl;
							
				vecLine.clear();
				UTIL_SELF::split(strTemp, vecLine, "|");
				if (vecLine.size() > 2) {
					UTIL_SELF::trim(vecLine[0]);
					UTIL_SELF::trim(vecLine[1]);					
					UTIL_SELF::trimNoSpace(vecLine[2]);	

                    string & strRef = vecLine[2];
                    transform(strRef.begin(), strRef.end(), strRef.begin(), (int(*)(int))tolower);

					clsInfo.weight = atoi(vecLine[0].c_str());
					clsInfo.className = vecLine[1];
					clsInfo.classValue = vecLine[2];
                   
					
					CWeightMngS->put(info.domainName, clsInfo);
					info.className = clsInfo.className;
					info.classValue = clsInfo.classValue;
					info.weight = clsInfo.weight; 
                    info.subUrl = clsInfo.subUrl;

					//LOG_INFO("(%s)(%d)(%s)=(%s)", subUrl.c_str(), clsInfo.weight, clsInfo.className.c_str(), clsInfo.classValue.c_str());
					//LOG_INFO("domain(%s) url(%s)", info.domainName.c_str(), info.url.c_str());
					{
						std::lock_guard<std::mutex> lck(m_lock);
						m_liNeedUrl.push_back(info);
						
					}
				}
			}					
		}
	}	
	//LOG_INFO("finish to read nedd file(%d)", m_liNeedUrl.size());
	//return false;
	return true;
}



void CNeedMng::getSplit(vector<string> & vecInfo, string & strInfo)
{
	string strTemp;
	string::size_type pos = 0;
	string::size_type next = 0;
	string flag = " \t";
	
	int i = 0;

	for (; i < 3; i++) {
		next = strInfo.find_first_not_of(flag, pos);
		if (string::npos == next) {		
			break;
		}
		pos = next;
		next = strInfo.find_first_of(flag, pos);
		if ((string::npos == next)|| (2==i)) {
			next = strInfo.size();
		}		
		strTemp = strInfo.substr(pos, next - pos);
		vecInfo.push_back(strTemp);
		pos = next;			
	}
	return;
}

bool CNeedMng::readIpRange(string & strPath)
{
	string strFilename = strPath + FILE_IP_RANGE;
	//LOG_INFO("%s", strFilename.c_str());
	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}
	struct in_addr addr1;
	struct in_addr addr2;
	string ipBegin;
	string ipEnd;
	IP_RANGE_INFO ipRange;
	while (ifs >> ipBegin >> ipEnd) {
		UTIL_SELF::trimstr(ipEnd);
		UTIL_SELF::trimstr(ipBegin);
		if (ipBegin.empty())
			break;
		if ((0 != inet_aton(ipBegin.c_str(), &addr1))
			&& (0 != inet_aton(ipEnd.c_str(), &addr2))
			) {
			ipRange.begin_addr = addr1.s_addr;
			ipRange.end_addr = addr2.s_addr;
			//LOG_INFO("--%s(%llu)---%s(%llu)--", ipBegin.c_str(), ipRange.begin_addr, ipEnd.c_str(), ipRange.end_addr);
			{
				std::lock_guard<std::mutex> lck(m_lock);
				m_liIpRange.push_back(ipRange);
			}
		}
		
	}
	return true;	
}

void CNeedMng::inputNeed(DOWNLOAD_URL_INFO & info)
{
	bool bFind = false;
	auto needEx = std::make_shared<NEED_INFO_EX>();

	{
		std::lock_guard<std::mutex> lck(m_lock);
		for (auto & item : m_liNeedUrl) {
			if (info.url == item.url) {
				bFind = true;
				break;
			}
		}
		if (!bFind) {
			html2need(info, needEx->need);
			m_liNeedBak.push_back(needEx);
		}
	}
	return;
}

void CNeedMng::html2need(DOWNLOAD_URL_INFO & info, NEED_INFO & need)
{
	need.domainName = info.domainName;
	need.firstName = info.firstName;	
	need.location = info.location;
	need.url = info.url;
	need.path = info.path;
	need.className = info.className;
	need.classValue = info.classValue;
	need.weight = info.weight;
    need.subUrl = info.subUrl;
}

void CNeedMng::need2html(NEED_INFO & need, DOWNLOAD_URL_INFO & info)
{
	info.domainName = need.domainName;
	info.firstName = need.firstName;	
	info.location = need.location;	
	info.url = need.url;
	info.path = need.path;	
	info.className = need.className;
	info.classValue = need.classValue;
	info.weight = need.weight;
    info.subUrl = need.subUrl;
}

bool CNeedMng::getNeedEx(NEED_INFO & need)
{
	bool bResult = false;
	auto it = m_liNeedBak.begin();
	for (; it != m_liNeedBak.end(); ++it) {
		if ((*it)->timer.elapsed_minutes() > 10) {
			m_liNeedBak.erase(it);
			bResult = true;
			break;
		}
	}
	return bResult;
}

bool CNeedMng::getNeedWebSite(DOWNLOAD_URL_INFO & info)
{
	bool bResult = false;
	std::lock_guard<std::mutex> lck(m_lock);
	int size = static_cast<int>(m_liNeedUrl.size());
	if (size > 0) {
		auto it = m_liNeedUrl.begin();
		need2html(*(it), info);		
		m_liNeedUrl.pop_front();		
		bResult = true;		
	}
	else if (m_liNeedBak.size() > 0){
		NEED_INFO need;
		if (getNeedEx(need)) {

			need2html(need, info);
			bResult = true;
		}
	}
	return bResult;
}

bool CNeedMng::isCnDomain(string & strDomain)
{	
	struct hostent* host = nullptr;	
	{
		std::lock_guard<std::mutex> lck(m_lock);
		host = gethostbyname(strDomain.c_str());	
		if (nullptr == host) {			
			LOG_WARNING("no to find %s", strDomain.c_str());
			return false;
		}
		//LOG_INFO("Address type: %s", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
		for (int i = 0; host->h_addr_list[i]; i++) {
			char *ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[i]);
			if (nullptr != ip) {
				string strIP = ip;			
				struct in_addr addr1;
				if (0 != inet_aton(strIP.c_str(), &addr1)) {
					uint64_t ipValue = addr1.s_addr;										
					for (auto & item : m_liIpRange) {
						if ((ipValue >= item.begin_addr) && (ipValue <= item.end_addr)) {							
							return true;
						}
					}					
				}
			}
		}
	}

	return false;
}