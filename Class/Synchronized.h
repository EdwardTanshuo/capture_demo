#pragma once 

#include <windows.h>
#include <process.h>


class Synchronized {
protected:
	HANDLE _hMutex;
	bool _acquired;
public:
	Synchronized() : _hMutex(NULL), _acquired(false) {
		_hMutex = CreateMutex(NULL, false, NULL);
		if (_hMutex == NULL) {
			CloseHandle(_hMutex);
		}
	}

	virtual ~Synchronized() {
		unlock();
		CloseHandle(_hMutex);
	}


	virtual bool lock() {
		if (_hMutex != NULL) {
			DWORD result = WaitForSingleObject(_hMutex, INFINITE);
			if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED) {
				_acquired = true;
			}
		}

		return _acquired;
	}

	virtual void unlock() const {
		if (_hMutex != NULL) {
			ReleaseMutex(_hMutex);
		}
	}

	virtual void wait(int millisec) {
		Sleep(millisec);

		if (isLocked()) {
			unlock();
		}

		lock();
	}

	virtual void notify() {
		if (isLocked()) {
			unlock();
		}
	}

	virtual bool isLocked() const {
		return _acquired;
	}

};


