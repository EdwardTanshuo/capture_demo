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

	// construct the post form
	std::vector<std::pair<std::wstring, std::wstring>> form;
	
	form.push_back(std::make_pair(L"options", L"info"));
	form.push_back(std::make_pair(L"types", L"1d,2d"));
	form.push_back(std::make_pair(L"tbr", L"119"));
	
	// setup request
	http_request request(methods::POST);
	MultipartParser parser;
	request.headers().add(L"Authorization", AUTH_CODE);
	parser.AddParameter("image", "info");
	parser.AddParameter("options", "info");
	parser.AddParameter("types", "1d,2d");
	parser.AddParameter("tbr", "119");
	auto body = parser.GenBodyContent();
	std::cout << body << std::endl;
	request.set_body(body, "multipart/form-data; boundary=" + parser.boundary());
	request.set_request_uri(U(BASE64_ENDPOINT));
	
	std::stringstream data;

	// start request
	return _client->request(request).then([](http_response response) {
		auto bodyStream = response.body();
		// TODO: Barcode Algotithm
		
		return pplx::task_from_result(web::json::value());
	});
}