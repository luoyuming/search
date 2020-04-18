#pragma once
#ifndef M_iconv_hpp
#define M_iconv_hpp
#include "common.h"
#include <algorithm>
#include <cerrno>
#include <exception>
#include <string>
#include <iconv.h>

enum class conv_type {
	gbk_to_utf8 = 1,
	utf8_to_gbk,
	big5_to_utf8,
	utf8_to_big5,
	gb18030_to_utf8,
	utf8_to_gb18030,
} ;

class CConvertStr {
	
public:
	CConvertStr(conv_type type);
	~CConvertStr();
	bool codeConvert(std::string & srcStr);
private:
	const char* type_from(const conv_type type);
	const char* type_to(const conv_type type);

private:
	iconv_t cd;
	bool bFault;
};


#endif //M_iconv_hpp
