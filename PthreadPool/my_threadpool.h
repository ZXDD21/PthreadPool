#ifndef MY_THREADPOOL
#define MY_THREADPOOL
#include <iostream>
#include <mutex>//锁
#include <thread>//线程库
#include <list>//链表
#include <functional>//
#include <condition_variable>//条件变量
#include <future>
#include <atomic>//原子操作
#include <chrono>
#include <memory>
const int MaxTaskCount = 20;//任务队列上限
const int InitThreadNum = 4;//初始线程个数
using namespace std;


template<class T>
class SyncQueue {
private:
	std::list<T>m_queue;//任务队列
	//1.为什么不使用vector，deque，queue？
	//因为需要反复删除和插入，list复杂度都是O(1)，性能来源于malloc，可以用内存池增强效率，然而deque在中间进行修改很慢，而list仍旧是O(1)
	mutable std::mutex m_mutex;//互斥访问队列
	//2.mutable异变关键字的作用？？？
	//即使是常方法，也可以修改此值
	//与const互斥，const只有初始化的时候可以赋值，其余任何情况下不能改变，除非去常转换
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
	int Add(F&& task) //生产者
	{//&&引用型别未定义
		std::unique_lock<std::mutex>locker(m_mutex);
		/*while (!m_needStop && IsFull()) {//防止虚假唤醒
			m_notFull.wait(locker);//1.弃锁，2.增加到条件变量的等待队列，3.唤醒，4.获锁
		}*/
		if (!m_notFull.wait_for(locker, std::chrono::milliseconds(1), [this]()->bool {return m_needStop || !IsFull(); })) {
			return -1;
		}
		//m_notFull.wait（locker,[this]()->bool{return m_needStop!!ISFull()})//为真则退出
		if (m_needStop) {
			return -2;//?
		}
		m_queue.push_back(std::forward<F>(task));
		m_notEmpty.notify_one();
		return 0;
	}
public:
	SyncQueue(int maxSize = MaxTaskCount) :m_maxSize(maxSize), m_needStop(false) {

	}
	~SyncQueue() {

	}
	SyncQueue(const SyncQueue&) = delete;//禁止拷贝构造
	SyncQueue& operator=(const SyncQueue&) = delete;//禁止赋值
	int Put(const T& tast) {
		return Add(tast);
	}
	int  Put(T&& task) {
		return Add(std::forward<T>(task));
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
//拒绝策略
//1.中止策略，抛出异常
//2.抛弃策略，什么都不做，直接抛弃被拒绝的策略
//3.抛弃最老策略，相当于队列中下一个要执行的任务。如果队列是优先队列，可能抛弃优先级最高的任务，建议不要一起使用
//4.调用者策略，当队列满时，将任务回退到调用者，让调用者来执行此任务。

class FixedThreadPool {
public:
	using Task = std::function<void(void)>;//任务类型为void(void)类型的函数
private:
	SyncQueue<Task>m_queue;//任务队列
	std::list<std::thread>m_threadgroup;//线程组
	std::atomic_bool m_running;//true running false stop
	std::once_flag m_flag;//标记

	void RunInThread() {
		while (m_running) {
			Task task;
			m_queue.Take(task);
			if (task && m_running) {
				task();
			}
		}
	}
	void Start(int numthreads) {
		m_running = true;
		for (int i = 0; i < numthreads; ++i) {
			m_threadgroup.push_back(std::thread(&FixedThreadPool::RunInThread, this));
		}
	}
	void StopThreadGroup() {
		m_queue.Stop();
		m_running = false;
		for (auto& thread : m_threadgroup) {
			if (thread.joinable()) {
				thread.join();//无法join两次
			}
		}
	}
public:
	FixedThreadPool(int numThread = InitThreadNum) :m_queue(MaxTaskCount), m_running(false) {
		Start(numThread);
	}
	~FixedThreadPool() {
		Stop();
	}
	void Stop() {
		std::call_once(m_flag, [this] {StopThreadGroup(); });
	}
	template<class Func, class ...Arg>
	auto AddTask(Func&& func, Arg&&...args) -> std::future<decltype(func(args...))> {
		using RetType = decltype(func(args...));
		std::packaged_task<RetType()>task(std::bind(std::forward<Func>(func), std::forward<Arg>(args)...));
		std::future<RetType>result = task.get_future();
		task();
		//m_queue.Put([task]() {task(); });
		return result;
	}
	/*
	void AddTask(const Task& task) {
			if (m_queue.Put(task) != 0) {
				std::cerr << "taask queue is full" << std::endl;
				task();
			}
			m_queue.Put(task);
		}
	void AddTask(Task&& task) {
		if (m_queue.Put(task) != 0) {
			std::cerr << "taask queue is full" << std::endl;
			task();
		}
		m_queue.Put(task);
	}*/
	size_t TashSize()const {
		return m_queue.Size();
	}
};
//void funa() {
//	for (int i = 0; i < 10; ++i) {
//		cout << "funa:" << i << endl;
//	}
//}
//void funb() {
//	for (int i = 0; i < 10; ++i) {
//		cout << "funb:" << i+10 << endl;
//	}
//}
//void func() {
//	for (int i = 0; i < 10; ++i) {
//		cout << "func:" << i + 100 << endl;
//	}
//}
//int add(int a, int b) {
//	return a + b;
//}
	//FixedThreadPool mypool;
	//auto a = mypool.AddTask(add, 5, 10);
	//auto b = mypool.AddTask(add, 5.5, 10);
	//auto c = mypool.AddTask(add, 9, 10);
	//cout << a.get() << endl;
	//cout << b.get() << endl;
	//cout << c.get() << endl;
	//std::this_thread::sleep_for(std::chrono::milliseconds(200));
#endif