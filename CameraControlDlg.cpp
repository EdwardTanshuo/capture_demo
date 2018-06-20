#include "stdafx.h"
#include <map>

#include "CameraControl.h"
#include "CameraControlDlg.h"
#include ".\CameraControldlg.h"
#include "CameraEventListener.h"
#include "CameraModelLegacy.h"

#include "EDSDK.h"
#include "EDSDKTypes.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define WM_USER_DOWNLOAD_COMPLETE		WM_APP+1
#define WM_USER_PROGRESS_REPORT			WM_APP+2


// CCameraControlDlg 
static CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo) {
	// if Legacy protocol.
	if (deviceInfo.deviceSubType == 0) {
		return new CameraModelLegacy(camera);
	}

	// PTP protocol.
	return new CameraModel(camera);
}


int CCameraControlDlg::init_camera() {
	if (_model != nullptr) {
		_model->release();
		_model = nullptr;
	}

	EdsError	 err = EDS_ERR_OK;
	EdsCameraListRef cameraList = nullptr;
	EdsUInt32	 count = 0;

	// acquisition of camera list
	if (err == EDS_ERR_OK) {
		err = EdsGetCameraList(&cameraList);
	}

	// acquisition of number of Cameras
	if (err == EDS_ERR_OK) {
		err = EdsGetChildCount(cameraList, &count);
		if (count == 0) {
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}

	// acquisition of camera at the head of the list
	if (err == EDS_ERR_OK) {
		err = EdsGetChildAtIndex(cameraList, 0, &_camera);
	}

	// acquisition of camera information
	EdsDeviceInfo deviceInfo;
	if (err == EDS_ERR_OK) {
		err = EdsGetDeviceInfo(_camera, &deviceInfo);
		if (err == EDS_ERR_OK && _camera == nullptr) {
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}

	// release camera list
	if (cameraList != nullptr) {
		EdsRelease(cameraList);
	}

	// create Camera model
	if (err == EDS_ERR_OK) {
		_model = cameraModelFactory(_camera, deviceInfo);
		if (_model == nullptr) {
			err = EDS_ERR_DEVICE_NOT_FOUND;
		}
		// retatin the smart pointer or it will be deleted soon in the next cycle of runloop.
		_model->retain();
	}
	else {
		goto end;
	}


	if (err == EDS_ERR_OK) {
		_controller->setCameraModel(_model);

		// set property event handler
		if (err == EDS_ERR_OK) {
			err = EdsSetPropertyEventHandler(_camera, kEdsPropertyEvent_All, CameraEventListener::handlePropertyEvent, (EdsVoid *)_controller);
		}
		else {
			goto end;
		}

		// set object event handler
		if (err == EDS_ERR_OK) {
			err = EdsSetObjectEventHandler(_camera, kEdsObjectEvent_All, CameraEventListener::handleObjectEvent, (EdsVoid *)_controller);
		}
		else {
			goto end;
		}

		// set event handler
		if (err == EDS_ERR_OK) {
			err = EdsSetCameraStateEventHandler(_camera, kEdsStateEvent_All, CameraEventListener::handleStateEvent, (EdsVoid*)_controller);
		}
		else {
			goto end;
		}

		if (err == EDS_ERR_OK) {
			_controller->setCameraModel(_model);
			setupObserver(_model);
			return err;
		}
	}

end:
	// release Camera
	if (_camera != nullptr) {
		EdsRelease(_camera);
		_camera = nullptr;
	}

	if (_model != nullptr) {
		_model->release();
		_model = nullptr;
	}

	if (_controller != nullptr) {
		_controller->setCameraModel(nullptr);
	}

	// since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return err;
}

void CCameraControlDlg::release_camera() {
	if (_controller != nullptr) {
		removeObserver(_model);
		_controller->setCameraModel(nullptr);
		_controller->stop();
	}

	// release Camera
	if (_camera != nullptr) {
		EdsRelease(_camera);
		_camera = nullptr;
	}

	if (_model != nullptr) {
		_model->release();
		_model = nullptr;
	}

}


CCameraControlDlg::CCameraControlDlg(CWnd* pParent)
	: CDialog(CCameraControlDlg::IDD, pParent) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCameraControlDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICT, _displayer);
	DDX_Control(pDX, IDC_BUTTON2, _btnConnect);
}

BEGIN_MESSAGE_MAP(CCameraControlDlg, CDialog)
	ON_MESSAGE(WM_USER_DOWNLOAD_COMPLETE, OnDownloadComplete)
	ON_MESSAGE(WM_USER_PROGRESS_REPORT, OnProgressReport)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CCameraControlDlg message handlers

BOOL CCameraControlDlg::OnInitDialog() {
	CDialog::OnInitDialog();
	_camera = nullptr;
	_model = nullptr;

	setupListener(_controller);
	_controller->setDelegate(this);

	//Execute controller
	_controller->run();

	// A set value of the camera is acquired. 
	// The value to which the camera can be set is acquired. 

	return TRUE;   // return TRUE  unless you set the focus to a control
}

void CCameraControlDlg::setupListener(ActionListener* listener) {
	addActionListener(listener);
	_btnConnect.setActionCommand("start");
	_btnConnect.addActionListener(listener);
	_displayer.setActionCommand("downloadEVF");
	_displayer.addActionListener(listener);
	_pusher.setActionCommand("downloadEVF");
	_pusher.addActionListener(listener);
}


void CCameraControlDlg::setupObserver(Observable* ob) {
	ob->addObserver(static_cast<Observer*>(&_displayer));
	ob->addObserver(static_cast<Observer*>(&_pusher));
}

void CCameraControlDlg::removeObserver(Observable* ob) {
	ob->deleteObservers();
}

void CCameraControlDlg::OnClose() {
	// TODO : The control notification handler code is added here or Predetermined processing is called. 
	fireEvent("closing");
	__super::OnClose();
}


void CCameraControlDlg::update(Observable* from, CameraEvent *e) {
	std::string event = e->getEvent();

	//End of download of image
	if (event == "DownloadComplete") {
		//The update processing can be executed from another thread. 
		::PostMessage(this->m_hWnd, WM_USER_DOWNLOAD_COMPLETE, NULL, NULL);
	}

	//Progress of download of image
	if (event == "ProgressReport") {
		EdsInt32 percent = *static_cast<EdsInt32 *>(e->getArg());

		//The update processing can be executed from another thread. 
		::PostMessage(this->m_hWnd, WM_USER_PROGRESS_REPORT, percent, NULL);
	}

	//shutdown event
	if (event == "shutDown") {
		::PostMessage(this->m_hWnd, WM_CLOSE, 0, NULL);
	}
}


LRESULT CCameraControlDlg::OnDownloadComplete(WPARAM wParam, LPARAM lParam) {
	//End of download of image	
	return 0;
}

LRESULT CCameraControlDlg::OnProgressReport(WPARAM wParam, LPARAM lParam) {
	return 0;
}

// update status
void CCameraControlDlg::updateStatus(char* status) {
	CWnd *label = GetDlgItem(IDC_TEXT1);
	label->SetWindowText(status);

}

// Run Control Delegate
bool CCameraControlDlg::connectCamera(void) {
	int ret = init_camera();
	_controller->stop();
	Sleep(500);
	if (ret == EDS_ERR_OK)
		_controller->start();
	return true;
}

bool CCameraControlDlg::disconnectCamera(void) {
	release_camera();

	return true;
}

bool CCameraControlDlg::startStreaming(void) {
	return true;
}

bool CCameraControlDlg::stopStreaming(void) {
	return true;
}

