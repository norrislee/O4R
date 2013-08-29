///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ParameterMap.h
///
/// Implementation of the Parameter Map helper functions.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _PARAMETER_MAP_INCLUDE_
#define _PARAMETER_MAP_INCLUDE_

#include <map>
#include <string>
#include <utility>
#include <vector>

typedef std::map<std::wstring, std::wstring>	ParameterValueMap;
typedef std::pair<std::wstring, std::wstring>	ParameterPair;

namespace Simba
{

namespace Test
{

namespace Utilities
{

// Helper function to construct a map of parameters and thier associated values.
void BuildParameterMap( const std::wstring parameters, ParameterValueMap&	parameterValues );


// Helper function to construct a map of parameters and thier associated values.
void BuildRestrictionParameterMap( const std::wstring restrictionValues, const std::vector<std::wstring>& restrictions, ParameterValueMap&	restrictionMap );


// Helper function to search a string based map for the associated value.
template <class A, class B>
bool SearchMap( const A map, const std::wstring& propertyName, B& value )
{
	typename A::const_iterator iter;
	bool ret = false;

	iter = map.find( propertyName );
	if (iter != map.end())
	{
		value = iter->second;

		ret = true;
	}

	return ret;
}


// Helper function to determine if 2 maps contain the same parameter sets.
template <class A>
bool IdenticalParameterSets( const A& map1, const A& map2 )
{
	bool	ret = true;

	if (map1.size() == map2.size())
	{
		typename A::const_iterator	iter1;
		typename A::const_iterator	iter2;

		for (iter1 = map1.begin(), iter2 = map2.begin();
			 iter1 != map1.end();
			 iter1++, iter2++)
		{
			if ((iter1->first != iter2->first) || (iter1->second != iter2->second))
			{
				ret = false;

				break;
			}
		}
	}
	else
	{
		ret = false;
	}

	return ret;
}


} // end namespace Utilities

} // end namespace Test

} // end namespace Simba

#endif
