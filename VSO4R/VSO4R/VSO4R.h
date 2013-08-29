/*
 *  O4R/VSO4R/VSO4R/VSO4R.h by Norris Lee and George Chow. Copyright (C) 2013
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available at
 *  http://www.r-project.org/Licenses/
 */

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

	ADDAPI void ADDCALL odboDisconnectAll();

	ADDAPI bool ADDCALL odboExecute(int connectionNum, const char *query);

	ADDAPI void ADDCALL odboGetDataset(char** results);

#ifdef __cplusplus
}
#endif