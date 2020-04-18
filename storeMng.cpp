#include "storeMng.h"
#include "log.h"
#include "util.h"
#include "handleJson.h"
#include "mapMng.h"
#include "handleXml.h"

CStoreMng::CStoreMng() :m_bSearch(false)
{	
	m_options.create_if_missing = true;
}


void CStoreMng::Quit()
{
	for (auto & item : m_dbInfo) {
		delete item.second;
	}
	m_dbInfo.clear();
}

bool CStoreMng::init()
{
	std::set<string> dbName;
	dbName.insert(SEARCH_DB);
	dbName.insert(DOCUMENT_DB);
	m_bSearch = CHandleXmlS->onlySearchMode();
	if (!m_bSearch) {
		dbName.insert(TERM_DB);
		dbName.insert(FRESH_URL_DB);
		dbName.insert(VISIT_DB);
	}

	string strDataPath = "./data";
	UTIL_SELF::MakeDir(strDataPath.c_str());
	strDataPath += "/";
	for (auto & item : dbName) {
		string strPath = strDataPath + item;
		UTIL_SELF::MakeDir(strPath.c_str());
		leveldb::DB* db = nullptr;
		leveldb::Status status = leveldb::DB::Open(m_options, strPath.c_str(), &db);
		if (!status.ok()) {
			LOG_ERROR("error to open %s %s", strPath.c_str(), status.ToString().c_str());
			return false;
		}
		m_dbInfo[item] = db;
	}	
	if (!loadInit(m_dbInfo[SEARCH_DB])) {
		LOG_ERROR("error to loadInit ");
		return false;
	}	

	if (CHandleXmlS->onlySearchMode()) {

		auto it = dbName.find(DOCUMENT_DB);
		if (dbName.end() != it) {
			dbName.erase(it);
		}
		eraseDb(dbName);
	}

	return true;
}

void CStoreMng::eraseDb(std::set<string> & setDb)
{
	for (auto & name : setDb) {
		auto it = m_dbInfo.find(name);
		if (m_dbInfo.end() != it) {
			m_dbInfo.erase(it);
		}
	}
	return;
}

bool CStoreMng::getFreshUrl(std::set<string> & setInfo, int count)
{
	bool bResult = false;
	if (-1 == count) {
		count = 10000;
	}
	leveldb::ReadOptions options;
	//options.fill_cache = false;
	{
		std::lock_guard<std::mutex>lck(m_lock);
		leveldb::Iterator *iterator = m_dbInfo[FRESH_URL_DB]->NewIterator(options);
		if (!iterator) {
			LOG_ERROR("can not new iterator");
			return bResult;
		}
		iterator->SeekToFirst();
		while (iterator->Valid() && (--count >= 0)) {
			leveldb::Slice sValue = iterator->value();			
			string value(sValue.data(), sValue.size());
			setInfo.insert(value);

            iterator->Next();
		}
		delete(iterator);
	}
	if (setInfo.size() > 0) {
		bResult = true;
	}
	return bResult;
}

bool CStoreMng::loadInit(leveldb::DB *db)
{
	leveldb::ReadOptions options;
	options.fill_cache = false;
	leveldb::Iterator *iterator = db->NewIterator(options);
	if (!iterator) {
		LOG_ERROR("can not new iterator");
		return false;
	}
	int num = 0;
	std::multimap<int, string> mpSort;
	string strExcept;
	
	std::map<string, std::shared_ptr<string>>  mpFilePtr; //filename-->ptr;
	vector<TERM_DB_INFO> vecTerm;
	auto handleJson(UTIL_SELF::make_unique<CHandleJson>());
	iterator->SeekToFirst();	
	while (iterator->Valid()) {	
		leveldb::Slice sKey = iterator->key();
		leveldb::Slice sValue = iterator->value();

		string strKey(sKey.data(), sKey.size());
		string strValue(sValue.data(), sValue.size());
		vecTerm.clear();
		
		if (handleJson->getTermByJson(vecTerm, strValue)) {
			CMapMngS->loadKeyword(strKey, vecTerm, mpFilePtr);
		}
		else {
			LOG_WARNING("warning to getTermByJson");
		}
				
		iterator->Next();
		num++;

		int size = static_cast<int>(vecTerm.size());
		mpSort.insert(std::make_pair(size, strKey));
		
	}
	delete(iterator);	

	string strPrint;
	string strData;
	string strResultKey = "/";
	for (auto & item : mpSort) {
		strResultKey += item.second;
		strResultKey += "(";
		strResultKey += std::to_string(item.first);
		strResultKey += ")";
		strResultKey += "/";

		if (strData.empty()) {
			strData = item.second;
		}
		else {
			strData += "\n";
			strData += item.second;
		}
		strPrint += "\n";
		strPrint += item.second;
		
	}
	if (!mpSort.empty()) {
		//cout << strResultKey << endl;	
		LOG_INFO("keyword count :%d", num);
		//cout << strPrint << endl;
	}
	
    LOG_INFO("total:%d", num);
	string file = "sample.utf8";
	//UTIL_SELF::saveFile(file, strPrint);
	
	
	

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
bool CStoreMng::getInfo(leveldb::DB*db, const string & key, string & content)
{
	bool bResult = false;
	leveldb::ReadOptions options;	
	leveldb::Status s;
	s = db->Get(options, key, &content);
	if (s.ok()) {
		bResult = true;
	}
	return bResult;
}

bool CStoreMng::putInfo(leveldb::DB*db, const string & key, string & content, bool bSync)
{
	bool bResult = false;
	leveldb::WriteOptions options;
	options.sync = bSync;
	leveldb::Status s;		
	s = db->Put(options, key, content);
	if (s.ok()) {
		bResult = true;
	}
	else {
		LOG_ERROR("fault to putInfo (%s)", s.ToString().c_str());
	}
	return bResult;
}

bool CStoreMng::verifyData(UMAP_TYPE & mpInfo)
{
	bool bResult = true;
	auto it = mpInfo.begin();
	for (; it != mpInfo.end(); ) {
		if ((it->first.empty()) || (it->second.empty())) {
			if (it->first.empty()) {
				LOG_ERROR("empty to find key");
			}
			if (it->second.empty()) {
				LOG_ERROR("empty to find value");
			}			
			mpInfo.erase(it++);
		}
		else {
			++it;
		}
	}
	if (mpInfo.empty()) {
		bResult = false;
	}
	return bResult;
}

bool CStoreMng::putbatch(leveldb::DB*db, UMAP_TYPE & mpInfo, bool bSync)
{
	bool bResult = false;
	if (!verifyData(mpInfo)) {
		return bResult;
	}
	
	auto batch(UTIL_SELF::make_unique<leveldb::WriteBatch>());
	for (auto & item : mpInfo) {	
		batch->Delete(item.first);
		batch->Put(item.first, item.second);
	}
	leveldb::Status s;
	leveldb::WriteOptions options;
	options.sync = bSync;
	s = db->Write(options,batch.get());
	if (!s.ok()) {		
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		s = db->Write(options, batch.get());
		if (!s.ok()) {
			LOG_ERROR("fault to WriteBatch(%s)(%s--%s)", s.ToString().c_str());
		}
	}
	else {
		bResult = true;
	}
	return bResult;
}

void CStoreMng::deleteInfo(leveldb::DB*db, const string & key, bool bSync)
{
	leveldb::WriteOptions options;
	options.sync = bSync;
	leveldb::Status s;
	s = db->Delete(options, key);
}

void CStoreMng::deletebatch(leveldb::DB*db, vector<string> & vecKey, bool bSync)
{
	if (vecKey.empty())
		return;
	leveldb::Status s;
	auto batch(UTIL_SELF::make_unique<leveldb::WriteBatch>());
	for (auto & item : vecKey) {
		batch->Delete(item);
	}
	leveldb::WriteOptions options;
	options.sync = bSync;
	s = db->Write(options, batch.get());
	if (!s.ok()) {
		LOG_ERROR("fault to WriteBatch(%s)", s.ToString().c_str());
	}	
}

bool CStoreMng::putDb(const string & dbName, const string & key, string & value, bool bSync)
{
	bool bResult = false;
	auto it = m_dbInfo.find(dbName);
	if (m_dbInfo.end() != it) {
		bResult = putInfo(it->second, key, value, bSync);
	}
	else {
		LOG_WARNING("not to find db (%s)", dbName.c_str());
	}
	return bResult;
}

bool CStoreMng::putDb(const string & dbName, UMAP_TYPE & mpInfo, bool bSync)
{
	bool bResult = false;
	if (mpInfo.empty()) {
		return bResult;
	}
	auto it = m_dbInfo.find(dbName);
	if (m_dbInfo.end() != it) {
		bResult = putbatch(it->second, mpInfo, bSync);
	}
	else {
		LOG_WARNING("not to find db (%s)", dbName.c_str());
	}
	return bResult;
}

bool CStoreMng::getDb(const string & dbName, const string & key, string & value)
{	
	bool bResult = false;
	auto it = m_dbInfo.find(dbName);
	if (m_dbInfo.end() != it) {
		bResult = getInfo(it->second, key, value);
	}
	else {
		LOG_WARNING("not to find db (%s)", dbName.c_str());
	}
	return bResult;
}

bool CStoreMng::deleteDb(const string & dbName, const string & key, bool bSync)
{
	bool bResult;
	auto it = m_dbInfo.find(dbName);
	if (m_dbInfo.end() != it) {
		deleteInfo(it->second, key, bSync);
		bResult = true;
	}
	else {
		LOG_WARNING("not to find db (%s)", dbName.c_str());
	}
	return  bResult;
}

bool CStoreMng::deleteDb(const string & dbName, vector<string> & vecKey, bool bSync)
{
	bool bResult;
	auto it = m_dbInfo.find(dbName);
	if (m_dbInfo.end() != it) {	
		deletebatch(it->second, vecKey, bSync);
		bResult = true;
	}
	else {
		LOG_WARNING("not to find db (%s)", dbName.c_str());
	}
	return  bResult;
}