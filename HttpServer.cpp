#include "HttpServer.h"
#include "HttpException.h"

#include <iostream>

#include "EDSDK.h"
#include "EDSDKErrors.h"

#include "CameraEvent.h"
#include "InliteClient.h"

#include "Config.h"

#define DEFAULT_REQUEST_BUFFER_LEN 2048


HttpServer::HttpServer() {
	int ret = HttpInitialize(
		HTTPAPI_VERSION_1,
		HTTP_INITIALIZE_SERVER,
		nullptr
	);

	if (ret != NO_ERROR) {
		throw HttpException(ret, "can not init a http server");
	}

	ret = HttpCreateHttpHandle(&_req_queue, 0);

	if (ret != NO_ERROR) {
		throw HttpException(ret, "can not create a request queue handle");
	}

	// alloc buffer for request 
	_request_buffer_len = sizeof(HTTP_REQUEST) + DEFAULT_REQUEST_BUFFER_LEN;
	_request = (PHTTP_REQUEST)(HeapAlloc(GetProcessHeap(), 0, _request_buffer_len));

	if (_request == nullptr) {
		throw HttpException(ERROR_NOT_ENOUGH_MEMORY, "can not alloc a request buffer");
	}

	HTTP_SET_NULL_ID(&_request_id);
}

HttpServer::~HttpServer() {
	if (_request != nullptr) {
		HeapFree(GetProcessHeap(), 0, (PCHAR)_request);
	}
}

ULONG HttpServer::add_url(PCWSTR url) {
	int ret = HttpAddUrl(
		_req_queue,
		url,
		nullptr
	);

	if (ret != NO_ERROR) {
		throw HttpException(ret, "can not add a url");
	}
	else {
		_url_added++;
	}

	return _url_added;
}

DWORD HttpServer::poll() {
	ULONG				result;
	DWORD				bytes_read;
	std::wstring		query;
	int					dev_ret;
	web::json::value	list;

	RtlZeroMemory(_request, _request_buffer_len);
	result = HttpReceiveHttpRequest(
		_req_queue,
		_request_id,
		0,
		_request,
		_request_buffer_len,
		&bytes_read,
		nullptr
	);

	if (result == NO_ERROR) {
		switch (_request->Verb) {
		case HttpVerbGET:
			query = std::wstring(_request->CookedUrl.pQueryString);
			
			// take picture
			dev_ret = takePicture();

			// process image
			//list = _client.post_image_base64(U(INLITE_URL), L"").wait();

			char timestamp[200];
			char json[1024];
			gen_timestamp(timestamp);
			gen_json(json, timestamp, dev_ret);
			result = SendHttpResponse(_request->RequestId, 200, "OK", json);
			break;

		case HttpVerbPOST:
			result = SendHttpResponse(_request->RequestId, 500, "bad request", nullptr);
			break;

		default:
			result = SendHttpResponse(_request->RequestId, 500, "bad request", nullptr);
		}

		if (result != NO_ERROR) {
			throw HttpException(result, "can not handle the request");
		}

		HTTP_SET_NULL_ID(&_request_id);
	}
	else if (result == ERROR_MORE_DATA) {
		// request buffer is not enough, reallocate the buffer
		_request_buffer_len = bytes_read;
		HeapFree(GetProcessHeap(), 0, (PCHAR)_request);
		_request = (PHTTP_REQUEST)(HeapAlloc(GetProcessHeap(), 0, _request_buffer_len));
		if (_request == nullptr) {
			throw HttpException(ERROR_NOT_ENOUGH_MEMORY, "can not alloc a request buffer");
		}
	}
	else if (ERROR_CONNECTION_INVALID == result && !HTTP_IS_NULL_ID(&_request_id)) {
		// connection error
		HTTP_SET_NULL_ID(&_request_id);
	}
	else {
		throw HttpException(result, "unknown error");
	}

	return 0;
}

int HttpServer::takePicture() {
	EdsError err = -1;
	if (_model) {
		_model->retain();
		//Taking a picture
		err = EdsSendCommand(_model->getCameraObject(), kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Completely);
		err = EdsSendCommand(_model->getCameraObject(), kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_OFF);
		_model->release();
	}

	return err;
}

DWORD HttpServer::SendHttpResponse(IN HTTP_REQUEST_ID request_id, IN USHORT code, IN PSTR reason, IN PSTR entity) {
	HTTP_RESPONSE   response;
	HTTP_DATA_CHUNK data_chunk;
	DWORD           result;
	DWORD           bytes_sent;

	// init response
	init_http_reponse(&response, code, reason);

	// add a known header.
	add_known_header(&response, HttpHeaderContentType, "application/json");

	// add an entity chunk.
	if (entity) {
		data_chunk.DataChunkType = HttpDataChunkFromMemory;
		data_chunk.FromMemory.pBuffer = entity;
		data_chunk.FromMemory.BufferLength = (ULONG)strlen(entity);

		response.EntityChunkCount = 1;
		response.pEntityChunks = &data_chunk;
	}

	result = HttpSendHttpResponse(
		_req_queue,
		request_id,
		0,
		&response,
		nullptr,
		&bytes_sent,
		nullptr,
		0,
		nullptr,
		nullptr
	);

	return result;
}

DWORD HttpServer::SendHttpPostResponse() {
	return 0;
}

// start the run proc
void HttpServer::run() {
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	while (true) {
		try {
			poll();
		}
		catch (HttpException e) {
			std::cerr << e.getMessage();
		}
		Sleep(500);
	}
	CoUninitialize();
}

// observer
void HttpServer::update(Observable* from, CameraEvent* e) {
	std::string event = e->getEvent();

	//End of download of image
	if (event == "DownloadComplete") {
		// TODO: image processing here
	}
}
