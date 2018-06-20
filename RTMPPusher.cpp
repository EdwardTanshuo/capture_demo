#include "stdafx.h"
#include <atlimage.h>

#include "CameraControl.h"
#include "RTMPPusher.h"

#define WM_USER_EVF_DATA_CHANGED		WM_APP+1

IMPLEMENT_DYNAMIC(RTMPPusher, CStatic)
RTMPPusher::RTMPPusher() {
	active = FALSE;
	streamer = RTMPStreamer();
	bool ret = streamer.init_componets();
}

RTMPPusher::~RTMPPusher() {

}


// RTMPPusher messge handler

void RTMPPusher::update(Observable* from, CameraEvent *e) {

	std::string event = e->getEvent();

	if (event == "EvfDataChanged") {
		EVF_DATASET data = *static_cast<EVF_DATASET *>(e->getArg());

		// push buffer
		dataChanged(&data);

		EdsInt32 propertyID = kEdsPropID_FocusInfo;
		fireEvent("get_Property", &propertyID);

		// Download image data.
		fireEvent("downloadEVF");
	}

	if (event == "PropertyChanged") {
		EdsInt32 proeprtyID = *static_cast<EdsInt32 *>(e->getArg());
		if (proeprtyID == kEdsPropID_Evf_OutputDevice) {
			CameraModel* model = (CameraModel *)from;
			EdsUInt32 device = model->getEvfOutputDevice();

			// PC live view has started.
			if (!active && (device & kEdsEvfOutputDevice_PC) != 0) {
				active = TRUE;
				// Start download of image data.
				fireEvent("downloadEVF");
			}

			// PC live view has ended.
			if (active && (device & kEdsEvfOutputDevice_PC) == 0) {
				active = FALSE;
			}
		}
	}
}

LRESULT RTMPPusher::dataChanged(EVF_DATASET* dataset) {
	EdsUInt64 size;

	unsigned char* pbyteImage = NULL;

	// Get image (JPEG) pointer.
	EdsGetPointer(dataset->stream, (EdsVoid**)&pbyteImage);

	if (pbyteImage != NULL) {
		EdsGetLength(dataset->stream, &size);
		// Push stream
		streamer.push_buffer(pbyteImage, size);
	}
	return 0;
}