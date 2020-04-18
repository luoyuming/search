
// MFCOne.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once
#include <stdio.h>
#include <math.h>
#include <atomic>
#include <stdlib.h>
#include <limits.h>
#include <memory>
#include <list>
#include <map>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ratio>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <cctype>
#include <algorithm>
#include <iterator>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <string.h>
#include <regex>
#include <limits.h>
#include <locale>         // std::wstring_convert
#include <malloc.h>

#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/resource.h>
#include <sys/epoll.h>
#include <unistd.h>  
#include <pthread.h>  
#include <signal.h> 
#include <netinet/tcp.h>
#include <netdb.h>

using namespace std;
using namespace std::chrono;

#define LOG_OUT_PRINT 1



typedef signed char			byte1;
typedef signed short		byte2;
//typedef signed int			byte4;
typedef signed long long	byte8;
typedef unsigned char		ubyte1;
typedef unsigned short		ubyte2;
//typedef unsigned int		ubyte4;
typedef unsigned long long	ubyte8;
typedef unsigned char		uchar;
typedef unsigned short		ushort;

typedef  signed short			word2;
typedef  unsigned short			uword2;
typedef  int32_t				byte4;
typedef  uint32_t				ubyte4;
typedef  int32_t				SOCKET;
#define  INVALID_SOCKET			(SOCKET)(~0)



