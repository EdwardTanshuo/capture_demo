#include "FlimClient.h"
#include <vector>


web::json::value FlimClient::constructCaptureResults(web::json::value barcodes, const std::string& base64_image) {
	// setup json body
	web::json::value elem;
	elem[L"barcodes"] = barcodes;

	std::wstring w_well(base64_image.length(), L' ');
	std::copy(base64_image.begin(), base64_image.end(), w_well.begin());
	elem[L"image"] = web::json::value::string(w_well);
		
	return elem;
}

web::json::value FlimClient::barcodes2json(std::vector<Barcode> barcodes) {
	// setup json body
	std::vector<web::json::value> temp_arr;
	for (auto iter : barcodes) {
		web::json::value elem;
		elem[L"left"] = web::json::value::number(iter.left);
		elem[L"right"] = web::json::value::number(iter.right);
		elem[L"top"] = web::json::value::number(iter.top);
		elem[L"bottom"] = web::json::value::number(iter.bottom);

		std::wstring w_well(iter.well.length(), L' ');
		std::copy(iter.well.begin(), iter.well.end(), w_well.begin());
		elem[L"well"] = web::json::value::string(w_well);

		std::wstring w_text(iter.text.length(), L' ');
		std::copy(iter.text.begin(), iter.text.end(), w_text.begin());
		elem[L"text"] = web::json::value::string(w_text);

		temp_arr.push_back(elem);
	}

	auto body = std::move(web::json::value::array(temp_arr));
	return body;
}

pplx::task<web::json::value> FlimClient::notify_flim(web::json::value body) {//std::vector<Barcode> barcodes) {
	// set host
	uri host(U(FLIM_HOST));
	init_client(host);
	
	// TODO: this is for test
	return _client->request(web::http::methods::POST, U(ENDPOINT), body).then([](http_response response) {
		auto bodyStream = response.body();
		
		if (response.status_code() == status_codes::OK) {
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(web::json::value());
	});
}