#include "handleXml.h"
#include "util.h"
using namespace std;


CHandleXml::CHandleXml()
{

   
}

void CHandleXml::Quit()
{

}

bool CHandleXml::readXmlFile(string & strFilename)
{
	std::lock_guard<std::mutex>lck(m_lock);	
	xml_system tempXml;
	m_xml_system = tempXml;

	auto mydoc(UTIL_SELF::make_unique<TiXmlDocument>());
	bool loadOk = mydoc->LoadFile(strFilename.c_str());
	if (!loadOk)
	{
		LOG_ERROR("Error:%s", mydoc->ErrorDesc());
		return false;
	}

	TiXmlElement *RootElement = mydoc->RootElement();      //根元素  
	TiXmlElement *pEle = NULL;

	string strNodeName = XML_NODE_NAME;
	GetNodePointerByName(RootElement, strNodeName.c_str(), pEle);   //找到值为SearchModes的结点  

	for (TiXmlElement *SearchModeElement = pEle->FirstChildElement(); SearchModeElement; SearchModeElement = SearchModeElement->NextSiblingElement())
	{
		vector<string> vecXml;
		//输出子元素的值  
		for (TiXmlElement *RegExElement = SearchModeElement->FirstChildElement(); RegExElement; RegExElement = RegExElement->NextSiblingElement())
		{
			string strValue = RegExElement->FirstChild()->Value();

			vecXml.push_back(strValue);

			string key = RegExElement->Value();
			LOG_INFO("xml: %s : %s", key.c_str(), strValue.c_str());
		}
		xml_system & data = m_xml_system;
		if (serialSystemXml(data, vecXml))
		{	
			checkRule();
			return true;
		}

		
	}
	return false;
}


bool CHandleXml::readDefaultXml()
{
	if (m_strPath.empty())
		m_strPath = UTIL_SELF::getPwd();	
	string strFilename = m_strPath + XML_FILE_SYSTEM;
	//LOG_INFO("%s ", strFilename.c_str());

	return readXmlFile(strFilename);
}

bool CHandleXml::serialSystemXml(xml_system & data, vector<string> & vecInfo)
{
	bool bResult = true;
	int total = static_cast<int>(vecInfo.size());
	int i = 0;
	do {
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.host, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.port, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.thread_serach, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.thread_html, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.thread_download, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.search_mode, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.prevPage, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.nextPage, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.spider_interval, vecInfo, i, total));
		VALUE_BREAK_IF(UTIL_SELF::setValue(data.hotWord, vecInfo, i, total));

		VALUE_BREAK_IF(UTIL_SELF::setValue(data.test, vecInfo, i, total));
	} while (0);

	return bResult;
}

void CHandleXml::checkRule()
{
	string & strHost = m_xml_system.host;
	string::size_type size = strHost.size();
	if (size > 0) {
		if ('/' == strHost[size - 1]) {
			strHost.erase(size - 1, 1);
		}
	}
	return;
}

xml_system CHandleXml::getSystemXml()
{
	std::lock_guard<std::mutex>lck(m_lock);
	return m_xml_system;
}

int CHandleXml::getPort()
{
	std::lock_guard<std::mutex>lck(m_lock);
	return std::atoi(m_xml_system.port.c_str());
}

bool CHandleXml::onlySearchMode()
{
	bool bResult = false;
	std::lock_guard<std::mutex>lck(m_lock);
	if (SEARCH_MODE_RUN == m_xml_system.search_mode) {
		bResult = true;
	}
	return bResult;
}

string CHandleXml::getPahtEx()
{
	std::lock_guard<std::mutex>lck(m_lock);
	auto pos = m_strPath.rfind("/");
	if (string::npos != pos) {		
		return (m_strPath.substr(0, pos));
	}
	return m_strPath;
}

string CHandleXml::getPath()
{
	std::lock_guard<std::mutex>lck(m_lock);
	return m_strPath;
}

int CHandleXml::getThreadSearchNum()
{
	std::lock_guard<std::mutex>lck(m_lock);
	return std::atoi(m_xml_system.thread_serach.c_str());
}

int CHandleXml::getThreadHtmlNum()
{
	std::lock_guard<std::mutex>lck(m_lock);
	return std::atoi(m_xml_system.thread_html.c_str());
}

int CHandleXml::getThreadDownloadNum()
{
	std::lock_guard<std::mutex>lck(m_lock);
	return std::atoi(m_xml_system.thread_download.c_str());
}

bool CHandleXml::GetNodePointerByName(TiXmlElement* pRootEle, const char* strNodeName, TiXmlElement* &destNode)
{
	if (0 == strcmp(strNodeName, pRootEle->Value()))
	{
		destNode = pRootEle;
		return true;
	}

	TiXmlElement* pEle = pRootEle;
	for (pEle = pRootEle->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())
	{
		// 递归处理子节点，获取节点指针      
		if (0 != strcmp(pEle->Value(), strNodeName))
		{
			GetNodePointerByName(pEle, strNodeName, destNode);
		}
		else
		{
			destNode = pEle;
			printf("destination node name: %s\n", pEle->Value());
			return true;
		}
	}
	return false;
}





