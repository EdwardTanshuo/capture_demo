/******************************************************************************
*                                                                             *
*   PROJECT : EOS Digital Software Development Kit EDSDK                      *
*      NAME : Command.h	                                                      *
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

#include  "CameraModel.h"

class Command {

protected:
	// Camera Model
	CameraModel* _model;

public:
	Command(CameraModel *model) : _model(model) {
		if (_model != nullptr) {
			_model->retain();
		}
	}

	virtual ~Command() {
		if (_model != nullptr) {
			_model->release();
		}
	}

	CameraModel* getCameraModel(){
		return _model;
	}

	// Execute command	
	virtual bool execute() = 0;
};

