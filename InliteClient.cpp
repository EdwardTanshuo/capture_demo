#include "InliteClient.h"

#include <iostream>
#include <utility>
#include <vector>

//#include "base64.h"
#include "BarcodeSorter.h"
#include "MultipartParser.h"

#define AUTH_CODE L"AOah5cKhAxgrd2YrCIYYVqDzFgz539Zn"

/*pplx::task<std::vector<Barcode>> InliteClient::post_image(const unsigned char* data, int in_len) {
    uri host(U(INLITE_HOST));
    std::string base64_str = base64_encode(data, in_len);

    return this->post_image_base64(host, base64_str);
}*/

pplx::task<std::vector<Barcode>> InliteClient::post_image(const std::string& base64_image) {
    uri host(U(INLITE_HOST));

    return this->post_image_base64(host, base64_image);
}

pplx::task<std::vector<Barcode>> InliteClient::post_image_base64(const uri& host, const std::string& base64_image) {
    // set host
    init_client(host);

    // contruct base64 image body 
    auto base64_image_value = "data:image/jpegdata:image/jpeg;base64," + base64_image + ":::" + "image.jpeg";

    // setup request
    http_request request(methods::POST);
    MultipartParser parser;
    request.headers().add(L"Authorization", AUTH_CODE);
    parser.AddParameter("image", base64_image_value);
    parser.AddParameter("options", "info");
    parser.AddParameter("types", "1d,2d");
    parser.AddParameter("tbr", "101");
    auto body = std::move(parser.GenBodyContent());

    request.set_body(body, "multipart/form-data; boundary=" + parser.boundary());
    request.set_request_uri(U(BASE64_ENDPOINT));

    // start request
    return _client->request(request).then([](http_response response) {
        auto bodyStream = response.body();
        // TODO: Barcode Algotithm
        if (response.status_code() == status_codes::OK) {
            return response.extract_json();
        }

        // Handle error cases, for now return empty json value... 
        return pplx::task_from_result(web::json::value());
    }).then([](web::json::value json) {
        auto barcodes_json = json[L"Barcodes"].as_array();
        std::vector<Barcode> barcodes;
        for (auto json : barcodes_json) {
            try {
                barcodes.push_back(std::move(Barcode(json)));
            }
            catch (std::exception e) {
                continue;
            }
        }
        if (barcodes.size() == 0) {
            return std::vector<Barcode>();
        }

        return barcodes;
    });
}