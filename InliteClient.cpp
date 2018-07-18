#include "InliteClient.h"

#include <iostream>
#include <utility>
#include <vector>

#include "MultipartParser.h"
#include "base64.h"

#define AUTH_CODE L"AOah5cKhAxgrd2YrCIYYVqDzFgz539Zn"

pplx::task<web::json::value> InliteClient::post_image(const unsigned char* data, int in_len) {
	uri host(U(INLITE_HOST));
	std::string base64_str = base64_encode(data, in_len);
	
	return this->post_image_base64(host, base64_str);
}

pplx::task<web::json::value> InliteClient::post_image_base64(const uri& host, const std::string& base64_image) {
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
	auto body = parser.GenBodyContent();
	
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
		auto barcodes = json[L"Barcodes"].as_array();
		for (auto iter : barcodes) {
			auto barcode = iter.as_object();
			auto text = barcode[L"Text"].as_string();
			auto s = barcode[L"Data"].as_string();
		}
		return json;
	});
}