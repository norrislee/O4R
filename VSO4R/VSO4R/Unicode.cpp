///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Unicode.cpp
///
/// Implementation of the Unicode conversion helper function.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "Unicode.h"

namespace Simba
{

namespace Test
{

namespace Utilities
{

std::string UnicodeToANSI( const std::wstring& wstring )
{
	size_t				length = ::wcstombs( NULL, wstring.c_str(), 0 ) + 1;
	std::vector<char>	buffer( length );

	::memset( &buffer[0], 0, length );
	::wcstombs( &buffer[0], wstring.c_str(), length );

	return std::string( &buffer[0] );
}

} // end namespace Utilities

} // end namespace Test

} // end namespace Simba
