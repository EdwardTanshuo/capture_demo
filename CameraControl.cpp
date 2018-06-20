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
	_controller = nullptr;
}


// The one and only CCameraControlApp object

CCameraControlApp theApp;

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
	
	//err = init_camera();

	//Create View Dialog
	CCameraControlDlg			view;

	//_controller->setCameraModel(_model);
	//_model->addObserver(&view);
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
