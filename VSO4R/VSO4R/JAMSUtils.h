///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file JAMSUtils.h
///
/// Helper functions used by JAMS
///
/// Copyright: Copyright (C) 2010 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _JAMS_UTILS_INCLUDE_
#define _JAMS_UTILS_INCLUDE_

#include <oledb.h>
#include <vcclr.h>
#include <string>
#include <xstring>
#include "Unicode.h"
#include "ODBOProviderProxy.h"
#include "ODBOUtils.h"
#include "COMError.h"
#include <cliext/vector>
#include <cliext/map>
#include <cliext/hash_map>
#include <cliext/utility>

using namespace System;
using namespace System::Reflection;
using namespace cliext;

namespace JAMS{
	private ref class Utils
	{
	private:
		map<String^, DBORDINAL>		preventLNK2022DONOTUSE2di8;
		vector<String^>				preventLNK2022DONOTUSEadsf;
		vector<vector<String^>>		preventLNK2022DONOTUSE123d;

	public:
		Utils(){;}
		~Utils(){;}

		static std::wstring _clistring2stdwstring(const System::String^ s);

		// Takes a DBCOLUMNINFO and a column name and outputs the column number (0-based)
		static bool findColumnNumber(
			const System::String ^ columnName,	// in
			const ::DBORDINAL cColumns,			// in
			const ::DBCOLUMNINFO * rgInfo,		// in
			::DBORDINAL &column);			// out

		// The order of elements in the columns vector is the same as the
		// order of the columnNames vector. If a column is not found, its
		// value would be the largest value of DBORDINAL.
		static System::Void findMultiColumnNumber(
			cliext::map<String^, DBORDINAL>^	columnNames,	// in
			const ::DBORDINAL						cColumns,		// in
			const ::DBCOLUMNINFO *					rgInfo			// in
			);

		// Given a propery name and its value, this function will appends
		// them to the end of connectionString with a semi-colon at the end.
		static void Utils::buildCntString(
			String ^% connectionString, // in, out
			String ^ prop,	// in
			String ^ text	// in 
			);

		static String ^ JAMSProductNameVersion();

		// Schema rowset column names
		static const String ^ CATALOG_NAME =			gcnew String("CATALOG_NAME");
		static const String ^ CUBE_NAME =				gcnew String("CUBE_NAME");
		static const String ^ DIMENSION_NAME =			gcnew String("DIMENSION_NAME");
		static const String ^ DIMENSION_UNIQUE_NAME =	gcnew String("DIMENSION_UNIQUE_NAME");
		static const String ^ HIERARCHY_NAME =			gcnew String("HIERARCHY_NAME");
		static const String ^ HIERARCHY_UNIQUE_NAME =	gcnew String("HIERARCHY_UNIQUE_NAME");
		static const String ^ LEVEL_NAME =				gcnew String("LEVEL_NAME");
		static const String ^ LEVEL_UNIQUE_NAME =		gcnew String("LEVEL_UNIQUE_NAME");
		static const String ^ MEMBER_NAME =				gcnew String("MEMBER_NAME");
		static const String ^ MEMBER_UNIQUE_NAME =		gcnew String("MEMBER_UNIQUE_NAME");
		static const String ^ MEMBER_CAPTION =			gcnew String("MEMBER_CAPTION");
		static const String ^ CHILDREN_CARDINALITY =	gcnew String("CHILDREN_CARDINALITY");

	private:

	};
}// namespace JAMS

#endif
