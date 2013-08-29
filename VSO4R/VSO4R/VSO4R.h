#pragma once

#include "ODBOProviderProxy.h"
#include "ODBOUtils.h"
#include "COMError.h"

#ifdef ADD_EXPORTS
#define ADDAPI __declspec(dllexport)
#else
#define ADDAPI __declspec(dllimport)
#endif

#define ADDCALL __cdecl

#ifdef __cplusplus
extern "C" {
#endif

	ADDAPI int ADDCALL odboConnect(const char *provider, const char *parameters);

	ADDAPI void ADDCALL odboDisconnect(int connectionNum);

	void ADDCALL odboDisconnectAll();

	ADDAPI bool ADDCALL odboExecute(int connectionNum, const char *query);

	ADDAPI void ADDCALL odboGetDataset(char** results);

#ifdef __cplusplus
}
#endif