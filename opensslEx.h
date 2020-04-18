#pragma once
#include "common.h"
#include "log.h"
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>



struct Channel {
	int epollfd;
	int events_;
	int fd_;
	SSL *ssl_;
	bool sslConnected_;
	Channel(int fd)
	{
		fd_ = fd;
		ssl_ = nullptr;
		sslConnected_ = false;
	}
	void update() {
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.events = events_;
		ev.data.fd = fd_;	
		int r = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd_, &ev);
		
	}
	void updateEx(int value) {
		struct epoll_event ev;
		memset(&ev, 0, sizeof(ev));
		ev.events = value;//EPOLLIN | EPOLLET | EPOLLOUT;
		ev.data.fd = fd_;
		int r = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd_, &ev);
	}
	~Channel() {
		
		if (ssl_) {
			SSL_shutdown(ssl_);
			SSL_free(ssl_);
		}
	}
};

class CSSMng{

public:
	void setSSL(SOCKET sock, std::shared_ptr<Channel> & ch);
	bool getSSL(SOCKET sock, std::shared_ptr<Channel> & ch);
	void eraseSSL(SOCKET sock);

private:
	
	map<int, std::shared_ptr<Channel> >	m_mpSSL;  //socket-->sslConnected_(false)
};