#include "opensslEx.h"


void CSSMng::setSSL(SOCKET sock, std::shared_ptr<Channel> & ch)
{
	m_mpSSL[sock] = ch;
}

bool CSSMng::getSSL(SOCKET sock, std::shared_ptr<Channel> & ch)
{
	bool bResult = false;
	{	
		auto it = m_mpSSL.find(sock);
		if (m_mpSSL.end() != it)
		{
			ch = it->second;
			bResult = true;
		}
	}
	return bResult;
}

void CSSMng::eraseSSL(SOCKET sock)
{
	{		
		auto it = m_mpSSL.find(sock);
		if (m_mpSSL.end() != it)
		{
			m_mpSSL.erase(it);
			if (m_mpSSL.empty())
			{
				map<int, std::shared_ptr<Channel> >().swap(m_mpSSL);
			}
		}
	}
}