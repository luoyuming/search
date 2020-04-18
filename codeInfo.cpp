
#include "codeInfo.h"

#define B0(a) (a & 0xFF)
#define B1(a) (a >> 8 & 0xFF)
#define B2(a) (a >> 16 & 0xFF)
#define B3(a) (a >> 24 & 0xFF)

char CCodeInfo::GetB64Char(int index)
{
    const char szBase64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (index >= 0 && index < 64)
        return szBase64Table[index];
    
    return '=';
}

int CCodeInfo::Base64Encode(char * base64code, const char * src, int src_len) 
{   
    int i = 0;
    if (src_len == 0)
        src_len = strlen(src);
    
    int len = 0;
    unsigned char* psrc = (unsigned char*)src;
    char * p64 = base64code;
    for (i = 0; i < src_len - 3; i += 3)
    {
        unsigned long ulTmp = *(unsigned long*)psrc;
        register int b0 = GetB64Char((B0(ulTmp) >> 2) & 0x3F); 
        register int b1 = GetB64Char((B0(ulTmp) << 6 >> 2 | B1(ulTmp) >> 4) & 0x3F); 
        register int b2 = GetB64Char((B1(ulTmp) << 4 >> 2 | B2(ulTmp) >> 6) & 0x3F); 
        register int b3 = GetB64Char((B2(ulTmp) << 2 >> 2) & 0x3F); 
        *((unsigned long*)p64) = b0 | b1 << 8 | b2 << 16 | b3 << 24;
        len += 4;
        p64  += 4; 
        psrc += 3;
    }
    
    
    if (i < src_len)
    {
        int rest = src_len - i;
        unsigned long ulTmp = 0;
        for (int j = 0; j < rest; ++j)
        {
            *(((unsigned char*)&ulTmp) + j) = *psrc++;
        }
        
        p64[0] = GetB64Char((B0(ulTmp) >> 2) & 0x3F); 
        p64[1] = GetB64Char((B0(ulTmp) << 6 >> 2 | B1(ulTmp) >> 4) & 0x3F); 
        p64[2] = rest > 1 ? GetB64Char((B1(ulTmp) << 4 >> 2 | B2(ulTmp) >> 6) & 0x3F) : '='; 
        p64[3] = rest > 2 ? GetB64Char((B2(ulTmp) << 2 >> 2) & 0x3F) : '='; 
        p64 += 4; 
        len += 4;
    }
    
    *p64 = '\0'; 
    
    return len;
}


int CCodeInfo::GetB64Index(char ch)
{
    int index = -1;
    if (ch >= 'A' && ch <= 'Z')
    {
        index = ch - 'A';
    }
    else if (ch >= 'a' && ch <= 'z')
    {
        index = ch - 'a' + 26;
    }
    else if (ch >= '0' && ch <= '9')
    {
        index = ch - '0' + 52;
    }
    else if (ch == '+')
    {
        index = 62;
    }
    else if (ch == '/')
    {
        index = 63;
    }

    return index;
}


int CCodeInfo::Base64Decode(char * buf, const char * base64code, int src_len) 
{ 
    int i = 0;
    if (src_len == 0)
        src_len = strlen(base64code);

    int len = 0;
    unsigned char* psrc = (unsigned char*)base64code;
    char * pbuf = buf;
    for (i = 0; i < src_len - 4; i += 4)
    {
        unsigned long ulTmp = *(unsigned long*)psrc;
        
        register int b0 = (GetB64Index((char)B0(ulTmp)) << 2 | GetB64Index((char)B1(ulTmp)) << 2 >> 6) & 0xFF;
        register int b1 = (GetB64Index((char)B1(ulTmp)) << 4 | GetB64Index((char)B2(ulTmp)) << 2 >> 4) & 0xFF;
        register int b2 = (GetB64Index((char)B2(ulTmp)) << 6 | GetB64Index((char)B3(ulTmp)) << 2 >> 2) & 0xFF;
        
        *((unsigned long*)pbuf) = b0 | b1 << 8 | b2 << 16;
        psrc  += 4; 
        pbuf += 3;
        len += 3;
    }
    
    if (i < src_len)
    {
        int rest = src_len - i;
        unsigned long ulTmp = 0;
        for (int j = 0; j < rest; ++j)
        {
            *(((unsigned char*)&ulTmp) + j) = *psrc++;
        }
        
        register int b0 = (GetB64Index((char)B0(ulTmp)) << 2 | GetB64Index((char)B1(ulTmp)) << 2 >> 6) & 0xFF;
        *pbuf++ = b0;
        len  ++;

        if ('=' != B1(ulTmp) && '=' != B2(ulTmp))
        {
            register int b1 = (GetB64Index((char)B1(ulTmp)) << 4 | GetB64Index((char)B2(ulTmp)) << 2 >> 4) & 0xFF;
            *pbuf++ = b1;
            len  ++;
        }
        
        if ('=' != B2(ulTmp) && '=' != B3(ulTmp))
        {
            register int b2 = (GetB64Index((char)B2(ulTmp)) << 6 | GetB64Index((char)B3(ulTmp)) << 2 >> 2) & 0xFF;
            *pbuf++ = b2;
            len  ++;
        }

    }
        
    *pbuf = '\0'; 
    
    return len;
} 

int CCodeInfo::GetCodeInfo(string & strDest, string & strSrc)
{
	strDest.clear();
	if (strSrc.empty())
		return 0;
	int nLen = static_cast<int>(strSrc.size());
	unsigned char* pSrc = reinterpret_cast<unsigned char*>(&strSrc[0]);
	Encode(strDest, pSrc, nLen);   
    return 0;
}

int CCodeInfo::GetDecodeInfo(string & strDest, string & strSrc)
{
	strDest.clear();
	if (strSrc.empty()) {
		return 0;
	}
	int nLen = static_cast<int>(strSrc.size());	
	Decode(strDest, strSrc.data(), nLen);	
    return 0;
}



void CCodeInfo::Encode(string & strEncode, const unsigned char* Data, int DataByte)
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

void CCodeInfo::Decode(string &  strDecode, const char* Data, int DataByte)
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


	int nValue;
	int i = 0;
	while (i < DataByte)
	{
		if (*Data != '\r' && *Data != '\n')
		{
			nValue = DecodeTable[int(*Data++)] << 18;
			nValue += DecodeTable[int(*Data++)] << 12;
			strDecode += (nValue & 0x00FF0000) >> 16;			
			if (*Data != '=')
			{
				nValue += DecodeTable[int(*Data++)] << 6;
				strDecode += (nValue & 0x0000FF00) >> 8;				
				if (*Data != '=')
				{
					nValue += DecodeTable[int(*Data++)];
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
