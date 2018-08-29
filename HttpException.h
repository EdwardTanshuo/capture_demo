#pragma once

#include <string>
#include <iostream>


class HttpException : public std::exception {

    std::string message;
    int code;
public:
    HttpException(int err, std::string msg) : code(err), message(std::string(msg)) {};
    ~HttpException() throw() { std::cerr << "Exiting Exception" << std::endl; };
    std::string getMessage(void) { return message; };
    int getCode(void) { return code; };
};