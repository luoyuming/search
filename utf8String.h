#pragma once
#include "common.h"


class iutf8string
{
public:
	
	iutf8string(const std::string &str);
	iutf8string(const std::string &&str);
	iutf8string(const char*str);
	~iutf8string();

public:
	int length();	
	std::string get(int index);	
	iutf8string operator + (iutf8string&str);
	std::string operator [](int index);
	std::string stlstring();
	const char* c_str();
	iutf8string utf8substr(int u8start_index, int u8_length);
	std::string substr(int u8start_index, int u8_length);
	std::string substrEx(int u8start_index, int u8_length);
	
	bool getStrByOffset(string & str, int offset, int num);
	bool getWordPos(std::set<int> & setPos, int offset, int num, bool bNeedEmpty =false);
	bool getReverseWord(std::set<int> & setPos, int offset, int num, bool bNeedEmpty = false);
	void extracdStrByPos(string & str, std::set<int> & setPos);
	void testSample();
private:
	bool isSpace(int index);
	void refresh();
private:
	std::map<int, int> mpPos;  //offset-->×ÖÎ»ÖÃ
	std::string data;	
	int _length;	
	vector<string> characters;
	
};

class utfStringEx {
public:
	void input(const string & word);

public:
	vector<string> characters;
	string m_data;
};
