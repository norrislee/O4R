///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ODBOProviderProxy.h
///
/// Implements the client side proxy class for all ODBO Providers. Implements the
/// required COM interfaces for communicating with a generic ODBO provider.
///
/// Copyright: Copyright (C) 2013 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ODBO_PROVIDER_PROXY_INCLUDE_
#define _ODBO_PROVIDER_PROXY_INCLUDE_

#include <oledb.h>
#include <map>
#include "IQueryRowset.h"
#include "VariantProxy.h"
#include "ParameterMap.h"

typedef std::pair< ::DBPROPID, ::VARTYPE>			PropertyIDTypePair;
typedef std::map<std::wstring, PropertyIDTypePair>	PropertyIDTypeMap;
typedef std::pair<std::wstring, PropertyIDTypePair>	PropertyPair;

namespace Simba
{

namespace Test
{

namespace ODBO
{


class ODBOProviderProxy
{
public:

	ODBOProviderProxy();
	virtual ~ODBOProviderProxy();
	bool Initialize( const std::wstring& provider, const std::wstring& properties );
	bool IsInitialized() { return m_initialized; }
	void GetRowset( ::GUID rowsetGuid, IQueryRowset* queryRowset );
	void GetRowset( ::GUID rowsetGuid, VariantVector& restrictionVector, IQueryRowset* queryRowset );
	void GetRowset( std::wstring& mdxQuery, IQueryRowset* queryRowset );
	const std::wstring& GetProviderName() { return m_provider; }
	void executeDataset( std::wstring& mdxQuery, ::IUnknown ** dataset );
	::IDBInitialize* GetDbInitialize();
	::IDBProperties* GetDbProperties();
	::IDBCreateCommand* GetDbCreateCommand();
	::IDBSchemaRowset*	GetDbSchemaRowset();

private:
	void ProviderConnect( const std::wstring& provider, const std::wstring& properties );
	void ProviderDisconnect();
	void InitializePropertyIDTypeMap();

private:
	bool					m_initialized;
	std::wstring			m_provider;
	PropertyIDTypeMap		m_referencePropertyValues;
	ParameterValueMap		m_currentParameterSettings;

private:
	::IDBInitialize*		m_dbInitialize;
	::IDBProperties*		m_dbProperties;
	::IDBCreateSession*		m_dbCreateSession;
	::IDBCreateCommand*		m_dbCreateCommand;
	::IDBSchemaRowset*		m_dbSchemaRowset;

private:
	// Helper class that releases and allocates memory in the DBPROP object.
	class DBPROPProxy
	{
	public:
		DBPROPProxy() { ; }
		DBPROPProxy( const DBPROPProxy& proxy );
		~DBPROPProxy();

	public:
		// Do not add ANY new data members to this class!!
		// This class is intended as a proxy for DBPROP and
		// must only have the memory footprint of the DBPROP
		// object.
		::DBPROP	prop;
	};
};


} // end namespace ODBO

} // end namespace Test

} // end namespace Simba

#endif