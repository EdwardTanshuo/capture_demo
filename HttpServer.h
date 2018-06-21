#pragma once

#include <http.h>
#include "ActionSource.h"
#include "Thread.h"

class HttpServer: public ActionSource, public Thread {
protected:
	// request queue
	HANDLE			_req_queue = nullptr;
	PHTTP_REQUEST	_request = nullptr;
	HTTP_REQUEST_ID	_request_id;
	ULONG			_url_added = 0;
	ULONG			_request_buffer_len;

	bool			_running = false;

protected:
	// poll the request queue
	DWORD poll() throw();

public:
	HttpServer() throw();
	virtual ~HttpServer();

	// add url after initialized
	ULONG add_url(PCWSTR url) throw();

	// start the run proc
	virtual void run();

	DWORD SendHttpResponse(IN USHORT code, IN PSTR reason, IN PSTR entity);

	DWORD SendHttpPostResponse();

protected:
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