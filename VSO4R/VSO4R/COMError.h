///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file COMError.h
///
/// Implementation of the COM helper functions for manipulating COM VARIANT objects.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _COM_ERROR_INCLUDE_
#define _COM_ERROR_INCLUDE_

#include <string>
#include <vector>


namespace Simba
{

namespace Test
{

namespace Utilities
{

	std::string GetErrorMessage( std::string& function, std::string& defaultMsg, ::IUnknown* iUnknown );
	void GetErrorMessages( ::IUnknown* unknown, std::vector<std::string>& messageVector );


} // end namespace Utilities

} // end namespace Test

} // end namespace Simba

#endif