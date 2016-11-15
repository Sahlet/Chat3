#pragma once
#include <stdio.h>
#include <mutex>

namespace LOG_FILES {
	extern FILE *out, *warn/*warning*/, *err;
	extern std::mutex out_mut, warn_mut, err_mut;
}


template< typename ... Args >
void LOG(FILE* file_desc, const char* str, Args ... args) {
	if (file_desc) {
		std::unique_lock< std::mutex > out_lock(LOG_FILES::out_mut, std::defer_lock);
		std::unique_lock< std::mutex > warn_lock(LOG_FILES::warn_mut, std::defer_lock);
		std::unique_lock< std::mutex > err_lock(LOG_FILES::err_mut, std::defer_lock);

		if (file_desc == LOG_FILES::out) out_lock.lock();
		if (file_desc == LOG_FILES::warn) warn_lock.lock();
		if (file_desc == LOG_FILES::err) err_lock.lock();

		fprintf(file_desc, str, args...);
	}
}

template< typename ... Args >
void LOGI(const char* str, Args ... args) {
	LOG(LOG_FILES::out, str, args...);
}

template< typename ... Args >
void LOGW(const char* str, Args ... args) {
	LOG(LOG_FILES::warn, str, args...);
}

template< typename ... Args >
void LOGE(const char* str, Args ... args) {
	LOG(LOG_FILES::err, str, args...);
}