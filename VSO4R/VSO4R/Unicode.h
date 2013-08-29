///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Unicode.h
///
/// Implementation of the Unicode conversion helper function.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _UNICODE_INCLUDE_
#define _UNICODE_INCLUDE_

#include <string>


namespace Simba
{

namespace Test
{

namespace Utilities
{

std::string UnicodeToANSI( const std::wstring& wstring );

} // end namespace Utilities

} // end namespace Test

} // end namespace Simba

#endif