#pragma once

#include <deque>
#include "Thread.h"
#include "Synchronized.h"
#include "Command.h"
#include "DownloadEvfCommand.h"
#include "StartEvfCommand.h"
#include "EndEvfCommand.h"


class Processor: public Thread {

protected:
	// whether it is executing it or not?
	bool _running;
	
	// command queue
	std::deque<Command*>  _cmd_queue;

	// frame queue
	std::deque<Command*>  _frame_queue;

	// command when ending
	Command*	_closeCommand;

	// synchronized object
	Synchronized _syncFrameObject;

	// synchronized object
	Synchronized _syncCmdObject;


public:
	// Constructor  
	Processor() : _running(false), _closeCommand(0) {}

	// Destoracta
	virtual ~Processor() { 
		clear_cmd(); 
		clear_frame();
	}

	// Set command when ending
	void setCloseCommand(Command* closeCommand) { _closeCommand = closeCommand; }


	/*
	void enqueue(Command* command)
	{
		_syncObject.lock();
		_queue.push_back(command);
		_syncObject.unlock();
		resume();
	}*/


	void enqueue(Command* command) {
		StartEvfCommand* start_p = dynamic_cast<StartEvfCommand*>(command);
		DownloadEvfCommand* download_p = dynamic_cast<DownloadEvfCommand*>(command);
		EndEvfCommand* end_p = dynamic_cast<EndEvfCommand*>(command);
		if (download_p != nullptr || end_p != nullptr) {
			_syncFrameObject.lock();
			_frame_queue.push_back(command);
			_syncFrameObject.notify();
			_syncFrameObject.unlock();
		}
		else {
			_syncCmdObject.lock();
			_cmd_queue.push_back(command);
			_syncCmdObject.notify();
			_syncCmdObject.unlock();
		}
		
	}



	void stop() {
		_running = false;
		//resume();
	}


	void clear_cmd() {
		_syncCmdObject.lock();

		std::deque<Command*>::iterator it = _cmd_queue.begin();
		while (it != _cmd_queue.end()) {
			delete (*it);
			++it;
		}
		_cmd_queue.clear();

		_syncCmdObject.unlock();
	}

	void clear_frame() {
		_syncFrameObject.lock();

		std::deque<Command*>::iterator it = _frame_queue.begin();
		while (it != _frame_queue.end()) {
			delete (*it);
			++it;
		}
		_frame_queue.clear();

		_syncFrameObject.unlock();
	}


public:
	virtual void run() {
		//When using the SDK from another thread in Windows, 
		// you must initialize the COM library by calling CoInitialize 
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		_running = true;
		while (_running) {
			Sleep(1);

			Command* command = take_cmd();
			Command* frame = take_frame();
			if (frame != nullptr && frame->getCameraModel() != nullptr) {
				bool complete = frame->execute();

				if (complete == false) {
					//If commands that were issued fail ( because of DeviceBusy or other reasons )
					// and retry is required , note that some cameras may become unstable if multiple 
					// commands are issued in succession without an intervening interval.
					//Thus, leave an interval of about 20 ms before commands are reissued.
					Sleep(20);
					enqueue(frame);
				}
				else {
					delete frame;
				}
			}

			if (command != nullptr && command->getCameraModel() != nullptr) {
				bool complete = command->execute();
				if (complete == false && dynamic_cast<StartEvfCommand*>(command)) {
					//If commands that were issued fail ( because of DeviceBusy or other reasons )
					// and retry is required , note that some cameras may become unstable if multiple 
					// commands are issued in succession without an intervening interval.
					//Thus, leave an interval of about 20 ms before commands are reissued.
					Sleep(20);
					enqueue(command);
				}
				else {
					delete command;
				}
			}
		}

		// Clear que
		clear_cmd();

		// Clear que
		clear_frame();

		// Command of end
		if (_closeCommand != NULL) {
			_closeCommand->execute();
			delete _closeCommand;
			_closeCommand = NULL;
		}

		CoUninitialize();

	}

protected:

	//The command is taken out of the que

	/*
	Command* take()
	{

		Command* command = NULL;

		// Que stands by between emptiness.
		while (true)
		{
			_syncObject.lock();
			bool empty = _queue.empty();
			_syncObject.unlock();

			if(empty == false)break;

			suspend();

			if(_running == false)
			{
				return NULL;
			}
		}

		_syncObject.lock();

		command = _queue.front();
		_queue.pop_front();

		_syncObject.unlock();

		return command;
	}*/


	Command* take_cmd() {

		Command* command = NULL;

		_syncCmdObject.lock();

		// Que stands by between emptiness.
		while (_cmd_queue.empty() && _running) {
			_syncCmdObject.wait(10);
		}

		if (_running) {
			command = _cmd_queue.front();
			_cmd_queue.pop_front();
		}

		_syncCmdObject.unlock();

		return command;
	}

	Command* take_frame() {

		Command* command = NULL;

		_syncFrameObject.lock();

		// Que stands by between emptiness.
		if (_frame_queue.empty()) {
			return command;
		}

		if (_running) {
			command = _frame_queue.front();
			_frame_queue.pop_front();
		}

		_syncFrameObject.unlock();

		return command;
	}


	bool cmdIsEmpty() {
		_syncCmdObject.lock();
		bool ret = _cmd_queue.empty();
		_syncCmdObject.unlock();

		return ret;
	}

};






