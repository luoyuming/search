#include "epollMng.h"
#include "handleXml.h"
#include "log.h"
#include "mapMng.h"
#include "fileMng.h"

CEpollMng::CEpollMng():m_quit(false)
{	
	g_sslTemp = nullptr;
	errBio = nullptr;
	m_epfd = -1;
	setContentType();

	m_setMothed.insert(GET_MOTHED);
	m_setMothed.insert(POST_MOTHED);
	m_liLN.push_back(HEAD_END_FLAG);
	m_liLN.push_back(HEAD_END_FLAG_ONE);
	m_liLN.push_back(HEAD_END_FLAG_TWO);
}

void CEpollMng::Quit()
{
	m_sslTcpSock = -1;
	m_tcpSock = -1;
	m_epfd = -1;
	BIO_free(errBio);
	

	for (auto & ctx : m_mpCtxSSL) {
		SSL_CTX_free((SSL_CTX*)ctx.second);
	}
	ERR_free_strings();

	//SSL_CTX_free(g_sslTemp);
}

void CEpollMng::stop()
{
	close(m_sslTcpSock);
	close(m_tcpSock);
	close(m_epfd);	
	m_quit.store(true);
}

bool CEpollMng::init()
{
	bool bResult = false;	
	bResult = CHandleXmlS->readDefaultXml();
	if (bResult) {
		int port = CHandleXmlS->getPort();
		bResult = BuildSvr(port);
	}
	else {
		LOG_ERROR("error to read xml config file");
	}	
    string path = CHandleXmlS->getPath();
	initSSL(path);

	return bResult;
}

bool CEpollMng::BuildSvr(int port)
{
	if (port <= 0) {
		port = TCP_PORT;
	}
	m_epfd = epoll_create(1000);
	if (-1 == m_epfd)
	{
		LOG(ERROR) << "error to create epoll!";
		return false;
	}

	

	if (!buildEpoll(port, PORT_TYPE::PORT_TYPE_HTTP))
	{
		LOG_ERROR("error to build tcp sock");
		return false;
	}
	if (!buildEpoll(443, PORT_TYPE::PORT_TYPE_SSL_TCP))
	{
		LOG_ERROR("error to build tcp sock");
		return false;
	}
	return true;
}

int CEpollMng::SetNonblocking(int sock)
{
	int opts;
	opts = fcntl(sock, F_GETFL);
	if (opts < 0)
	{
		LOG_WARNING("fault to SetNonblocking %d", sock);
		return -1;
	}

	opts |= O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0)
	{
		LOG_WARNING("fault to SetNonblocking %d", sock);
		return -1;
	}
	
	return 0;
}


bool CEpollMng::buildEpoll(int nPort, PORT_TYPE portType)
{
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SetNonblocking(sock);

	int reuseaddr = 1;
	int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseaddr, sizeof(reuseaddr));
	if (res != 0)
	{
		close(sock);
		return false;
	}
	int optint = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&optint, sizeof(optint));

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (0 != ::bind(sock, (sockaddr*)&addr, sizeof(addr)))
	{
		LOG_INFO("error to bind port %d fail: %s", nPort, strerror(errno));
		return false;
	}

	if (listen(sock, 10) != 0)
	{
		close(sock);
		return false;
	}

	if (PORT_TYPE::PORT_TYPE_HTTP == portType) {
		m_tcpSock = sock;
	}
	else {
		m_sslTcpSock = sock;
	}

	struct epoll_event ev;
	ev.data.fd = sock;
	ev.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLRDHUP;
	int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, sock, &ev);
	if (0 != ret) {
		string strError = strerror(errno);
		LOG_ERROR("epoll_ctl add sock fail %d: %s", errno, strError.c_str());
		return false;
	}

	LOG_INFO("successful to build server port %d", nPort);
	return true;
}

void CEpollMng::DoAccept(int sock, PORT_TYPE portType)
{

	do {
		sockaddr_in addr;
		socklen_t len = sizeof(addr);
		int ns = accept(sock, (sockaddr*)&addr, &len);
		if (-1 == ns)
		{
			return;
		}

		SetNonblocking(ns);

		int optint = 1;
		setsockopt(ns, SOL_SOCKET, SO_KEEPALIVE, (char *)&optint, sizeof(optint));
		setsockopt(ns, IPPROTO_TCP, TCP_NODELAY, (char *)&optint, sizeof(optint));
		//int nZero = 0;
		//setsockopt(ns, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
		// 接收缓冲区
		//int nRecvBuf = 32 * 1024;//设置为32K
		//setsockopt(ns, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, sizeof(int));
		//发送缓冲区
		//int nSendBuf = 32 * 1024;//设置为32K
		//setsockopt(ns, SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuf, sizeof(int));


		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));

		if (PORT_TYPE::PORT_TYPE_SSL_TCP == portType)
		{
			ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
		}
		else
		{
			ev.events = EPOLLIN | EPOLLET;
		}	
		ev.data.fd = ns;
		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, ns, &ev) != 0)
		{
			string strError = strerror(errno);
			LOG_ERROR("epoll_ctl add sock fail %d: %s", errno, strError.c_str());
			return;
		}



		string strIP;
		strIP = inet_ntoa(addr.sin_addr);
		LOG_INFO("sock %d connect from : %s", ns, strIP.c_str());
		CFileMngS->put(strIP);

		auto sockInfo = std::make_shared<SOCKINFO>();
		SOCKINFO &info = *sockInfo;
		info.ssl_ = nullptr;
		info.ssl_connect = false;
		info.portType = portType;
		info.count = 0;
		info.sock = ns;
		info.ip = strIP;
		info.port = ntohs(addr.sin_port);
		info.pTimer = std::make_shared<UTIL_SELF::Timer>();
		info.pos = 0;
		info.pkgLen = 0;

		if (PORT_TYPE::PORT_TYPE_SSL_TCP == portType) {
			sslAcept(ns, sockInfo);
		}
		inputSockInfo(sockInfo);
		

		
	} while (true);
	return;
}

void CEpollMng::eraseSockInfo(int fd)
{
	auto it = m_mpSockInfo.find(fd);
	if (m_mpSockInfo.end() != it)
	{		
		m_mpSockInfo.erase(it);
		if (m_mpSockInfo.empty()) {
			map<int, shared_ptr<SOCKINFO> >().swap(m_mpSockInfo);
		}
	}	
	return;
}

void CEpollMng::inputSockInfo(shared_ptr<SOCKINFO> sockInfo)
{
	eraseSockInfo(sockInfo->sock);
	m_mpSockInfo[sockInfo->sock] = sockInfo;
	return;
}

void CEpollMng::DoEvent()
{
	vector<struct epoll_event> vecEvent(MAXEVENT);
	struct epoll_event *events = &vecEvent[0];
	while (!m_quit)
	{
		int size = static_cast<int>(vecEvent.size());
		int nfds = epoll_wait(m_epfd, events, size, 50);
		
		for (int i = 0; i < nfds; ++i)
		{
			if (m_quit)
			{
				break;
			}

			if ((events[i].data.fd == m_tcpSock)
				|| (events[i].data.fd == m_sslTcpSock)
				)
			{
				PORT_TYPE portType = (events[i].data.fd == m_tcpSock) ? PORT_TYPE::PORT_TYPE_HTTP : PORT_TYPE::PORT_TYPE_SSL_TCP;
				DoAccept(events[i].data.fd, portType);
			}
			else if (events[i].events & EPOLLIN)
			{
				SSL * ssl = nullptr;
				PORT_TYPE portType = getSockType(events[i].data.fd);
				if (PORT_TYPE::PORT_TYPE_SSL_TCP == portType)
				{
					handleReadSSL(events[i].data.fd);
				}
				else {
					DoRecv(events[i].data.fd, PORT_TYPE::PORT_TYPE_HTTP, ssl);
				}
			}
			else if ((events[i].events & EPOLLHUP) 
				|| (events[i].events & EPOLLERR)
				|| (events[i].events & EPOLLRDHUP)
				)
			{
				string strError = strerror(errno);
				LOG_INFO("EPOLLHUP sock fail %d: %s", errno, strError.c_str());
				CloseConnect(events[i].data.fd);
			}
			else if (events[i].events & EPOLLOUT)
			{
				bool bClose = false;
				int fd = events[i].data.fd;
				auto it = m_mpSockInfo.find(fd);
				if (m_mpSockInfo.end() != it) {
				
					bool bNext = false;					
					if (PORT_TYPE::PORT_TYPE_HTTP == it->second->portType){
						bNext = true;
					}
					else {
						if (it->second->ssl_connect) {
							bNext = true;
						}
						else {
							handleDataWrite(fd, it->second);
						}
					}

					if (bNext) {
						struct epoll_event ev;
						memset(&ev, 0, sizeof(ev));
						ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
						ev.data.fd = fd;
						//ch->events_ |= EPOLLOUT; 
						//ch->events_ &= ~EPOLLIN;

						std::shared_ptr<SND_MSG_INFO> info;
						while (getSndMsg(fd, info)) {
							if (!sndRawData(fd, info, bClose, it->second->portType, it->second->ssl_)) {
								ev.events = EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
								break;
							}
						}

						if (bClose) {
							if (it->second->ssl_connect) {
								if (it->second->ssl_connect) {
									if (it->second->ssl_) {
										SSL_shutdown(it->second->ssl_);
										SSL_free(it->second->ssl_);
									}
								}
							}
							CloseConnect(fd);
							LOG_INFO("close sock .. ");
						}
						else if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev) != 0)
						{
							string strError = strerror(errno);
							LOG_ERROR("EPOLLOUT epoll_ctl fail %d: %s", errno, strError.c_str());

						}
					}
					else {
						LOG_INFO("EPOLLOUT ...");
					}
				}
				else {				
					LOG_ERROR("EPOLLOUT sock");
				}
			}
			else
			{
				string strError = strerror(errno);
				LOG_INFO("\n\n\n\n************************...other sock fail %d: %s\n\n\n", errno, strError.c_str());
				CloseConnect(events[i].data.fd);
			}			
		}
		//////////////////////////////////////////////////
		if (size <= nfds) {
			size += MAXEVENT;
			vecEvent.resize(size);
		}
		handleTimeover();
	}
	LOG_INFO("DoEvent exit");
}

void CEpollMng::CloseConnect(int fd, bool Erase)
{
	LOG_INFO("close sock %d", fd);

	close(fd);
	struct epoll_event ev = { 0, { 0 } };
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, &ev);

	if (Erase)
	{
		eraseSockInfo(fd);
	}
	return;
}

void CEpollMng::handleTimeover()
{
	//LOG_INFO("handleTimeover ..............");
	bool bErase = false;
	auto it = m_mpSockInfo.begin();
	for (; it != m_mpSockInfo.end();)
	{
		//LOG_INFO("check socket %d", it->second->sock);
		if (it->second->pTimer->elapsed_seconds() >= MAX_IDLE_SEC)
		{
			//if (it->second->ssl_connect) {
				if (it->second->ssl_) {
					SSL_shutdown(it->second->ssl_);
					SSL_free(it->second->ssl_);
				}
			//}
			eraseSndMsg(it->first);
			//LOG_INFO("close socket %d", it->second->sock);
			bErase = true;
			CloseConnect(it->second->sock, false);						
			m_mpSockInfo.erase(it++);
		}
		else
		{
			++it;
		}
	}

	if (bErase) {
		if (m_mpSockInfo.empty()) {
			map<int, shared_ptr<SOCKINFO> >().swap(m_mpSockInfo);
		}
	}

	CMapMngS->handleTimeOver();
	return;
}

void CEpollMng::handleSvr()
{		
	DoEvent();
	
	return;
}

PORT_TYPE CEpollMng::getSockType(int fd) {
	PORT_TYPE type = PORT_TYPE::PORT_TYPE_HTTP;
	auto it = m_mpSockInfo.find(fd);
	if (m_mpSockInfo.end() == it)
	{
		LOG_INFO("***** not to find sock %d", fd);
		return type;
	}
	return it->second->portType;
}

void CEpollMng::DoRecv(int fd, PORT_TYPE type, SSL * ssl)
{
	auto it = m_mpSockInfo.find(fd);
	if (m_mpSockInfo.end() == it)
	{
		LOG_INFO("not to find sock %d", fd);
		return;
	}
	int ret = 0;
	do {
		it->second->pTimer->reset();
		int i = it->second->pos;
		char *pData = it->second->data;
		if (PORT_TYPE::PORT_TYPE_HTTP == type) {
			ret = recv(fd, &pData[i], DATA_BUFSIZE - i, 0);
		}
		else {
			ret = SSL_read(ssl, &pData[i], DATA_BUFSIZE - i);
		}
        if (0 == ret) {

            CloseConnect(fd);
            LOG_INFO("disconnect client");
            return;
        }
		else if (0 >= ret)
		{					
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				break;
			}
			else if (errno == EINTR)
			{
				continue;
			}
			CloseConnect(fd);
			LOG_INFO("close error %d  errMsg:%s", errno, strerror(errno));
			return;
		}
		i += ret;
		

		it->second->pos = handleRawDataHttp(it->second, pData, i);		
		if (it->second->pos >= DATA_BUFSIZE)
		{
			//LOG_ERROR("*****************warning to be happened by recv %d", it->second->pos);
			//it->second->pos = DATA_BUFSIZE;			
			CloseConnect(fd);
			return;
		}		
	} while (true);
	it->second->pTimer->reset();

	return;
}

char * CEpollMng::headEnd(char *pMsg, string & strFlag, int & recvLen)
{
	char *pStr = nullptr;
	if (recvLen >= 4) {
		bool bFind = false;
		string::size_type pos = 0;
		string strMothed(pMsg, 4);
		transform(strMothed.begin(), strMothed.end(), strMothed.begin(), ::toupper);
		for (auto & item : m_setMothed) {
			pos = strMothed.find(item);
			if (string::npos != pos) {
				bFind = true;
				break;
			}
		}
		if (!bFind) {
			//LOG_WARNING("inlegal mothed to receive ...(%s)", strMothed.c_str());
			//DATA_BUFSIZE
			//recvLen = 0;
			recvLen = DATA_BUFSIZE;
			return pStr;
		}		
	}
	else {		
		return pStr;
	}	
	
	//http 头部解析
	for (auto & flagLN : m_liLN) {		
		pStr = strstr(pMsg, flagLN.data());		
		if (pStr) {	
			strFlag = flagLN;
			break;
		}
	}	
	
	return pStr;
}

int CEpollMng::handleRawDataHttp(shared_ptr<SOCKINFO>  & sockInfo, char *pMsg, int recvLen)
{	
	//string strTest(pMsg, recvLen);
	//cout << strTest << endl;

	bool bSurport = true; //支持post get 
	int nRet = 0;
	if (sockInfo->pkgLen <= 0)
	{	
		string strFieldFlag;
		auto pStr = headEnd(pMsg, strFieldFlag, recvLen);
		if (!pStr)
		{			
			return recvLen;
		}
		
		int offsetLen = static_cast<int>(strFieldFlag.size());
		int headLen = pStr - pMsg;
		sockInfo->pPkgInfo = std::make_shared<PACKAGE_INFO>();
		sockInfo->pPkgInfo->strHead.assign(pMsg, headLen);	
		sockInfo->pPkgInfo->strFieldFlag = strFieldFlag;
		sockInfo->pPkgInfo->bodyLen = recvLen - headLen - offsetLen;
		pStr += offsetLen;		
		if (sockInfo->pPkgInfo->bodyLen > 0) {
			auto pkgTemp = std::make_shared<string>();
			pkgTemp->assign(pStr, sockInfo->pPkgInfo->bodyLen);
			sockInfo->vecBody.push_back(pkgTemp);						
		}		
		
		handleMothed(sockInfo->pPkgInfo);
		handleField(sockInfo->pPkgInfo);	
		sockInfo->pkgLen = recvLen;
		
	}
	else {		
		auto pkgTemp = std::make_shared<string>();
		pkgTemp->assign(pMsg, recvLen);
		sockInfo->vecBody.push_back(pkgTemp);		
		sockInfo->pPkgInfo->bodyLen += recvLen;
		sockInfo->pkgLen += recvLen;
	}		

	bool bPost = false;
	int postLen = -1;
	if (fowardMsg(sockInfo->pPkgInfo, bPost, postLen))
	{			
		if (sockInfo->pPkgInfo->bodyLen > 0) {
			string & strRef = sockInfo->pPkgInfo->strBody;
			strRef.clear();
			strRef.resize(sockInfo->pPkgInfo->bodyLen);
			int i = 0;
			int len = 0;
			for (auto & it : sockInfo->vecBody) {
				len = static_cast<int>(it->size());
				memcpy(&strRef[i], it->data(), len);
				i += len;
			}
			sockInfo->vecBody.clear();
		}		
		if (bPost) {	
			if ((postLen < sockInfo->pPkgInfo->bodyLen) && (postLen > 0)) {			

				sockInfo->extData.clear();
				sockInfo->extLen = sockInfo->pPkgInfo->bodyLen - postLen;
				string & refExt = sockInfo->extData;
				refExt.resize(sockInfo->extLen);

				string & refBoyd = sockInfo->pPkgInfo->strBody;
				memcpy(&refExt[0], &refBoyd[postLen], sockInfo->extLen);
				refBoyd.erase(postLen, sockInfo->extLen);				
			}			
		}
		else if (HTTP_GET == sockInfo->pPkgInfo->commandID){
			
			sockInfo->pPkgInfo->vecFormData.clear();
			sockInfo->pPkgInfo->strBody.clear();			
		}
		else {
			bSurport = false;
			LOG_WARNING("no surport mothed %d", sockInfo->pPkgInfo->commandID);
		}
		
		sockInfo->pkgLen = 0;
		sockInfo->pPkgInfo->bodyLen = 0;

		if (bSurport) {	
			sockInfo->pPkgInfo->ssl_ = sockInfo->ssl_;
			sockInfo->pPkgInfo->ssl_connect = sockInfo->ssl_connect;
			sockInfo->pPkgInfo->portType = sockInfo->portType;
			sockInfo->pPkgInfo->ip = sockInfo->ip;
			sockInfo->pPkgInfo->sock = sockInfo->sock;
			if (!existCache(sockInfo->pPkgInfo)) {
				CMapMngS->inputMsg(sockInfo->pPkgInfo);
			}
		}
		else {
			nRet = DATA_BUFSIZE;
			//关闭sock
		}
		
		//http 1.1
	}
	return nRet;
}

bool CEpollMng::existCache(std::shared_ptr<PACKAGE_INFO> & pPkgInfo)
{
	bool bResult = false;	
	if (HTTP_GET == pPkgInfo->commandID) {
		pPkgInfo->strResp.clear();
		bResult = CMapMngS->getQueryCache(pPkgInfo->uri, pPkgInfo->strResp);
		if (bResult) {
			sndMsg(pPkgInfo->sock, pPkgInfo->strResp);
		}
	}
	return bResult;
}

void CEpollMng::resetPkg(std::shared_ptr<PACKAGE_INFO> & pPkgInfo)
{
	pPkgInfo->url.clear();
	pPkgInfo->strFieldFlag.clear();
	pPkgInfo->strResp.clear();
	pPkgInfo->strHead.clear();
	pPkgInfo->strBody.clear();
	pPkgInfo->mField.clear();
	pPkgInfo->mParam.clear();
	pPkgInfo->mOriginalParam.clear();
}

void CEpollMng::httpHtmlResp(std::shared_ptr<PACKAGE_INFO> & pPkgInfo)
{
	if (!pPkgInfo->strResp.empty()) {		
		string strRespData;
		httpBuildResp(strRespData, pPkgInfo);
		bool bRet = sndMsg(pPkgInfo->sock, strRespData);
		if (bRet) {
			bRet = sndMsg(pPkgInfo->sock, pPkgInfo->strResp);
			if (bRet) {
				//LOG_INFO("%s", pPkgInfo->strResp.c_str());
			}
		}
		if ((HTTP_GET == pPkgInfo->commandID) && (pPkgInfo->bRespOk)) {
			strRespData += pPkgInfo->strResp;
			CMapMngS->inputQueryCache(pPkgInfo->uri, strRespData);
		}
	}
	else {
		close(pPkgInfo->sock);
	}	
	resetPkg(pPkgInfo);
}

void CEpollMng::handleSndMsg(shared_ptr<SOCKINFO> & sockInfo)
{
	if (!sockInfo->pPkgInfo->strResp.empty()) {		
		string strRespData;
		httpBuildResp(strRespData, sockInfo->pPkgInfo);
		bool bRet = sndMsg(sockInfo->sock, strRespData);
		if (bRet) {
			bRet = sndMsg(sockInfo->sock, sockInfo->pPkgInfo->strResp);
			if (bRet) {
				//LOG_INFO("%s", sockInfo->pPkgInfo->strResp.c_str());
			}
		}		
	}
	return;
}

void CEpollMng::setContentType()
{
	//https://tool.oschina.net/commons/
	m_RespType[RESP_CONTENT_TYPE::TXT_TYPE] = "Content-Type: text/plain";
	m_RespType[RESP_CONTENT_TYPE::ORIGIN_TYPE] = "Content-Type: text/html";
	m_RespType[RESP_CONTENT_TYPE::JSON_TYPE] = "Content-Type: application/json";
	m_RespType[RESP_CONTENT_TYPE::HTML_TYPE] = "Content-Type: text/html;charset=utf-8";
	m_RespType[RESP_CONTENT_TYPE::JS_TYPE] = "Content-Type: application/x-javascript";
	m_RespType[RESP_CONTENT_TYPE::CSS_TYPE] = "Content-Type: text/css";
	m_RespType[RESP_CONTENT_TYPE::PNG_TYPE] = "Content-Type: image/png";
	m_RespType[RESP_CONTENT_TYPE::JPG_TYPE] = "Content-Type: image/jpeg";
	m_RespType[RESP_CONTENT_TYPE::GIF_TYPE] = "Content-Type: image/gif";
	m_RespType[RESP_CONTENT_TYPE::ICO_TYPE] = "Content-Type: image/x-icon";
	m_RespType[RESP_CONTENT_TYPE::BMP_TYPE] = "Content-Type: image/x-bmp";
	m_RespType[RESP_CONTENT_TYPE::JPG_TYPE] = "Content-Type: image/jpeg";

}

void CEpollMng::httpBuildResp(string & strInfo, std::shared_ptr<PACKAGE_INFO> & pPkgInfo)
{	
	string  strGmt;
	UTIL_SELF::getGmtTime(strGmt);
	strInfo = "HTTP/1.1 200 OK\r\nServer: faceSvr\r\nDate:";		
	strInfo += strGmt;
	strInfo += CRLN_FLAG;
	strInfo += ContentLength1;
	strInfo += std::to_string(pPkgInfo->strResp.size());
	strInfo += CRLN_FLAG;
	if (!pPkgInfo->contentEncoding.empty()) {
		strInfo += pPkgInfo->contentEncoding;
		strInfo += CRLN_FLAG;
	}
	auto it = m_RespType.find(pPkgInfo->respType);
	if (m_RespType.end() == it) {	
		strInfo += "Content-Type: application/json";
		LOG_ERROR("not to find the Content-Type");
	}
	else {
		strInfo += it->second;
	}	
	strInfo += HEAD_END_FLAG;
	return;
}

bool CEpollMng::sndMsg(int fd, string &pkg)
{	
	auto sndInfo = std::make_shared<SND_MSG_INFO>();
	sndInfo->pos = 0;
	sndInfo->msg = pkg;
	inputSndMsg(fd, sndInfo);

	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLOUT|EPOLLET|EPOLLHUP|EPOLLERR|EPOLLRDHUP;
	ev.data.fd = fd;
	if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev) != 0)
	{
		string strError = strerror(errno);
		LOG_ERROR("sndMsg epoll_ctl fail %d: %s", errno, strError.c_str());
		return false;
	}
	//LOG_INFO("notify EPOLLOUT");
	return true;
}

bool CEpollMng::sndRawData(int fd, std::shared_ptr<SND_MSG_INFO> & info,
	bool & bClose, PORT_TYPE type, SSL * ssl)
{
	bool bResult = true;
	string & pkg = info->msg;
	int len = static_cast<int>(pkg.size());
	char *pData = reinterpret_cast<char*>(&pkg[0]);
	int i = info->pos;
	int ret = 0;
	while (i < len) {
		if (PORT_TYPE::PORT_TYPE_HTTP == type) {
			ret = send(fd, &pData[i], len - i, 0);
		}
		else {
			ret = SSL_write(ssl, &pData[i], len - i);
		}
		if (ret > 0)
		{
			i += ret;			
		}
		else if (0 == ret)
		{
			bClose = true;			
			return false;
		}
		else {
			break;
		}
		info->pos = i;		
	}
	if (info->pos < len) {
		info->msg.erase(0, info->pos);
		info->pos = 0;
		inputSndMsg(fd, info, false);
		bResult = false;
	}

	return bResult;
}

void CEpollMng::getLNFlag(string & flag, string & strInfo)
{
	string::size_type size = strInfo.size();
	int len = size / 2;
	flag = len > 0 ? strInfo.substr(0, len) : strInfo;
	return;
}

void CEpollMng::handleField(std::shared_ptr<PACKAGE_INFO> & pInfo)
{
	std::unordered_map<string, string> & mField = pInfo->mField;
	string & strHead = pInfo->strHead;	
	string strLimit = ":";
	string strSpace = " ";
	string strFlag;
	getLNFlag(strFlag, pInfo->strFieldFlag);
	string::size_type pos = 0, next = 0;
	pos = strHead.find(strFlag);
	if (string::npos == pos) {		
		return;
	}
	pos += strFlag.size();
	do {
		next = strHead.find(strLimit, pos);
		if (string::npos == next) {			
			break;
		}
		string key = strHead.substr(pos, next - pos);
		pos = next;
		pos += strLimit.size();
		pos = strHead.find_first_not_of(strSpace, pos);
		if (string::npos == pos) {
			break;
		}
		next = strHead.find(strFlag, pos);
		if (string::npos == next) {
			break;
		}
		string value = strHead.substr(pos, next - pos);
		transform(key.begin(), key.end(), key.begin(), (int(*)(int))tolower);
		if (!value.empty()) {
			mField[key] = value;
		}
		pos = next;
		pos += strFlag.size();
	} while (true);

	return;
}

bool CEpollMng::getContentLength(int & len, string & strInfo, string flag)
{
	bool bResult = false;
	string::size_type pos = strInfo.find(flag);
	if (pos == string::npos)
	{
		return bResult;
	}

	pos += flag.size();
	string::size_type next = strInfo.find(CRLN_FLAG, pos);
	if (pos == string::npos)
	{
		LOG_INFO("not to find crln!!");
		return bResult;
	}
	string strTemp = strInfo.substr(pos, next - pos);
	len = std::atoi(UTIL_SELF::trim(strTemp).c_str());
	return true;
}

void CEpollMng::handleMothed(std::shared_ptr<PACKAGE_INFO> & pInfo)
{
	string sapceFlag = " ";
	string::size_type pos, next;
	pos = pInfo->strHead.find(sapceFlag);
	if (string::npos == pos)
	{
		return;
	}
	string strMothed = pInfo->strHead.substr(0, pos);
	pos = pInfo->strHead.find_first_not_of(sapceFlag, pos);
	if (string::npos == pos)
	{
		return;
	}

	next = pInfo->strHead.find(sapceFlag, pos);
	if (string::npos == next)
	{
		return;
	}
	string strParam = pInfo->strHead.substr(pos, next - pos);
	UTIL_SELF::trimstr(strParam);
	pInfo->uri = strParam;

	int size = static_cast<int>(strParam.size());
	if (size > 1) {
		int i = size - 1;
		if ('/' == strParam[i]) {
			strParam.pop_back();
		}
	}

	UTIL_SELF::trimstr(strMothed);
	transform(strMothed.begin(), strMothed.end(), strMothed.begin(), ::toupper);

	pInfo->url = strParam;
	pInfo->commandID = HTTP_POST;
	if (GET_MOTHED == strMothed) {
		pInfo->commandID = HTTP_GET;
	}
	getMothedParam(pInfo, strParam);
	//LOG_INFO("mothed %s param %s", strMothed.c_str(), strParam.c_str());
	return;
}

void CEpollMng::getMothedParam(std::shared_ptr<PACKAGE_INFO> & pInfo, string & strInfo)
{	
	string strFlag = "?";
	string::size_type pos, next;
	pos = strInfo.find(strFlag);
	if (string::npos == pos)
	{
		return;
	}
	string strTemp = strInfo.substr(0, pos);
	pInfo->url = UTIL_SELF::trim(strTemp);

	pos += strFlag.size();
	strFlag = "&";

	string::size_type size = strInfo.size();
	do {
		next = strInfo.find(strFlag, pos);
		if (string::npos == next)
		{
			strTemp = strInfo.substr(pos, size - pos);
			getUrlInfo(pInfo, strTemp);

			break;
		}
		else
		{
			strTemp = strInfo.substr(pos, next - pos);
			getUrlInfo(pInfo, strTemp);
			pos = next;
			pos += strFlag.size();
		}
	} while (true);

	return;
}

void CEpollMng::getUrlInfo(std::shared_ptr<PACKAGE_INFO> & pInfo, string & strInfo)
{
	
	auto & mpOriginal = pInfo->mOriginalParam;
	auto & mpInfo = pInfo->mParam;
	string strFlag = "=";
	string::size_type size = strInfo.size();
	string::size_type pos = strInfo.find(strFlag);
	string key = strInfo.substr(0, pos);
	if (key.empty()) {
		LOG_WARNING("empty key(%s)", strInfo.c_str());
		return;
	}
	pos += strFlag.size();
	string value = strInfo.substr(pos, size - pos);
	key = UTIL_SELF::UrlDecode(key);
	mpOriginal[key] = value;
	value = UTIL_SELF::UrlDecode(value);
	UTIL_SELF::trimstr(value);
	mpInfo[key] = value;
	
	return;
}

bool CEpollMng::fowardMsg(std::shared_ptr<PACKAGE_INFO> & pInfo, bool & bPost, int & len)
{
	len = -1;
	bool bResult = true;
	if (HTTP_POST == pInfo->commandID) {
		bPost = true;
		auto it = pInfo->mField.find("content-length");
		if (pInfo->mField.end() != it) {
			len = atoi(it->second.c_str());
			if (pInfo->bodyLen < len) {
				bResult = false;
			}
		}
	}
	return bResult;
}

void CEpollMng::inputSndMsg(int fd, std::shared_ptr<SND_MSG_INFO> & msg, bool back)
{	
	std::lock_guard<std::mutex>lck(m_sndLock); 
	auto it = m_sndMsg.find(fd);
	if (m_sndMsg.end() == it) {	

		m_sndMsg[fd] = list<std::shared_ptr<SND_MSG_INFO>>();
		m_sndMsg[fd].clear();
	}	
	if (back) {
		m_sndMsg[fd].push_back(msg);
	}
	else {
		m_sndMsg[fd].push_front(msg);
	}
}

bool CEpollMng::getSndMsg(int fd, std::shared_ptr<SND_MSG_INFO> & msg)
{
	bool bResult = false;
	{
		std::lock_guard<std::mutex>lck(m_sndLock);
		auto it = m_sndMsg.find(fd);
		if (m_sndMsg.end() != it) {
			if (it->second.size() > 0) {
				msg = it->second.front();
				it->second.pop_front();
				bResult = true;
				
			}
			if (it->second.size() <= 0) {
				m_sndMsg.erase(it);
			}
			if (m_sndMsg.empty()) {
				map<int, list<std::shared_ptr<SND_MSG_INFO>>>().swap(m_sndMsg);
			}
		}
	}
	return bResult;
}

void  CEpollMng::eraseSndMsg(int fd)
{
	std::lock_guard<std::mutex>lck(m_sndLock);
	auto it = m_sndMsg.find(fd);
	if (m_sndMsg.end() != it) {
		m_sndMsg.erase(it);
	}
}

SSL_CTX * CEpollMng::getCtx(string & key)
{
	SSL_CTX * ctx = nullptr;
	auto it = m_mpCtxSSL.find(key);
	if (m_mpCtxSSL.end() != it) {
		ctx = it->second;
	}
	else {
		LOG_ERROR("not to find %s", key.c_str());
	}

	return ctx;
}

static int ssl_servername_cb(SSL *s, int *ad, void *arg) {
	if (s == NULL) {
		LOG_ERROR("error!! ssl_servername_cb");
		//return SSL_TLSEXT_ERR_NOACK;
	}
	
	const char* servername = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
	//LOG_ERROR("ServerName: %s", servername);
	string key = servername;
	SSL_CTX * ctx = CEpollMngS->getCtx(key);
	if (ctx) {
		//SSL *ssl = SSL_new(ctx);
		//*s = *ssl;
		//memcpy(s, ssl, sizeof(ssl_st));
		SSL_set_SSL_CTX(s, ctx);
        SSL_set_verify(s, SSL_CTX_get_verify_mode(ctx),
            SSL_CTX_get_verify_callback(ctx));
        SSL_set_verify_depth(s, SSL_CTX_get_verify_depth(ctx));
        SSL_set_options(s, SSL_CTX_get_options(ctx));
        //LOG_ERROR("*** ssl_servername_cb %s", key.c_str());
	}

	return SSL_TLSEXT_ERR_OK;
}

void CEpollMng::initSSL(string & path)
{
#if OPENSSL_VERSION_NUMBER >= 0x10100003L
	if (OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL) == 0) {
	
	}

	/*
	* OPENSSL_init_ssl() may leave errors in the error queue
	* while returning success
	*/

	ERR_clear_error();
#else

	OPENSSL_config(NULL);
	SSL_library_init();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();

#endif

    string strClipher = "TLS13-AES-256-GCM-SHA384:TLS13-CHACHA20-POLY1305-SHA256:TLS13-AES-128-GCM-SHA256:TLS13-AES-128-CCM-8-SHA256:TLS13-AES-128-CCM-SHA256:EECDH+CHACHA20:EECDH+CHACHA20-draft:EECDH+ECDSA+AES128:EECDH+aRSA+AES128:RSA+AES128:EECDH+ECDSA+AES256:EECDH+aRSA+AES256:RSA+AES256:EECDH+ECDSA+3DES:EECDH+aRSA+3DES:RSA+3DES:!MD5";
	int r = 0;
	errBio = BIO_new_fd(2, BIO_NOCLOSE);
	//g_sslTemp = SSL_CTX_new(SSLv23_server_method());
	//SSL_CTX_set_tlsext_servername_callback(g_sslTemp, ssl_servername_cb);
    //SSL_CTX_set_cipher_list(g_sslTemp, strClipher.c_str());
	
	vector<SSL_CRT> vecPem;
    SSL_CRT sslInfo;
    sslInfo.ca = path + "1_root_bundle.crt";
    sslInfo.crt = path + "1_www.dswd.net_bundle.crt";
    sslInfo.key = path + "2_www.dswd.net.key";
    //sslInfo.ca = path + "1_root_bundle.crt";
    //sslInfo.crt = path + "2_www.dswd.net.crt";
    //sslInfo.key = path + "3_www.dswd.net.key";
	vecPem.push_back(sslInfo);
    sslInfo.ca = path + "1_root_bundle.crt";
    sslInfo.crt = path + "1_www.qytmail.com_bundle.crt";
    sslInfo.key = path + "2_www.qytmail.com.key";
    //sslInfo.crt = path + "2_www.qytmail.com.crt";
    //sslInfo.key = path + "3_www.qytmail.com.key";
	vecPem.push_back(sslInfo);
	
    int i = 0;
	for (auto & pem : vecPem) 
    {	
        auto  sslCtx = SSL_CTX_new(TLS_server_method());
		//auto  sslCtx = SSL_CTX_new(SSLv23_server_method());
        //SSL_CTX_set_ecdh_auto(sslCtx, 1);
        SSL_CTX_set_cipher_list(sslCtx, strClipher.c_str());
		//string cert = pem.crt;
		//string key = pem.key;
        //r = SSL_CTX_use_certificate_chain_file(sslCtx, cert.c_str());
        //SSL_CTX_load_verify_locations(sslCtx, pem.ca.c_str(), NULL);
		//r = SSL_CTX_use_certificate_file(sslCtx, pem.crt.c_str(), SSL_FILETYPE_PEM);
        //SSL_CTX_set_default_passwd_cb_userdata(ctx, "lym123");
        r = SSL_CTX_use_certificate_chain_file(sslCtx, pem.crt.c_str());
		r = SSL_CTX_use_PrivateKey_file(sslCtx, pem.key.c_str(), SSL_FILETYPE_PEM);

        //SSL_CTX_load_verify_locations(sslCtx, "1_root_bundle.crt", NULL);
        //r = SSL_CTX_use_certificate_file(sslCtx, "2_www.dswd.net.crt", SSL_FILETYPE_PEM);
        //r = SSL_CTX_use_PrivateKey_file(sslCtx, "3_www.dswd.net.key", SSL_FILETYPE_PEM);
		//r = SSL_CTX_check_private_key(sslCtx);	
        if (0 == i) {
            m_mpCtxSSL["www.dswd.net"] = sslCtx;
            g_sslTemp = sslCtx;
            SSL_CTX_set_tlsext_servername_callback(sslCtx, ssl_servername_cb);
        }
        else if (1 == i) {
            m_mpCtxSSL["www.qytmail.com"] = sslCtx;
        }
		//
        i++;
	}
    return;
}

void CEpollMng::sslAcept(int fd, shared_ptr<SOCKINFO> & info)
{
	string domain = "www.dswd.net";
	//info->ssl_ = SSL_new(m_mpCtxSSL[domain]);
	info->ssl_ = SSL_new(g_sslTemp);
	int r = SSL_set_fd(info->ssl_, fd);
	SSL_set_accept_state(info->ssl_);

	//LOG_ERROR("11111111111111111111111");
	return;
}

void CEpollMng::handleHandshake(int fd, shared_ptr<SOCKINFO> & info)
{
	if (nullptr == info->ssl_) {
		
		/*string domain = "www.dswd.net";
		info->ssl_ = SSL_new(m_mpCtxSSL[domain]);
		int r = SSL_set_fd(info->ssl_, fd);
		SSL_set_accept_state(info->ssl_);*/
		//LOG_ERROR("11111111111111111111111");
		
	}
	
	int r = SSL_do_handshake(info->ssl_);
	if (r == 1) {
		info->ssl_connect = true;	
        LOG_ERROR("8888888888888888888888 SSL_do_handshake");
		return;
	}
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	int err = SSL_get_error(info->ssl_, r);
	if (err == SSL_ERROR_WANT_WRITE) {		
		ev.events = EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
		ev.data.fd = fd;
		int ret = epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
        if (0 != ret) {
            string strError = strerror(errno);
            LOG_ERROR("epoll_ctl add sock fail %d: %s", errno, strError.c_str());
        }
        LOG_ERROR("2222222222222222222222222222 SSL_ERROR_WANT_WRITE");
	}
	else if (err == SSL_ERROR_WANT_READ) {
	
		ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
		ev.data.fd = fd;
		int ret = epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
        if (0 != ret) {
            string strError = strerror(errno);
            LOG_ERROR("epoll_ctl add sock fail %d: %s", errno, strError.c_str());
        }
        LOG_ERROR("3333333333333333333333333333 SSL_ERROR_WANT_READ");
	}
	else {
		LOG_ERROR("SSL_do_handshake return %d error %d errno %d msg %s\n", r, err, errno, strerror(errno));
		ERR_print_errors(errBio);
		
	}
	return;
}


void CEpollMng::handleDataRead(int fd, shared_ptr<SOCKINFO> & info)
{
	DoRecv(fd, PORT_TYPE::PORT_TYPE_SSL_TCP, info->ssl_);
}

void CEpollMng::handleReadSSL(int fd)
{
	auto it = m_mpSockInfo.find(fd);
	if (m_mpSockInfo.end() == it) {

		LOG_ERROR("error to be happended about openssl %d", fd);
		return;
	}
	if (it->second->ssl_connect) {
		handleDataRead(fd, it->second);
	}
	else {
		handleHandshake(fd, it->second);
	}

	return;
}

void CEpollMng::handleDataWrite(int fd, shared_ptr<SOCKINFO> & info)
{
	if (info->ssl_connect) {
	
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
		ev.data.fd = fd;
		int ret = epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &ev);
        if (0 != ret) {
            string strError = strerror(errno);
            LOG_ERROR("epoll_ctl add sock fail %d: %s", errno, strError.c_str());
        }
		return;
	}
	handleHandshake(fd, info);
	return;
}


