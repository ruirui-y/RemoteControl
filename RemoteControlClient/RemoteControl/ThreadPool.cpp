#include "ThreadPool.h"
#include <QMutexLocker>
#include <QEventLoop>

ThreadPool::ThreadPool(QObject *parent)
	: QObject(parent)
{}

ThreadPool::~ThreadPool()
{
	Stop();
}

void ThreadPool::Start(size_t threadNum)
{
	if (started_.load(std::memory_order_acquire)) return;

	if(threadNum == 0) threadNum = QThread::idealThreadCount();
	for (size_t i = 0; i < threadNum; ++i)
	{
		QSharedPointer<WorkerThread> thread = QSharedPointer<WorkerThread>(new WorkerThread());

		QEventLoop loop;
		QObject::connect(thread.get(), &WorkerThread::SigReady, &loop, &QEventLoop::quit, Qt::QueuedConnection);

		thread->start();
		thread->setObjectName(QString::number(i));
		loop.exec();																									// 等到 worker run() 里 emit Ready()

		{
			QMutexLocker locker(&mutex_);
			threads_.push_back(thread);																					// 仅在修改容器时加锁
		}
	}
	
	started_.store(true, std::memory_order_release);

	InitNetwork();
}

void ThreadPool::Stop()
{
    QMutexLocker locker(&mutex_);
	if (!started_) return;

	// 清理所有网络服务
	tcp_mgr_.reset();

	// 杀死所有线程
	for (auto& thread : threads_)
	{
		thread->quit();
		thread->wait();
	}
	threads_.clear();
	started_ = false;
}

QSharedPointer<WorkerThread> ThreadPool::GetThread()
{
    QMutexLocker locker(&mutex_);

	// 如果没启动或没有线程，返回一个空的弱指针
	if (!started_ || threads_.empty()) {
		return nullptr;
	}

	QSharedPointer<WorkerThread> thread = threads_[currentThreadIndex_];
	currentThreadIndex_ = (currentThreadIndex_ + 1) % threads_.size();

	return thread;
}

void ThreadPool::InitNetwork()
{
	tcp_mgr_ = CreateQObject<TCPMgr>();
}