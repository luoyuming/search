#include "fileMng.h"
#include "util.h"


CFileMng::CFileMng()
{

}


void CFileMng::Quit()
{

}

void CFileMng::init() 
{

}

void CFileMng::put(string & ip)
{
	bool bAdd = false;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		auto it = m_mpVisitInfo.find(ip);
		if (m_mpVisitInfo.end() != it) {
			it->second->num += 1;
		}
		else {
			m_mpVisitInfo[ip] = std::make_shared<VISIT_INFO>();
			m_mpVisitInfo[ip]->ip = ip;
			bAdd = true;
		}
	}
	
	if (bAdd) {	
		
		string strInfo;
		{
			std::lock_guard<std::mutex>lck(m_lock);
			for (auto & item : m_mpVisitInfo) {
				if (!strInfo.empty()) {
					strInfo += "\n";
				}
				strInfo += item.first;
				strInfo += " \t";
				strInfo += std::to_string(item.second->num);
				strInfo += " \t";
				strInfo += asctime(gmtime(&(item.second->begin_time)));
			}
		}
		string FILE_VISIT_IP = "ip_visit.txt";
		{
			std::lock_guard<std::mutex>lck(m_lock);
			UTIL_SELF::saveFile(FILE_VISIT_IP, strInfo);
		}
	}
}

void CFileMng::reset()
{
	std::lock_guard<std::mutex>lck(m_lock);
	m_mpVisitInfo.clear();
	std::map<string, std::shared_ptr<VISIT_INFO>>().swap(m_mpVisitInfo);
}

void CFileMng::get(vector<VISIT_INFO> & vecInfo)
{
	std::lock_guard<std::mutex>lck(m_lock);
	for (auto & item : m_mpVisitInfo) {
		vecInfo.push_back(*(item.second));
	}
	return;
}