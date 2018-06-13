#include "AVException.h"

AVException::AVException(int err, std::string msg) {
	code = err;
	message = std::string(msg);
}

AVException::~AVException() throw () {
	std::cerr << "Exiting Exception\n";
}

std::string AVException::getMessage(void) { return message; }
int AVException::getCode(void) { return code; }