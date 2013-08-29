///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file COMVariant.cpp
///
/// Implementation of the COM helper functions for manipulating COM VARIANT objects.
///
/// Copyright: Copyright (C) 2008-2009 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <oledb.h>

#include "COMError.h"
#include "Unicode.h"


namespace Simba
{

namespace Test
{

namespace Utilities
{

void GetErrorMessages( ::IUnknown* iUnknown, std::vector<std::wstring>& messageVector )
{
	ISupportErrorInfo*	iSupportErrorInfo;

	// Check if the COM object supports ErrorInfo interface.
	if (SUCCEEDED( iUnknown->QueryInterface(
				::IID_ISupportErrorInfo, (void**)& iSupportErrorInfo ) )
	)
	{
		IErrorInfo*		iErrorInfo = NULL;

		// Obtain the error info object.
		if(::GetErrorInfo( 0, &iErrorInfo ) == S_OK && iErrorInfo)
		{
			IErrorRecords*	iErrorRecords = NULL;
			IErrorInfo*		iErrorInfoRec = NULL;
			ULONG			count = 0;
			::ERRORINFO		errorInfo;
			::LCID			lcid = ::GetUserDefaultLCID(); 
			::BSTR			errorDescription = NULL;
			::BSTR			errorSource = NULL;
			wchar_t			errBuffer[20];
			wchar_t			codeBuffer[20];

			iErrorInfo->QueryInterface( IID_IErrorRecords, (void**)& iErrorRecords );
			iErrorRecords->GetRecordCount( &count );

			for (ULONG i = 0; i < count; i++)
			{
				iErrorRecords->GetBasicErrorInfo( i, &errorInfo );
				iErrorRecords->GetErrorInfo(i, lcid, &iErrorInfoRec );

				iErrorInfoRec->GetDescription( &errorDescription );
				iErrorInfoRec->GetSource( &errorSource );

				::_itow_s( errorInfo.hrError, errBuffer, 10 );
				::_itow_s( errorInfo.dwMinor, codeBuffer, 10 );

				messageVector.push_back(
					std::wstring( L"HRESULT: (" ) +
					errBuffer +
					std::wstring( L") Minor Code: (" ) +
					codeBuffer +
					std::wstring( L") Source: (" ) +
					errorSource +
					std::wstring( L") Description: (" ) +
					errorDescription +
					std::wstring( L")")
				);

				::SysFreeString( errorDescription );
				::SysFreeString( errorSource );
				iErrorInfoRec->Release();
			}

			iErrorInfo->Release();
			iErrorRecords->Release();
		}

		iSupportErrorInfo->Release();
	}
}


std::string GetErrorMessage( std::string& function, std::string& defaultMsg, ::IUnknown* iUnknown )
{
	std::vector<std::wstring>	messageVector;
	std::string					retMessage( function + ": " + defaultMsg );

	GetErrorMessages( iUnknown, messageVector );

	if (messageVector.size() > 0)
	{
		retMessage += " - ";

		std::vector<std::wstring>::iterator		itr;
		for (itr = messageVector.begin(); itr != messageVector.end(); itr++ )
		{
			retMessage += std::string( UnicodeToANSI( *itr ) ) + ";";
		}
	}

	return retMessage;
}


} // end namespace Utilities

} // end namespace Test

} // end namespace Simba
