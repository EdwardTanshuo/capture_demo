#include "HttpServer.h"
#include "HttpException.h"

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
	ULONG              result;
	DWORD              bytes_read;

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
			result = SendHttpResponse(200, "OK", nullptr);
			break;

		case HttpVerbPOST:

			break;

		default:
			result = SendHttpResponse(500, "bad request", nullptr);
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

DWORD HttpServer::SendHttpResponse(IN USHORT code, IN PSTR reason, IN PSTR entity) {
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
		_request->RequestId,
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

