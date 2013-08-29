///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file COMVariant.h
///
/// Implementation of the COM helper functions for manipulating COM VARIANT objects.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _COM_VARIANT_INCLUDE_
#define _COM_VARIANT_INCLUDE_

#include <oledb.h>

#include <string>


namespace Simba
{

namespace Test
{

namespace Utilities
{

void SetVariantValue(const ::VARTYPE type, const std::wstring& value, ::VARIANT& vValue );
void SetVariantValue(const ::VARIANT& vValueRef, ::VARIANT& vValue );

} // end namespace Utilities

} // end namespace Test

} // end namespace Simba

#endif