///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ParameterMap.cpp
///
/// Implementation of the Parameter Map helper functions.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdexcept>
#include <iostream>

#include "ParameterMap.h"
#include "Unicode.h"

namespace Simba
{

namespace Test
{

namespace Utilities
{

// Helper function to construct a map of parameters and thier associated values.
void BuildParameterMap( const std::wstring parameters, ParameterValueMap&	parameterValues )
{
	wchar_t			paramaterValueTerminator = L';';
	size_t			begin = 0;
	size_t			end;

	end = parameters.find( L"=", begin );

	while (end != std::wstring::npos)
	{
		ParameterPair	parameterValue;

		parameterValue.first = parameters.substr( begin, end-begin );
		begin = ++end;

		// Check for nested parameter values.
		if (parameters[begin] == L'{')
		{
			paramaterValueTerminator = L'}';
			begin++;
		}

		end = parameters.find( paramaterValueTerminator, begin );
		if(end-begin > 1) // If its not an empty value
		{
			parameterValue.second = parameters.substr( begin, end-begin );
			parameterValues.insert( parameterValue );
		}

		// Get next parameter pair.
		if (end != std::wstring::npos)
		{
			begin = end+1;

			// Change to original search character.
			if (parameters[end] == L'}')
			{
				begin++;
				paramaterValueTerminator = L';';
			}
			
			end = parameters.find( L"=", begin );
		}
	}
}


// Helper function to construct a map of parameters and thier associated values.
void BuildRestrictionParameterMap( const std::wstring restrictionValues, const std::vector<std::wstring>& restrictions, ParameterValueMap&	restrictionMap )
{
	ParameterValueMap		restrictionValuesMap;

	// Build the restriction value map based upon the configuration file test parameters.
	BuildParameterMap( restrictionValues, restrictionValuesMap );

	std::vector<std::wstring>::const_iterator	itr = restrictions.begin();

	// Loop over the test restrictions and retrieve the restriction values from the map.
	while (itr != restrictions.end())
	{
		std::wstring	value;

		if (!SearchMap( restrictionValuesMap, *itr, value ))
		{
			std::cerr << "BuildRestrictionParameterMap(): Failed to find the value for the " << UnicodeToANSI( (*itr).c_str() ) << " restriction." << std::endl;
		}

		// Add the test restriction and it's associated value to the output map.
		restrictionMap.insert( ParameterPair( *itr, value ) );

		itr++;
	}
}

} // end namespace Utilities.

} // end namespace Test.

} // end namespace Simba.
