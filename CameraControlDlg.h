#pragma once

#include <map>
#include "Observer.h"
#include "CameraController.h"
#include "CameraModel.h"

#include "Displayer.h"
#include "RTMPPusher.h"

#include "ActionButton.h"


// CCameraControlDlg Dialog
class CCameraControlDlg : public CDialog, public ActionSource, public Observer, public RunControllDelegate {

	// Construction
private:
	void setupListener(ActionListener* listener);
	void setupObserver(Observable* ob);
	void changeEnableWindow();

public:
	CCameraControlDlg(CWnd* pParent = NULL);	// standard constructor
	void setCameraController(CameraController* controller) { _controller = controller; }


public:
	// Observer 
	virtual void update(Observable* from, CameraEvent *e);

	// Delegate
	virtual bool connectCamera(void);
	virtual bool disconnectCamera(void);
	virtual bool startStreaming(void);
	virtual bool stopStreaming(void);

	//Dialog data
	enum { IDD = IDD_CAMERACONTROL_DIALOG };

	CActionButton	_btnStartEVF;
	CActionButton	_btnEndEVF;

	Displayer		_displayer;
	RTMPPusher      _pusher;

	CameraModel*	_model;
	EdsCameraRef	_camera;

	int		init_camera();
	void	release_camera();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnDownloadComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProgressReport(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

protected:
	CameraController * _controller;

public:
	afx_msg void OnClose();
};
