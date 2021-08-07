#pragma once

#undef mfuse_EXPORTS

#ifdef mfuse_DLL
	#ifdef __GNUC__
		#define mfuse_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define mfuse_EXPORTS __declspec(dllexport)
	#endif
	#define mfuse_TEMPLATE
#else
	#ifdef __GNUC__
		#define mfuse_EXPORTS
	#else
		#define mfuse_EXPORTS __declspec(dllimport)
	#endif
	#define mfuse_TEMPLATE extern
#endif
