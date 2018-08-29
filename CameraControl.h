#pragma once

#ifndef __AFXWIN_H__
#error Please include 'Stdafx.h' before including this file in PCH. 
#endif

#include "resource.h"

#include "CameraController.h"
#include "CameraModel.h"

#include "Displayer.h"
#include "RTMPPusher.h"

// CCameraControlApp:
// CCameraControlApp dialog used for App About

class CCameraControlApp : public CWinApp {
public:
    CCameraControlApp();

    Displayer				_displayer;
    RTMPPusher				_pusher;

    // Overrides
public:
    virtual BOOL InitInstance();

    CameraController*		_controller;

    bool	isSDKLoaded = false;
    bool	has_stated = false;

    friend CameraController*	getController();

    // Implementation

    DECLARE_MESSAGE_MAP()
};

inline CameraController* getCameraController() { return static_cast<CCameraControlApp*>(AfxGetApp())->_controller; }

extern CCameraControlApp theApp;
