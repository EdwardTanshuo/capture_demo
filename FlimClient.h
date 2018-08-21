#pragma once

#include <cpprest/http_client.h>
#include <vector>
#include "BarcodeSorter.h"

#define ENDPOINT	"/notify"
#define FLIM_HOST	"http://127.0.0.1:80"

using namespace web::http;
using namespace web::http::client;

class FlimClient {
public:
	FlimClient() {};
	virtual ~FlimClient() {
		if (_client) {
			delete _client;
			_client = nullptr;
		}
	};

	pplx::task<web::json::value> notify_flim(std::vector<Barcode> barcodes);

private:
	http_client * _client = nullptr;

private:
	void init_client(const uri &base_uri) {
		if (_client == nullptr) {
			http_client_config config;
			config.set_timeout(std::chrono::seconds(1000));
			_client = new http_client(base_uri, config);
		}
	};
};


