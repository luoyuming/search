// keyword.cpp : 定义控制台应用程序的入口点。
//



// Copyright 2015 Kevin B. Hendricks, Stratford, Ontario,  All Rights Reserved.
// loosely based on a greatly simplified version of BeautifulSoup4 decode() routine
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Kevin Hendricks
//
// Prettyprint back to html / xhtml

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <cstring>
#include "common.h"
#include "gumbo.h"
#include <strings.h>


#include "gumbo.h"
#include "html.h"
#include "log.h"
#include "splitWord.h"
#include "mapMng.h"
#include "storeMng.h"
#include "epollMng.h"
#include "gzipCode.h"
#include "httpClient.h"
#include "util.h"
#include "gumbo/examples/test_utils.h"
#include "handleXml.h"
#include "linksMng.h"
#include "html.h"
#include "uri.h"
#include "rankMng.h"
#include "faceCGI_C.h"

std::atomic<bool> g_stop(false);

void SignalHandler(int sig);
int main(int argc, char** argv)
{	
	//
	//信号处理
	signal(SIGABRT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGSEGV, SIG_IGN);
	signal(SIGTRAP, SIG_IGN);
	signal(SIGUSR1, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGINT, SignalHandler);
	signal(SIGHUP, SignalHandler);
	signal(SIGCHLD, SignalHandler);
	signal(SIGQUIT, SignalHandler);

	LOG_INIT(argv[0]);
	LOG_INFO("begin to run...%d", argc);		
	{
		CHttpClientS->init();	
		//CHttpClientS->test();		
		//CHtml html;		
		//html.test();
		//CFCGI_C cgi;
		//cgi.forward();
		//return 1;
	}
	
	if (!CEpollMngS->init()) {
		LOG_ERROR("error to init EpollMng ");
		exit(EXIT_FAILURE);
	}

	if (!CSplitWordS->init()){
		LOG_ERROR("error to run word model");
		exit(EXIT_FAILURE);
	}
	if (!CMapMngS->init()) {
		LOG_ERROR("error to init CMapMngS");
		exit(EXIT_FAILURE);
	}
	if (!CStoreMngS->init()) {
		LOG_ERROR("error to init CStoreMngS");
		exit(EXIT_FAILURE);
	}
	CMapMngS->buildThread();
	std::this_thread::sleep_for(std::chrono::seconds(2));
	CMapMngS->lanuchSpider(true);	
	cout << "\n\n\n***************************running service now*************************\n\n" << endl;
	CEpollMngS->handleSvr();
	LOG_INFO("quit to main loop...");	
	CMapMngS->join();
	CStoreMngS->Quit();
	LOG_INFO("quit to run service...");
	
	return 0;
}

void SignalHandler(int sig)
{
	LOG_INFO("server received a signal: %d", sig);
	switch (sig)
	{
	case SIGTERM:
	case SIGINT:
	case SIGKILL:
	case SIGQUIT:
	case SIGHUP: {
		LOG_INFO("quit signal: %d", sig);
		CEpollMngS->stop();
		CMapMngS->stop();	
		g_stop.store(true);
	}
		break;
	case SIGALRM:
		break;
	case SIGUSR1:
		break;
	default:
		break;
	}
}

