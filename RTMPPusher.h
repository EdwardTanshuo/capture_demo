#pragma once

#include "Observer.h"
#include "ActionSource.h"
#include "RTMPStreamer.h"

// RTMPPusher

class RTMPPusher : public CStatic, public ActionSource, public Observer {
	DECLARE_DYNAMIC(RTMPPusher)

private:
	BOOL active;
	RTMPStreamer    streamer;

public:
	RTMPPusher();
	virtual ~RTMPPusher();

	//observer
	virtual void update(Observable* from, CameraEvent *e);

protected:
	LRESULT dataChanged(EVF_DATASET* dataset);
	
};


