#include "patcher_service.h"

#ifdef BOOST_PATCH_SERVICE

void CPatchService::RunSingle()
{
	boost::system::error_code error;

	while (!m_Service.stopped())
	{
		try
		{
			m_Service.run_one(error);

			if (error)
			{
				break;
			}
		}
		catch (...)
		{
			break;
		}

		if (m_WorkCount-- == 1)
		{
			m_WorkMutex.lock();
			m_WorkCondition.notify_all();
			m_WorkMutex.unlock();
		}
	}

	m_WorkCondition.notify_all();
}

CPatchService::~CPatchService()
{
	m_WorkerGroup.interrupt_all();
}

CPatchService::CPatchService() :
	m_Strand(m_Service),
	m_WorkCount(0)
{
}

CPatchService::CPatchService(size_t iCount) :
	m_Strand(m_Service),
	m_WorkCount(0)
{
	SetupPatchThreads(iCount);
}

void CPatchService::ShutdownAll()
{
	m_Service.stop();

	for (std::vector<boost::thread*>::iterator it = m_Threads.begin(); it != m_Threads.end(); ++it)
	{
		m_WorkerGroup.remove_thread(*it);
		delete *it;
	}

	m_Threads.clear();
}

void CPatchService::InterruptService()
{
	m_Service.stop();
}

void CPatchService::JoinAll()
{
	boost::mutex::scoped_lock lock(m_WorkMutex);

	m_WorkCondition.wait(lock, [this]
	{
		return m_WorkCount < 1 || m_Service.stopped();
	});
}

void CPatchService::InterruptAll()
{
	m_WorkerGroup.interrupt_all();
}

size_t CPatchService::ThreadCount()
{
	return m_Threads.size();
}

bool CPatchService::SetupPatchThreads(size_t iCount)
{
	if (!m_Threads.empty())
	{
		ShutdownAll();
	}

	m_Work.reset(new boost::asio::io_service::work(m_Service));

	for (size_t i = 0; i < iCount; ++i)
	{
		try
		{
			m_Threads.push_back(new boost::thread(boost::bind(&CPatchService::RunSingle, this)));
		}
		catch (...)
		{
			return false;
		}
	}

	return true;
}

#endif