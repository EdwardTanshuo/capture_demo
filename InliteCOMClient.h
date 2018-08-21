#pragma once
#include "stdafx.h"
#include "BarcodeSorter.h"

class InliteCOMClient {
public: 
	InliteCOMClient() {
		_server.CreateInstance(__uuidof(CiServer));
	}

	~InliteCOMClient() {}

	std::vector<Barcode> post_image(const unsigned char* image_path);

private:
	ICiServerPtr _server;
};