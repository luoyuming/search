#include "log.h"


CGLog * CGLog::m_pInstance = nullptr;
CGLog::CGLog(void)
{
#ifdef LOG_OUT_PRINT
	//memset(pLogBuff, 0, MAX_BUFF_LOG_LEN);
#endif
}

CGLog::~CGLog(void)
{
#ifdef LOG_OUT_PRINT
	
	google::ShutdownGoogleLogging();
#endif
}

int32_t CGLog::InitGLog(const char * argv0)
{	
#ifdef LOG_OUT_PRINT
	mkdir("./log", 0755);
	google::InitGoogleLogging(argv0);

	google::SetStderrLogging(google::GLOG_INFO); //设置级别高于 google::INFO 的日志同时输出到屏幕
	google::SetLogDestination(google::GLOG_FATAL, "./log/log_fatal_"); // 设置 google::FATAL 级别的日志存储路径和文件名前缀
	google::SetLogDestination(google::GLOG_ERROR, "./log/log_error_"); //设置 google::ERROR 级别的日志存储路径和文件名前缀
	google::SetLogDestination(google::GLOG_WARNING, "./log/log_warning_"); //设置 google::WARNING 级别的日志存储路径和文件名前缀
	google::SetLogDestination(google::GLOG_INFO, "./log/log_info_"); //设置 google::INFO 级别的日志存储路径和文件名
	
	FLAGS_max_log_size = 100; //最大日志大小为 100MB
	FLAGS_stop_logging_if_full_disk = true; //当磁盘被写满时，停止日志输出
	FLAGS_alsologtostderr = true;
	FLAGS_colorlogtostderr = true; //设置输出到屏幕的日志显示相应颜色
	//FLAGS_log_prefix = true;  //设置日志前缀是否应该添加到每行输出
	//FLAGS_stop_logging_if_full_disk = true;  //设置是否在磁盘已满时避免日志记录到磁盘
#endif	
	return 0;
}

void CGLog::GLogMsg(const char * funName, int lineNum, uint32_t nLogSeverity, const char *format, ...)
{
#ifdef LOG_OUT_PRINT
	string strTemp;
	strTemp.resize(MAX_BUFF_LOG_LEN);
	char *pLogBuff = const_cast<char*>(strTemp.data());
	va_list arg_ptr;
	va_start(arg_ptr, format);
	vsnprintf(pLogBuff,MAX_BUFF_LOG_LEN, format, arg_ptr);
	switch (nLogSeverity)
	{
	case 0:
		LOG(INFO) << "[" << funName << ":" << lineNum <<"] " << pLogBuff;
		break;
	case 1:
		LOG(WARNING) << "[" << funName << ":" << lineNum << "] " << pLogBuff;
		break;
	case 2:
		LOG(ERROR) << "[" << funName << ":" << lineNum << "] " << pLogBuff;
		break;
	case 3:
		LOG(FATAL) << "[" << funName << ":" << lineNum << "] " << pLogBuff;
		break;
	default:
		break;
	}
	va_end(arg_ptr);

	//memset(pLogBuff, 0, MAX_BUFF_LOG_LEN);
	
#endif
	return;
}

