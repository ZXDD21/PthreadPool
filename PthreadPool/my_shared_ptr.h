#pragma once
#include <atomic>
#include <stdio.h>
template<class _Ty>
class my_deleter {
public:
	void operator()(_Ty* ptr)const {
		delete ptr;
	}
};
template<class _Ty>
class my_refcount {
public:
	using element_typr = _Ty;
	using pointer = _Ty*;
private:
	_Ty* _Ptr;
	std::atomic<int>_Uses;//引用计数
	std::atomic<int>_Weaks;//弱引用智能指针
public:
	my_refcount(_Ty* p = nullptr) :_Ptr(p), _Uses(0), _Weaks(0) {
		if (_Ptr != nullptr) {
			_Uses = 1;
			_Weaks = 1;
		}
	}
	~my_refcount() {}
	void IncRef() {++_Uses;}
	void IncWref() {++_Weaks;}
	int Decref() {
		if (--_Uses == 0) {
			Decwref();
		}
		return _Uses;
	}
	int Decwref(){
		return --_Weaks;
	}
	int _use_count()const {
		return _Uses.load();
	}
};
template<class _Ty,class _Dx=my_deleter<_Ty>>
class my_shared_ptr {
public:
	using element_type = _Ty;
	using pointer = _Ty*;
	using mDeleter_type = _Dx;
private:
	pointer mPtr;
	my_refcount<_Ty>* mRep;
	mDeleter_type mDeleter;
public:
	_Ty* get()const {
		return mPtr;
	}
	_Ty& operator*()const {
		return *get();
	}
	_Ty* operator->()const {
		return get();
	}
	operator bool()const {
		return get() != nullptr;
	}
	void reset() {
		if (mRep != nullptr&&mRep->Decref()==0) {
			mDeleter(mPtr);
			delete mRep;
		}
		mPtr = nullptr;
		mRep = nullptr;
	}
	void reset(_Ty* p) {
		reset();
		mPtr = p;
		if (nullptr != mPtr) {
			mRep = new my_refcount<_Ty>(mPtr);
		}
	}
	void swap(my_shared_ptr& other) {
		std::swap(this->mPtr, other.mPtr);
		std::swap(this->mRep, other.mRep);
	}
public:
	my_shared_ptr(_Ty* p = nullptr) :mPtr(p), mRep(nullptr) {
		if (nullptr != mPtr) {
			mRep = new my_refcount<_Ty>(mPtr);
		}
	}
	my_shared_ptr(const my_shared_ptr& other) :mPtr(other.mPtr), mRep(other.mRep) {
		if (nullptr != mRep) {
			mRep->IncRef();
		}
	}
	my_shared_ptr(my_shared_ptr&& other) :mPtr(other.mPtr), mRep(other.mRep) {
		other.mPtr = nullptr;
		other.mRep = nullptr;
	}
	my_shared_ptr& operator = (const my_shared_ptr & other){
		if (this == &other)return*this;
		my_shared_ptr(other).swap(*this);
		return *this;
	}
	my_shared_ptr& operator=(my_shared_ptr&& other) {
		if (this == &other)return*this;
		my_shared_ptr(std::move(other)).swap(*this);
		return *this;
	}
	~my_shared_ptr()
	{
		reset();
	}
};
