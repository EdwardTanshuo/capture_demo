/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : CameraControlDlg.h                                              *
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

#pragma once

#include <map>
#include "Observer.h"
#include "CameraController.h"


#include "Displayer.h"
#include "ActionButton.h"


// CCameraControlDlg Dialog
class CCameraControlDlg : public CDialog, public ActionSource, public Observer
{
	
// Construction
private:
	void setupListener(ActionListener* listener);
	void setupObserver(Observable* ob);
	void changeEnableWindow();

public:
	CCameraControlDlg(CWnd* pParent = NULL);	// standard constructor
	void setCameraController(CameraController* controller){_controller = controller;}


public:
	// Observer 
	virtual void update(Observable* from, CameraEvent *e);

	//Dialog data
	enum { IDD = IDD_CAMERACONTROL_DIALOG };
	
	CActionButton	_btnStartEVF;
	CActionButton	_btnEndEVF;
	
	Displayer		_displayer;
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnDownloadComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressReport(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
	CameraController* _controller;

public:
	afx_msg void OnClose();
};
