#include"../../std_lib_facilities (1).h"
#include<atomic>
#include<mutex>
#include<condition_variable>

template<typename T>
class RingBuffer {
public:
	RingBuffer();//tom constructør
	RingBuffer(T s);
	~RingBuffer();

	void put(T);//insert new data to the buffer
	T get();

	bool isFull();
	bool isEmpty();

	int size();//retunerer antall elementer i Buffer
	int bufferSize();//returns the size of the buffer

	
private:
	T* buffer;
	int buffer_Size;

	//head = in  , ta  il = out
	int in = 0;
	int out = 0;
	int count = 0;
	/*void incReadIndex();
	/*T* mbuffer;//
	*/

	mutex mreadMutex;
	mutex mwriteMutex;

	condition_variable cvIsEmpty;
	condition_variable cvIsFull;
};

template<typename T>
inline RingBuffer<T>::RingBuffer() {}

template<typename T>
inline RingBuffer<T>::RingBuffer(T s) :in(0), buffer_Size(s), buffer(NULL) {
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
//
inline void RingBuffer<T>::put(T value) {
	unique_lock<mutex>lock1(mwriteMutex);

	//wait until the buffer is no longer full
	cvIsFull.wait(lock1, [=] {return isFull() == false; });

	//locks the mread mutex
	lock_guard<mutex>lock2(mreadMutex);

	if (isEmpty()) {
		out = 0;
		in = 0;
		//
		buffer[in] = value;
		count++;
	}
	else {
		out = (out + 1) % buffer_Size;
		buffer[out] = value;
		count++;
	}
	//notify readers that the buffer is no longer empty
	//notify one so that one waiting obj can continue
	cvIsEmpty.notify_one();
}

template<typename T>
inline T RingBuffer<T>::get() {
	//lager en unique lock til mreadmutex
	unique_lock<mutex>lk(mreadMutex);

	/*vent til buffen er ikke tom leenger*/
	cvIsEmpty.wait(lk, [=] {return isEmpty() == false; });

	//lokker writemutex
	//ingen obj får tilgang til put func
	lock_guard<mutex>lock2(mwriteMutex);

	T tmp;
	if (in == out) {
		tmp = buffer[out];
		//buffer[out] = 0;
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
	cvIsFull.notify_one();//the buffer is no longer full to the readers
	return tmp;
}

template<typename T>
inline bool RingBuffer<T>::isEmpty() {
	return (in == -1 && out == -1);
}

template<typename T>
inline bool RingBuffer<T>::isFull() {
	return (out == buffer_Size - 1);
}

template<typename T>
inline int RingBuffer<T>::size() {
	//antal elemt = størrelsen til bufferen
	if (!isFull()) {
		if (in >= out) {
			return in - out;
		}
		else {
			return  buffer_Size - (in + out);
		}
	}
}

template<typename T>
inline int RingBuffer<T>::bufferSize() {
	return buffer_Size;
}

