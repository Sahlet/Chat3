#include "Log.h"

namespace LOG_FILES {
	FILE *out = nullptr, *warn/*warning*/ = nullptr, *err = nullptr;
	std::mutex out_mut, warn_mut, err_mut;
}