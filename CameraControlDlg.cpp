#include "stdafx.h"
#include <map>
#include <windows.h>
#include <process.h>

#include "CameraControl.h"
#include "CameraControlDlg.h"
#include ".\CameraControldlg.h"
#include "CameraEventListener.h"
#include "CameraModelLegacy.h"

#include "InliteClient.h"
#include "InliteCOMClient.h"
#include "FlimClient.h"

#include "EDSDK.h"
#include "EDSDKTypes.h"

#include "Config.h"

#include "HttpException.h"

#include "base64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef OUTPUT_NAME
#define OUTPUT_NAME "temp.jpg"
#endif

#define WM_USER_DOWNLOAD_COMPLETE		WM_APP+1
#define WM_USER_PROGRESS_REPORT			WM_APP+2

typedef enum _status_type {
    STREAMING_STATUS = 1,
    SERVER_STATUS = 2,
    DEVICE_STATUS = 3
} StatusType;

// CCameraControlDlg 
static CameraModel* cameraModelFactory(EdsCameraRef camera, EdsDeviceInfo deviceInfo) {
    // if Legacy protocol.
    if (deviceInfo.deviceSubType == 0) {
        return new CameraModelLegacy(camera);
    }

    // PTP protocol.
    return new CameraModel(camera);
}

static std::string loadImageAsBase64(const char *filename) {
    // read the downloaded image
    std::ifstream file(OUTPUT_NAME, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    return base64_encode((const unsigned char*)reinterpret_cast<char*>(buffer.data()), size);
}

static std::vector<Barcode> sortBarcodes(std::vector<Barcode> barcodes) {
    BarcodeSorter sorter(0.866, 1400, 1400, 0, 0);
    std::vector<std::pair<tree::Coordinate, tree::Coordinate>> edges;
    auto sorted_arr = sorter.process_barcodes(barcodes, edges);

    std::vector<Barcode> result;
    for (auto iter : sorted_arr) {
        auto barcode = BarcodeSorter::peek_barcode(barcodes, iter.second);
        barcode.well = iter.first;
        result.push_back(barcode);
    }
    return result;
}

// process image
static void process_image(void* lParam) {
    std::string base64_image = loadImageAsBase64(OUTPUT_NAME);

#ifdef WEB_SERVER_API
    InliteClient client;
    auto promise = client.post_image(base64_image);
    try {
        promise.wait();
        auto result = sortBarcodes(promise.get());
    }
    catch (std::exception e) {

    }
#else
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    InliteCOMClient client;
    auto result = sortBarcodes(client.post_image((const char*)OUTPUT_NAME));
    CoUninitialize();
#endif

    FlimClient	flimsClient;
    auto barcodes = flimsClient.barcodes2json(result);
    auto body = flimsClient.constructCaptureResults(barcodes, base64_image);
    auto strBody = body.serialize();
    //::MessageBoxW(NULL, strBody.c_str(), L"barcodes", MB_OK);
    auto flimsRspPromise = flimsClient.notify_flim(body);
    flimsRspPromise.wait();
    auto flimsRsp = flimsRspPromise.get();
    //::MessageBoxW(NULL, flimsRsp.serialize().c_str(), L"flims response" , MB_OK);
    /*char buffer[1024];
    sprintf(buffer, "Device Status: connected: [ %s ]", "Ready");
    updateStatus(DEVICE_STATUS, buffer);*/

    _endthread();
}


int CCameraControlDlg::init_camera() {
    if (_model != nullptr) {
        return EDS_ERR_OK;
    }

    EdsError	 err = EDS_ERR_OK;
    EdsCameraListRef cameraList = nullptr;
    EdsUInt32	 count = 0;

    // Initialization of SDK
    err = EdsInitializeSDK();
    if (err == EDS_ERR_OK) {
        isSDKLoaded = true;
    }
    else {
        goto end;
    }

    // acquisition of camera list
    if (err == EDS_ERR_OK) {
        err = EdsGetCameraList(&cameraList);
    }
    else {
        goto end;
    }

    // acquisition of number of Cameras
    if (err == EDS_ERR_OK) {
        err = EdsGetChildCount(cameraList, &count);
        if (count == 0) {
            err = EDS_ERR_DEVICE_NOT_FOUND;
        }
    }
    else {
        goto end;
    }

    // acquisition of camera at the head of the list
    if (err == EDS_ERR_OK) {
        err = EdsGetChildAtIndex(cameraList, 0, &_camera);
    }
    else {
        goto end;
    }

    // acquisition of camera information
    EdsDeviceInfo deviceInfo;
    if (err == EDS_ERR_OK) {
        err = EdsGetDeviceInfo(_camera, &deviceInfo);
        if (err == EDS_ERR_OK && _camera == nullptr) {
            err = EDS_ERR_DEVICE_NOT_FOUND;
        }
    }
    else {
        goto end;
    }

    // release camera list
    if (cameraList != nullptr) {
        EdsRelease(cameraList);
    }
    else {
        goto end;
    }

    // create Camera model
    if (err == EDS_ERR_OK) {
        _model = cameraModelFactory(_camera, deviceInfo);
        _http->setupModel(_model);
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

    if (isSDKLoaded) {
        EdsTerminateSDK();
        isSDKLoaded = false;
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

    if (isSDKLoaded) {
        EdsTerminateSDK();
        isSDKLoaded = false;
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

    try {
        _http = new HttpServer();

        // add configured url
        char url[] = HTTP_URL;
        wchar_t wurl[200];
        mbstowcs(wurl, url, strlen(url) + 1);
        LPWSTR ptr = wurl;
        _http->add_url(wurl);

        // start server
        _http->start();

        // update status
        char buffer[200];
        sprintf(buffer, "Server Status: listen on [ %s ]", HTTP_URL);
        updateStatus(SERVER_STATUS, buffer);

        this->startStreaming();
    }
    catch (HttpException e) {
        ::MessageBox(NULL, "Cannot setup http server", NULL, MB_OK);
    }


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

    _http->setActionCommand("TakePicture");
    _http->addActionListener(listener);
}


void CCameraControlDlg::setupObserver(Observable* ob) {
    ob->addObserver(static_cast<Observer*>(&_displayer));
    ob->addObserver(static_cast<Observer*>(&_pusher));
    ob->addObserver(this);
}

void CCameraControlDlg::removeObserver(Observable* ob) {
    ob->deleteObservers();
}

void CCameraControlDlg::OnClose() {
    // TODO : The control notification handler code is added here or Predetermined processing is called. 
    delete _http;

    fireEvent("closing");
    __super::OnClose();
}


void CCameraControlDlg::update(Observable* from, CameraEvent *e) {
    std::string event = e->getEvent();

    // end of download of image
    if (event == "DownloadComplete") {
        //The update processing can be executed from another thread. 
        ::PostMessage(this->m_hWnd, WM_USER_DOWNLOAD_COMPLETE, NULL, NULL);
    }

    // progress of download of image
    if (event == "ProgressReport") {
        EdsInt32 percent = *static_cast<EdsInt32 *>(e->getArg());

        //The update processing can be executed from another thread. 
        ::PostMessage(this->m_hWnd, WM_USER_PROGRESS_REPORT, percent, NULL);
    }

    if (event == "stateChange") {
        int status = *(static_cast<int*>(e->getArg()));
        char buffer[200];
        switch (status) {
        case kEdsStateEvent_Shutdown:
            // update status
            sprintf(buffer, "Device Status: [ Disconnected ]");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_JobStatusChanged:
            sprintf(buffer, "kEdsStateEvent_JobStatusChanged:");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_WillSoonShutDown:
            sprintf(buffer, "kEdsStateEvent_WillSoonShutDown");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_ShutDownTimerUpdate:
            sprintf(buffer, " kEdsStateEvent_ShutDownTimerUpdate");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_CaptureError:
            sprintf(buffer, "kEdsStateEvent_CaptureError");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_InternalError:
            sprintf(buffer, "kEdsStateEvent_InternalError");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_AfResult:
            sprintf(buffer, "kEdsStateEvent_AfResult");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        case kEdsStateEvent_BulbExposureTime:
            sprintf(buffer, "kEdsStateEvent_BulbExposureTime");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        default:
            sprintf(buffer, "Device Status: Unknown");
            updateStatus(DEVICE_STATUS, buffer);
            break;
        }
    }
}


LRESULT CCameraControlDlg::OnDownloadComplete(WPARAM wParam, LPARAM lParam) {
    auto _hThread = (HANDLE)_beginthread(process_image, 0, this);
    return 0;
}

LRESULT CCameraControlDlg::OnProgressReport(WPARAM wParam, LPARAM lParam) {
    return 0;
}

// update status
void CCameraControlDlg::updateStatus(int type, const char* status) {
    CWnd * label;
    switch (type) {
    case 1:
        label = GetDlgItem(IDC_TEXT1);
        label->SetWindowText(status);
        break;
    case 2:
        label = GetDlgItem(IDC_TEXT2);
        label->SetWindowText(status);
        break;
    case 3:
        label = GetDlgItem(IDC_TEXT3);
        label->SetWindowText(status);
        break;
    }

}

// Run Control Delegate
bool CCameraControlDlg::connectCamera(void) {
    int ret = init_camera();
    switch (ret) {
    case EDS_ERR_OK:
        char buffer[1024];
        sprintf(buffer, "Device Status: connected: [ %s ]", "Ready");
        updateStatus(DEVICE_STATUS, buffer);
        break;
    case EDS_ERR_DEVICE_NOT_FOUND:
        updateStatus(DEVICE_STATUS, "Device Status: Device not found");
        break;
    case EDS_ERR_DEVICE_BUSY:
        updateStatus(DEVICE_STATUS, "Device Status: Device busy");
        break;
    case EDS_ERR_DEVICE_INVALID:
        updateStatus(DEVICE_STATUS, "Device Status: Device invalid");
        break;
    case EDS_ERR_DEVICE_EMERGENCY:
        updateStatus(DEVICE_STATUS, "Device Status: Device in emergency");
        break;
    case EDS_ERR_DEVICE_MEMORY_FULL:
        updateStatus(DEVICE_STATUS, "Device Status: Memory full");
        break;
    case EDS_ERR_DEVICE_INTERNAL_ERROR:
        updateStatus(DEVICE_STATUS, "Device Status: Internal error");
        break;

    case EDS_ERR_DEVICE_NOT_RELEASED:
        updateStatus(DEVICE_STATUS, "Device Status: Not released");
        break;

    case EDS_ERR_DEVICE_NOT_INSTALLED:
        updateStatus(DEVICE_STATUS, "Device Status: Not installed");
        break;
    }

    // restart session 
    /*_controller->stop();
    Sleep(500);
    if (ret == EDS_ERR_OK)
     _controller->start();*/
    _controller->start();
    return true;
}

bool CCameraControlDlg::disconnectCamera(void) {
    release_camera();
    updateStatus(DEVICE_STATUS, "Device Status: Disconnected");
    return true;
}

bool CCameraControlDlg::startStreaming(void) {
    bool ret = _pusher.startStreaming(RTMP_URL);
    if (ret) {
        char buffer[200];
        sprintf(buffer, "Streaming Status: started [ %s ]", RTMP_URL);
        updateStatus(STREAMING_STATUS, buffer);
    }
    else {
        updateStatus(STREAMING_STATUS, "Streaming Status: failed");
    }

    return ret;
}

bool CCameraControlDlg::stopStreaming(void) {
    _pusher.stopStreaming();
    updateStatus(STREAMING_STATUS, "Streaming Status: end");
    return true;
}

