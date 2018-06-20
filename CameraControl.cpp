/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CameraControl.cpp                                               *
*                                                                             *
*   Description: This is the Sample code to show the usage of EDSDK.          *
*                                                                             *
*                                                                             *
*******************************************************************************
*                                                                             *
*   Written and developed by Camera Design Dept.53                            *
*   Copyright Canon Inc. 2006-2008 All Rights Reserved                        *
*                                                                             *
*******************************************************************************/

#include "stdafx.h"
#include "CameraControl.h"
#include "CameraControlDlg.h"
#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "CameraModel.h"
#include "CameraModelLegacy.h"
#include "CameraController.h"
#include "CameraEventListener.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo);

// CCameraControlApp

BEGIN_MESSAGE_MAP(CCameraControlApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCameraControlApp construction

CCameraControlApp::CCameraControlApp() {
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	_camera		= nullptr;
	_controller = nullptr;
	_model		= nullptr;
}


// The one and only CCameraControlApp object

CCameraControlApp theApp;

int CCameraControlApp::init_camera() {
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
	} else {
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
			err = EdsSetCameraStateEventHandler(_camera, kEdsStateEvent_All, CameraEventListener::handleStateEvent, (EdsVoid *)_controller);
		}
		else {
			goto end;
		}

		if (err == EDS_ERR_OK)
			return err;
	}

end:
	// release Camera
	if (_camera != nullptr) {
		EdsRelease(_camera);
		_camera = nullptr;
	}

	if (_model != nullptr) {
		delete _model;
		_model = nullptr;
	}

	if (_controller != nullptr) {
		_controller->setCameraModel(nullptr);
	}

	// since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return err;
}

void CCameraControlApp::release_camera() {
	if (_controller != nullptr) {
		_controller->setCameraModel(nullptr);
		_controller->stop();
	}

	// release Camera
	if (_camera != nullptr) {
		EdsRelease(_camera);
		_camera = nullptr;
	}

	if (_model != nullptr) {
		delete _model;
		_model = nullptr;
	}

}


// CCameraControlApp initialization
BOOL CCameraControlApp::InitInstance() {
	InitCommonControls();
	CWinApp::InitInstance();

	//Create CameraController
	_controller = new CameraController();

	// Initialization of SDK
	int err = EdsInitializeSDK();
	if (err == EDS_ERR_OK) {
		isSDKLoaded = true;
	}
	else {
		::MessageBox(NULL, "Cannot init sdk", NULL, MB_OK);
	}
	
	err = init_camera();

	//Create View Dialog
	CCameraControlDlg			view;

	_controller->setCameraModel(_model);
	_model->addObserver(&view);
	// Send Model Event to view	
	view.setCameraController(_controller);

	m_pMainWnd = &view;
	INT_PTR nResponse = view.DoModal();
	return true;

	return false;
}


static CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo) {
	// if Legacy protocol.
	if (deviceInfo.deviceSubType == 0) {
		return new CameraModelLegacy(camera);
	}

	// PTP protocol.
	return new CameraModel(camera);
}
