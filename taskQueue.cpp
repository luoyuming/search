
#include "taskQueue.h"


CTaskQueue::CTaskQueue() :m_bQuit(false)
{
	
}

void CTaskQueue::setQuit()
{	
	m_bQuit = true;	
	m_cvCmd.notify_all();
}

void CTaskQueue::inputCmd(std::shared_ptr<PACKAGE_INFO> & cmd)
{
	std::unique_lock <std::mutex> lck(m_lockCmd);
	m_CmdQ.push_back(cmd);
	m_cvCmd.notify_all();
}

bool CTaskQueue::full()
{
	bool bResult = false;
	int len = getLen();
	if (len >= MAX_QUEUE_LEN) {
		bResult = true;
	}
	return bResult;
}

int CTaskQueue::getLen()
{
	int size = 0;
	{ 
		std::lock_guard <std::mutex> lck(m_lockCmd);
		size = static_cast<int>(m_CmdQ.size());
	}
	return size;
}

bool CTaskQueue::getCmd(std::shared_ptr<PACKAGE_INFO> & cmd)
{
	std::unique_lock <std::mutex> lck(m_lockCmd);
	while (m_CmdQ.empty() && (!m_bQuit))
	{
		m_cvCmd.wait(lck);
	}
	if (m_bQuit) {
		return false;
	}
	cmd = std::move(m_CmdQ.front());
	m_CmdQ.pop_front();
	return true;
}