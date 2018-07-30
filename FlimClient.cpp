#include "FlimClient.h"

pplx::task<web::json::value> FlimClient::notify_flim(std::vector<Barcode> barcodes) {
	// set host
	uri host(U(FLIM_HOST));
	init_client(host);

	// setup request
	http_request request(methods::POST);
	request.set_request_uri(U(ENDPOINT));

	// start request
	return _client->request(request).then([](http_response response) {
		auto bodyStream = response.body();
		// TODO: Barcode Algotithm
		if (response.status_code() == status_codes::OK) {
			return response.extract_json();
		}

		// Handle error cases, for now return empty json value... 
		return pplx::task_from_result(web::json::value());
	});
}