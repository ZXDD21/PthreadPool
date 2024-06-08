#ifndef MY_UNIQUE_PTR
#define MY_UNIQUE_PTR
#include <iostream>
using namespace std;
template<class T,class...Args>
std::unique_ptr<T>my_make_unique(Args&&...args) {
	return std::unique_ptr<T>(new T(std::forward(Args)...));
}
template<class T>
struct my_default_deleter {//ɾ����
	void operator() (T* ptr)const {
		delete ptr;
	}
};
template<class T>
struct my_default_deleter<T[]> {//ɾ����
	void operator() (T* ptr)const {
		delete[]ptr;
	}
};
template<class T, class _Dx = my_default_deleter<T>>
class my_unique_ptr {
public:
	using pointer = T*;
	using elemtype = T;
	using deleter_type = _Dx;
private:
	pointer mptr;//ָ�룬ָ����Դ
	deleter_type mDeleter;//ָ�룬ָ��ɾ����
public:
	pointer release() {//���ؾ���Դ���ÿ�
		pointer old = mptr;
		mptr = nullptr;
		return old;
	}
	void reset(pointer p = nullptr) {//��������Դ
		if (mptr != nullptr) {
			mDeleter(mptr);
		}
		mptr = p;
	}
	void swap(my_unique_ptr& other) {
		std::swap(this->mptr, other.mptr);
	}
	operator bool()const { return mptr != nullptr; }
	my_unique_ptr(pointer p = nullptr) :mptr(p) {}
	~my_unique_ptr() {
		reset();
	}
	my_unique_ptr(const my_unique_ptr&) = delete;
	my_unique_ptr& operator=(const my_unique_ptr&) = delete;
	my_unique_ptr(my_unique_ptr&& other) :mptr(nullptr) {
		reset(other.release());
	}
	my_unique_ptr& operator=(my_unique_ptr&& other) {
		if (this != &other) {
			reset(other.replase());
		}
		return *this;
	}
	pointer get()const { return mptr; }
	T& operator*()const { return *get(); }
	pointer operator->()const { return get(); }
};
template<class T, class _Dx>
class my_unique_ptr <T[], _Dx> {
public:
	using pointer = T*;
	using elemtype = T;
	using deleter_type = _Dx;
private:
	pointer mptr;//ָ�룬ָ����Դ
	deleter_type mDeleter;//ָ�룬ָ��ɾ����
public:
	pointer release() {//���ؾ���Դ���ÿ�
		pointer old = mptr;
		mptr = nullptr;
		return old;
	}
	void reset(pointer p = nullptr) {//��������Դ
		if (mptr != nullptr) {
			mDeleter(mptr);
		}
		mptr = p;
	}
	void swap(my_unique_ptr& other) {
		std::swap(this->mptr, other.mptr);
	}
	operator bool()const { return mptr != nullptr; }
	my_unique_ptr(pointer p = nullptr) :mptr(p) {}
	~my_unique_ptr() {
		reset();
	}
	my_unique_ptr(const my_unique_ptr&) = delete;
	my_unique_ptr& operator=(const my_unique_ptr&) = delete;
	my_unique_ptr(my_unique_ptr&& other) :mptr(nullptr) {
		reset(other.release());
	}
	my_unique_ptr& operator=(my_unique_ptr&& other) {
		if (this != &other) {
			reset(other.replase());
		}
		return *this;
	}
	pointer get()const { return mptr; }
	T& operator*()const { return *get(); }
	T& operator[](std::size_t index)const {
		return get()[index];
	}
	pointer operator->()const { return get(); }
};
#endif