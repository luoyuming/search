#include "keywordDict.h"
#include "log.h"
#include "util.h"

void CKeywordDict::initUserDict(string & strCurPath)
{
	string strFilename = strCurPath + KEYWORD_DICT_FILE;
	LOG_INFO("%s", strFilename.c_str());

	ifstream ifs(strFilename.c_str());
	if (!ifs.is_open()) {

		return;
	}
	
	string line;
	while (getline(ifs, line)) {
		UTIL_SELF::trimstr(line);		
		if (string::npos != line.find(FLAG_ASTERISK)) {
			eraseFlag(line, FLAG_ASTERISK);
			insertDict(m_liDigitDict, line);
			
		}		
		else  {			
			insertDict(m_liUserDict, line);
		}
		
		line.clear();
	}
   
	/*for (auto & word : m_liUserDict) {
		cout << " " << word << endl;
	}
	for (auto & word : m_liDigitDict) {
		cout << "* " << word << endl;
	}
	for (auto & word : m_liDotDict) {
		cout << "# " << word << endl;
	}*/
	return;
}

void CKeywordDict::eraseFlag(string & line, string flag)
{
	string::size_type  pos = line.find(flag);
	if (string::npos != pos) {
		pos += flag.size();
		line.erase(0, pos);
	}
	return;
}

void CKeywordDict::insertDict(std::list<string> & liDict, string & word)
{
	bool bInset = false;	
	string::size_type size = static_cast<int>(word.size());
	auto it = liDict.begin();
	auto itEnd = liDict.end();
	for (; it != itEnd; ++it) {
		if (size > it->size()) {
			liDict.insert(it, word);
			bInset = true;
			break;
		}
	}
	if (!bInset) {
		liDict.push_back(word);
	}
	return;
}

void CKeywordDict::getUserKeyword(vector<cppjieba::Word> & vccWord, const string & strData)
{
	std::list<string> userDict;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		userDict = m_liUserDict;
	}
	for (auto & keyword : userDict) {		
		string::size_type pos = 0;
		do {
			pos = strData.find(keyword, pos);
			if (string::npos == pos) {
				
				break;
			}	
			vccWord.emplace_back(cppjieba::Word(keyword, (uint32_t)pos));			
			pos += keyword.size();
		} while (true);		
	}

	std::list<string> digitDict;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		digitDict = m_liDigitDict;
	}
	for (auto & keyword : digitDict) {
		string::size_type pos = 0;
		do {
			pos = strData.find(keyword, pos);
			if (string::npos == pos) {

				break;
			}
			bool add = false;
			bool bFind = false;
			int i = (int) pos;
			while (i >= 0) {
				i--;				
				if ((strData[i] >= '0' && strData[i] <= '9') 
					|| ('.'== strData[i]) 
					|| (',' == strData[i])

					){
					i--;
					bFind = true;
					if (i < 0) {
						add = true;
					}
				}
				else {
					if (bFind) {
						add = true;
					}
				}
				if (add) {
					i++;
					int len = pos - i + keyword.size();
					string strTemp = strData.substr(i, len);
					vccWord.emplace_back(cppjieba::Word(strTemp, (uint32_t)i));					
					break;
				}							
			} //while (i >= 0) 			
			pos += keyword.size();
		} while (true);
		
	}
	
	//cout << strData << endl;
	//for (auto & item : vccWord) {
	//	cout << item.word << ":" << item.offset <<" "<< item.word .size()<< endl;
	//}
	

	return;
}