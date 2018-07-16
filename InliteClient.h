#pragma once

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#define BARCODES_ENDPOINT	""
#define URL_ENDPOINT		"/url"
#define FILE_ENDPOINT		"/file"
#define BASE64_ENDPOINT		"/base64"

#define INLITE_HOST			"http://10.100.100.144:80/wabr/barcodes"

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

	pplx::task<web::json::value> post_image(const unsigned char* data, int in_len);

private:
	http_client* _client = nullptr;

	pplx::task<web::json::value> post_image_base64(const uri& host, const std::wstring& base64_image);

private:
	void init_client(const uri &base_uri) {
		if (_client == nullptr)
			_client = new http_client(base_uri);
	};
};


