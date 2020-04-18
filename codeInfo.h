#ifndef CODE_INFO_H
#define CODE_INFO_H
#include "common.h"



class CCodeInfo {
public:
     int GetDecodeInfo(string & strDest, string & strSrc);
     int GetCodeInfo(string & strDest, string & strSrc);

private:
     int Base64Encode(char * base64code, const char * src, int src_len = 0); 
     int Base64Decode(char * buf, const char * base64code, int src_len = 0);
     char GetB64Char(int index);
     int GetB64Index(char ch);


	 /*编码
	 DataByte
	 [in]输入的数据长度,以字节为单位
	 */
	 void Encode(string & strEncode, const unsigned char* Data, int DataByte);
	 /*解码
	 DataByte
	 [in]输入的数据长度,以字节为单位
	 OutByte
	 [out]输出的数据长度,以字节为单位,请不要通过返回值计算
	 输出数据的长度
	 */
	 void Decode(string &  strDecode, const char* Data, int DataByte);
};



#endif