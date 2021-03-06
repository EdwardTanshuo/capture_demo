#pragma once

#include <windows.h>
#include <process.h>
#include "Synchronized.h"

class Thread {
private:
	HANDLE		_hThread;
	bool		_active;

public:
	Thread() : _hThread(), _active(false) {}

	virtual ~Thread() {}


	bool start() {
		_hThread = (HANDLE)_beginthread(threadProc, 0, this);
		return (_hThread != NULL);
	}

	void join() {
		if (_hThread) {
			::WaitForSingleObject(_hThread, INFINITE);
			_hThread = NULL;
		}
	}

	void sleep(int millisec) const {
		if (_hThread) {
			Sleep(millisec);
		}
	}

	HANDLE handle() const {
		return this->_hThread;
	}

	/*
	void resume() const
	{
		if(_hThread )
		{
			ResumeThread(_hThread);
		}
	}

	void suspend() const
	{
		if(_hThread)
		{
			SuspendThread(_hThread);
		}
	}
	*/

public:
	virtual void run() = 0;

protected:

	static void threadProc(void* lParam) {
		Thread* thread = (Thread *)lParam;
		if (thread != NULL) {
			thread->_active = true;
			thread->run();
			thread->_active = false;
		}
		_endthread();
	}

};
