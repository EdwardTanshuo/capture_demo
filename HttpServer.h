#pragma once

#include <http.h>
#include <time.h>

#include "ActionSource.h"
#include "Observer.h"
#include "Thread.h"

#include "CameraModel.h"
#include "InliteClient.h"

class HttpServer: public ActionSource, public Thread, public Observer {
protected:
	// request queue
	HANDLE			_req_queue = nullptr;
	PHTTP_REQUEST	_request = nullptr;
	HTTP_REQUEST_ID	_request_id;
	ULONG			_url_added = 0;
	ULONG			_request_buffer_len;
	CameraModel*	_model = nullptr;
	bool			_running = false;
	InliteClient	_client;

protected:
	// poll the request queue
	DWORD poll() throw();

	int takePicture();

public:
	HttpServer() throw();
	virtual ~HttpServer();

	// add url after initialized
	ULONG add_url(PCWSTR url) throw();

	// start the run proc
	virtual void run();

	//observer
	virtual void update(Observable* from, CameraEvent *e);

	DWORD SendHttpResponse(IN HTTP_REQUEST_ID request_id, IN USHORT code, IN PSTR reason, IN PSTR entity);

	DWORD SendHttpPostResponse();

	// setup camera model
	void setupModel(CameraModel* model) { _model = model; }

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

	void add_unknown_header(HTTP_RESPONSE* resp, PSTR name, PSTR content) {
		auto unknownHeader = new HTTP_UNKNOWN_HEADER;
		unknownHeader->NameLength = (USHORT)strlen(name);
		unknownHeader->RawValueLength = (USHORT)strlen(content);
		unknownHeader->pName = name;
		unknownHeader->pRawValue = content;
		resp->Headers.pUnknownHeaders = unknownHeader;
		resp->Headers.UnknownHeaderCount = 1;
	}
	
	void gen_timestamp(char* buffer) {
		time_t timer;
		struct tm* tm_info;

		time(&timer);
		tm_info = localtime(&timer);

		strftime(buffer, 26, "%Y-%m-%d-%H-%M-%S", tm_info);
		puts(buffer);
	}

	void gen_json(char* buffer, const char* id, int dev_status) {
		sprintf(buffer, "{ \"id\" : \"%s\", \"result\" : %d }", id, dev_status);
	}

};