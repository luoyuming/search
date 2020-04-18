#pragma once
#include "common.h"
#include <zlib.h>

class CGzip {

public:
	CGzip();
	~CGzip();
	bool codeGzip(string & dest, string & src);
	bool decodeGzip(string & dest, string & src);
	bool transferConentEncode(string & strData, string head);
private:
	int gzcompress(Bytef *data, uLong ndata, Bytef  *zdata, uLong *nzdata);
	int gzdecompress(Bytef *zdata, uLong nzdata, Bytef *data, uLong *ndata);
	
private:
	vector<string> m_vecRNLN;
};