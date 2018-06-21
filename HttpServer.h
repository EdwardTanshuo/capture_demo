#pragma once

#include <http.h>

class HttpServer {
protected:
	// request queue
	HANDLE req_queue = nullptr;
	int url_added = 0;

protected:
	// poll the request queue
	DWORD poll();

public:
	HttpServer();
	virtual ~HttpServer();

	// add url after initialized
	ULONG add_url(const char* url);

	// start the run proc
	void run();

	// utils
	void init_http_reponse(HTTP_RESPONSE* resp, USHORT status, PSTR reason) {
		RtlZeroMemory(resp, sizeof(*resp));
		resp->StatusCode = status;
		resp->pReason = (reason);
		resp->ReasonLength = (USHORT)strlen(reason);
	}

	void add_known_header(HTTP_RESPONSE* resp, ULONG type, PSTR content) {
		resp->Headers.KnownHeaders[type].pRawValue = (content);
		resp->Headers.KnownHeaders[type].RawValueLength = (USHORT)strlen(content);
	}

};