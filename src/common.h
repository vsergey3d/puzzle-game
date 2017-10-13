#pragma once
#include <stdint.h>
#include <string>

template <class Type, typename... Args>
Type* make_qt_owned(Args... args) {

	return new Type(args...);
}

template <class Type>
void delete_qt_forced(Type* obj) {

	delete obj;
}

inline std::string formatTimeMSec(uint32_t sec) {

	const auto Hour = 60u * 60u;
	const auto Minute = 60u;

	auto hour = sec / Hour;
	auto min = sec / Minute;
	sec -= (hour * Hour + min * Minute);

	char buff[32];
	snprintf(buff, sizeof(buff), "%02d:%02d:%02d", hour, min, sec);
	return std::string(buff);
}
