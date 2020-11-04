#include"../../std_lib_facilities (1).h"
#include<mutex>
#include<condition_variable>
#include<atomic>

template<typename T>
class RingBuffer {
public:
	RingBuffer();
	RingBuffer(T s);
	~RingBuffer();

	void put(T);//insert new data to the buffer
	T get();//get data from buffer

	bool isFull();
	bool isEmpty();

	int size();//retunere antall elementer i bufferen
	int bufferSize();

private:
	T* buffer;
	int buffer_Size;

	//head = in og tail = out
	int in = 0;
	int out = 0;
	int count = 0;

	mutex mReadMutex;
	mutex mWriteMutex;

	condition_variable cvIsEmpty;
	condition_variable cvIsFull;
};

template<typename T>
inline RingBuffer<T>::RingBuffer() {}

template<typename T>
inline RingBuffer<T>::RingBuffer(T s) : in(0), buffer_Size(s), buffer(NULL) {
	buffer = new T[s];
	buffer_Size = s;
}

template<typename T>
inline RingBuffer<T>::~RingBuffer() {
	if (buffer != nullptr) {
		delete[]buffer;
	}
}

template<typename T>
inline void RingBuffer<T>::put(T value) {
	unique_lock<mutex>lock1(mWriteMutex);

	//wait until the buffer is no longer full
	cvIsFull.wait(lock1, [=] {return isFull() == false; });

	//locks the mreadMUtex
	lock_guard<mutex>lock2(mReadMutex);

	is(isEmpty()) {
		out = 0;
		in = 0;
		buffer[in] = value;
		count++;
	}
	else {
	out = (out + 1) % buffer_Size;
	buffer[out] = value;
	count++;
	}
	//notify that the buffer is no longer empty
	//and one waiting obj can continue
	cvIsEmpty.notify_one();
}

template<typename T>
inline T RingBuffer<T>::get() {
	//en unique lcok til mReadMutex
	unique_lock<mutex>lk(mReadMutex);

	/*vent til bufferen er ikke tom lenger*/
	cvIsEmpty.wait(lk, [=] {return isEmpty() == false; });

	//lokker mWriteMutex
	//ingen obj får tingang til put func
	lock_guard<mutex>lock2(mWriteMutex);

	T tmp;
	if (in == out) {
		tmp = buffer[out];
		in = -1;
		out = -1;
		count--;
	}
	else {
		tmp = buffer[out];
		buffer[out] = 0;
		in = (in + 1) % buffer_Size;
		count--;
	}
	cvIsFull.notify_one();
	return tmp;
}

template<typename T>
inline bool RingBuffer<T>::isFull() {
	return (out == buffer_Size - 1);
}

template<typename T>
inline bool RingBuffer<T>::isEmpty() {
	return (in == -1 && out == -1);
}

template<typename T>
inline int RingBuffer<T>::size() {
	if (!isFull()) {
		if (in >= out) {
			return in - out;
		}
		else {
			return buffer_Size - (in + out);
		}
	}
}

template<typename T>
inline int RingBuffer<T>::bufferSize() {
	return buffer_Size;
}
