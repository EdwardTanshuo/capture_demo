#pragma once

#include <vector>
#include <algorithm>
#include <string>

#include "Synchronized.h"


class Observable;
class CameraEvent;

class Observer {
public:
	virtual void update(Observable* from, CameraEvent *e) = 0;	
};


class Observable {
private:
	std::vector<Observer*> _observers;

protected:
	Synchronized _syncObject;

public:
	Observable() {}
	virtual ~Observable() { deleteObservers(); }

	// Addition of Observer
	void addObserver(Observer* ob) {
		_syncObject.lock();
		if (std::find(_observers.begin(), _observers.end(), ob) == _observers.end()) {
			_observers.push_back(ob);
		}
		_syncObject.unlock();
		_syncObject.notify();
	}

	// Deletion of Observer
	void deleteObserver(const Observer* ob) {
		_syncObject.lock();
		std::vector<Observer*>::iterator i = std::find(_observers.begin(), _observers.end(), ob);
		if (i != _observers.end()) {
			_observers.erase(i);
		}
		_syncObject.unlock();
		_syncObject.notify();
	}

	// It notifies Observer
	void notifyObservers(CameraEvent *e = NULL) {
		_syncObject.lock();
		for (auto observer: _observers) {
			observer->update(this, e);
		}
		
		_syncObject.unlock();
		_syncObject.notify();
	}

	void deleteObservers() {
		_syncObject.lock();
		_observers.clear();
		_syncObject.unlock();
		_syncObject.notify();
	}
	
	int countObservers() { 
		_syncObject.lock();
		int result =  (int)_observers.size(); 
		_syncObject.unlock();
		_syncObject.notify();
		return result;
	}

};