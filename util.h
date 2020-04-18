#pragma once
#include "common.h"


namespace UTIL_SELF {
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
#define VALUE_BREAK_IF(cond) if(cond) break
	inline bool setValue(string & value, vector<string> & vecInfo, int & i, int max)
	{
		if ((i >= max) && (!vecInfo.empty()))
		{
			return true;
		}
		value = vecInfo[i++];
		return false;
	}

#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()


	class Timer {
public:
	Timer() : m_begin(high_resolution_clock::now()) {}
	void reset() { m_begin = high_resolution_clock::now(); }	
	int64_t elapsed() const
	{
		return duration_cast<chrono::milliseconds>(high_resolution_clock::now() - m_begin).count();
	}	
	int64_t elapsed_micro() const
	{
		return duration_cast<chrono::microseconds>(high_resolution_clock::now() - m_begin).count();
	}	
	int64_t elapsed_nano() const
	{
		return duration_cast<chrono::nanoseconds>(high_resolution_clock::now() - m_begin).count();
	}	
	int64_t elapsed_seconds() const
	{
		return duration_cast<chrono::seconds>(high_resolution_clock::now() - m_begin).count();
	}	
	int64_t elapsed_minutes() const
	{
		return duration_cast<chrono::minutes>(high_resolution_clock::now() - m_begin).count();
	}	
	int64_t elapsed_hours() const
	{
		return duration_cast<chrono::hours>(high_resolution_clock::now() - m_begin).count();
	}
private:
	time_point<high_resolution_clock> m_begin;
};
	void replaceUtfSpace(string & strInfo, char flag);	
	void eraseUtfSpace(string & strInfo);
	void eraseMoreSpace(string & strInfo);
	void joinStr(string & strInfo, vector<string> & vecInfo);
	string::size_type getVectorLen(vector<string> & vecInfo);
	std::string chToHex(unsigned char ch);
	std::string strToHex(const std::string str, std::string separator = "");
	void trimstr(string & s);
	string trim(string & str);
	void trimNoSpace(string & s);
	void splitByChar(std::vector<std::string> & vecTokens, const std::string& s, char delimiter);
	void splitAll(const string& src, vector<string>& res, const string& pattern, size_t maxsplit = string::npos);	
	int split(const string& src, vector<string>& res, string pattern);
	void extratSubStr(string & str, string flag);
	void prevSubStr(string & strRet, const string & strInput, string & flag, string::size_type & pos);
	string replace(const string & str, const string & src, const string & dest);
	void replaceEx(string & str, string flag, string & value, bool once = false);
	void getGmtTime(string & strTime);	
	void eraseStr(string & strData, string flag);
	void eraseMoreStr(string & strData, char flag);
	bool existFile(string & filename, int & size);
	bool readFile(string & filename, string & strData);
	bool isExistFile(string & filename);
	bool existStr(string & info, string flag);
	string buildPath(string url, string & strFirst, string & random);
	string getPwd();
	void saveFile(string & strFilename, string & strData);
	void appFile(string & strFilename, string & strData);
	int rmFile(std::string & file_name);
	string getExtName(string extention);
	string getThreadID();
	unsigned char ToHex(unsigned char x);
	unsigned char FromHex(unsigned char x);
	std::string UrlEncode(const std::string& str);
	std::string UrlDecode(const std::string& str);
	void Base64Encode(string & strEncode, const unsigned char* Data, int DataByte);
	void Base64Decode(string &  strDecode, const char* Data, int DataByte);
	int MakeDir(const char* dirname);
	int SetNonblocking(int sock);
    bool is_letters(const string& str);
	bool is_chinese(const string& str);
	bool is_engAndNum(const string& str);
	bool is_allNum(const string& str);
	bool is_allNumAndDot(const string& str);
	string randomStr(int num = 10);
	string getRandomValue();
	int GetUtf8LetterNumber(string & str);
	int GetUtf8Word(const char *s, int wantedNum, int & offset, int end);
	int getUtfByteNum(char ch);
	std::string utf8SubStr(const std::string & name, size_t need, int star);
	std::string utf8SubStrReverse(std::string & name, size_t need, int star);
}


class CCheckTime {

public:
	CCheckTime();
	bool checkTime();
	void beginTime();
	void endTime();
private:
	int  m_year;
};