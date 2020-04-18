#include "adMng.h"
#include "log.h"


CAdMng::CAdMng()
{

}


void CAdMng::Quit()
{

}

bool CAdMng::init(string & strPath)
{
	string AD_FILE = "dict/ad.utf8";
	string strFilename = strPath + AD_FILE;

	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {
		LOG_ERROR("error to open %s", strFilename.c_str());
		return false;
	}

	AD_INFO adInfo;
	vector<string> vecInfo;
	string line;
	while (getline(ifs, line)) {
		UTIL_SELF::trim(line);		
		UTIL_SELF::split(line, vecInfo, "|");
		if (vecInfo.size() >= 3) {
			//cout << vecInfo[0] << " * " << vecInfo[1] << " * " << vecInfo[2] << " * " << endl;
			adInfo.ad_type = std::atoi(vecInfo[2].c_str());
			adInfo.view = vecInfo[1];
			adInfo.url = vecInfo[0];
			m_vecAdInfo.push_back(adInfo);
		}
		vecInfo.clear();
		line.clear();
	}
	return true;
}

string CAdMng::createStr(string prev, string mid, string last)
{
	return (prev + mid + last);
}

void CAdMng::fillAd(string & strInfo)
{
	//<a href="$(qyt_ref_1)">$(qyt_view_1)</a>
	//string flag = "$(qyt_ref_1)"  $(qyt_view_1)
	string strTemp;
	std::map<int, string> mpRef;
	std::map<int, string> mpView;
	for (int i = 1; i <= 6; i++) {
		string strNum = std::to_string(i);
		strTemp = createStr("$(qyt_ref_", strNum, ")");
		mpRef[i]= strTemp;
		strTemp = createStr("$(qyt_view_", strNum, ")");
		mpView[i] = strTemp;
	}

	std::vector<AD_INFO> vecAdInfo;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		vecAdInfo = m_vecAdInfo;
	
	}
	if (vecAdInfo.empty()) {
		LOG_ERROR("ad information is empty");
		return;
	}

	AD_INFO adInfo = vecAdInfo[0];
	int size = static_cast<int>(vecAdInfo.size());
	if (size < 6) {
		for (int i = 0; i < (6 - size); i++) {
			vecAdInfo.push_back(adInfo);
		}
	}
	for (int i = 1; i <= 6; i++) {
		int pos = i - 1;
		UTIL_SELF::replaceEx(strInfo, mpRef[i], vecAdInfo[pos].url);
		UTIL_SELF::replaceEx(strInfo, mpView[i], vecAdInfo[pos].view);
		//cout << mpRef[i] << "" << vecAdInfo[pos].url << endl;
		//cout << mpView[i] << "" << vecAdInfo[pos].view << endl;

	}
	return;
}