/*#include <iostream>
#include <mutex>
#include <thread>
#include <list>
#include <condition_variable>
const int MaxTaskCount = 200;//任务队列上限
template<class T>
class SyncQueue {
private:
	std::list<T>m_queue;//队列
	std::mutex m_mutex;//互斥访问队列
	std::condition_variable m_notEmpty;//条件变量，消费者
	std::condition_variable m_notFull;//生产者
	int m_maxSize;
	bool m_needStop;//false running//true stop
	bool IsFull()const {
		bool full = m_queue.size() >= m_maxSize;
		if (full) {
			cout << "m_queue 满，添加任务需要等待。" << endl;
		}
		return full;
	}
	bool IsEmpty()const {
		bool empty = m_queue.empty();
		if (empty) {
			cout << "m_queue 空，获取任务需要等待。" << endl;
		}
		return empty;
	}
	template<class F>
	void Add(F&& task) {
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_needStop&&IsFull()) {
			m_notFull.wait(locker);//1.弃锁，2.增加到条件变量的等待队列，3.唤醒，4.获锁
		}
		if (m_needStop) {
			return;
		}
		m_queue.push_back(std::forward<F>(task));
		m_notEmpty.notify_one();
	}
public:
	SynQueue(int maxSize = MaxTaskCount) :m_MaxSize(maxSize), m_needStop(false) {
	
	}
	~SyncQueue() {

	}
	SyncQueue(const SyncQueue&) = delete;//禁止拷贝构造
	SyncQueue& operator=(const SyncQueue&) = delete;
	void Put(const T& tast) {
		Add(task);
	}
	void Put(T&& task) {
		Add(std::forward<T>(task));
	}
	void Take(std::list<T>& tlist);
	void Take(T& task);
	void Stop();
	bool Empty()const;
	bool Full()const;
	size_t Size()const;
	size_t Count()const;
};*/