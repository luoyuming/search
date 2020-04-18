#include "util.h"
#include "fastcgi.h"
#include "fcgi.h"
#include "log.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

static const int PARAMS_BUFF_LEN = 2048;  //环境参数buffer的大小
static const int CONTENT_BUFF_LEN = 2048; //内容buffer的大小

void FastCgi_init(FastCgi_t *c)
{
    c->sockfd_ = 0;    //与php-fpm 建立的 sockfd
    c->flag_ = 0;      //record 里的请求ID
    c->requestId_ = 0; //用来标志当前读取内容是否为html内容
}

void FastCgi_finit(FastCgi_t *c)
{
    close(c->sockfd_);
}

void setRequestId(FastCgi_t *c, int requestId)
{
    c->requestId_ = requestId;
}

void makeHeader(FCGI_Header *ptrHead, int type, int requestId,
                       int contentLength, int paddingLength)
{
    FCGI_Header & header = *ptrHead;

    header.version = FCGI_VERSION_1;

    header.type = (unsigned char)type;

    /* 两个字段保存请求ID */
    header.requestIdB1 = (unsigned char)((requestId >> 8) & 0xff);
    header.requestIdB0 = (unsigned char)(requestId & 0xff);

    /* 两个字段保存内容长度 */
    header.contentLengthB1 = (unsigned char)((contentLength >> 8) & 0xff);
    header.contentLengthB0 = (unsigned char)(contentLength & 0xff);

    /* 填充字节的长度 */
    header.paddingLength = (unsigned char)paddingLength;

    /* 保存字节赋为 0 */
    header.reserved = 0;

    return;
}

FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection)
{
    FCGI_BeginRequestBody body;

    /* 两个字节保存期望 php-fpm 扮演的角色 */
    body.roleB1 = (unsigned char)((role >> 8) & 0xff);
    body.roleB0 = (unsigned char)(role & 0xff);

    /* 大于0长连接，否则短连接 */
    body.flags = (unsigned char)((keepConnection) ? FCGI_KEEP_CONN : 0);

    
	memset(&body.reserved, 0, sizeof(body.reserved));

    return body;
}

int makeNameValueBody(const char *name, int nameLen,
	const char *value, int valueLen,                   
	unsigned char *bodyBuffPtr, int *bodyLenPtr)
{
    /* 记录 body 的开始位置 */
    unsigned char *startBodyBuffPtr = bodyBuffPtr;

    /* 如果 nameLen 小于128字节 */
    if (nameLen < 128)
    {
        *bodyBuffPtr++ = (unsigned char)nameLen; //nameLen用1个字节保存
    }
    else
    {
        /* nameLen 用 4 个字节保存 */
        *bodyBuffPtr++ = (unsigned char)((nameLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char)(nameLen >> 16);
        *bodyBuffPtr++ = (unsigned char)(nameLen >> 8);
        *bodyBuffPtr++ = (unsigned char)nameLen;
    }

    /* valueLen 小于 128 就用一个字节保存 */
    if (valueLen < 128)
    {
        *bodyBuffPtr++ = (unsigned char)valueLen;
    }
    else
    {
        /* valueLen 用 4 个字节保存 */
        *bodyBuffPtr++ = (unsigned char)((valueLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char)(valueLen >> 16);
        *bodyBuffPtr++ = (unsigned char)(valueLen >> 8);
        *bodyBuffPtr++ = (unsigned char)valueLen;
    }

    /* 将 name 中的字节逐一加入body中的buffer中 */
    for (size_t i = 0; i < strlen(name); i++)
    {
        *bodyBuffPtr++ = name[i];
    }

    /* 将 value 中的值逐一加入body中的buffer中 */
    for (size_t i = 0; i < strlen(value); i++)
    {
        *bodyBuffPtr++ = value[i];
    }

    /* 计算出 body 的长度 */
    *bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;
    return 1;
}

/*
 * 如果有配置文件的话，可以将一些信息，比如IP 从配置文件里读出来
 *
char *getIpFromConf()
{
    return getMessageFromFile("IP");
}
*/

void startConnect(FastCgi_t *c)
{
    int rc;
    int sockfd;
    struct sockaddr_in server_address;

    /* 固定 */
    const char *ip = "127.0.0.1";

    /* 获取配置文件中的ip地址 */
    //ip = getIpFromConf();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd > 0);

	memset(&server_address, 0, sizeof(server_address));


    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(9000);

    rc = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    if(rc >= 0) {
		printf("sockfd-----%d\n", sockfd);
	}
	else {
			printf("error connect\n");
	}
    c->sockfd_ = sockfd;
}
int sendStartRequestRecord(FastCgi_t *c)
{
    int rc;
    FCGI_BeginRequestRecord beginRecord;


	makeHeader(&(beginRecord.header), FCGI_BEGIN_REQUEST, c->requestId_, sizeof(beginRecord.body), 0);
    beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

    rc = cgi_write(c->sockfd_, &beginRecord, sizeof(beginRecord)); 
    return rc;
}

int cgi_write(int fd, void *buffer, int length)
{
	int bytes_left;
	int written_bytes;
	char *ptr = (char *)buffer;
	bytes_left = length;
	while (bytes_left>0)
	{

		written_bytes = write(fd, ptr, bytes_left);
		if (written_bytes <= 0)
		{
			if (errno == EINTR)
				written_bytes = 0;
			else
				return(-1);
		}
		bytes_left -= written_bytes;
		ptr += written_bytes;
	}
	return (0);
}

int cgi_read(int fd, void *buffer, int length)
{
	int bytes_left = 0;
	int bytes_read = 0;

	char *ptr = (char *)buffer;
	bytes_left = length;
	while (bytes_left>0)
	{
		bytes_read = read(fd, ptr, bytes_left);
		if (bytes_read <= 0)
		{
			if (errno == EINTR)
				bytes_read = 0;
			else
				return(-1);
		}	
		bytes_left -= bytes_read;
		ptr += bytes_read;
	}
	return(length - bytes_left);
}



int sendParams(FastCgi_t *c, const  char *name, const char *value)
{
    int rc = 0;

	string strBuff;
	strBuff.resize(PARAMS_BUFF_LEN);  
	unsigned char *bodyBuff = reinterpret_cast<unsigned char*>(&strBuff[0]);
	memset(bodyBuff, 0, PARAMS_BUFF_LEN);

    /* 保存 body 的长度 */
    int bodyLen;

    /* 生成 PARAMS 参数内容的 body */
    makeNameValueBody(name, strlen(name), value, strlen(value), bodyBuff, &bodyLen);

    FCGI_Header nameValueHeader;
    makeHeader(&nameValueHeader, FCGI_PARAMS, c->requestId_, bodyLen, 0);
    /*8 字节的消息头*/

    int nameValueRecordLen = bodyLen + FCGI_HEADER_LEN;
	string strTemp;
	strTemp.resize(nameValueRecordLen);
    char *nameValueRecord = reinterpret_cast<char*>(&strTemp[0]);

    /* 将头和body拷贝到一块buffer 中只需调用一次write */
    memcpy(nameValueRecord, (char *)&nameValueHeader, FCGI_HEADER_LEN);
    memcpy(nameValueRecord + FCGI_HEADER_LEN, bodyBuff, bodyLen);

    rc = cgi_write(c->sockfd_, nameValueRecord, nameValueRecordLen); 
    return rc;
}

int sendEndRequestRecord(FastCgi_t *c)
{
    int rc;
    FCGI_Header endHeader;
    makeHeader(&endHeader, FCGI_PARAMS, c->requestId_, 0, 0);
    rc = cgi_write(c->sockfd_, (char *)&endHeader, FCGI_HEADER_LEN);
    return rc;
}

bool readFromPhp(FastCgi_t *c, string & strResp)
{
	FCGI_Header responderHeader;	
	strResp.clear();
	int contentLen;
	char tmp[8]; //用来暂存padding字节
	int ret;

	/* 先将头部 8 个字节读出来 */
	while (cgi_read(c->sockfd_, &responderHeader, FCGI_HEADER_LEN) > 0)
	{		
		if (responderHeader.type == FCGI_STDOUT)
		{
			/* 获取内容长度 */
			contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
			if (contentLen > 0) {
				
				string strTemp;
				strTemp.resize(contentLen);
				char *content = reinterpret_cast<char *>(&strTemp[0]);
				/* 读取获取内容 */
				ret = cgi_read(c->sockfd_, content, contentLen);
				if (ret < 0) {
					LOG_ERROR("FCGI_STDOUT");
					return false;
				}

				strResp += strTemp;
				/* 跳过填充部分 */
				if (responderHeader.paddingLength > 0)
				{
					ret = cgi_read(c->sockfd_, tmp, responderHeader.paddingLength);
					if (ret < 0) {
						LOG_ERROR("FCGI_STDOUT--");
						return false;
					}
				}
			}
		} //end of type FCGI_STDOUT
		else if (responderHeader.type == FCGI_STDERR)
		{
			contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
			if (contentLen > 0) {

				string strTemp;
				strTemp.resize(contentLen);
				char *content = reinterpret_cast<char *>(&strTemp[0]);

				ret = cgi_read(c->sockfd_, content, contentLen);
				if (ret < 0) {
					LOG_ERROR("FCGI_STDERR");
					return false;
				}		

				strResp += strTemp;
				/* 跳过填充部分 */
				if (responderHeader.paddingLength > 0)
				{
					ret = cgi_read(c->sockfd_, tmp, responderHeader.paddingLength);
					if (ret < 0) {
						LOG_ERROR("FCGI_STDERR--");
						return false;
					}
				}
			}
		} // end of type FCGI_STDERR
		else if (responderHeader.type == FCGI_END_REQUEST)
		{
			FCGI_EndRequestBody endRequest;

			ret = cgi_read(c->sockfd_, &endRequest, sizeof(endRequest));
			if (ret < 0) {
				LOG_ERROR("FCGI_END_REQUEST");
				return false;
			}
			LOG_INFO("FCGI_END_REQUEST protocolStatus = %d ", endRequest.protocolStatus);
		}
	}

	//LOG_INFO("%s", strResp.c_str());

    return true;
}

char *findStartHtml(char *p)
{
    enum
    {
        S_NOPE,
        S_CR,
        S_CRLF,
        S_CRLFCR,
        S_CRLFCRLF
    } state = S_NOPE;

    for (char *content = p; *content != '\0'; content++) //状态机
    {
        switch (state)
        {
        case S_NOPE:
            if (*content == '\r')
                state = S_CR;
            break;
        case S_CR:
            state = (*content == '\n') ? S_CRLF : S_NOPE;
            break;
        case S_CRLF:
            state = (*content == '\r') ? S_CRLFCR : S_NOPE;
            break;
        case S_CRLFCR:
            state = (*content == '\n') ? S_CRLFCRLF : S_NOPE;
            break;
        case S_CRLFCRLF:
            return content;
        }
    }
    // fprintf(stderr, "%%%%%%%%%%RETURN NULL!!!!!\n");
    return p;
}
void getHtmlFromContent(FastCgi_t *c, char *content)
{
    /* 保存html内容开始位置 */
    char *pt;

    /* 读取到的content是html内容 */
    if (c->flag_ == 1)
    {
        printf("%s", content);
    }
    else
    {
        if ((pt = findStartHtml(content)) != NULL)
        {
            c->flag_ = 1;
            for (char *i = pt; *i != '\0'; i++)
            {
                printf("%c", *i);
            }
        }
    }
}