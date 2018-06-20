#pragma once

#include "ActionSource.h"
// CActionButton

class CActionButton : public CButton, public ActionSource {

public:
	CActionButton();

public:
	virtual ~CActionButton();

protected:
	void OnClicked();
	DECLARE_MESSAGE_MAP()
};

