#pragma once

#include "EDSDK.h"


class CameraEvent {
	std::string _event;
	void* _arg;
public:
	const std::string getEvent() const { return _event; }
	void* getArg() const { return _arg; }

	CameraEvent(std::string event, void* arg = 0) :
		_event(event), _arg(arg) {
	}

};