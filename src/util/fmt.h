/* coding: utf-8 */
/**
 * nicopp
 *
 * Copyright 2015, PSI
 */
#pragma once

#include <stdio.h>

namespace nicopp {

template <typename T>
T conv(T& t) {
	return T(t);
}

char const* conv(std::string const& t) {
	return t.c_str();
}
char const* conv(std::string & t) {
	return t.c_str();
}

template <typename... Ts>
std::string sprintf(std::string const& fmt,Ts... args) {
	char buff[2048];
	::snprintf(buff, sizeof(buff), fmt.c_str(), conv(args)...);
	return buff;
}

}
