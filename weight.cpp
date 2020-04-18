#include "weight.h"
#include "log.h"
#include "util.h"
#include "linksMng.h"

CWeightMng::CWeightMng()
{
	
}


void CWeightMng::Quit()
{
	
}

bool CWeightMng::init(string & strPath)
{	
	return true;
}

void CWeightMng::put(string & domain, CLASS_INFO & info)
{
	std::lock_guard<std::mutex>lck(m_lock);
	m_mpDomanScore[domain] = info;
}

bool CWeightMng::get(const string & url, CLASS_INFO & info)
{

	bool bFind = false;
	string::size_type pos = 0;
	info.reset();
	CLinksMng links;
	links.parseUrl(url);
	string domain = links.getFullDomain();
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto it = m_mpDomanScore.find(domain);
		if (m_mpDomanScore.end() != it) {		
			pos = url.find(it->second.subUrl);
			if (string::npos != pos) {
				info = it->second;
                bFind = true;			
			}			
		}
	}   
    return bFind;
}

bool CWeightMng::exist(const string & url)
{
	bool bResult = false;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto it = m_mpDomanScore.find(url);
		if (m_mpDomanScore.end() != it) {			
			bResult = true;
		}
	}
	return bResult;
}

