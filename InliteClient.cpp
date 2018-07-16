#include "InliteClient.h"

#include <iostream>
#include <utility>
#include <vector>

#include "base64.h"

#define AUTH_CODE L"AOah5cKhAxgrd2YrCIYYVqDzFgz539Zn"

static utility::string_t make_form(const std::vector<std::pair<std::wstring, std::wstring>>& form) {
	auto iter = std::vector<std::pair<std::wstring, std::wstring>>::iterator();
	utility::string_t result = L"";
	for (; iter != form.end(); iter ++) {
		if (iter != form.begin()) {
			result += L"&";
		}

		auto key = iter->first;
		auto value = iter->first;

		result = result + key + L"=" + value;
	}
	return result;
}

pplx::task<web::json::value> InliteClient::post_image(const unsigned char* data, int in_len) {
	uri host(U(INLITE_HOST));
	std::string base64_str = base64_encode(data, in_len);
	std::wstring base64_wstr(base64_str.begin(), base64_str.end());

	return this->post_image_base64(host, base64_wstr);
}

pplx::task<web::json::value> InliteClient::post_image_base64(const uri& host, std::wstring base64_image) {
	// set host
	init_client(host);

	// contruct base64 image body 
	auto base64_image_value = L"data:application/jpg;base64," + base64_image + L":::" + L"image.jpg";

	// construct the post form
	std::vector<std::pair<std::wstring, std::wstring>> form;
	form.push_back(std::make_pair(L"image", base64_image_value));
	form.push_back(std::make_pair(L"options", L"info"));
	form.push_back(std::make_pair(L"types", L"1d,2d"));
	form.push_back(std::make_pair(L"tbr", L"119"));
	auto encoded_form = make_form(form);

	// setup request
	http_request request(methods::POST);
	request.headers().add(L"Authorization", AUTH_CODE);
	request.set_body(encoded_form);
	request.set_request_uri(U(BASE64_ENDPOINT));

	// start request
	return _client->request(request).then([](http_response response) {
		auto bodyStream = response.body();
		// TODO: Barcode Algotithm
		
		return pplx::task_from_result(web::json::value());
	});
}