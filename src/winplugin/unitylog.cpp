#include "unitylog.h"

// MedamaP
#ifdef _UNITY

#if defined(_WIN32) || defined(_WIN64)
#define UNITY_INTERFACE_EXPORT __declspec(dllexport)
#else
#define UNITY_INTERFACE_EXPORT
#endif

extern "C"
{
	using debug_log_func_type = void(*)(const char*);

	namespace
	{
		debug_log_func_type debug_log_func = nullptr;
	}

	void debug_log(const char* msg)
	{
		if (debug_log_func != nullptr) debug_log_func(msg);
	}

	UNITY_INTERFACE_EXPORT void set_debug_log_func(debug_log_func_type func)
	{
		debug_log_func = func;
	}

	UNITY_INTERFACE_EXPORT void debug_log_test()
	{
		debug_log("hogehoge");
	}
}

#endif
