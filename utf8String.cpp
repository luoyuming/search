#include "utf8String.h"
#include "log.h"

iutf8string::iutf8string(const std::string& str)
{
	data = str;
	refresh();
}

iutf8string::iutf8string(const std::string &&str)
{
	data = str;
	refresh();
}

iutf8string::iutf8string(const char* str)
{
	data = string(str);
	refresh();
}

iutf8string::~iutf8string()
{
	
}

string iutf8string::stlstring()
{
	return data;
}

const char* iutf8string::c_str()
{
	return data.c_str();
}

iutf8string iutf8string::operator +(iutf8string& ustr)
{
	string temp = data + ustr.stlstring();

	return iutf8string(temp);
}

int iutf8string::length()
{
	return _length;
}

string iutf8string::get(int index)
{
	if ((index >= _length) || (index < 0))
		return "";	

	return characters[index];
}

string iutf8string::operator [](int index)
{
	if ((index >= _length) || (index < 0))
		return "";
	return characters[index];
}

void iutf8string::extracdStrByPos(string & str, std::set<int> & setPos)
{
	str.clear();
	for (auto & index : setPos) {	
		str += get(index);
	}
	return;
}

bool iutf8string::getReverseWord(std::set<int> & setPos, int offset, int num, bool bNeedEmpty)
{
	if (offset <= 0) {
		return false;
	}

	auto it = mpPos.find(offset);
	if (mpPos.end() == it)
		return false;

	int u8_start_index = it->second;
	for (int i = 0; i < num; i++) {
		--u8_start_index;
		if (u8_start_index < 0) {
			break;
		}		
		if (!bNeedEmpty) {
			do {
				if (!isSpace(u8_start_index)) {
					setPos.insert(u8_start_index--);
					break;
				}
				u8_start_index--;
			} while (u8_start_index > 0);
		}
		else {
			setPos.insert(u8_start_index--);
		}
	}

	return true;
}

bool iutf8string::getWordPos(std::set<int> & setPos, int offset, int num, bool bNeedEmpty)
{	
	int u8_start_index = 0;
	if (0 < offset) {
		auto it = mpPos.find(offset);
		if (mpPos.end() == it)
			return false;
		u8_start_index = it->second;
	}	
	//LOG_INFO("%d  num(%d) _length(%d)", u8_start_index, num, _length);
	for (int i = 0; i < num; i++) {
		if (u8_start_index >= _length) {
			//LOG_INFO("%d  _length(%d)", u8_start_index, _length);
			break;
		}
		if (!bNeedEmpty) {
			do {
				if (!isSpace(u8_start_index)) {
					setPos.insert(u8_start_index++);
					break;
				}
				u8_start_index++;
			} while (u8_start_index < _length);
		}
		else {
			setPos.insert(u8_start_index++);
		}
	}
	//LOG_INFO("%d   _length(%d)", setPos.size(), _length);
	return true;
}

bool iutf8string::isSpace(int index)
{	
	string strTemp = get(index);
	if (" " == strTemp) {
		return true;
	}
	return false;
}

bool iutf8string::getStrByOffset(string & str, int offset, int num)
{	
	auto it = mpPos.find(offset);
	if (mpPos.end() == it)
		return false;
		
	str = substr(it->second, num);
	return true;
}

std::string iutf8string::substrEx(int u8start_index, int u8_length)
{
	int max = 10000;
	string str;
	int index = u8start_index;
	for (int i = 0; i < u8_length; i++) {
		do {
			if (!isSpace(index)) {
				str += get(index++);				
				break;
			}
			index++;
			if (index > max) {				
				break;
			}
		} while (index < _length);
	}
	return str;		
}

string iutf8string::substr(int u8_start_index, int u8_length)
{
	string str;
	for (int i = 0; i < u8_length; i++) {
		str += get(u8_start_index+i);
	}
	return str;	
}

iutf8string iutf8string::utf8substr(int u8_start_index, int u8_length)
{	
	return iutf8string(substr(u8_start_index, u8_length));
}

void iutf8string::refresh()
{	
	_length = 0;
	string & word = data;
	int num = word.size();
	int i = 0;
	int size = 0;
	while (i < num)
	{
		size = 1;
		if (word[i] & 0x80)
		{
			char temp = word[i];
			temp <<= 1;
			do {
				temp <<= 1;
				++size;
			} while (temp & 0x80);
		}		
		mpPos[i] = _length++;		
		characters.push_back(word.substr(i, size));
		i += size;
	}	
	return;	
}

void iutf8string::testSample()
{
	cout << data << endl;
	cout << "num:" << _length << " size:" << data.size() << endl;
}


void utfStringEx::input(const string & word)
{
	int num = word.size();
	int i = 0;
	while (i < num)
	{
		int size = 1;
		if (word[i] & 0x80)
		{
			char temp = word[i];
			temp <<= 1;
			do {
				temp <<= 1;
				++size;
			} while (temp & 0x80);
		}
		string subWord;
		subWord = word.substr(i, size);
		characters.push_back(subWord);
		i += size;
	}
}