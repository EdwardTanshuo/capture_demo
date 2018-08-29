#pragma once

#include <cpprest/http_client.h>
#include <vector>
#include "BarcodeSorter.h"

#define ENDPOINT	"/captureResults"
#define FLIM_HOST	"http://localhost:5000/"

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

    web::json::value barcodes2json(std::vector<Barcode> barcodes);

    web::json::value constructCaptureResults(web::json::value barcodes, const std::string& base64_image);

    pplx::task<web::json::value> notify_flim(web::json::value body);

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


