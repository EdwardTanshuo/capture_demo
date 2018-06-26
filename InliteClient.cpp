#include "InliteClient.h"

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
		// TODO: Barcode Algptithm
		
		return pplx::task_from_result(web::json::value());
	});
}