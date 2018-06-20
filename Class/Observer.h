#pragma once

#include <vector>
#include <algorithm>
#include <string>


class Observable;
class CameraEvent;

class Observer {
public:
	virtual void update(Observable* from, CameraEvent *e) = 0;
};


class Observable {
private:
	std::vector<Observer*> _observers;

public:
	Observable() {}
	virtual ~Observable() { deleteObservers(); }

	// Addition of Observer
	void addObserver(Observer* ob) {
		if (std::find(_observers.begin(), _observers.end(), ob) == _observers.end()) {
			_observers.push_back(ob);
		}
	}

	// Deletion of Observer
	void deleteObserver(const Observer* ob) {
		std::vector<Observer*>::iterator i = std::find(_observers.begin(), _observers.end(), ob);
		if (i != _observers.end()) {
			_observers.erase(i);
		}
	}

	// It notifies Observer
	void notifyObservers(CameraEvent *e = NULL) {
		std::vector<Observer*>::reverse_iterator i = _observers.rbegin();
		while (i != _observers.rend()) {
			(*i++)->update(this, e);
		}
	}

	void deleteObservers() { _observers.clear(); }
	int countObservers() const { return (int)_observers.size(); }

};