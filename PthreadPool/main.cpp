/*#include <iostream>
#include <mutex>
#include <thread>
#include <list>
#include <condition_variable>
const int MaxTaskCount = 200;//�����������
template<class T>
class SyncQueue {
private:
	std::list<T>m_queue;//����
	std::mutex m_mutex;//������ʶ���
	std::condition_variable m_notEmpty;//����������������
	std::condition_variable m_notFull;//������
	int m_maxSize;
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
	template<class F>
	void Add(F&& task) {
		std::unique_lock<std::mutex>locker(m_mutex);
		while (!m_needStop&&IsFull()) {
			m_notFull.wait(locker);//1.������2.���ӵ����������ĵȴ����У�3.���ѣ�4.����
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
	SyncQueue(const SyncQueue&) = delete;//��ֹ��������
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