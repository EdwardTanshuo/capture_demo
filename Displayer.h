#pragma once

#include "Observer.h"
#include "ActionSource.h"

// Displayer

class Displayer : public CStatic, public ActionSource, public Observer {
	DECLARE_DYNAMIC(Displayer)

private:
	BOOL active;
	EdsFocusInfo	m_focusInfo;
	EdsBool			m_bDrawZoomFrame;

	void OnDrawImage(CDC *pDC, unsigned char* pbyteImage, int size);
	void OnDrawFocusRect(CDC *pDC, CRect zoomRect, CSize sizeJpegLarge);
public:
	Displayer();
	virtual ~Displayer();

	//observer
	virtual void update(Observable* from, CameraEvent *e);

protected:
	afx_msg LRESULT OnEvfDataChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


