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
	InliteClient();
	virtual ~InliteClient();

	pplx::task<void> post_image_base64(const uri &host, std::wstring base64_image);

private:
	http_client _client;

private:
	void init_client(const uri &base_uri) {
		_client = http_client(base_uri);
	};
};


