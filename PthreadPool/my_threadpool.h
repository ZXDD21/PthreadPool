#ifndef MY_THREADPOOL
#define MY_THREADPOOL
#include <iostream>
#include <mutex>//��
#include <thread>//�߳̿�
#include <list>//����
#include <functional>//
#include <condition_variable>//��������
#include <future>
#include <atomic>//ԭ�Ӳ���
#include <chrono>
#include <memory>
const int MaxTaskCount = 20;//�����������
const int InitThreadNum = 4;//��ʼ�̸߳���
using namespace std;


template<class T>
class SyncQueue {
private:
	std::list<T>m_queue;//�������
	//1.Ϊʲô��ʹ��vector��deque��queue��
	//��Ϊ��Ҫ����ɾ���Ͳ��룬list���Ӷȶ���O(1)��������Դ��malloc���������ڴ����ǿЧ�ʣ�Ȼ��deque���м�����޸ĺ�������list�Ծ���O(1)
	mutable std::mutex m_mutex;//������ʶ���
	//2.mutable���ؼ��ֵ����ã�����
	//��ʹ�ǳ�������Ҳ�����޸Ĵ�ֵ
	//��const���⣬constֻ�г�ʼ����ʱ����Ը�ֵ�������κ�����²��ܸı䣬����ȥ��ת��
	std::condition_variable m_notEmpty;//����������������
	std::condition_variable m_notFull;//������
	int m_maxSize;//�����������
	bool m_needStop;//false running//true stop
	bool IsFull()const {
		bool full = m_queue.size() >= m_maxSize;
		if (full) {
			cout << "m_queue �������������Ҫ�ȴ���" << endl;
		}
		return full;
	}
	bool IsEmpty()const {
		bool empty = m_queue.empty();
		if (empty) {
			cout << "m_queue �գ���ȡ������Ҫ�ȴ���" << endl;
		}
		return empty;
	}
	template<typename F>
	int Add(F&& task) //������
	{//&&�����ͱ�δ����
		std::unique_lock<std::mutex>locker(m_mutex);
		/*while (!m_needStop && IsFull()) {//��ֹ��ٻ���
			m_notFull.wait(locker);//1.������2.���ӵ����������ĵȴ����У�3.���ѣ�4.����
		}*/
		if (!m_notFull.wait_for(locker, std::chrono::milliseconds(1), [this]()->bool {return m_needStop || !IsFull(); })) {
			return -1;
		}
		//m_notFull.wait��locker,[this]()->bool{return m_needStop!!ISFull()})//Ϊ�����˳�
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
	SyncQueue(const SyncQueue&) = delete;//��ֹ��������
	SyncQueue& operator=(const SyncQueue&) = delete;//��ֹ��ֵ
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
//�ܾ�����
//1.��ֹ���ԣ��׳��쳣
//2.�������ԣ�ʲô��������ֱ���������ܾ��Ĳ���
//3.�������ϲ��ԣ��൱�ڶ�������һ��Ҫִ�е�����������������ȶ��У������������ȼ���ߵ����񣬽��鲻Ҫһ��ʹ��
//4.�����߲��ԣ���������ʱ����������˵������ߣ��õ�������ִ�д�����

class FixedThreadPool {
public:
	using Task = std::function<void(void)>;//��������Ϊvoid(void)���͵ĺ���
private:
	SyncQueue<Task>m_queue;//�������
	std::list<std::thread>m_threadgroup;//�߳���
	std::atomic_bool m_running;//true running false stop
	std::once_flag m_flag;//���

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
				thread.join();//�޷�join����
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