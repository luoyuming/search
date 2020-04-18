#pragma once
#include "msgDef.h"
#include "common.h"
#include "json/document.h"  
#include "json/prettywriter.h"
#include "json/stringbuffer.h"  
#include "json/rapidjson.h" 
#include "json/reader.h" 
#include "json/writer.h" 


//http://www.cnblogs.com/ziyouchutuwenwu/p/3790322.html



#define  JSON_OK					"ok"
#define  JSON_ERR					"error"
#define  JSON_DATA					"data"
#define  JSON_FILE					"file"
#define  JSON_WEIGHT				"weight"
#define  JSON_OFFSET				"offset"
#define  JSON_MD5					"md5"
#define  JSON_URL					"url"
#define  JSON_PATH					"web_path"
#define  JSON_BUILD_TIME			"build_time"
#define  JSON_KEYWORD_INDEX			"keyword"
#define  JSON_FULL_PATH				"html_path"
#define  JSON_TXT_PATH				"txt_path"
#define  JSON_DOMAIN_NAME			"domain_name"
#define  JSON_FRIST_NAME			"first_name"
#define  JSON_LOCATION				"location"
#define  JSON_WEIGHT				"weight"
#define  JSON_BUSS					"buss"
#define  JSON_CLASS_NAME			"class_name"
#define  JSON_CLASS_VALUE			"class_value"
#define  JSON_SNAPSHOT				"snapshot_url"
#define  JSON_CMD				    "cmd"
#define  JSON_IP				    "ip"
#define  JSON_NUM				    "num"

// 基础变量的校验  

#define json_check_is_bool(value, strKey) (value.HasMember(strKey) && value[strKey].IsBool())  
#define json_check_is_string(value, strKey) (value.HasMember(strKey) && value[strKey].IsString())  
#define json_check_is_int32(value, strKey) (value.HasMember(strKey) && value[strKey].IsInt())  
#define json_check_is_uint32(value, strKey) (value.HasMember(strKey) && value[strKey].IsUint())  
#define json_check_is_int64(value, strKey) (value.HasMember(strKey) && value[strKey].IsInt64())  
#define json_check_is_uint64(value, strKey) (value.HasMember(strKey) && value[strKey].IsUint64())  
#define json_check_is_float(value, strKey) (value.HasMember(strKey) && value[strKey].IsFloat())  
#define json_check_is_double(value, strKey) (value.HasMember(strKey) && value[strKey].IsDouble())  

#define json_check_is_number(value, strKey) (value.HasMember(strKey) && value[strKey].IsNumber())  
#define json_check_is_array(value, strKey) (value.HasMember(strKey) && value[strKey].IsArray())  


// 得到对应类型的数据，如果数据不存在则得到一个默认值  
#define json_check_bool(value, strKey) (json_check_is_bool(value, strKey) && value[strKey].GetBool())  
#define json_check_string(value, strKey) (json_check_is_string(value, strKey) ? value[strKey].GetString() : "")  
#define json_check_int32(value, strKey) (json_check_is_int32(value, strKey) ? value[strKey].GetInt() : 0)  
#define json_check_uint32(value, strKey) (json_check_is_uint32(value, strKey) ? value[strKey].GetUint() : 0)  
#define json_check_int64(value, strKey) (json_check_is_int64(value, strKey) ? ((value)[strKey]).GetInt64() : 0)  
#define json_check_uint64(value, strKey) (json_check_is_uint64(value, strKey) ? ((value)[strKey]).GetUint64() : 0)  
#define json_check_float(value, strKey) (json_check_is_float(value, strKey) ? ((value)[strKey]).GetFloat() : 0)  
#define json_check_double(value, strKey) (json_check_is_double(value, strKey) ? ((value)[strKey]).GetDouble() : 0)  

// 得到Value指针  
#define json_check_value_ptr(value, strKey) (((value).HasMember(strKey)) ? &((value)[strKey]) : nullptr)  

class CHandleJson
{
public:
    CHandleJson();
    ~CHandleJson();
    
public:  
	bool getTermByJson(vector<TERM_DB_INFO> & vecInfo, const string & strJson);
	bool getHtmlByJson(HTML_INFO & info, const string & strJson);
	bool getVisitIP(int & cmd, const string & strJson);

	string buildVisitIP(vector<VISIT_INFO> & vecInfo);
	string buildHtmlJson(HTML_INFO & htmlInfo);
	string buildTermJson(vector<TERM_DB_INFO> & vecInfo);
	
};