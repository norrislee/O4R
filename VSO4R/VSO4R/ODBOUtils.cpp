///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Unicode.cpp
///
/// Implementation of the Unicode conversion helper function.
///
/// Copyright: Copyright (C) 2013 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <oledb.h>
#include <stdexcept>
#include <iostream>
#include "ODBOUtils.h"
#include "Unicode.h"
#include "COMError.h"
#include <assert.h>

using namespace Simba::Test::Utilities;

namespace Simba
{

namespace Test
{

namespace Utilities
{

DBLENGTH GetColumnLength( DBTYPE type, DBLENGTH columnSize, bool isFixedLength )
{
	::DBLENGTH		length = 0;

    // column size gets set to ~0 if it is a non-fixed length column
    // with no defined maximum.
    if (!isFixedLength && 
        static_cast<::DBLENGTH> (-1) == columnSize)
    {
        columnSize = 5000;
    }


	switch( type )
	{
		case ::DBTYPE_NULL:
		case ::DBTYPE_EMPTY:
		case ::DBTYPE_I1:
		case ::DBTYPE_I2:
		case ::DBTYPE_I4:
		case ::DBTYPE_UI1:
		case ::DBTYPE_UI2:
		case ::DBTYPE_UI4:
		case ::DBTYPE_R4:
		case ::DBTYPE_BOOL:
		case ::DBTYPE_I8:
		case ::DBTYPE_UI8:
		case ::DBTYPE_R8:
		case ::DBTYPE_CY:
		case ::DBTYPE_ERROR:
			length = (25 + 1) * sizeof( ::WCHAR );				// All fit in 25 char + NULL.
			break;
		case ::DBTYPE_DECIMAL:
		case ::DBTYPE_NUMERIC:
		case ::DBTYPE_DATE:
		case ::DBTYPE_DBDATE:
		case ::DBTYPE_DBTIMESTAMP:
		case ::DBTYPE_GUID:
			length = (50 + 1) * sizeof( ::WCHAR );				// All fit in 50 char + NULL.
			break;
		case ::DBTYPE_BYTES:
			length = (columnSize * 2 + 1) * sizeof( ::WCHAR );	// Each byte is 2 characters + NULL.
			break;
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BSTR:
			length = (columnSize + 1) * sizeof( ::WCHAR );		// Size + NULL.
			break;
		default:
			length = MAX_COLUMN_SIZE;
			break;
	}

	return length;
}


void _GetColumnInfo(
					IUnknown	*pUnknown,
					DBORDINAL	*pcColumns,
					DBCOLUMNINFO **prgInfo,
					OLECHAR      **ppStringsBuffer,
					::IID		RowsetORdataset
					)
{
	::IColumnsInfo*		_columnInfo = NULL;

	try
	{
		// Retrieve the column info interface.
		if(IsEqualGUID(RowsetORdataset, IID_IRowset))
		{
				if (FAILED(static_cast<::IRowset*>(pUnknown)->QueryInterface( ::IID_IColumnsInfo, (void**)& _columnInfo )))
				{
					std::cerr << GetErrorMessage( std::string( "_GetColumnInfo()" ),
															   std::string( "Failed to obtain the IID_IColumnsInfo interface." ),
															   static_cast<::IRowset*>(pUnknown) ) << std::endl;
				}
		}
		else if(IsEqualGUID(RowsetORdataset, IID_IMDDataset))
		{
				if (FAILED(static_cast<::IMDDataset*>(pUnknown)->QueryInterface( ::IID_IColumnsInfo, (void**)& _columnInfo )))
				{
					std::cerr << GetErrorMessage( std::string( "_GetColumnInfo()" ),
															   std::string( "Failed to obtain the IID_IColumnsInfo interface." ),
															   static_cast<::IMDDataset*>(pUnknown) ) << std::endl;
				}
		}
		else
		{
				std::cerr << "Unexpected IID. Expect IID_IRowset or IID_IMDDataset" << std::endl;
		}

		// Retreive the result set column info.
		if (FAILED(_columnInfo->GetColumnInfo( pcColumns, prgInfo, ppStringsBuffer )))
		{
			std::cerr << GetErrorMessage( std::string( "_GetColumnInfo()" ),
													   std::string( "Failed to obtain the column information." ),
													   _columnInfo ) << std::endl;
		}

		if(_columnInfo) _columnInfo->Release();
	}
	catch(...)
	{
		if(_columnInfo) _columnInfo->Release();
		std::cerr << "Get column info error." << std::endl;
	}
}

void _CreateBinding(
					DBORDINAL		cColumns,
					DBBYTEOFFSET	&rowlength,
					DBCOLUMNINFO *	rgColumnInfo,
					DBBINDING**		pbindings
			   )
{
	::DBBYTEOFFSET		offset = 0;
	::DBBINDING *		bindings;
	bindings = reinterpret_cast<::DBBINDING*>( ::CoTaskMemAlloc( cColumns * sizeof( ::DBBINDING ) ) );
	::memset( bindings, 0, cColumns * sizeof( DBBINDING ) );

	// Construct the binding array
	for (::DBORDINAL i = 0; i < cColumns; i++)
	{

		bindings[i].iOrdinal = rgColumnInfo[i].iOrdinal;
		bindings[i].dwPart = ::DBPART_VALUE | ::DBPART_LENGTH | ::DBPART_STATUS;
		bindings[i].obStatus = offset;
		bindings[i].obLength = offset + sizeof( ::DBSTATUS );
		bindings[i].obValue = offset + sizeof( ::DBSTATUS ) + sizeof( ::DBLENGTH );
		bindings[i].dwMemOwner = ::DBMEMOWNER_CLIENTOWNED;
		bindings[i].eParamIO = ::DBPARAMIO_NOTPARAM;
		bindings[i].bPrecision = rgColumnInfo[i].bPrecision;
		bindings[i].bScale = rgColumnInfo[i].bScale;
		bindings[i].wType = ::DBTYPE_WSTR;			// Convert all results to string values.
		bindings[i].cbMaxLen = GetColumnLength( rgColumnInfo[i].wType, rgColumnInfo[i].ulColumnSize, (rgColumnInfo[i].dwFlags & ::DBCOLUMNFLAGS_ISFIXEDLENGTH) != 0 );

		offset = bindings[i].cbMaxLen + bindings[i].obValue;
	}

	rowlength = offset;
	*pbindings = bindings;
}

void _CreateAccessor(IUnknown	*pDataSet, 
					 HACCESSOR	&hAccessor, 
					 DBORDINAL	cColumns, 
					 DBBINDING	*bindings)
{
	::IAccessor*		accessor = NULL;
	try
	{
		// Obtain the accessor interface.
		if (FAILED(pDataSet->QueryInterface( ::IID_IAccessor, (void**)& accessor )))
		{
			std::cerr << GetErrorMessage( std::string( "_CreateAccessor()" ),
				std::string( "Failed to obtain the IID_IAccessor interface." ),
				pDataSet ) << std::endl;
		}

		// Create the accessor.
		if (FAILED(accessor->CreateAccessor( ::DBACCESSOR_ROWDATA, cColumns, bindings, 0, &hAccessor, NULL )))
		{
			std::cerr << GetErrorMessage( std::string( "_CreateAccessor()" ),
				std::string( "Failed to create the dataset accessor." ),
				pDataSet ) << std::endl;
		}
		if(accessor) accessor->Release();
	}
	catch(...)
	{
		if(accessor) accessor->Release();
	}
}

void _FreeAxisInfo(IMDDataset	*pDataSet,
				   DBCOUNTITEM		cAxes,
				   MDAXISINFO		*rgAxisInfo)
{
	assert(pDataSet);
	if (FAILED(pDataSet->FreeAxisInfo(cAxes, rgAxisInfo)))
	{
		std::cerr << GetErrorMessage( std::string( "_FreeAxisInfo()" ),
			std::string( "FreeAxisInfo failed." ),
			pDataSet) << std::endl;
	}
}

void _CreateAccessorPlusPrerequisite(
									 ::IUnknown * pUnknown, 
									 ::HACCESSOR &hAccessor, 
									 ::DBBYTEOFFSET &rowlength,
									 ::IID RowsetORdataset
									 )
{
	assert(hAccessor == NULL);
	assert((IsEqualGUID(RowsetORdataset, IID_IRowset)) || (IsEqualGUID(RowsetORdataset, IID_IMDDataset)));

	//ColumnInfo
	::DBORDINAL			cColumns = 0;
	::DBCOLUMNINFO *	rgColumnInfo = 0;
	::OLECHAR *			pStringsBuffer;

	//Bindings
	::DBBINDING*		bindings = NULL;

	_GetColumnInfo(pUnknown, &cColumns, &rgColumnInfo, &pStringsBuffer, RowsetORdataset);

	_CreateBinding(cColumns, rowlength, rgColumnInfo, &bindings);

	_CreateAccessor(pUnknown, hAccessor, cColumns, bindings);

	return;
}

void _CreateInstance(
					 std::wstring		provider, // [in]
					 ::IDBInitialize*	&dbInitialize //[out]
					 )
{
	// Initialize COM
	if (FAILED( ::CoInitializeEx( 0, COINIT_MULTITHREADED ) ))
	{
		std::cerr << "_CreateInstance(): Failed to initialize COM." << std::endl;
	}

	::CLSID			provClass;
	if(FAILED( ::CLSIDFromProgID( provider.c_str(), &provClass ) ))
	{
		std::cerr << "_CreateInstance(): Failed to find the CLSID for " << UnicodeToANSI( provider ) << " provider." << std::endl;
	}

	// Load the provider.
	if (FAILED(::CoCreateInstance(
			provClass,
			NULL,
			::CLSCTX_INPROC_SERVER,
			::IID_IDBInitialize,
			(void**)& dbInitialize ))
	)
	{
		std::cerr << "_CreateInstance(): Failed to load the " << UnicodeToANSI( provider ) << " provider." << std::endl;
	}
}

void _CreateCommand(::IDBCreateCommand * dbCreateCommand, const IID iid, ::IUnknown ** iUnknown)
{
	if (FAILED( dbCreateCommand->CreateCommand( NULL, iid, iUnknown ) ))
	{
		std::cerr << GetErrorMessage( std::string( "_CreateCommand()" ),
			std::string( "Failed to create the command text object." ),
			dbCreateCommand ) << std::endl;
	}
}

void _freeDbPropInfoSet(ULONG cPropertyInfoSets, DBPROPINFOSET * &rgPropertyInfoSets)
{
	if(rgPropertyInfoSets)
	{
		for(ULONG i=0; i < cPropertyInfoSets; i++)
		{
			for(ULONG j = 0; j < rgPropertyInfoSets[i].cPropertyInfos; j++)
			{
				VariantClear(&(rgPropertyInfoSets[i].rgPropertyInfos[j].vValues));
			}
			CoTaskMemFree(rgPropertyInfoSets[i].rgPropertyInfos);
		}
		CoTaskMemFree(rgPropertyInfoSets);
		rgPropertyInfoSets = NULL;
	}
}

void _freeDbPropSet(ULONG cPropertySets, DBPROPSET * &rgPropertySets)
{
	if(rgPropertySets)
	{
		for(ULONG i = 0; i < cPropertySets; i++)
		{
			for(ULONG j = 0; j < rgPropertySets[i].cProperties; j++)
			{
				VariantClear(&(rgPropertySets[i].rgProperties[j].vValue));
			}
			::CoTaskMemFree(rgPropertySets[i].rgProperties);
		}
		::CoTaskMemFree(rgPropertySets);
		rgPropertySets = NULL;
	}
}

HRESULT _GetRowset(
				   IDBSchemaRowset *schemaRowset,
				   IUnknown       *pUnkOuter,
				   REFGUID         rguidSchema,
				   ULONG           cRestrictions,
				   const VARIANT   rgRestrictions[],
				   REFIID          riid,
				   ULONG           cPropertySets,
				   DBPROPSET       rgPropertySets[],
				   IUnknown      **ppRowset)
{
	::HRESULT hr = E_FAIL;

	hr = schemaRowset->GetRowset(
		pUnkOuter,
		rguidSchema,
		cRestrictions,
		rgRestrictions,
		riid,
		cPropertySets,
		rgPropertySets,
		ppRowset
		);

	if(FAILED(hr))
	{
		std::cerr << GetErrorMessage( 
			std::string( "_GetRowset()" ),
			std::string( "GetRowset() failed." ),
			schemaRowset ) << std::endl;
	}

	return hr;
}

HRESULT _GetNextRows (
					  IRowset		*rowset,
					  HCHAPTER		hChapter,
					  DBROWOFFSET   lRowsOffset,
					  DBROWCOUNT    cRows,
					  DBCOUNTITEM   *pcRowsObtained,
					  HROW			**prghRows)
{
	::HRESULT hr = E_FAIL;

	hr = rowset->GetNextRows(
		hChapter,
		lRowsOffset,
		cRows,
		pcRowsObtained,
		prghRows
		);

	if(FAILED(hr))
	{
		std::cerr << GetErrorMessage( 
			std::string( "_GetNextRows()" ),
			std::string( "_GetNextRows() failed." ),
			rowset ) << std::endl; 
	}

	return hr;
}

} // end namespace Utilities

} // end namespace Test

} // end namespace Simba
