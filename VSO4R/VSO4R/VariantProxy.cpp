///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file VariantProxy.cpp
///
/// Implements a simple class that wraps a VARIANT object.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "COMVariant.h"
#include "VariantProxy.h"

VariantProxy::VariantProxy()
{
	;	// do nothing.
}


VariantProxy::VariantProxy( const VariantProxy& proxy )
{
	Simba::Test::Utilities::SetVariantValue( proxy.var, this->var );
}


VariantProxy::~VariantProxy()
{
	if ((this->var.vt == ::VT_BSTR) && (this->var.bstrVal))
	{
		::SysFreeString( this->var.bstrVal );
	}
}


