#pragma once

#ifndef __unitylog_H__
#define __unitylog_H__


// MedamaP
#ifdef _UNITY

#if defined(_WIN32) || defined(_WIN64)
#define UNITY_INTERFACE_EXPORT __declspec(dllexport)
#else
#define UNITY_INTERFACE_EXPORT
#endif

extern "C"
{
	extern void debug_log(const char* msg);
}

#endif

#endif // ! __unitylog_H__
