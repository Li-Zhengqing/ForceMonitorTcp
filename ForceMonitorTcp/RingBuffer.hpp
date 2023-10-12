#pragma once

#include <iostream>
#include <mutex>
#include <shared_mutex>
#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#endif
// #include <QDebug>

enum BufferStatus {
	EMPTY, NORMAL, FULL
};

template<typename T>
class RingBuffer {
private:
	T* data;
	unsigned int size;
	unsigned int entry;
	unsigned int exit;
	BufferStatus status;

	// Mutex Lock
	std::mutex mtx;
public:
	RingBuffer();
	RingBuffer(unsigned int size);
	~RingBuffer();

	unsigned int length();
	long enqueue_memcpy(T* src, size_t size);
	long dequeue_memcpy(T* dst, size_t size);


	// TODO:
	// long enqueue_from_array_force(T* src, size_t size);
};

// #include "RingQueue.h"

template<typename T>
RingBuffer<T>::RingBuffer(unsigned size) {
	this->size = size;
	this->data = new T[size];
	this->entry = 0;
	this->exit = 0;
	this->status = QueueStatus::EMPTY;
}

template<typename T>
RingBuffer<T>::RingBuffer() {
	this->size = 0;
	this->data = NULL;
	this->entry = 0;
	this->exit = 0;
	this->status = QueueStatus::FULL;
}

template<typename T>
RingBuffer<T>::~RingBuffer() {
	delete[] this->data;
	// std::cout << "RingQueue Destructed" << std::endl;
}

template<typename T>
unsigned int RingBuffer<T>::length() {
	// std::cout << this->status << std::endl;
	if (this->status == QueueStatus::EMPTY) {
		return 0;
	}
	else if (this->status == QueueStatus::FULL) {
		return this->size;
	}
	else {
		if (this->exit < this->entry) {
			return this->entry - this->exit;
		}
		else {
			return this->entry + (this->size - this->exit);
		}
	}
}

template<typename T>
long RingBuffer<T>::enqueue_memcpy(T* src, size_t size) {
	
}

template<typename T>
long RingBuffer<T>::dequeue_memcpy(T* src, size_t size) {
	
}
