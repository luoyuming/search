#include "cookieMng.h"
#include "util.h"
#include "log.h"


static const std::string CRLN = "\r\n";
CCookieMng::CCookieMng()
{
	
}


void CCookieMng::Quit()
{

}



void CCookieMng::extractCookie(const string & url, string & strData)
{	
	auto linksMng = std::make_shared<CLinksMng>();
	linksMng->parseUrl(url);
	string strDomain = linksMng->getFullDomain();
	string strSub;
	string demiliter;
	vector<string> vecValue;
	vector<string> vecDic;
	std::string key = "set-cookie:";
	std::string::size_type pos = 0;
	do {
		pos = strData.find(key, pos);
		if (string::npos != pos)
		{
			pos += key.size();
			string::size_type next = strData.find_first_of(CRLN, pos);
			if (string::npos != next)
			{				
				strSub = strData.substr(pos, next - pos);
				UTIL_SELF::trim(strSub);				
				vecValue.clear();
				demiliter = ";";
				UTIL_SELF::split(strSub, vecValue, demiliter);
				for (auto & item : vecValue) {
					auto cookie = std::make_shared<CookieValue>();
					demiliter = "=";
					vecDic.clear();
					UTIL_SELF::split(item, vecDic, demiliter);
					if (vecDic.size() >= 2) {
						cookie->key = vecDic[0];
						cookie->value = vecDic[1];
						UTIL_SELF::trim(cookie->key);
						UTIL_SELF::trim(cookie->value);
						setCookie(strDomain, cookie);
					}
				}				
			}
		}
		else
		{
			break;
		}
	} while (true);
  
	return;
}

void CCookieMng::setCookie(string & strDomain, shared_ptr<CookieValue> & cookie)
{	
	if ((!cookie->value.empty()) 
		&& (!strDomain.empty())
		&& (!cookie->key.empty())
		){
		//LOG_INFO("%s(%s=%s)", strDomain.c_str(),cookie->key.c_str(), cookie->value.c_str());
		{
			lock_guard<mutex> guard(m_lock);
			auto it = m_mpCookie.find(strDomain);
            if (m_mpCookie.end() == it) {				
                m_mpCookie[strDomain] = std::set<std::pair<string, string>>();
				m_mpCookie[strDomain].clear();
			}			
			m_mpCookie[strDomain].insert(std::make_pair(cookie->key, cookie->value));
		}
	}
	return;
}

bool CCookieMng::getCookie(const string & url, string & cookie)
{
	auto linksMng = std::make_shared<CLinksMng>();
	linksMng->parseUrl(url);
	string strDomain = linksMng->getFullDomain();
    bool bResult = false;
    cookie.clear();
  
    {
        lock_guard<mutex> guard(m_lock);
        auto it = m_mpCookie.find(strDomain);
        if (m_mpCookie.end() != it) {
            for (auto & item : it->second) {
                bResult = true;
                if (cookie.empty()) {
                    cookie = "Cookie: ";
                    cookie += item.first;
                    cookie += "=";
                    cookie += item.second;                  
                }
                else {
                    cookie += "; ";
                    cookie += item.first;
                    cookie += "=";
                    cookie += item.second;
                }
            }
        }
    }
    return bResult;
}