#include "common.h"
#include "handleJson.h"
#include "log.h"
#include "util.h"

using namespace rapidjson;

CHandleJson::CHandleJson()
{
}


CHandleJson::~CHandleJson()
{

}

bool CHandleJson::getTermByJson(vector<TERM_DB_INFO> & vecInfo, const string & strJson)
{	
	auto pDoc(UTIL_SELF::make_unique<rapidjson::Document>());
	rapidjson::Document & doc = *pDoc;
	doc.Parse<0>(strJson.c_str());
	if (doc.HasParseError())
	{
		LOG_ERROR(" it's error to analysis");
		return false;
	}
	
	if (json_check_is_array(doc, JSON_DATA)) {
		rapidjson::Value & jsonResult = doc[JSON_DATA];
		if (jsonResult.Empty())
		{
			return true;
		}
		
		for (rapidjson::SizeType i = 0; i < jsonResult.Size(); i++) {
			auto term(UTIL_SELF::make_unique<TERM_DB_INFO>());
			rapidjson::Value & refValue = jsonResult[i];
			term->strFilename = json_check_string(refValue, JSON_FILE);
			term->weight = json_check_uint32(refValue, JSON_WEIGHT);
			if (json_check_is_array(refValue, JSON_OFFSET)) {				
				rapidjson::Value & offsetArray = refValue[JSON_OFFSET];
				for (rapidjson::SizeType n = 0; n < offsetArray.Size(); n++) {					
					const rapidjson::Value& object = offsetArray[n];
					term->offset.push_back(object.GetUint());
				}				
			}
			vecInfo.push_back(*term);
		}
	}	
	return true;
}

bool CHandleJson::getVisitIP(int & cmd, const string & strJson)
{
	auto pDoc(UTIL_SELF::make_unique<rapidjson::Document>());
	rapidjson::Document & doc = *pDoc;
	doc.Parse<0>(strJson.c_str());
	if (doc.HasParseError())
	{
		LOG_ERROR(" it's error to analysis");
		return false;
	}

	cmd = json_check_int32(doc, JSON_CMD);
	return true;
}

bool CHandleJson::getHtmlByJson(HTML_INFO & info,const string & strJson)
{
	auto pDoc(UTIL_SELF::make_unique<rapidjson::Document>());
	rapidjson::Document & doc = *pDoc;
	doc.Parse<0>(strJson.c_str());
	if (doc.HasParseError())
	{
		LOG_ERROR(" it's error to analysis");
		return false;
	}
	
	info.url = json_check_string(doc, JSON_URL);
	info.md5 = json_check_string(doc, JSON_MD5);
	info.downloadTime = json_check_int32(doc, JSON_BUILD_TIME);	
	info.domainName = json_check_string(doc, JSON_DOMAIN_NAME);
	info.firstName = json_check_string(doc, JSON_FRIST_NAME);
	info.location = json_check_string(doc, JSON_LOCATION);	
	info.className = json_check_string(doc, JSON_CLASS_NAME);
	info.classValue = json_check_string(doc, JSON_CLASS_VALUE);
	info.weight = json_check_int32(doc, JSON_WEIGHT);
	info.bBuss = json_check_bool(doc, JSON_BUSS);
	return true;
}


string CHandleJson::buildVisitIP(vector<VISIT_INFO> & vecInfo)
{
	StringBuffer s;
	Writer<StringBuffer> writer(s);
	writer.StartObject();
	writer.Key(JSON_NUM);
	writer.Uint(vecInfo.size());
	writer.Key(JSON_DATA);
	writer.StartArray();
	for (auto & item : vecInfo) {
		writer.StartObject();
		writer.Key(JSON_IP);
		writer.String(item.ip.c_str());
		writer.Key(JSON_NUM);
		writer.Uint(item.num);
		writer.Key(JSON_BUILD_TIME);
		writer.String(asctime(gmtime(&(item.begin_time))));
		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();
	return s.GetString();
}

string CHandleJson::buildHtmlJson(HTML_INFO & htmlInfo)
{
	StringBuffer s;
	Writer<StringBuffer> writer(s);
	writer.StartObject();
	
	writer.Key(JSON_BUILD_TIME);
	writer.Int(htmlInfo.downloadTime);
	writer.Key(JSON_MD5);
	writer.String(htmlInfo.md5.c_str());	
	writer.Key(JSON_URL);
	writer.String(htmlInfo.url.c_str());
	writer.Key(JSON_DOMAIN_NAME);
	writer.String(htmlInfo.domainName.c_str());
	writer.Key(JSON_FRIST_NAME);
	writer.String(htmlInfo.firstName.c_str());
	writer.Key(JSON_LOCATION);	
	writer.String(htmlInfo.location.c_str());
	writer.Key(JSON_CLASS_NAME);	
	writer.String(htmlInfo.className.c_str());
	writer.Key(JSON_CLASS_VALUE);
	writer.String(htmlInfo.classValue.c_str());
	writer.Key(JSON_WEIGHT);
	writer.Int(htmlInfo.weight);
	writer.Key(JSON_BUSS);
	writer.Bool(htmlInfo.bBuss);
	writer.EndObject();
	return s.GetString();
}


string CHandleJson::buildTermJson(vector<TERM_DB_INFO> & vecInfo)
{
	StringBuffer s;
	Writer<StringBuffer> writer(s);
	writer.StartObject();	
	writer.Key(JSON_DATA);
	writer.StartArray();
	for (auto & item : vecInfo) {
		writer.StartObject();
		writer.Key(JSON_FILE);
		writer.String(item.strFilename.c_str());
		writer.Key(JSON_WEIGHT);
		writer.Uint(item.weight);

		writer.Key(JSON_OFFSET);
		writer.StartArray();			
		for (auto & value : item.offset) {
			writer.Uint(value);
		}
		writer.EndArray();

		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();
	return s.GetString();
}

