#pragma once
#include <string>
#include <iostream>


class AVException : public std::exception {

	std::string message;
	int code;
public:
	AVException(int err, std::string msg);
	~AVException() throw();
	std::string getMessage(void);
	int getCode(void);
};