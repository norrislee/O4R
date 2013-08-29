///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file COMVariant.cpp
///
/// Implementation of the COM helper functions for manipulating COM VARIANT objects.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include <stdexcept>
#include <string>
#include <iostream>

#include "COMVariant.h"


namespace Simba
{

namespace Test
{

namespace Utilities
{

// Helper function to perform data conversion of the input string value.
void SetVariantValue(const ::VARTYPE type, const std::wstring& value, ::VARIANT& vValue )
{
	switch(type)
	{
	case ::VT_BSTR:
		vValue.bstrVal = ::SysAllocString( value.c_str() );
		break;
	case ::VT_I2:
		vValue.iVal = ::_wtoi( value.c_str() );
		break;
	case ::VT_I4:
		vValue.lVal = ::_wtol( value.c_str() );
		break;
	case ::VT_I8:
		vValue.llVal = ::_wtoi64( value.c_str() );
		break;
	case ::VT_UI4:
		vValue.ulVal = ::_wtol( value.c_str() );
		break;
	case ::VT_BOOL:
		vValue.boolVal = (value == L"TRUE") ? -1 : 0;
		break;
	case ::VT_EMPTY:
		break;
	default:
		// Unsupported variant type. Add appropriate conversion logic.
		assert(false);
		std::cerr << "SetVariantValue(): Unsupported data type." << std::endl;
	}

	vValue.vt = type;
}


// Helper function for the DBPROPProxy copy constructor.
void SetVariantValue(const ::VARIANT& vValueRef, ::VARIANT& vValue )
{
	switch(vValueRef.vt)
	{
	case ::VT_BSTR:
		vValue.bstrVal = ::SysAllocString( vValueRef.bstrVal );
		break;
	case ::VT_I2:
		vValue.iVal = vValueRef.iVal;
		break;
	case ::VT_I4:
		vValue.lVal = vValueRef.lVal;
		break;
	case ::VT_I8:
		vValue.llVal = vValueRef.llVal;
		break;
	case ::VT_UI4:
		vValue.ulVal = vValueRef.ulVal;
		break;
	case ::VT_BOOL:
		vValue.boolVal = vValueRef.boolVal;
		break;
	case ::VT_EMPTY:
		break;
	default:
		// Unsupported variant type.
		assert(false);
		std::cerr << "SetVariantValue(): Unsupported data type." << std::endl;
	}

	vValue.vt = vValueRef.vt;
}

} // end namespace Utilities

} // end namespace Test

} // end namespace Simba
