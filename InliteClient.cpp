#include "InliteClient.h"

#include <iostream>
#include "base64.h"


pplx::task<web::json::value> InliteClient::post_image(const uri &host, const unsigned char* data, int in_len) {
	std::string base64_str = base64_encode(data, in_len);
	std::wstring base64_wstr(base64_str.begin(), base64_str.end());

	return this->post_image_base64(host, base64_wstr);
}

pplx::task<web::json::value> InliteClient::post_image_base64(const uri &host, std::wstring base64_image) {
	// set host
	init_client(host);

	// contruct base64 image body 
	auto base64_image_body = L"data:application/jpg;base64," + base64_image + L":::" + L"image.jpg";
	utility::string_t form = L"image=" + base64_image_body;
	
	// setup request
	http_request request(methods::POST);
	request.set_body(form);
	request.set_request_uri(U(BASE64_ENDPOINT));

	// start request
	return _client->request(request).then([](http_response response) {
		auto bodyStream = response.body();
		// TODO: Barcode Algotithm
		
		return pplx::task_from_result(web::json::value());
	});
}