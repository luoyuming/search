#pragma once
#include "singleton.h"
#include "common.h"
#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "msgDef.h"

static const string  SYSTIME_AUTO_ID = "026a39f9-cc69-4289-bee9-6d9bcfc94f17";
static const string  FRESH_URL_DB = "freshUrlDb";   //所有没有访问过的URL-->HTML_INFO; 
static const string  VISIT_DB = "visitDb";   //所有已经访问过的URL-->HTML_INFO; 
static const string  SEARCH_DB = "searchDb";  //关键字-->json(vector<TERM_DB_INFO>) 
static const string  DOCUMENT_DB = "documentDb";   // url-->纯文本内容/html内容
static const string  TERM_DB = "termDb"; //url-->(关键字,关键字)


typedef std::unordered_map<string, string>  UMAP_TYPE;
class CStoreMng : public SingletionEX<CStoreMng> {
	SINGLETON_INIT_EX(CStoreMng);
	CStoreMng();
public:
	bool init();	
	
	bool getFreshUrl(std::set<string> & setInfo, int count);
	bool putDb(const string & dbName, UMAP_TYPE & mpInfo, bool bSync = false);
	bool putDb(const string & dbName, const string & key, string & value, bool bSync = false);
	bool getDb(const string & dbName, const string & key, string & value);
	bool deleteDb(const string & dbName, vector<string> & vecKey, bool bSync = false);
	bool deleteDb(const string & dbName, const string & key, bool bSync = false);
private:
	bool getInfo(leveldb::DB*db, const string & key, string & content);
	bool putInfo(leveldb::DB*db, const string & key, string & content, bool bSync);
	bool putbatch(leveldb::DB*db, UMAP_TYPE & mpInfo, bool bSync);
	void deleteInfo(leveldb::DB*db, const string & key, bool bSync);
	void deletebatch(leveldb::DB*db, vector<string> & vecKey, bool bSync);
private:	
	bool verifyData(UMAP_TYPE & mpInfo);
	void eraseDb(std::set<string> & setDb);
	bool loadInit(leveldb::DB *db);	
	
private:
	bool							m_bSearch;
	std::mutex                      m_lock;
	leveldb::Options				m_options;
	std::map<string, leveldb::DB*>	m_dbInfo;
	
};

#define CStoreMngS  CStoreMng::getInstance()
