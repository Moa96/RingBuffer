#include<thread>
#include"RingBuffer.h"

//genrer
void writer(RingBuffer<char>& buffer, bool& closeRequested)
{
	string s = "0123456789";
	while (!closeRequested) {
		for (char c : s) {
			/* same as buffer.put(c) (<< has been overloaded)*/
			buffer.put(c);
			//buffer << c;
			this_thread::sleep_for(chrono::milliseconds(800));
		}
	}
}

//input
void keyboardInput(RingBuffer<char>& buffer, bool& closeRequested)
{
	string s;
	do
	{
		cin >> s;
		for (char c : s)
		{
			/* same as buffer.put(c) (<< has been overloaded)*/
			buffer.put(c);
			//buffer << c;
			this_thread::sleep_for(chrono::milliseconds(300));
		}
	} while (s != "EXIT");
	closeRequested = true;
}

//reader
void reader(RingBuffer<char>& buffer, bool& closeRequested)
{
	int numOfElements = 1;
	/* Run as long as there are elements remaining and close has not been requested*/
	while (!(closeRequested && buffer.isEmpty()))
	{
		char tmp;
		/* same as buffer.put(c) (<< has been overloaded)*/
		//uffer.put(tmp);
		tmp = buffer.get();
		//buffer >> tmp;
		this_thread::sleep_for(chrono::milliseconds(300));
		if (tmp == '\n')
		{
			std::cout << "\t" << numOfElements;
			numOfElements++;
		}
		cout << tmp;
	}
}

int main() {
	RingBuffer<char> ringBuffer(10);
	bool closeRequested = false;
	//Create all the threads with references to the buffer and closeRequested
	thread writerThread(&writer, ref(ringBuffer), ref(closeRequested));
	thread keyboardInputThread(&keyboardInput, ref(ringBuffer), ref(closeRequested));
	thread readerThread(&reader, ref(ringBuffer), ref(closeRequested));
	//Wait for threads to finish
	keyboardInputThread.join();
	//Get a value in case the buffer is full
	ringBuffer.get();
	writerThread.join();
	/* Write a value in case the buffer is empty*/
	ringBuffer.put('A');
	readerThread.join();

	return 0;
}