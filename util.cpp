#include "util.h"


namespace UTIL_SELF {

	void replaceUtfSpace(string & strInfo, char flag)
	{
		char one = 0xc2;
		char two = 0xa0;
		char space = 0x20;
		int len = static_cast<int>(strInfo.size());
		for (int i = 0; i < len; i++) {
			if ((i + 1) >= len) {
				break;
			}
			if ((one == strInfo[i]) && (two == strInfo[i + 1])) {
				strInfo[i] = flag;
				strInfo[i + 1] = space;
			}
		}
		return;
	}	

	void eraseUtfSpace(string & strInfo)
	{
		char one = 0xc2;
		char two = 0xa0;		
		char space = 0x20;	
		int len = static_cast<int>(strInfo.size());	
		for (int i = 0; i < len; i++) {			
			if ((i + 1) >= len) {
				break;
			}
			if ((one == strInfo[i]) && (two == strInfo[i + 1])) {
				strInfo[i] = space;
				strInfo[i+1] = space;
			}
		}
		return;
	}

	void eraseMoreSpace(string & strInfo)
	{
		char spaceFlag = 32;
		for (auto & value : strInfo) {
			if ((value == 9) || (value == 13) || (value == 10))
			{
				value = spaceFlag;
			}
		}

		bool bFind = false;
		auto it = strInfo.begin();
		for (; it != strInfo.end(); ) {
			if ((*it) == spaceFlag) {
				if (bFind) {
					it = strInfo.erase(it);
				}
				else {
					bFind = true;
					++it;
				}
			}
			else {
				bFind = false;
				++it;
			}
		}
		return;
	}

	std::string chToHex(unsigned char ch)
	{
		const std::string hex = "0123456789ABCDEF";

		std::stringstream ss;
		ss << hex[ch >> 4] << hex[ch & 0xf];

		return ss.str();
	}

	std::string strToHex(const std::string str, std::string separator)
	{
		const std::string hex = "0123456789ABCDEF";
		std::stringstream ss;

		for (std::string::size_type i = 0; i < str.size(); ++i)
			ss << hex[(unsigned char)str[i] >> 4] << hex[(unsigned char)str[i] & 0xf] << separator;

		return ss.str();
	}

	std::string utf8SubStrReverse(std::string & name, size_t need, int star)
	{
		size_t i = 0;
		size_t j = star-1;
		while (i<need && j >= 0) {
			unsigned char c = (unsigned char)name[j--];
			i += ((c & 0xc0) != 0x80);
		}

		while (j >= 0) {
			unsigned char c = (unsigned char)name[j];
			if ((c & 0xc0) == 0x80) {
				j--;
			}
			else {
				break;
			}
		}
		
		if (j < 0) {
			j = 0;
		}	
		
		return name.substr(j, star-j);
	}


	std::string utf8SubStr(const std::string & name, size_t need, int star)
	{
		size_t i = 0;
		size_t j = star;
		while (i<need && j<name.length()) {
			unsigned char c = (unsigned char)name[j++];
			i += ((c & 0xc0) != 0x80);
		}

		while (j < name.length()) {
			unsigned char c = (unsigned char)name[j];
			if ((c & 0xc0) == 0x80) {
				j++;
			}
			else {
				break;
			}
		}
		if (j > name.length())
			j = name.length();

		return name.substr(star, j- star);
	}
	
	int getUtfByteNum(char ch)
	{
		int nCode = (unsigned char)ch;
		if (nCode < 128)
			return 1;
		else if (nCode >= 192 && nCode <= 223)
			return 2;
		else if (nCode >= 224 && nCode <= 239)
			return 3;
		else if (nCode >= 240 && nCode <= 247)
			return 4;
		else if (nCode >= 248 && nCode <= 251)
			return 5;
		else if (nCode >= 252 && nCode <= 253)
			return 6;
		else
			return 7;
	}


	int GetUtf8Word(const char *s, int wantedNum, int & offset, int end)
	{
		int i = 0, j = 0;
		int readedNum = 0;
		int isReach = 0;
		while (s[i])
		{
			if ((s[i] & 0xc0) != 0x80)
			{
				if (isReach)
				{					
					break;
				}
				++j;
				readedNum = j;
				if (j == wantedNum)
				{
					isReach = 1;
				}
			}

			++i;
			if (i >= end) {
				break;
			}
		}
		offset = i;
		return readedNum;
	}


	int GetUtf8LetterNumber(string & str)
	{		
		int size = static_cast<int>(str.size());
		int i = 0, j = 0;
		if (!str.empty()) {
			const char *s = str.data();
			while (s[i])
			{
				if ((s[i] & 0xc0) != 0x80)
					j++;
				i++;
				if (i >= size) {
					break;
				}
			}
		}
		return j;
	}

	string getRandomValue()
	{
		int value = (int)time(nullptr);
		string strRet = std::to_string(value);
		strRet += randomStr(15);
		return strRet;
	}

	string randomStr(int num)
	{
		static string strId = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz123456789";
		srand(static_cast<unsigned int >(time(nullptr)));
		random_shuffle(strId.begin(), strId.end());
		return strId.substr(0, num);
	}

	bool is_allNumAndDot(const string& str)
	{		
		int len = static_cast<int>(str.length());
		for (int i = 0; i < len; i++) {

			if (str[i] >= '0' && str[i] <= '9') {

			}
			else if ((str[i] != '.') && (str[i] != ',')){
				return false;
			}
		}
		return true;
	}

	bool is_allNum(const string& str)
	{
		int len = static_cast<int>(str.length());
		for (int i = 0; i < len; i++) {

			if (str[i] >= '0' && str[i] <= '9') {

			}
			else {
				return false;
			}
		}
		return true;
	}
	bool is_engAndNum(const string & str)
	{
		bool bResult = true;
		for (auto & cValue : str) {			
			//if (isalnum(cValue) || ('+' == cValue)|| ('#' == cValue)|| ('*' == cValue)) 
			if (isascii(cValue))
			{
				
			}
			else {
				bResult = false;
				break;
			}
		}		
		return bResult;
	}

    bool is_letters(const string& str)
    {
        bool bResult = false;
        for (const auto & cValue : str) {
            if ( ((cValue >= 'a') && (cValue <= 'z'))
                || ((cValue >= 'A') && (cValue <= 'Z'))
                ) {
                bResult = true;                
            }
            else {
                bResult = false;
                break;
            }
        }
        return bResult;
    }

	bool is_chinese(const string& str)
	{		
		int len = static_cast<int>(str.length());		
		if (len < 3) {
			return false;
		}	

		unsigned char utf[3] = { 0 };
		unsigned char unicode[2] = { 0 };
		for (int i = 0; i < len; i++) {			
			if ((str[i] & 0x80) == 0x80) {
				if ((i + 2) >= len) {
					return false;
				}				
				utf[0] = str[i];
				utf[1] = str[i + 1];
				utf[2] = str[i + 2];
				i++;
				i++;
				unicode[0] = ((utf[0] & 0x0F) << 4) | ((utf[1] & 0x3C) >> 2);
				unicode[1] = ((utf[1] & 0x03) << 6) | (utf[2] & 0x3F);
				//printf("[\\u4e00-\\u9fa5] unicode(%x,%x)\n", unicode[0], unicode[1]);
				//   [\u4e00-\u9fa5]
				if(unicode[0] >= 0x4e && unicode[0] <= 0x9f) {			
					if ((unicode[0] == 0x9f) && (unicode[1] > 0xa5))
					{

					}
					else {
						return true;
					}
				}
			}			
		}
		return false;
	}

	int SetNonblocking(int sock)
	{
		int opts = 0;
		opts = fcntl(sock, F_GETFL);
		if (opts < 0)
		{
			return -1;
		}

		opts |= O_NONBLOCK;
		if (fcntl(sock, F_SETFL, opts) < 0)
		{
			return -1;
		}
		return 0;
	}

	void trimstr(string & s)
	{
		s.erase(s.find_last_not_of(" \n\r\t") + 1);
		s.erase(0, s.find_first_not_of(" \n\r\t"));
		return ;
	}	

	void trimNoSpace(string & s)
	{
		s.erase(s.find_last_not_of("\n\r\t") + 1);
		s.erase(0, s.find_first_not_of("\n\r\t"));
		return;
	}

	string trim(string & str)
	{
		string & s = str;
		s.erase(s.find_last_not_of(" \n\r\t") + 1);
		s.erase(0, s.find_first_not_of(" \n\r\t"));
		return s;		
	}

	void splitByChar(std::vector<std::string> & vecTokens, const std::string& s, char delimiter)
	{		
		std::string token;
		std::istringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			vecTokens.push_back(token);
		}
		return;
	}

	void extratSubStr(string & str, string flag)
	{
		string::size_type next = str.size();
		string::size_type pos = str.find(flag);
		if (string::npos != pos) {
			str.erase(pos, next-pos);
		}
		return;
	}

	void prevSubStr(string & strRet, const string & strInput, string & flag, string::size_type & pos)
	{		
		string::size_type next = strInput.find(flag, pos);
		if (string::npos == next) {
			next = strInput.size();
		}
		strRet = strInput.substr(pos, next - pos);
		pos = next;
		pos += flag.size();
	}

	int split(const string& src, vector<string>& res, string pattern)
	{
		res.clear();
		size_t Start = 0;
		size_t end = 0;
		string sub;
		while (Start < src.size()) {
			end = src.find_first_of(pattern, Start);
			if (string::npos == end || res.size() >= string::npos) {
				sub = src.substr(Start);
				if (!sub.empty()) {
					end = sub.find_first_of(pattern, 0);
					if (string::npos == end) {
						res.push_back(sub);
					}
				}
				return 0;
			}
			sub = src.substr(Start, end - Start);
			if (!sub.empty())
				res.push_back(sub);
			Start = end + 1;
		}
		return 0;
	}

	void splitAll(const string& src, vector<string>& res, const string& pattern, size_t maxsplit) {
		res.clear();
		size_t Start = 0;
		size_t end = 0;
		string sub;
		while (Start < src.size()) {
			end = src.find_first_of(pattern, Start);
			if (string::npos == end || res.size() >= maxsplit) {
				sub = src.substr(Start);
				res.push_back(sub);
				return;
			}
			sub = src.substr(Start, end - Start);
			res.push_back(sub);
			Start = end + 1;
		}
		return;
	}

	
	string::size_type getVectorLen(vector<string> & vecInfo)
	{
		string::size_type nRet = 0;
		for (auto & str : vecInfo) {
			nRet += str.size();
		}
		return nRet;
	}

	void joinStr(string & strInfo, vector<string> & vecInfo)
	{		
		string::size_type len = getVectorLen(vecInfo);
		if (len <= 0)
			return;

		strInfo.clear();
		strInfo.resize(len);
		string::size_type i = 0;
		for (auto & str : vecInfo) {
			memcpy(&strInfo[i], str.data(), str.size());
			i += str.size();			
		}
		return;
	}

	void replaceEx(string & str, string flag, string & value, bool once) {
		string::size_type pos = 0;
		do {
			pos = str.find(flag, pos);
			if (string::npos == pos)
				return;
			str.replace(pos, flag.size(), value);	
			if (once) {
				break;
			}
		} while (true);
		return;
	}


	string replace(const string & str, const string & src, const string & dest)
	{
		string ret;

		string::size_type pos_begin = 0;
		string::size_type pos = str.find(src);
		while (pos != string::npos)
		{
			//cout << "replacexxx:" << pos_begin << " " << pos << "\n";
			ret.append(str.data() + pos_begin, pos - pos_begin);
			ret += dest;
			pos_begin = pos + 1;
			pos = str.find(src, pos_begin);
		}
		if (pos_begin < str.length())
		{
			ret.append(str.begin() + pos_begin, str.end());
		}
		return ret;
	}

	void getGmtTime(string & strTime)
	{
		time_t rawTime;
		struct tm* timeInfo;
		char szTemp[40] = { 0 };

		time(&rawTime);
		timeInfo = localtime(&rawTime);
		strftime(szTemp, sizeof(szTemp), "%a, %d %b %Y %H:%M:%S", timeInfo);	
		strTime = szTemp;
	}

	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}

	string getThreadID()
	{
		std::ostringstream oss;
		oss << std::this_thread::get_id();		
		return oss.str();
	}

	string getExtName(string extention)
	{
		string str = extention.erase(0, extention.find_last_of('.') + 1);
		transform(str.begin(), str.end(), str.begin(), (int(*)(int))tolower);
		return str;
	}

	int rmFile(std::string & file_name)
	{
		std::string file_path = file_name;
		struct stat st;
		if (lstat(file_path.c_str(), &st) == -1)
		{
			return -1;
		}
		if (S_ISREG(st.st_mode))
		{
			if (unlink(file_path.c_str()) == -1)
			{
				return -1;
			}
		}

		return 0;
	}

	void appFile(string & strFilename, string & strData)
	{
		ofstream ouF;
		ouF.open(strFilename.c_str(), std::ios::binary| std::ios::app);
		ouF.write(strData.data(), strData.size());
		ouF.flush();
		ouF.close();
	}

	void saveFile(string & strFilename, string & strData)
	{
		ofstream ouF;
		ouF.open(strFilename.c_str(), std::ios::binary);
		ouF.write(strData.data(), strData.size());
		ouF.flush();
		ouF.close();
	}

	bool isExistFile(string & filename)
	{
		bool bResult = true;
		fstream _file;
		_file.open(filename.c_str(), ios::in | ios::binary);
		if (!_file)
		{
			bResult = false;
		}
		return bResult;
	}

	std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	std::string UrlDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') 
				strTemp += ' ';
			else if (str[i] == '%')
			{
				assert(i + 2 < length);
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}

	string buildPath(string url, string & strFirst, string & random)
	{
		string path = "download/";
		if (!strFirst.empty()) {
			path += strFirst;
			path += "/";
		}
		strFirst = path;
		MakeDir(path.c_str());	
		random = getRandomValue();
		path += random;
		path += ".html";

		strFirst += random;
		strFirst += ".txt";
		
		return path;
	}

	bool existStr(string & info, string flag) 
	{
		bool bFind = false;
		string::size_type pos = info.rfind(flag);
		if (string::npos != pos) {
			bFind = true;			
		}
		return bFind;
	}

	void eraseMoreStr(string & strData, char flag)
	{
		int size = static_cast<int>(strData.size());
		int i = 0;
		for (auto & cValue : strData){
			i++;
			if ((flag == cValue) && (i < size)){				
				if (flag == strData[i]) {
					strData[i] = 0x20;
				}
			}
		}
		return;
	}

	void eraseStr(string & strData, string flag)
	{
		string::size_type pos = 0;
		do {
			pos = strData.find(flag, pos);
			if (string::npos == pos) {
				break;
			}
			strData.erase(pos, flag.size());
		} while (true);
		return;
	}

	bool existFile(string & filename, int & size)
	{
		long l, m;
		ifstream file(filename, ios::in | ios::binary);
		if (!file.is_open()) {
			return  false;
		}
		l = file.tellg();
		file.seekg(0, ios::end);
		m = file.tellg();
		file.close();
		size = m - l;
		return true;
	}

	bool readFile(string & filename, string & strData)
	{
		bool bResul = false;
		long size;
		ifstream file(filename, ios::in|ios::binary|ios::ate);
		if (!file.is_open()) {
			return  bResul;
		}
		size = file.tellg();
		if (size > 0) {
			file.seekg(0, ios::beg);
			strData.resize(size);
			char *buffer = reinterpret_cast<char *>(&strData[0]);
			file.read(buffer, size);

			bResul = true;
		}
		file.close();
		return bResul;
	}

	string getPwd()
	{
		string strPath;		
		char  path1[1024] = { 0 };
		if (nullptr == getcwd(path1, sizeof(path1)))
		{
			strcpy(path1, "/");
		}
		else
		{
			strcat(path1, "/");
		}
		strPath = path1;
		return strPath.c_str();
	}


	void Base64Encode(string & strEncode, const unsigned char* Data, int DataByte)
	{
		strEncode.clear();
		//编码表
		const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		unsigned char Tmp[4] = { 0 };
		//int LineLength = 0;
		for (int i = 0; i<(int)(DataByte / 3); i++)
		{
			Tmp[1] = *Data++;
			Tmp[2] = *Data++;
			Tmp[3] = *Data++;
			strEncode += EncodeTable[Tmp[1] >> 2];
			strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
			strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
			strEncode += EncodeTable[Tmp[3] & 0x3F];

			//if(LineLength += 4,LineLength == 76)
			//{
			//strEncode+="\r\n";
			//LineLength=0;
			//}
		}
		//对剩余数据进行编码
		int Mod = DataByte % 3;
		if (Mod == 1)
		{
			Tmp[1] = *Data++;
			strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
			strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
			strEncode += "==";
		}
		else if (Mod == 2)
		{
			Tmp[1] = *Data++;
			Tmp[2] = *Data++;
			strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
			strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
			strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
			strEncode += "=";
		}

		return;
	}

	void Base64Decode(string &  strDecode, const char* Data, int DataByte)
	{
		//解码表
		const char DecodeTable[] =
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			62, // '+'
			0, 0, 0,
			63, // '/'
			52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
			0, 0, 0, 0, 0, 0, 0,
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
			13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
			0, 0, 0, 0, 0, 0,
			26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
			39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
		};

		unsigned char Tmp[4] = { 0 };
		int nValue;
		int i = 0;
		while (i < DataByte)
		{
			
			if (*Data != '\r' && *Data != '\n')
			{
				Tmp[0] = *Data++;
				Tmp[1] = *Data++;

				nValue = DecodeTable[Tmp[0]] << 18;
				nValue += DecodeTable[Tmp[1]] << 12;
				strDecode += (nValue & 0x00FF0000) >> 16;
				if (*Data != '=')
				{
					Tmp[2] = *Data++;

					nValue += DecodeTable[Tmp[2]] << 6;
					strDecode += (nValue & 0x0000FF00) >> 8;
					if (*Data != '=')
					{
						Tmp[3] = *Data++;

						nValue += DecodeTable[Tmp[3]];
						strDecode += nValue & 0x000000FF;
					}
				}
				i += 4;
			}
			else// 回车换行,跳过
			{
				Data++;
				i++;
			}
		}
		return;
	}

	int MakeDir(const char* dirname)
	{
		int res;
		char path[512] = { 0 };
		memcpy(path, dirname, strlen(dirname));
		char* pEnd = strstr(path + 1, "/");
		if (NULL == pEnd)
		{
			res = mkdir(path, 0777);
		}
		while (pEnd)
		{
			*pEnd = 0;
			res = mkdir(path, 0777);
			*pEnd = '/';
			if (NULL == strstr(pEnd + 1, "/"))
			{
				if ('\0' != *(pEnd + 1))
				{
					mkdir(path, 0777);
				}
				break;
			}
			else
			{
				pEnd = strstr(pEnd + 1, "/");
			}
		}

		if (-1 == res)
		{
			return -1;
		}
		return 0;
	}

}



CCheckTime::CCheckTime()
{
	m_year = 0;
}
bool CCheckTime::checkTime()
{
	bool bResult = true;
//#ifdef  LINUX_PLATFORM
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep); //取得当地时间
	if (115 == p->tm_year)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(800));
		time(&timep);
		p = localtime(&timep); 
		if (115 == p->tm_year)
		{
			return true;
		}
	}

	//LogInstance->setYear(p->tm_year);

	time_t utc_t = mktime(p);
	tv.tv_sec = utc_t;
	tv.tv_usec = 0;
	if (settimeofday(&tv, &tz) < 0)
	{
		bResult = false;
	}
//#endif
	return bResult;
}

void CCheckTime::beginTime()
{
//#ifdef  LINUX_PLATFORM
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	int max_loop = 2;
	int num = 0;
	bool bLoop = false;
	time_t timep;
	struct tm *p = nullptr;
	do {
		bLoop = false;
		time(&timep);
		p = localtime(&timep); //取得当地时间 
		if (115 == p->tm_year)
		{
			bLoop = true;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			//cout << "LOOP...LOOP" << endl;
		}
		num++;
		if (num > max_loop)
		{
			break;
		}
	} while (bLoop);

	//cout << "ok....ok..." << endl;

	m_year = p->tm_year;
	if (115 == m_year)
	{
		return;
	}

	p->tm_year = 115;
	time_t utc_t = mktime(p);
	tv.tv_sec = utc_t;
	tv.tv_usec = 0;
	if (settimeofday(&tv, &tz) < 0)
	{
		cout << "error to settimeofday" << endl;
	}
//#endif
}

void CCheckTime::endTime()
{
//#ifdef  LINUX_PLATFORM
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep); //取得当地时间  

	if (115 == m_year)
	{
		return;
	}

	p->tm_year = m_year;
	time_t utc_t = mktime(p);
	tv.tv_sec = utc_t;
	tv.tv_usec = 0;
	if (settimeofday(&tv, &tz) < 0)
	{
		cout << "error to settimeofday" << endl;
	}

	//cout << "end....end..." << endl;
//#endif
}