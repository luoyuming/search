#include "rankMng.h"
#include "log.h"
#include "util.h"
#include "codeInfo.h"
#include "escapeSequence.h"

CRankMng::CRankMng(std::string & str, string & webSite)
	:pos(0)
{
	setInvalid.insert(" ");
	setInvalid.insert("\"");
	setInvalid.insert("\'");
	setInvalid.insert(";");
	setInvalid.insert(":");
	setInvalid.insert(",");
	setInvalid.insert("?");
	setInvalid.insert("#");
	setInvalid.insert("$");
	setInvalid.insert("%");
	setInvalid.insert("^");
	setInvalid.insert("&");
	setInvalid.insert("*");
	setInvalid.insert("~");
	setInvalid.insert("_");
	setInvalid.insert("-");
	//setInvalid.insert("+");
	setInvalid.insert("@");
	setInvalid.insert(".");
	setInvalid.insert("/");
	setInvalid.insert("<");
	setInvalid.insert(">");
	setInvalid.insert("|");
	

	bInit = false;
	auto base64 = std::make_shared<CCodeInfo>();
	//分成三部分，网页主题,短地址,TXT信息
	this->url = webSite;
	string docInfo;
	string::size_type size = str.size();
	string::size_type pos =0, next = 0;
	int i = 0;
	vector<string> vecInfo;
	string strTemp;
	do {
		i++;
		next = str.find(DOC_SPLIT_FLAG, pos);
		if (string::npos == next) {
			break;
		}
		strTemp = str.substr(pos, next - pos);
		vecInfo.push_back(strTemp);
		pos = next;
		pos += DOC_SPLIT_FLAG.size();
		if (i >= 2) {
			strTemp = str.substr(pos, size - pos);
			vecInfo.push_back(strTemp);
			break;
		}
	} while (true);

	if (vecInfo.size() >= 3) {
		bInit = true;		
		
		base64->GetDecodeInfo(head, vecInfo[0]);
		snapshot_url = vecInfo[1];
		utfString = std::make_shared<iutf8string>(vecInfo[2]);
	}
	else {
		bInit = false;
	}

	return;
}

CRankMng::~CRankMng()
{

}

void CRankMng::inputField(std::list<uint32_t> & liOffset, string & keyword)
{	
	int num = 0;
	std::shared_ptr<iutf8string> utfkeyword;	
	if (pos < MAX_FIELD_LEN) {
		utfkeyword = std::make_shared<iutf8string>(keyword);
		num = utfkeyword->length();
	}
	setKeyword.insert(keyword);
	int size = static_cast<int>(liOffset.size());
	int pad = size > 10 ? 10 : 15;
	for (auto & offset : liOffset) {		
		if (pos < MAX_FIELD_LEN) {
			int value = (int)offset;
			if (value > 0) {
				utfString->getReverseWord(setQueryWord, offset, 3);
			}
			utfString->getWordPos(setQueryWord, offset, num + pad);
			pos = static_cast<int>(setQueryWord.size());			
		}	
		else {
			checkFrontStr();
			pos = static_cast<int>(setQueryWord.size());
			if (pos > MAX_FIELD_LEN)
				break;
		}
	}
	return;
}

void CRankMng::delMoreSpace()
{
	string strSpace = " ";
	int i = 0;
	int size = static_cast<int>(setQueryWord.size());
	auto it = setQueryWord.begin();
	for (; it != setQueryWord.end(); i++) {
		bool bFind = false;
		int index = *it;
		string strTemp = utfString->get(index);
		if (strSpace == strTemp) {
			int next = i + 1;
			if (next < size) {
				string strTemp = utfString->get(next);
				if (strSpace == strTemp) {
					bFind = true;
				}
			}
		}
		if (bFind) {
			setQueryWord.erase(it++);
			//LOG_ERROR("more space to delete ");
		}
		else {
			++it;
		}
	}
	return;
}

void CRankMng::checkFrontStr()
{
	//delMoreSpace();
	
	for(;;) {
		if (setQueryWord.empty())
			break;
		auto it = setQueryWord.begin();
		int index = *it;
		string strTemp = utfString->get(index);		
		auto find = setInvalid.find(strTemp);
		if (setInvalid.end() != find) {
			LOG_ERROR("first string is space");
			setQueryWord.erase(it);			
		}
		break;
	}
	return;
}

void CRankMng::fillTemplate(string & strInfo, PACKAGE_INFO & info)
{
	if (field.empty()) {
		LOG_WARNING("field is empty");
		return;
	}
	
	string strTemp = url;
	string::size_type url_len = 30;
	if (url.size() > url_len) {
		strTemp = url.substr(0, url_len);		
	}	
	//LOG_WARNING("url %s", url.c_str());

	string shortUrl = *(info.localHost);
	shortUrl += "/";
	shortUrl += SNAPSHOT_URL;
	shortUrl += QUERY_KEY_WORD_PAD;
	shortUrl += snapshot_url;

	strInfo = info.htmlTemplate->h3_main_herf_prev;
	strInfo += url;
	strInfo += info.htmlTemplate->h3_view__herf_prev;
	strInfo += head;
	strInfo += info.htmlTemplate->content_prev;
	strInfo += field;
	strInfo += info.htmlTemplate->url_ref_prev;
	strInfo += url;
	strInfo += info.htmlTemplate->url_display_prev;
	strInfo += strTemp;
	strInfo += info.htmlTemplate->url_snapshot_prev;
	strInfo += shortUrl;	
	strInfo += info.htmlTemplate->url_snapshot;
	return;
}

void CRankMng::extractField()
{	
	combineField();
	quoteSeqHtml();
	for (auto & item : setKeyword) {		
		strongField(head, item);		
		strongField(field, item);
	}	
	return;
}

void CRankMng::strongField(string & str, const string & flag)
{
	vector<string> vecInfo;
	string empty;
	string::size_type pos = 0;
	do {
		pos = str.find(flag);
		if (string::npos == pos) {
			vecInfo.push_back(str);
			break;
		}
		if (0 == pos) {
			vecInfo.push_back(empty);
		}
		else {
			vecInfo.push_back(str.substr(0, pos));
			vecInfo.push_back(empty);			
		}
		pos += flag.size();
		str.erase(0, pos);
	} while (true);	

	if (vecInfo.size() > 1) {
		str.clear();
		for (auto & item : vecInfo) {
			if (item.empty()) {
				str += "<em>";
				str += flag;
				str += "</em>";
			}
			else {
				str += item;
			}
		}
	}
	return;
}

void CRankMng::combineField()
{
	if (!bInit) {
		LOG_ERROR("not to init the utfString");
		return;
	}
	if (pos < MAX_FIELD_LEN) {		
		int total = utfString->length();
		if (total < MAX_FIELD_LEN) {			
			field = utfString->stlstring();	
			LOG_ERROR("-- %s", field.c_str());
			if (setQueryWord.size() > 0) {
				int first_pos = *(setQueryWord.begin());
				if (head.empty()) {				
					head = utfString->substr(first_pos, 25);
				}
			}			
			return;
		}
		else {	
			int diff = MAX_FIELD_LEN - pos;
			if (setQueryWord.size() > 0) {			
				int size = 0;
				int first_pos = *(setQueryWord.begin());
				int last_pos = *(setQueryWord.rbegin());
				do {
					//从中间取
					setQueryWord.insert(++first_pos);
					size = static_cast<int>(setQueryWord.size());
					if (size >= MAX_FIELD_LEN) {
						break;
					}
				} while (first_pos < last_pos);				
				if (size < MAX_FIELD_LEN) {
					diff = MAX_FIELD_LEN - size;
					int num = diff;
					for (int i = 0; i < diff; i++) {
						--first_pos;
						if (first_pos < 0) {
							break;
						}
						setQueryWord.insert(first_pos);
						--num;
						if (num < 0) {
							break;
						}
					}
					for (int i = 0; i < num; i++) {
						last_pos++;
						if (last_pos >= total) {
							break;
						}
						setQueryWord.insert(last_pos);
					}
				}
			}						
		}
	}	
		
	utfString->extracdStrByPos(field, setQueryWord);
	if (setQueryWord.size() > 0) {		
		int first_pos = *(setQueryWord.begin());
		if (head.empty()) {			
			head = utfString->substr(first_pos, 25);
		}
		
	}
	return;
}


void CRankMng::quoteSeqHtml()
{
	CEscapeSequenceS->quoteHtml(head);
	CEscapeSequenceS->quoteHtml(field);
	return;
}