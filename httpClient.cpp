#include "httpClient.h"
#include "log.h"
#include "util.h"
#include "iconvCode.h"
#include "gzipCode.h"
#include "html.h"
#include "brotliMng.h"
#include "needMng.h"
#include "linksMng.h"
#include "cookieMng.h"

/*
#define USE_OPENSSL 
#include <stdio.h>  
#include <pthread.h>  
#include <curl/curl.h>  



static pthread_mutex_t *lockarray;

#ifdef USE_OPENSSL
#include <openssl/crypto.h>

static unsigned long thread_id(void);
static void lock_callback(int mode, int type, char *file, int line);

static void lock_callback(int mode, int type, char *file, int line)
{
	(void)file;
	(void)line;
	if (mode & CRYPTO_LOCK) {
		pthread_mutex_lock(&(lockarray[type]));
	}
	else {
		pthread_mutex_unlock(&(lockarray[type]));
	}
}

static unsigned long thread_id(void)
{
	unsigned long ret;

	ret = (unsigned long)pthread_self();
	return(ret);
}

static void init_locks(void)
{
	int i;

	lockarray = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
		sizeof(pthread_mutex_t));
	for (i = 0; i<CRYPTO_num_locks(); i++) {
		pthread_mutex_init(&(lockarray[i]), NULL);
	}

	CRYPTO_set_id_callback((unsigned long(*)())thread_id);
	CRYPTO_set_locking_callback((void(*)())lock_callback);
}

static void kill_locks(void)
{
	int i;

	CRYPTO_set_locking_callback(NULL);
	for (i = 0; i<CRYPTO_num_locks(); i++)
		pthread_mutex_destroy(&(lockarray[i]));

	OPENSSL_free(lockarray);
}
#endif

#ifdef USE_GNUTLS
#include <gcrypt.h>
#include <errno.h>

GCRY_THREAD_OPTION_PTHREAD_IMPL;

void init_locks(void)
{
	gcry_control(GCRYCTL_SET_THREAD_CBS);
}

#define kill_locks()
#endif
*/

static const int HTTP_CODE_OK = 200;
static const int HTTP_CODE_MT = 302;  //Moved Temporarily
static const int HTTP_CODE_MP = 301;  //Moved Permanently

static const int MAX_LOOP_NUM = 10;
static const int LOW_SPEED_TIME = 10; //网速小于MAX_SPEED_LIMIT 且时间超过当前设定 断开
static const int LOW_SPEED_LIMIT = 50; //50 BYTE/S
static const int MAX_CONNECT_TIMEOUT = 2500;
static const int MAX_TIME_OUT_CN = 3500;

static const int TIME_OVER_VALUE = 2000;

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid);
static int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *lpVoid);

int OnDebug(CURL *, curl_infotype itype, char * pData, size_t size, void *lpVoid)
{
	if (itype == CURLINFO_TEXT)
	{
		//printf("[TEXT]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_OUT)
	{		
		//printf("[CURLINFO_HEADER_OUT]%s\n", pData);		
	}
	else if (itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);

	}
	else if (itype == CURLINFO_DATA_OUT)
	{
		//printf("[CURLINFO_DATA_OUT]%s\n", pData);
	}
	return 0;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if (NULL == str || NULL == buffer)
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}


CHttpClient::CHttpClient() :m_bDebug(false)
{
    curl_global_init(CURL_GLOBAL_ALL);
	//init_locks();
}


void CHttpClient::Quit()
{
	//kill_locks();
    curl_global_cleanup();
}

void CHttpClient::init()
{
	
}

int CHttpClient::Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{    
    //std::lock_guard<std::mutex> lock(m_lock);
    CURLcode res = CURLE_OK;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
		LOG_ERROR("curl_easy_init failed: \n");
        return CURLE_FAILED_INIT;
    }
   
    curl_slist *plist = curl_slist_append(NULL,
        "Content-Type:application/json;charset=UTF-8");
    if (nullptr == plist)
    {
		LOG_ERROR("curl_slist_append failed: \n");
        return CURLE_FAILED_INIT;
    }
	static const char buf[] = "Expect:";
	plist = curl_slist_append(plist, buf); // initalize custom header list

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, TIME_OVER_VALUE);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIME_OVER_VALUE);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        LOG_ERROR("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    curl_slist_free_all(plist);
    curl_easy_cleanup(curl);
    return res;
}



bool CHttpClient::PostEx(const std::string & strUrl, const std::string & strPost, std::string & strResponse)
{
	//strResponse = "{\"code\":200,\"msg\":\"ok\",\"user_id\":1}";
	//return true;

	bool bResult = false;
	int ret = Post(strUrl, strPost, strResponse);
	if (CURLE_OK == ret)
	{
		bResult = true;
	}
	return bResult;
}

int CHttpClient::postForm(string & strUrl, string & strImage,
	string & json, string & strResponse)
{
	//std::lock_guard<std::mutex> lock(m_lock);
	CURLcode res = CURLE_OK;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		fprintf(stderr, "curl_easy_init failed \n");
		return CURLE_FAILED_INIT;
	}
	CURLM *multi_handle = curl_multi_init();
	if (NULL == multi_handle)
	{
		fprintf(stderr, "curl_multi_init failed \n");
		curl_easy_cleanup(curl);
		return CURLE_FAILED_INIT;
	}

	long contextLen = static_cast<int>(strImage.size());
	const char *pContext = strImage.data();
	/***
	POST数据的大于1024个字节
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";
	headerlist = curl_slist_append(headerlist, buf); // initalize custom header list 
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist); // set header
	curl_slist_free_all(headerlist); // free slist
	**/

	struct curl_httppost *post = NULL;
	struct curl_httppost *last = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";	
	headerlist = curl_slist_append(headerlist, buf); // initalize custom header list 

	curl_formadd(
		&post, &last, CURLFORM_COPYNAME, "face_image",
		CURLFORM_BUFFER, "image.jpg", CURLFORM_BUFFERPTR,
		pContext, CURLFORM_BUFFERLENGTH,
		contextLen,
		CURLFORM_CONTENTTYPE, "image/jpeg", CURLFORM_END);

	if (!json.empty())
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, "json",
			CURLFORM_COPYCONTENTS, json.c_str(),
			CURLFORM_END);
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist); // set header
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3000);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3000);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
	curl_slist_free_all(headerlist); // free slist
	curl_easy_cleanup(curl);
	curl_formfree(post);
	curl_multi_cleanup(multi_handle);

	return res;
}

bool CHttpClient::httpGetEx(const string& strUrl,
	std::vector<string>& vecHttpHeader,
	string& strResponse,
	bool bReserveHeaders,
	bool location,
	const char * pCaPath)
{	
	strResponse.clear();
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl) {
		return false;
	}
	if (m_bDebug) 
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	
	if (bReserveHeaders) {
		//响应结果中保留头部信息
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	}

	struct curl_slist* headers = nullptr;
	if (!vecHttpHeader.empty()) {
		for (auto & headInfo : vecHttpHeader) {
			headers = curl_slist_append(headers, headInfo.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}
	}

	if (location) {
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 100);
		//1表示重定向次数，最多允许一次重定向
		//curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &res);
		//该函数若返回301，说明是永久重定向；若返回302，说明临时重定向
		//string str(res);
	}	
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());	
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);	

	if (NULL == pCaPath) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//设定为不验证证书和HOST
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else {
		//curl_easy_setopt(curl, CURLOPT_SSLVERSION, 3);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, MAX_CONNECT_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, MAX_TIME_OUT_CN);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);

	bool bResult = false;
	res = curl_easy_perform(curl);

	if (CURLE_OK == res) {
		int code = -1;
		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		if ((HTTP_CODE_OK == code) || (HTTP_CODE_MT == code) || (HTTP_CODE_MP == code))
			bResult = true;
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	return bResult;
}

bool CHttpClient::getEx(string & strUrl, string & head, string & body, vector<string> & vecHttpHeader)
{
	bool bResult = false;
	
	if (getHttpMothed(strUrl, head, body, vecHttpHeader)) {
		string & strHead = head;
		transform(strHead.begin(), strHead.end(), strHead.begin(), (int(*)(int))tolower);
		string flag = "content-type:";
		string::size_type pos = strHead.find(flag);
		if (string::npos != pos) {
			flag = "text/html";
			pos = strHead.find(flag, pos);
			if (string::npos != pos) {
				bResult = true;
			}
		}
		if (bResult) {
			CCookieMngS->extractCookie(strUrl, strHead);
		}
	}	
	return bResult;
}

bool CHttpClient::getHttpMothed(string & strUrl, string & head, string & body, vector<string> & vecHttpHeader)
{
	bool bResult = false;
	vecHttpHeader.push_back("User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; rv:65.0) Gecko/20100101 Firefox/65.0");
	vecHttpHeader.push_back("Accept: */*");
	vecHttpHeader.push_back("Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2");
	vecHttpHeader.push_back("Accept-Encoding: gzip, deflate, br");
	vecHttpHeader.push_back("Connection: keep-alive");
	vecHttpHeader.push_back("Upgrade-Insecure-Requests: 1");
	vecHttpHeader.push_back("Pragma: no-cache");
	vecHttpHeader.push_back("Cache-Control: no-cache");	

    string strCookie;
    if (CCookieMngS->getCookie(strUrl, strCookie)) {
        if (!strCookie.empty()){
            LOG_INFO("%s", strCookie.c_str());
            vecHttpHeader.push_back(strCookie);            
        }
    }

	string strResp;
	if (httpGetEx(strUrl, vecHttpHeader, strResp)) {
		//cout << strResp.substr(0, 2000) << endl;
		//cout << "strResp=" << strResp.size() << endl;
		vecHttpHeader.clear();
		vector<string> vecFlag;		
		vecFlag.push_back("HTTP/1.1 200");
		vecFlag.push_back("HTTP/1.0 200");
		vecFlag.push_back("http/1.1 200");
		vecFlag.push_back("http/1.0 200");

		bool bFind = false;
		string::size_type pos = string::npos;
		string::size_type next = string::npos;
		for (auto & flag : vecFlag) {
			pos = strResp.find(flag);
			if (string::npos != pos) {
				bFind = true;			
				break;
			}
		}
		if (!bFind) {
			LOG_ERROR("%s", strResp.c_str());
		}
		else {
			if (pos > 0) {
				string strLocation = strResp.substr(0, pos);				
				getLocation(strLocation, vecHttpHeader);
			}
			string::size_type offset = 0;
			vecFlag.clear();
			vecFlag.push_back("\r\n\r\n");			
			vecFlag.push_back("\n\n");
			vecFlag.push_back("\r\r");	
			for (auto & item : vecFlag)
			{
				next = strResp.find(item, pos);
				if (string::npos != next) {
					offset = item.size();
					break;
				}
			}
			if (string::npos == next) {
				LOG_ERROR("%s", strResp.c_str());
			}
			else {			
				head = strResp.substr(pos, next - pos); 
				next += offset;
				pos = next;
				next = strResp.size();
				body = strResp.substr(pos, next - pos);
				if (!body.empty()) {
					next = body.size();
					pos = body.find_last_not_of(" \r\n");
					if (string::npos != pos) {
						body.erase(pos, next - pos);
					}
					bResult = true;					
				}
			}
		}
	}
	else {
		LOG_ERROR("fault to call url(%s)", strUrl.c_str());
	}
	if (bResult) {	
		//cout << "body=" << body.size() << endl;
		auto gzip(std::make_unique<CGzip>());
		bResult = gzip->transferConentEncode(body, head);
		if (!bResult)
			LOG_ERROR("fault to decode");
	}
	return bResult;
}

void CHttpClient::getLocation(string & strInfo, vector<string> & vecHead)
{
	string strTemp;
	vector<string> vecFlag;
	vecFlag.push_back("Location:");
	vecFlag.push_back("location:");
	for (auto & item : vecFlag) {
		string::size_type pos = strInfo.rfind(item);
		if (string::npos != pos) {
			pos += item.size();
			string::size_type next = strInfo.find("\r\n", pos);
			if (string::npos != next) {
				strTemp = strInfo.substr(pos, next - pos);				
				UTIL_SELF::trim(strTemp);
				next = strTemp.size();
				if (next > 0) {
					next--;
					if (strTemp.at(next) == '/') {
						strTemp.erase(next, 1);
					}
				}
				vecHead.push_back(strTemp);				
			}
			break; 
		}
	}

	return;
}

int CHttpClient::Get(const std::string & strUrl, std::string & strResponse)
{

	CURLcode res = CURLE_OK;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
	/**
	* 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
	* 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
	*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, TIME_OVER_VALUE);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIME_OVER_VALUE);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
	}
	curl_easy_cleanup(curl);
	return res;
}


static std::string find_line(
	const std::string& original_text, const GumboAttribute& attr) {
	size_t attr_index = attr.original_value.data - original_text.data();
	size_t begin = original_text.rfind("\n", attr_index) + 1;
	size_t end = original_text.find("\n", attr_index);
	if (end != std::string::npos) {
		end--;
	}
	else {
		end = (size_t)original_text.length() - 1;
	}
	end = min(end, attr_index + 40);
	begin = max(begin, attr_index - 40);
	return original_text.substr(begin, end - begin);
}

static void search_for_class(
	GumboNode* node, const std::string& original_text, const char* cls_name) {
	if (node->type != GUMBO_NODE_ELEMENT) {
		return;
	}
	GumboAttribute* cls_attr;
	if ((cls_attr = gumbo_get_attribute(&node->v.element.attributes, "id"))
		&&	strstr(cls_attr->value, cls_name) != NULL
		) 
	{
		cout << "charset=" << cls_attr->value << endl;
		std::cout << cls_attr->value_start.line << ":"
			<< cls_attr->value_start.column << " - ("
			<< find_line(original_text, *cls_attr) <<")" << std::endl;

		CHtml html;
		string txt = html.cleantext(node);
	
		UTIL_SELF::eraseUtfSpace(txt);
		UTIL_SELF::eraseMoreSpace(txt);
		cout << txt << endl;
	}

	GumboVector* children = &node->v.element.children;
	for (int i = 0; i < (int)children->length; ++i) {
		search_for_class(
			static_cast<GumboNode*>(children->data[i]), original_text, cls_name);
	}
}

void CHttpClient::test()
{
	//CLinksMng links;
	//string test = "http://www.iteye.com/blog/2510590";
	//links.parseUrl(test); 
	//cout << "begin" << endl;
	//cout << links.getFullDomain() << endl;
	//cout << links.getLocation() << endl;
	//cout << links.getPath() << endl;
	//cout << links.getSubDomain() << endl;
	//cout << "finish" << endl;
	////return;
	//not to find <head> url(https://my.oschina.net/u/4441013/blog/widgets/_blog_detail_list_news?p=2&type=ajax)
	// not to find </head> url(http://www.zmask.cn/news/show-442560.html)
	// not to find <head> url(https://www.llxs11.com/a/147679.html)
	//not to find <head> url(http://www.123xiaoqiang.in/10/10056/944021.html)
	//not to find </head> url(http://www.123xiaoqiang.in/10/10056/944021.html)
	string strPath = "./";
	CNeedMngS->init(strPath);
	
	string url = "http://www.123xiaoqiang.in/10/10056/944021.html";
	std::vector<string> vecHead;
	string strHead, strBody;
	getEx(url, strHead, strBody, vecHead);	

	//cout << strBody << endl;
	//cout << "-----------------------------------" << endl;
	//cout << strHead << endl;
	cout << strBody.substr(0, 3500) << endl;
	cout << "-----------------------------------" << endl;
	auto info = std::make_shared<HTML_INFO>();
	info->url = url;
	
	info->head = strHead;
	CHtml html;
	html.charset2utf8(strBody, info);
	//string test = "test.html";
	//UTIL_SELF::saveFile(test, strBody);
	//cout << strBody << endl;
	/*string cls = "bbs_detail_wrap";
	GumboOutput* output = gumbo_parse_with_options(
		&kGumboDefaultOptions, strBody.data(), strBody.length());
	search_for_class(output->root, strBody, cls.data());
	gumbo_destroy_output(&kGumboDefaultOptions, output);*/
	
	
	
}

