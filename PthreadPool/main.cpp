#include <iostream>
#include <mutex>//锁
#include <thread>//线程库
#include <list>//链表
#include <functional>//
#include <condition_variable>//条件变量
#include <atomic>//原子操作
const int MaxTaskCount = 200;//任务队列上限
using namespace std;
template<typename T>
class SyncQueue {
private:
	std::list<T>m_queue;//任务队列
	mutable std::mutex m_mutex;//互斥访问队列
	std::condition_variable m_notEmpty;//条件变量，消费者
	std::condition_variable m_notFull;//生产者
	int m_maxSize;//任务队列上限
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
	template<typename F>
	void Add(F&& task) //生产者
	{//&&引用型别未定义
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_needStop&&IsFull()) {
			m_notFull.wait(locker);//1.弃锁，2.增加到条件变量的等待队列，3.唤醒，4.获锁
		}
		//m_notFull.wait（locker,[this]()->bool{return m_needStop!!ISFull()})
		if (m_needStop) {
			return;//?
		}
		m_queue.push_back(std::forward<F>(task));
		m_notEmpty.notify_one();
	}
public:
	SyncQueue(int maxSize = MaxTaskCount) :m_maxSize(maxSize), m_needStop(false) {
	
	}
	~SyncQueue() {

	}
	SyncQueue(const SyncQueue&) = delete;//禁止拷贝构造
	SyncQueue& operator=(const SyncQueue&) = delete;//禁止赋值
	void Put(const T& tast) {
		Add(tast);
	}
	void Put(T&& task) {
		Add(std::forward<T>(task));
	}
	void Take(std::list<T>& tlist) {
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_needStop && IsEmpty()) {
			m_notEmpty.wait(locker);
		}
		if (m_needStop) {
			return;
		}
		tlist = std::move(m_queue);
		m_notFull.notify_one();
	}
	void Take(T& task) {
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_needStop && IsEmpty()) {
			m_notEmpty.wait(locker);
		}
		if (m_needStop) {
			return;
		}
		task = m_queue.front();
		m_queue.pop_front();
		m_notFull.notify_one();
	}
	void Stop() {
		{	
			std::lock_guard<std::mutex>locker(m_mutex);
			m_needStop = true; 
		}
		m_notEmpty.notify_all();
		m_notFull.notify_all();
	}
	bool Empty()const {
		std::lock_guard<std::mutex>locker(m_mutex);
		return m_queue.empty();
	}
	bool Full()const {
		std::lock_guard<std::mutex>locker(m_mutex);
		return m_queue.size() >= m_maxSize;
	}
	size_t Size()const {
		std::lock_guard<std::mutex>locker(m_mutex);
		return m_queue.size();
	}
	size_t Count()const {
		return m_queue.size();
	}
};
class FixedThreadPool {
public:
	using Task = std::function<void(void)>;//任务类型为void(void)类型的函数
private:
	SyncQueue<Task>m_queue;//任务队列
public:
	void AddTask(Task&& task) {

	}
};
int main() {

}