#pragma once
#include "common.h"


struct FRESH_ULR_INFO {
	string url;
	string strInfo;
};

struct TERM_INFO {
	string url;
	string strInfo;
	//////////////////////
	int weight;
	int offset;
	TERM_INFO() {
		offset = 0;
		weight = 0;
	}
};

struct DOC_INFO {
	string keyword;  //

	//////////////////////////////////////
	std::shared_ptr<string> keyFile;	//唯一文件名	 //url
	int weight;			//此关键字对应的此文档的权重
	std::list<uint32_t> offset;  //编移量

	DOC_INFO() {
		weight = 0;		
	}
};

struct MAP_INFO {
	std::list<std::shared_ptr<DOC_INFO> > liDoc;
};



struct TERM_DB_INFO{
	string strFilename;
	int weight;
	std::list<uint32_t> offset;  //编移量
};
/*
{
"data":[{
	"file":"file1",
	"weight":1,
	"offset":[1,2,3]
	},{
	"file":"file2",
	"weight":1,
	"offset":[1,2,3]
	}]}
*/



struct SPIDER_INFO {
	string md5;	  //保存关键字对应索引值
	time_t downloadTime; //the time to download	
};
/*
{
"md5":"weerere"
"build_time":1234398493,
}
*/

struct VISIT_INFO {
	string ip;
	time_t begin_time;
	int num;
	VISIT_INFO() {
		num = 1;
		begin_time = time(nullptr);
	}
};

struct CLASS_INFO {
    string subUrl;
	string className;
	string classValue;
	int weight;

	void reset() {
		subUrl.clear();
		className.clear();
		classValue.clear();
		weight = 0;
	}
};

struct NEED_INFO : public CLASS_INFO {
	string url;  // http://www.test.com/test/index.html	
	string domainName;	// www.test.com
	string firstName;	// test
	string location;	// http://www.test.com/test	
	string path;     // /test/index.html 
	bool bBuss;
	NEED_INFO() {
		bBuss = false;
	}
};

struct DOWNLOAD_URL_INFO : NEED_INFO {
	std::list<string>  liCookie;	
	string head;	
	string prevDomain;
	
};

struct HTML_INFO : DOWNLOAD_URL_INFO {
	bool bEmptyFrame;
	bool bNeed;
	bool bUpdate;
	int  hotWordNum;
	string fullPath; //.html 文件路径;
	string txtPath; //.txt文件路径
	string snapshotUrl;
	string md5;	  //保存关键字对应索引值
	string oldMd5; //有更新时旧的md5
	time_t downloadTime; //the time to download	
	HTML_INFO() {
		bNeed = false;
		bUpdate = false;
		bBuss = false;
		bEmptyFrame = false;
		hotWordNum = 30;
	}
};

struct IP_RANGE_INFO {
	uint64_t begin_addr;
	uint64_t end_addr;
};


struct AD_INFO {
	string url;
	string view;
	int ad_type; //广告类型
};


struct LINKS_INFO {
	DOWNLOAD_URL_INFO downladInfo;	
	std::set<string>  hrefLinks;	
};



struct HTML_INSERT_INFO {	
	string total_page_prev;
	string total_page_next;
	
	string h3_main_herf_prev;
	string h3_view__herf_prev;
	string content_prev;	
	string url_ref_prev;
	string url_display_prev;
	string url_snapshot_prev;
	string url_snapshot;

	string pagePart;
	string lastPart;
};
static const string  SNAPSHOT_URL = "snapshot?";

static const string QUERY_KEY_WORD_PAD = "wd=";
static const string QUERY_KEY_WORD_GET = "wd";

static const int HTML_MAX_CONTENT_LEN = 89;
static const std::string  QYT_KEYWORD = "$(qyt_keyword)";
static const std::string  QYT_HOST = "$(qyt_host)";
static const std::string  QYT_TOTAL_PAGE = "$(qyt_total_page)";
static const std::string  QYT_H3_MAIN_HERF = "$(qyt_main_url_herf)";
static const std::string  QYT_H3_VEIW_HERF = "$(qyt_main_title)";
static const std::string  QYT_CONTENT = "$(qyt_content)";  //89个字
static const std::string  QYT_URL_HERF = "$(qyt_url_ref_1)";  
static const std::string  QYT_URL_DISPLAY = "$(qyt_url_display)";
static const std::string  QYT_URL_SNAPSHOT = "$(qyt_snapshot_href)";


static const std::string  SPLIIT_FLAG_COMMA = ",";
static const std::string  HTML_INSERT_BEGIN_FLAG = "<!--qyt_begin-->";
static const std::string  HTML_INSERT_END_FLAG = "<!--qyt_end-->";

static const std::string  PAGE_BEGIN_FLAG = "<!--qyt_page_begin-->";
static const std::string  PAGE_END_FLAG = "<!--qyt_page_end-->";


typedef std::unordered_map<string, std::shared_ptr<MAP_INFO>>			MAP_INFO_TYPE;
typedef	std::unordered_map<string, list<std::shared_ptr<DOC_INFO>>>		DOC_INFO_TYPE;
typedef	std::multimap<int, list<std::shared_ptr<DOC_INFO>>>					WEIGHT_INFO_TYPE;

static const std::string  DOC_SPLIT_FLAG = "|";
