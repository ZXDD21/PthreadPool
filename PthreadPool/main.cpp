#include <iostream>
#include <mutex>//��
#include <thread>//�߳̿�
#include <list>//����
#include <functional>//
#include <condition_variable>//��������
#include <atomic>//ԭ�Ӳ���
const int MaxTaskCount = 200;//�����������
using namespace std;
template<typename T>
class SyncQueue {
private:
	std::list<T>m_queue;//�������
	mutable std::mutex m_mutex;//������ʶ���
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
	void Add(F&& task) //������
	{//&&�����ͱ�δ����
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_needStop&&IsFull()) {
			m_notFull.wait(locker);//1.������2.���ӵ����������ĵȴ����У�3.���ѣ�4.����
		}
		//m_notFull.wait��locker,[this]()->bool{return m_needStop!!ISFull()})
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
	SyncQueue(const SyncQueue&) = delete;//��ֹ��������
	SyncQueue& operator=(const SyncQueue&) = delete;//��ֹ��ֵ
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
	using Task = std::function<void(void)>;//��������Ϊvoid(void)���͵ĺ���
private:
	SyncQueue<Task>m_queue;//�������
public:
	void AddTask(Task&& task) {

	}
};
int main() {

}