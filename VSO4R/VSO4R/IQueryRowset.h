///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file IQueryRowset.h
///
/// Interface for the IQueryRowset. Defines the interface for all classes that will
/// process a query result set.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _IQUERY_ROW_SET_INCLUDE_
#define _IQUERY_ROW_SET_INCLUDE_

#include <oledb.h>


namespace Simba
{

namespace Test
{

namespace ODBO
{


class IQueryRowset
{
public:
	virtual ~IQueryRowset() { ; }

	// Derived class takes ownership of the rowset.
	virtual void SetRowset( ::IUnknown* rowset ) = 0;
};


} // end namespace ODBO

} // end namespace Test

} // end namespace Simba

#endif