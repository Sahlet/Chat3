#pragma once
#include <stdio.h>
#include <gcroot.h>

using namespace System::Threading;

namespace LOG_FILES {
	extern FILE *out, *warn/*warning*/, *err;
	extern gcroot<System::Threading::Mutex^> out_mut, warn_mut, err_mut;
}


template< typename ... Args >
void LOG(FILE* file_desc, const char* str, Args ... args) {
	if (file_desc) {

		struct mutex_guard {
			gcroot<System::Threading::Mutex^>& mut;
			bool locked = false;
			mutex_guard(gcroot<System::Threading::Mutex^>& mut) : mut(mut) {}
			void lock() {
				mut->WaitOne();
				locked = true;
			}
			~mutex_guard() {
				if (locked) mut->ReleaseMutex();
			}
		};

		/*mutex_guard out_mut(LOG_FILES::out_mut);
		mutex_guard warn_mut(LOG_FILES::warn_mut);
		mutex_guard err_mut(LOG_FILES::err_mut);

		if (file_desc == LOG_FILES::out) out_mut.lock();
		if (file_desc == LOG_FILES::warn) warn_mut.lock();
		if (file_desc == LOG_FILES::err) err_mut.lock();*/

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