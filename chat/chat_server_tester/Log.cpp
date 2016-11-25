#include "Log.h"

namespace LOG_FILES {
	FILE *out = nullptr, *warn/*warning*/ = nullptr, *err = nullptr;
	gcroot<System::Threading::Mutex^> out_mut(gcnew System::Threading::Mutex), warn_mut(gcnew System::Threading::Mutex), err_mut(gcnew System::Threading::Mutex);
}