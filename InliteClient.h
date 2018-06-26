#pragma once

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#define BARCODES_ENDPOINT	"/barcodes"
#define URL_ENDPOINT		"/barcodes/url"
#define FILE_ENDPOINT		"/barcodes/file"
#define BASE64_ENDPOINT		"/barcodes/base64"

using namespace web::http;
using namespace web::http::client;

class InliteClient {
public:
	InliteClient() {};
	virtual ~InliteClient() {
		if (_client) {
			delete _client;
			_client = nullptr;
		}
	};

	pplx::task<web::json::value> post_image_base64(const uri &host, std::wstring base64_image);

private:
	http_client* _client = nullptr;

private:
	void init_client(const uri &base_uri) {
		if (_client == nullptr)
			_client = new http_client(base_uri);
	};
};


