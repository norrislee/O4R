///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ODBOProviderProxy.cpp
///
/// Implements the client side proxy class for all ODBO Providers. Implements the
/// required COM interfaces for communicating with a generic ODBO provider.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

// Enable specfic COM functionality
#define _WIN32_DCOM 

#include <objbase.h>

#include <stdexcept>
#include <vector>
#include <iostream>

#include "COMError.h"
#include "COMVariant.h"
#include "ParameterMap.h"
#include "Unicode.h"
#include "ODBOProviderProxy.h"
#include "ODBOUtils.h"

using namespace Simba::Test::Utilities;

namespace Simba
{

namespace Test
{

namespace ODBO
{

ODBOProviderProxy::DBPROPProxy::DBPROPProxy( const DBPROPProxy& proxy )
{
	this->prop.dwPropertyID = proxy.prop.dwPropertyID;
	this->prop.dwOptions = proxy.prop.dwOptions;

	SetVariantValue( proxy.prop.vValue, this->prop.vValue );
}


ODBOProviderProxy::DBPROPProxy::~DBPROPProxy()
{
	if ((this->prop.vValue.vt == ::VT_BSTR) && (this->prop.vValue.bstrVal))
	{
		::SysFreeString( this->prop.vValue.bstrVal );
	}
}


ODBOProviderProxy::ODBOProviderProxy() :
m_initialized( false ),
m_dbInitialize( NULL ),
m_dbProperties( NULL ),
m_dbCreateSession( NULL ),
m_dbCreateCommand( NULL ),
m_dbSchemaRowset( NULL )
{
	this->InitializePropertyIDTypeMap();
}


ODBOProviderProxy::~ODBOProviderProxy()
{
	// Release the COM resources.
	this->ProviderDisconnect();
}


bool ODBOProviderProxy::Initialize( const std::wstring& provider, const std::wstring& parameters )
{
    // Case 1: Not connected to a provider.
	//if (!m_initialized)
	//{
		this->ProviderConnect( provider, parameters );

		m_provider = provider;
		m_initialized = true;
	/*}
    // Case 2: Already connected to a provider, but the target provider is different.
    else if (m_provider != provider)
    {
        this->ProviderDisconnect();
        this->ProviderConnect( provider, parameters );

        m_provider = provider;
        m_initialized = true;
    }
    // Case 3: Already connected to the target provider, check if parameters have changed.
	else
	{
        ParameterValueMap	tempMap;

        BuildParameterMap( parameters, tempMap );

        // The provider hasn't changed so check if the connection parameters have changed.
        if (!IdenticalParameterSets( m_currentParameterSettings, tempMap ))
        {
            this->ProviderDisconnect();
            this->ProviderConnect( provider, parameters );
            
            m_provider = provider;
            m_initialized = true;
        }
    }
	*/
	return m_initialized;
}


void ODBOProviderProxy::ProviderConnect( const std::wstring& provider, const std::wstring& parameters )
{
    try
    {
	    // Initialize COM
		// http://www.codeproject.com/script/Forums/view.aspx?fid=3785&df=4&mpp=50&select=987044&msg=987044#xx987044xx
	    if (FAILED( ::CoInitializeEx( 0, COINIT_MULTITHREADED ) ))
	    {
			//std::cerr << "ProviderConnect(): Failed to initialize COM." << std::endl;
	    }

	    ::CLSID			provClass;

	    if(FAILED( ::CLSIDFromProgID( provider.c_str(), &provClass ) ))
	    {
			std::cerr << "ProviderConnect(): Failed to find the CLSID for " << UnicodeToANSI( provider ) << " provider." << std::endl;
	    }

	    // Load the provider.
	    if (FAILED(::CoCreateInstance(
			    provClass,
			    NULL,
			    ::CLSCTX_INPROC_SERVER,
			    ::IID_IDBInitialize,
			    (void**)& this->m_dbInitialize ))
	    )
	    {
			std::cerr << "ProviderConnect(): Failed to load the " << UnicodeToANSI( provider ) << " provider." << std::endl;
	    }

	    // Retrive the properties interface.
	    if (FAILED( this->m_dbInitialize->QueryInterface(
				    ::IID_IDBProperties, (void**)& this->m_dbProperties ) )
	    )
	    {
			std::cerr << GetErrorMessage( std::string( "ProviderConnect()" ),
												       std::string( "Failed to obtain the IID_IDBProperties interface." ),
												       this->m_dbInitialize ) << std::endl;
	    }

	    std::vector<DBPROPProxy>		propertyVector;

	    // Build a map of the provider initialization properties.
	    m_currentParameterSettings.clear();
	    BuildParameterMap( parameters, m_currentParameterSettings );

	    // Build the provider initialization properties.
	    {
		    PropertyIDTypePair	propertyPair;

		    // Iterate over the property map and build the DBPROP objects.
		    for( ParameterValueMap::iterator itr = m_currentParameterSettings.begin();
			       itr != m_currentParameterSettings.end(); itr++)
		    {
				// Ignore empty properties and properties not specified in m_referencePropertyValues
			    if (!SearchMap( this->m_referencePropertyValues, (std::wstring) itr->first, propertyPair ))
			    {
					continue;
			    }
				if(itr->second.compare(L"\"\"") == 0) // If its an empty value
				{
					continue;
				}

			    // Skip ::DBPROP_INIT_ASYNCH or ::DBPROP_INIT_HWND as they are set later.
			    if ((propertyPair.first != ::DBPROP_INIT_ASYNCH) || (propertyPair.first != ::DBPROP_INIT_HWND))
			    {
				    DBPROPProxy		_property;

				    _property.prop.dwPropertyID = propertyPair.first;
				    _property.prop.dwOptions = ::DBPROPOPTIONS_REQUIRED;

				    SetVariantValue( propertyPair.second, itr->second, _property.prop.vValue );

				    propertyVector.push_back( _property );
			    }
		    }
	    }

	    {
		    DBPROPProxy		_property;

		    // Current test framework does not support asyncronous initialization.
		    _property.prop.dwPropertyID = DBPROP_INIT_ASYNCH;
		    _property.prop.dwOptions = ::DBPROPOPTIONS_REQUIRED;
		    _property.prop.vValue.vt = ::VT_I4;
		    _property.prop.vValue.ulVal = 0;
		    propertyVector.push_back( _property );
	    }

	    {
		    DBPROPProxy		_property;

		    _property.prop.dwPropertyID = ::DBPROP_INIT_HWND;
		    _property.prop.dwOptions = ::DBPROPOPTIONS_REQUIRED;
#ifdef WIN64
			_property.prop.vValue.vt = ::VT_I8;
			_property.prop.vValue.llVal = (LONGLONG) ::GetDesktopWindow();
#else
		    _property.prop.vValue.vt = ::VT_I4;
			_property.prop.vValue.lVal = (long) ::GetDesktopWindow();
#endif
		    propertyVector.push_back( _property );
	    }

	    std::vector<::DBPROPSET>	propSetVector;
	    {
		    ::DBPROPSET		_propertySet;

		    _propertySet.guidPropertySet = DBPROPSET_DBINIT;
		    _propertySet.cProperties = static_cast<ULONG>( propertyVector.size() );
		    _propertySet.rgProperties = (::DBPROP*) &propertyVector[0];		// DBPROPProxy is a thin wrapper for DBPROP.

		    propSetVector.push_back( _propertySet );
	    }

	    // Set the provider initialization properties.
	    if (FAILED( this->m_dbProperties->SetProperties( 1, &propSetVector[0] )))
	    {
			std::cerr << GetErrorMessage( std::string( "ProviderConnect()" ),
												       std::string( "Failed to set the initialization properties on the " ) + UnicodeToANSI( provider ) + " provider.",
												       this->m_dbProperties ) << std::endl;
	    }

	    // Initialize the provider.
	    if (FAILED( this->m_dbInitialize->Initialize()))
	    {
			std::cerr << GetErrorMessage( std::string( "ProviderConnect()" ),
												       std::string( "Failed to initialize the " ) + UnicodeToANSI( provider ) + " provider.",
												       this->m_dbInitialize ) << std::endl;
	    }

	    // Retrieve the DB session creation interface.
	    if (FAILED( this->m_dbInitialize->QueryInterface( ::IID_IDBCreateSession, (void **)& this->m_dbCreateSession ) ))
	    {
			std::cerr << GetErrorMessage( std::string( "ProviderConnect()" ),
												       std::string( "Failed to obtain the IID_IDBCreateSession interface." ),
												       this->m_dbInitialize ) << std::endl;
	    }

	    // Create the session and retrieve the command creation interface.
	    if (FAILED( this->m_dbCreateSession->CreateSession( NULL, ::IID_IDBCreateCommand, (IUnknown**)& this->m_dbCreateCommand ) ))
	    {
			std::cerr << GetErrorMessage( std::string( "ProviderConnect()" ),
												       std::string( "Failed to create the session and the IID_IDBCreateCommand interface." ),
												       this->m_dbCreateSession ) << std::endl;
	    }

		// Retrieve the schema rowset interface.
		if(FAILED( this->m_dbCreateCommand->QueryInterface(
			::IID_IDBSchemaRowset, (void**)& this->m_dbSchemaRowset)))
		{
			std::cerr << GetErrorMessage( std::string( "ProviderConnect()" ),
				std::string( "Failed to obtain the IID_IDBSchemaRowset interface." ),
				this->m_dbSchemaRowset ) << std::endl;
		}
    }
    catch (...)
    {
        this->ProviderDisconnect();
		std::cerr << "Connection Error." << std::endl;
    }
}


void ODBOProviderProxy::ProviderDisconnect()
{
	if (this->m_dbInitialize)
	{
		this->m_dbInitialize->Uninitialize();
		this->m_dbInitialize = NULL;
	}
	if (this->m_dbProperties)
	{
		this->m_dbProperties->Release();
		this->m_dbProperties = NULL;
	}
	if (this->m_dbCreateSession)
	{
		this->m_dbCreateSession->Release();
		this->m_dbCreateSession = NULL;
	}
	if (this->m_dbCreateCommand)
	{
		this->m_dbCreateCommand->Release();
		this->m_dbCreateCommand = NULL;
	}

	::CoUninitialize();	// Uninitialize COM

	m_currentParameterSettings.clear();

    m_provider = std::wstring();

	m_initialized = false;
}


// Each new supported initialization property should be added to this map.
void ODBOProviderProxy::InitializePropertyIDTypeMap()
{
	// Data Link Dialog's names for initialization properties
	m_referencePropertyValues.insert( PropertyPair(
							L"Data Source",
								PropertyIDTypePair( ::DBPROP_INIT_DATASOURCE, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"User ID",
								PropertyIDTypePair( ::DBPROP_AUTH_USERID, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"Password",
								PropertyIDTypePair( ::DBPROP_AUTH_PASSWORD, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"Extended Properties",
								PropertyIDTypePair( ::DBPROP_INIT_PROVIDERSTRING, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"Initial Catalog",
								PropertyIDTypePair( ::DBPROP_INIT_CATALOG, ::VT_BSTR ) ) );

	// Touchstone names (also used in JAMS connection files)
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_DATASOURCE",
								PropertyIDTypePair( ::DBPROP_INIT_DATASOURCE, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_ASYNCH",
								PropertyIDTypePair( ::DBPROP_INIT_ASYNCH, ::VT_I4 ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_PROMPT",
								PropertyIDTypePair( ::DBPROP_INIT_PROMPT, ::VT_I2 ) ) );
#ifdef WIN64
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_HWND",
								PropertyIDTypePair( ::DBPROP_INIT_HWND, ::VT_I8 ) ) );
#else
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_HWND",
								PropertyIDTypePair( ::DBPROP_INIT_HWND, ::VT_I4 ) ) );
#endif
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO",
								PropertyIDTypePair( ::DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, ::VT_BOOL ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_LCID",
								PropertyIDTypePair( ::DBPROP_INIT_LCID, ::VT_I4 ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_AUTH_USERID",
								PropertyIDTypePair( ::DBPROP_AUTH_USERID, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_AUTH_PASSWORD",
								PropertyIDTypePair( ::DBPROP_AUTH_PASSWORD, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_PROVIDERSTRING",
								PropertyIDTypePair( ::DBPROP_INIT_PROVIDERSTRING, ::VT_BSTR ) ) );
	m_referencePropertyValues.insert( PropertyPair(
							L"DBPROP_INIT_CATALOG",
								PropertyIDTypePair( ::DBPROP_INIT_CATALOG, ::VT_BSTR ) ) );
}


void ODBOProviderProxy::GetRowset( ::GUID rowsetGuid, IQueryRowset* queryRowset )
{
	this->GetRowset( rowsetGuid, VariantVector(), queryRowset );
}


void ODBOProviderProxy::GetRowset( ::GUID rowsetGuid, VariantVector& restrictionVector, IQueryRowset* queryRowset )
{
    ::IDBSchemaRowset*	dbSchemaRowset = NULL;
    IUnknown*		rowset = NULL;
	try
    {
	    if(FAILED(this->m_dbCreateCommand->QueryInterface(::IID_IDBSchemaRowset, (void**)& dbSchemaRowset)))
	    {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
												   std::string( "Failed to obtain the IID_IDBSchemaRowset interface." ),
												   this->m_dbCreateCommand ) << std::endl;
	    }

	    ::VARIANT*		restrictions;
	    if (restrictionVector.size() > 0)
	    {
			// Casting a VariantVector to VARIANT * only works because VariantProxy only has one
			// member, which is a variant. If new members are added to VariantProxy, then
			// we need to write a new method that produces a proper array of VARIANT's.
		    restrictions = (::VARIANT*) &restrictionVector[0];
	    }
	    else
	    {
		    restrictions = NULL;
	    }

	    if (FAILED( dbSchemaRowset->GetRowset(NULL, rowsetGuid, static_cast<ULONG>( restrictionVector.size() ), restrictions, ::IID_IUnknown, 0, NULL, &rowset) ))
	    {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
												       std::string( "Failed to obtain the rowset object." ),
												       dbSchemaRowset ) << std::endl;
	    }
	    queryRowset->SetRowset( rowset );
    	
        if (rowset)
        {
            rowset->Release();
            rowset = NULL;
        }
        if (dbSchemaRowset)
        {
            dbSchemaRowset->Release();
            dbSchemaRowset = NULL;
        }
    }
    catch (...)
    {
        if (rowset)
        {
            rowset->Release();
            rowset = NULL;
        }
        if (dbSchemaRowset)
        {
            dbSchemaRowset->Release();
            dbSchemaRowset = NULL;
        }
		std::cerr << "Get Rowset error." << std::endl;
    }

}


void ODBOProviderProxy::GetRowset( std::wstring& mdxQuery, IQueryRowset* queryRowset )
{
	::ICommandText*         commandText = NULL;
    ::ICommandProperties*   cmdProperties = NULL;
    IUnknown*		rowset = NULL;
    try
    {
	    if (FAILED( this->m_dbCreateCommand->CreateCommand( NULL, ::IID_ICommandText, (::IUnknown**) &commandText ) ))
	    {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
												   std::string( "Failed to create the command text object." ),
												   this->m_dbCreateCommand ) << std::endl;
	    }

	    if (FAILED( commandText->SetCommandText( ::DBGUID_MDX, mdxQuery.c_str() ) ))
	    {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
												       std::string( "Failed to set the MDX querycommand text." ),
												       commandText ) << std::endl;
	    }

        // set flattening property
        std::vector<DBPROPProxy> rowsetProps;
        std::vector<::DBPROPSET> cmdPropSets;
        {
            // setting the DBPROP_IRowset property to VARIANT_TRUE forces a flattened rowset.
            DBPROPProxy _property;
            _property.prop.dwPropertyID = ::DBPROP_IRowset;
            _property.prop.dwOptions = ::DBPROPOPTIONS_REQUIRED;
            _property.prop.vValue.vt = ::VT_BOOL;
            _property.prop.vValue.lVal = VARIANT_TRUE;
            rowsetProps.push_back( _property );
        }

        {
	        ::DBPROPSET		_propertySet;

            _propertySet.guidPropertySet = ::DBPROPSET_ROWSET;
	        _propertySet.cProperties = static_cast<ULONG>( rowsetProps.size() );
	        _propertySet.rgProperties = (::DBPROP*) &rowsetProps[0];
	        cmdPropSets.push_back( _propertySet );
        }

        if (FAILED( commandText->QueryInterface( IID_ICommandProperties, (void**) &cmdProperties ) ) )
        {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
											           std::string( "Failed to get command properties" ),
    										           commandText ) << std::endl;
        }

        if (FAILED( cmdProperties->SetProperties( static_cast<ULONG> (cmdPropSets.size()), &cmdPropSets[0] ) ) )
        {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
											           std::string( "Failed to set command properties" ),
    										           commandText ) << std::endl;
        }

        if (FAILED( commandText->Execute( NULL, ::IID_IRowset, NULL, NULL, &rowset ) ))
        {
			std::cerr << GetErrorMessage( std::string( "GetRowset()" ),
		    								           std::string( "Failed to execute the MDX query." ),
			    							           commandText ) << std::endl;
	    }

        queryRowset->SetRowset( rowset );
        if (rowset)
        {
	        rowset->Release();
            rowset = NULL;
	    }

        if (cmdProperties)
        {
            cmdProperties->Release();
            cmdProperties = NULL;
        }

        if (commandText)
        {
            commandText->Release();
            commandText = NULL;
        }
    }
    catch (...)
    {
        if (rowset)
        {
            rowset->Release();
            rowset = NULL;
        }

        if (cmdProperties)
        {
            cmdProperties->Release();
            cmdProperties = NULL;
        }
        if (commandText)
        {
            commandText->Release();
            commandText = NULL;
        }
		std::cerr << "Get Rowset error." << std::endl;
    }
}

void ODBOProviderProxy::executeDataset( std::wstring& mdxQuery, ::IUnknown ** dataset )
{
	::ICommandText* commandText = NULL;

	_CreateCommand(GetDbCreateCommand(), ::IID_ICommandText, reinterpret_cast<::IUnknown **>(&commandText));

	if (FAILED( commandText->SetCommandText( ::DBGUID_MDX, mdxQuery.c_str() ) ))
    {
		std::cerr << GetErrorMessage( std::string( "executeDataset()" ),
											       std::string( "Failed to set the MDX querycommand text." ),
											       commandText ) << std::endl;
    }

	if (FAILED( commandText->Execute( NULL, ::IID_IMDDataset, NULL, NULL, dataset ) ))
    {
		std::cerr << GetErrorMessage( std::string( "executeDataset()" ),
	    								           std::string( "Failed to execute the MDX query." ),
		    							           commandText ) << std::endl;
    }
}

::IDBInitialize* ODBOProviderProxy::GetDbInitialize()
{
	return this->m_dbInitialize;
}

::IDBProperties* ODBOProviderProxy::GetDbProperties()
{
	return this->m_dbProperties;
}

::IDBCreateCommand* ODBOProviderProxy::GetDbCreateCommand()
{
	return this->m_dbCreateCommand;
}

::IDBSchemaRowset* ODBOProviderProxy::GetDbSchemaRowset()
{
	return this->m_dbSchemaRowset;
}

} // end namespace ODBO

} // end namespace Test

} // end namespace Simba
