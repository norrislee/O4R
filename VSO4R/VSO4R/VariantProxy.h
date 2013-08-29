///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file VariantProxy.h
///
/// Implements a simple class that wraps a VARIANT object.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "COMVariant.h"
#ifndef _VARIANT_PROXY_INCLUDE_
#define _VARIANT_PROXY_INCLUDE_

#include <oledb.h>

#include <vector>


// Helper class that releases and allocates memory in the VARIANT object.
class VariantProxy
{
public:
	VariantProxy();
	VariantProxy( const VariantProxy& proxy );
	~VariantProxy();

public:
	// Do not add ANY new data members to this class!!
	// This class is intended as a proxy for VARIANT and
	// must only have the memory footprint of the VARIANT
	// object.
	// ODBOProviderProxy::GetRowset (and maybe elsewhere) 
	// takes a vector of this class and then casts it to 
	// an array of variants, so adding new members will
	// break these code.
	::VARIANT	var;
};

typedef std::vector<VariantProxy>	VariantVector;


#endif
