///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file JAMSUtils.cpp
///
/// Helper functions used by JAMS
///
/// Copyright: Copyright (C) 2010 Simba Technologies Incorporated
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "JAMSUtils.h"

using namespace JAMS;

std::wstring Utils::_clistring2stdwstring(const System::String^ s)
{
	pin_ptr<const wchar_t> wch = PtrToStringChars(s);
	return std::wstring(wch);
}

bool Utils::findColumnNumber(
							 const System::String ^ columnName,	// in
							 const ::DBORDINAL cColumns,			// in
							 const ::DBCOLUMNINFO * rgInfo,		// in
							 ::DBORDINAL &column)			// out
{
	bool found = false;
	std::wstring wColumnName = _clistring2stdwstring(columnName);

	for(DBORDINAL i = 0; i < cColumns; i++)
	{
		if(rgInfo[i].pwszName == NULL) continue;

		if(0 == _wcsicmp(wColumnName.c_str(), rgInfo[i].pwszName))
		{
			column = i;
			found = true;
			break;
		}
	}
	return found;
}

System::Void Utils::findMultiColumnNumber(
								   cliext::map<String^, DBORDINAL>^		columnNames,	// in
								   const ::DBORDINAL					cColumns,		// in
								   const ::DBCOLUMNINFO *				rgInfo			// in
								   )
{
	bool found = false;
	DBORDINAL column = 0;

	for each (Microsoft::VisualC::StlClr::GenericPair<String^, DBORDINAL> ^ mb in columnNames)
	{
		found = findColumnNumber(mb->first, cColumns, rgInfo, column);
		if(found)
		{
			mb->second = column;
		}
		else
		{
			mb->second = DBORDINAL_MAX;
		}
	}
	return;
}

void Utils::buildCntString(String ^% connectionString, String ^ prop, String ^ text)
{
	if(!System::String::IsNullOrEmpty(text))
	{
		connectionString += prop + "=" + text + ";";
	}
}

String ^ Utils::JAMSProductNameVersion()
{
	Assembly ^ assem = Assembly::GetEntryAssembly();
	AssemblyName ^ assemName = assem->GetName();
	Version ^ ver = assemName->Version;

	String ^ nameversion = assemName->Name + L" " + ver->ToString();
#ifdef WIN64
	nameversion += L" (64-bit)";
#else
	nameversion += L" (32-bit)";
#endif
	return nameversion;
}
