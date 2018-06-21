#pragma once

#include <map>
#include "Observer.h"
#include "CameraController.h"
#include "CameraModel.h"

#include "Displayer.h"
#include "RTMPPusher.h"
#include "HttpServer.h"

#include "ActionButton.h"

// CCameraControlDlg Dialog
class CCameraControlDlg : public CDialog, public ActionSource, public Observer, public RunControllDelegate {

	// Construction
private:
	void setupListener(ActionListener* listener);
	void setupObserver(Observable* ob);
	void removeObserver(Observable* ob);
	void changeEnableWindow();

	void updateStatus(char* status);

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

	CActionButton	_btnConnect;

	Displayer		_displayer;
	RTMPPusher      _pusher;

	HttpServer*		_http = nullptr;

	CameraModel*	_model = nullptr;
	EdsCameraRef	_camera;

	int		init_camera();
	void	release_camera();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);


// Implementation
protected:
	HICON m_hIcon;

	bool isSDKLoaded = false;

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
