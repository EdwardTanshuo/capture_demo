/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CameraControlDlg.cpp                                            *
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
#include <map>

#include "CameraControl.h"
#include "CameraControlDlg.h"
#include ".\CameraControldlg.h"

#include "EDSDK.h"
#include "EDSDKTypes.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define WM_USER_DOWNLOAD_COMPLETE		WM_APP+1
#define WM_USER_PROGRESS_REPORT			WM_APP+2


// CCameraControlDlg 

CCameraControlDlg::CCameraControlDlg(CWnd* pParent )
	: CDialog(CCameraControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCameraControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICT, _displayer);
	DDX_Control(pDX, IDC_BUTTON2, _btnStartEVF);
	DDX_Control(pDX, IDC_BUTTON3, _btnEndEVF);
}

BEGIN_MESSAGE_MAP(CCameraControlDlg, CDialog)
	ON_MESSAGE(WM_USER_DOWNLOAD_COMPLETE, OnDownloadComplete)
	ON_MESSAGE(WM_USER_PROGRESS_REPORT, OnProgressReport)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CCameraControlDlg message handlers

BOOL CCameraControlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	setupListener(_controller);
	setupObserver(getCameraModel());

	//Execute controller
	_controller->run();
	_controller->start();
	// A set value of the camera is acquired. 
	// The value to which the camera can be set is acquired. 
	
	return TRUE;   // return TRUE  unless you set the focus to a control
}

void CCameraControlDlg::setupListener(ActionListener* listener)
{
	addActionListener(listener);
	_btnStartEVF.setActionCommand("startEVF");
	_btnStartEVF.addActionListener(listener);
	_btnEndEVF.setActionCommand("endEVF");
	_btnEndEVF.addActionListener(listener);
	_displayer.setActionCommand("downloadEVF");
	_displayer.addActionListener(listener);
	_pusher.setActionCommand("downloadEVF");
	_pusher.addActionListener(listener);
}


void CCameraControlDlg::setupObserver(Observable* ob)
{
	ob->addObserver(static_cast<Observer*>(&_displayer));
	ob->addObserver(static_cast<Observer*>(&_pusher));
}

void CCameraControlDlg::OnClose()
{
	// TODO : The control notification handler code is added here or Predetermined processing is called. 
	fireEvent("closing");
	__super::OnClose();
}


void CCameraControlDlg::update(Observable* from, CameraEvent *e)
{
	std::string event = e->getEvent();

	//End of download of image
	if(event == "DownloadComplete")
	{
		//The update processing can be executed from another thread. 
		::PostMessage(this->m_hWnd, WM_USER_DOWNLOAD_COMPLETE, NULL, NULL);
	}

	//Progress of download of image
	if(event == "ProgressReport")
	{
		EdsInt32 percent = *static_cast<EdsInt32 *>(e->getArg());
		
		//The update processing can be executed from another thread. 
		::PostMessage(this->m_hWnd, WM_USER_PROGRESS_REPORT, percent, NULL);
	}

	//shutdown event
	if(event == "shutDown")
	{
		::PostMessage(this->m_hWnd, WM_CLOSE, 0, NULL);
	}
}


LRESULT CCameraControlDlg::OnDownloadComplete(WPARAM wParam, LPARAM lParam)
{
	//End of download of image	
	return 0;
}

LRESULT CCameraControlDlg::OnProgressReport(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

