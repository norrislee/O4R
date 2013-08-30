/*
 *  O4R/VSO4R/VSO4R/VSO4R.cpp by Norris Lee and George Chow. Copyright (C) 2013
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available at
 *  http://www.r-project.org/Licenses/
 */

#include "VSO4R.h"
#include <iostream>
#include <atlconv.h>
#include "rapidjson\document.h"
#include "rapidjson\writer.h"
#include "rapidjson\stringbuffer.h"

#define MAX_CONNECTIONS 1000

using namespace Simba::Test::Utilities;

static Simba::Test::ODBO::ODBOProviderProxy *openedConnections[MAX_CONNECTIONS];
unsigned connectionNum = 0;

::IUnknown *unknown;

int ADDCALL odboConnect(const char *provider, const char *parameters)
{
	std::string sprovider(provider);
	std::string sparameters(parameters);
	std::wstring wstr1(sprovider.begin(), sprovider.end());
	std::wstring wstr2(sparameters.begin(), sparameters.end());
	connectionNum++;
	Simba::Test::ODBO::ODBOProviderProxy *	m_provider = new Simba::Test::ODBO::ODBOProviderProxy();
	m_provider->Initialize(wstr1, wstr2);
	openedConnections[connectionNum] = m_provider;
	return connectionNum;
}

void ADDCALL odboDisconnect(int connectionNum)
{
	openedConnections[connectionNum]->~ODBOProviderProxy();
	openedConnections[connectionNum] = NULL;
}

void ADDCALL odboDisconnectAll()
{
	for (unsigned i = 1; i <= connectionNum; i++)
	{
		if (openedConnections[i] != NULL)
			odboDisconnect(i);
	}
	connectionNum = 0;
}

bool ADDCALL odboExecute(int connectionNum, const char *query)
{
	unknown = NULL;
	::ICommandText *commandText = NULL;
	std::string squery(query);
	std::wstring wquery(squery.begin(), squery.end());
	_CreateCommand(openedConnections[connectionNum]->GetDbCreateCommand(), ::IID_ICommandText, reinterpret_cast<::IUnknown **>(&commandText));

	if (FAILED( commandText->SetCommandText( ::DBGUID_MDX, wquery.c_str() ) ))
	{
		std::cerr << GetErrorMessage( std::string( "odboExecute()" ),
											       std::string( "Failed to set the MDX querycommand text." ),
											       commandText ) << std::endl;
	}

	if (FAILED( commandText->Execute( NULL, ::IID_IMDDataset, NULL, NULL, reinterpret_cast<::IUnknown **>(&unknown) ) ))
	{
		std::cerr << GetErrorMessage( std::string( "odboExecute()" ),
	    								           std::string( "Failed to execute the MDX query." ),
		    							           commandText ) << std::endl;
	}
	if (unknown)
		return true;
	else
		return false;
}

void deleteCharPtrs(std::vector<char *> &ptrs)
{
	for (unsigned i = 0; i < ptrs.size(); i++)
	{
		delete[] ptrs[i];
	}
}

void getRowset(std::vector<std::vector<char *> > &table, ::IRowset *rowset)
{
	::HRESULT hr = E_FAIL;

	// Get column info for displaying column headers
	::DBORDINAL cColumns = 0;
	::DBCOLUMNINFO *rgInfo = NULL;
	::OLECHAR *pStringsBuffer = NULL;
	_GetColumnInfo(rowset, &cColumns, &rgInfo, &pStringsBuffer, ::IID_IRowset);

	std::vector<char *> aRow;
	// This is the first row; ie. the column headers
	for(DBORDINAL i = 0; i < cColumns; i++)
	{
		USES_CONVERSION;
		aRow.push_back(OLE2A(rgInfo[i].pwszName));
	}
	table.push_back(aRow);
	aRow.clear();

	::DBROWCOUNT cRows = 2000; // Arbitraily set the number of rows to fetch 2000 at a time
	::DBCOUNTITEM cRowsObtained = 0;
	::HROW *rghRows = NULL;
	::HRESULT getNextRowsHr = S_OK;
	do // Get rows
	{
		getNextRowsHr = _GetNextRows(rowset, DB_NULL_HCHAPTER, 0, cRows, &cRowsObtained, &rghRows);

		// Create bindings and accessor in order to call GetData...
		::DBBYTEOFFSET rowlength = 0;
		::DBBINDING *bindings = NULL;
		_CreateBinding(cColumns, rowlength, rgInfo, &bindings);

		::HACCESSOR hAccessor;
		_CreateAccessor(reinterpret_cast<::IUnknown *>(rowset), hAccessor, cColumns, bindings);

		void *data = ::CoTaskMemAlloc(rowlength);
		void *value = NULL;
		
		for (::DBCOUNTITEM i = 0; i < cRowsObtained; i++)
		{
			hr = rowset->GetData(rghRows[i], hAccessor, data);
			if(FAILED(hr))
			{
				std::cerr  << GetErrorMessage( std::string( "getRowset()" ),
					std::string( "GetData() failed." ),
					rowset ) << std::endl;
			}

			// Populate a row vector and push it to the table vector
			for(::DBORDINAL j = 0; j < cColumns; j++)
			{
				// Display the data normally if success
				// Display an empty cell if data is NULL or error
				if((DBSTATUS_S_OK == *(::DBSTATUS*)((::BYTE*) data + bindings[j].obStatus )) ||
					(DBSTATUS_S_TRUNCATED == *(::DBSTATUS*)((::BYTE*) data + bindings[j].obStatus)))
				{
					value = (::BYTE*) data + bindings[j].obValue;
					wchar_t *wvalue = ((static_cast<wchar_t*>(value)));
					char *cvalue = new char[wcslen(wvalue)+1];
					wcstombs_s(NULL, cvalue, wcslen(wvalue)+1, wvalue, wcslen(wvalue)+1);
					aRow.push_back(cvalue);
				}
				else
				{
					aRow.push_back("");
				}
			}
			table.push_back(aRow);
			aRow.clear();
		}
		::CoTaskMemFree(data);
	} while (getNextRowsHr == S_OK); // end get rows;
}

void _getAxesRowsetMemberCaptions(std::vector<std::vector<std::vector<char *> > > &axesMemberCaptions, // [in, out] this is just a sebset of axesinfo that contains member captions only
									std::vector<std::vector<std::vector<char *> > > *axesInfo, // [in]
									::MDAXISINFO *rgAxisInfo, // [in]
									::DBCOUNTITEM cAxes)
{

	::DBCOUNTITEM currentDimension = 0;
	std::vector<std::vector<char *> > axesHeaders;	// This is a table of axis headers
	std::vector<std::vector<char *> > temp_axesHeaders;
	std::vector<char *> temp_axisHeader;	// ie. Member, eg. [Country].[Canada], [Country].[Mexico], etc.
	std::vector<char *> axisColumns;

	char * s;
	unsigned axisColumnsSize = 0;
	for (unsigned iAxis = 0; iAxis < cAxes-1; iAxis++)
	{
		axesHeaders = axesInfo->at(iAxis);
		for (unsigned iRow = 1; iRow < axesHeaders.size(); iRow++)	// first row is axis rowset headers. We don't need it
		{
			axisColumns = axesHeaders.at(iRow);
			axisColumnsSize = axisColumns.size();
			for (unsigned iCol = 2; iCol < axisColumnsSize; iCol += rgAxisInfo[iAxis].rgcColumns[currentDimension])
			{
				s = axisColumns.at(iCol);
				temp_axisHeader.push_back(s);
				if(currentDimension >= rgAxisInfo[iAxis].cDimensions) break;
			}
			temp_axesHeaders.push_back(temp_axisHeader);
			temp_axisHeader.clear();
		}
		axesMemberCaptions.push_back(temp_axesHeaders);
		temp_axesHeaders.clear();
	}
}

char * createJson(std::vector<std::vector<char *> > table, std::vector<std::vector<std::vector<char *> > > memberCaptions)
{
	// Table is organized as a vector of rows containing a vector of columns.
	// MemberCaptions is organized as a vector containing 2 elements, column and row. Within each is a vector of columns or rows respectively. Within each of
	// those contains a vector of member captions (eg. 2 captions if you have a crossjoin).
	std::vector<char *> rowPtrs;
	rapidjson::Document data;
	rapidjson::Document::AllocatorType& allocator = data.GetAllocator();
	
	data.SetObject();
	rapidjson::Value colNames(rapidjson::kArrayType);
	rapidjson::Value rowNames(rapidjson::kArrayType);
	rapidjson::Value rowData(rapidjson::kObjectType);

	for (unsigned i = 0; i < memberCaptions[0].size(); i++)
	{
		std::string name = "";
		for (unsigned j = 0; j < memberCaptions[0].at(j).size(); j++)
		{
			name = name + memberCaptions[0].at(i).at(j) + ", ";
		}
		colNames.PushBack<char*>(strdup(name.substr(0, name.size() - 2).c_str()), allocator);
	}
	for (unsigned i = 0; i < memberCaptions[1].size(); i++)
	{
		std::string name = "";
		for (unsigned j = 0; j < memberCaptions[1].at(j).size(); j++)
		{
			name = name + memberCaptions[1].at(i).at(j) + ", ";
		}
		rowNames.PushBack<char *>(strdup(name.substr(0, name.size() - 2).c_str()), allocator);
	}
	for (unsigned iRow = 0; iRow < table.size(); iRow++)
	{
		rapidjson::Value row(rapidjson::kArrayType);
		for (unsigned iCol = 0; iCol < table[iRow].size(); iCol++)
		{
			row.PushBack<char *>(table[iRow].at(iCol), allocator);
		}
		char *rowNum = new char[10];
		itoa(iRow, rowNum, 10);
		rowPtrs.push_back(rowNum);
		rowData.AddMember(rowNum, row, allocator);
	}

	data.AddMember("colNames", colNames, allocator);
	data.AddMember("rowNames", rowNames, allocator);
	data.AddMember("rowData", rowData, allocator);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	data.Accept(writer);
	char *result = strdup(buffer.GetString());
	deleteCharPtrs(rowPtrs);
	return result;
}

void ADDCALL odboGetDataset(char **results)
{
	std::vector<std::vector<char *> > result;
	std::vector<std::vector<std::vector<char *> > > axesInfo;
	::DBCOUNTITEM cAxes = 0;
	::MDAXISINFO *rgAxisInfo = 0;
	
	::IMDDataset *dataset = static_cast<::IMDDataset*>(unknown);

	if (FAILED(dataset->GetAxisInfo(&cAxes, &rgAxisInfo)))
	{
		std::cerr << GetErrorMessage( std::string( "odboGetDataset()" ),
	    												   std::string( "GetAxisInfo failed." ),
		    											   dataset ) << std::endl;
	}
	if (cAxes > 3)
	{
		std::cerr << "Cellsets with two or more axes are not supported." << std::endl;
		return;
	}

	// Figure out axis headers
	::IRowset *rowset = NULL;
	std::vector<std::vector<std::vector<char *> > > axisInfo;
	std::vector<std::vector<char *> > headers;

	for (DBCOUNTITEM i = 0; i < cAxes-1; i++) // -1 because we dont need slicer axis
	{
		if (FAILED(dataset->GetAxisRowset(NULL, i, ::IID_IRowset, 0, NULL, reinterpret_cast<::IUnknown **>(&rowset))))
		{
			std::cerr << GetErrorMessage( std::string( "odboGetDataset()" ),
	    													   std::string( "GetAxisInfo failed." ),
		    												   dataset ) << std::endl;
		}
		getRowset(headers, rowset); 
		axisInfo.push_back(headers);

		headers.clear();
		rowset->Release();
		rowset = NULL;
	}

	// Get the MEMBER_CAPTIONS (from the results of GetAxisRowset)
	_getAxesRowsetMemberCaptions(axesInfo, &axisInfo, rgAxisInfo, cAxes);

	// Calculate number of cells
	size_t numcells = 1;

	for (DBCOUNTITEM i = 0; i < cAxes-1; i++)
	{
		numcells = numcells * rgAxisInfo[i].cCoordinates;
	}

	// Create accessor
	::DBORDINAL cColumns = 0;
	::DBCOLUMNINFO *rgInfo = NULL;
	::OLECHAR *pStringsBuffer = NULL;
	_GetColumnInfo(dataset, &cColumns, &rgInfo, &pStringsBuffer, ::IID_IMDDataset);

	::DBBYTEOFFSET rowlength = 0;
	::DBBINDING *bindings = NULL;
	_CreateBinding(cColumns, rowlength, rgInfo, &bindings);

	::HACCESSOR hAccessor;
	_CreateAccessor(reinterpret_cast<::IUnknown *>(dataset), hAccessor, cColumns, bindings);

	DBCOUNTITEM numcolumns = rgAxisInfo[0].cCoordinates;

	std::vector<char *> aRow;
	int iRow = 0;
	std::vector<char *> valuePtrs;
	for (DBCOUNTITEM i = 0; i < numcells; )
	{
		for (DBCOUNTITEM j = 0; j < numcolumns; j++)
		{
			void *data = ::CoTaskMemAlloc(rowlength);
			void *value = NULL;
			dataset->GetCellData(hAccessor, i, i, data);
			if((DBSTATUS_S_OK == *(::DBSTATUS*)((::BYTE*) data + bindings[0].obStatus)) ||
				(DBSTATUS_S_TRUNCATED == *(::DBSTATUS*)((::BYTE*) data + bindings[0].obStatus)))
			{
				value = (::BYTE*) data + bindings[0].obValue;
				wchar_t *wvalue = ((static_cast<wchar_t*>(value)));
				char *cvalue = new char[wcslen(wvalue)+1];
				wcstombs_s(NULL, cvalue, wcslen(wvalue)+1, wvalue, wcslen(wvalue)+1);
				valuePtrs.push_back(cvalue);
				aRow.push_back(cvalue);
			}
			else
			{
				aRow.push_back("");
			}
			i++;
			::CoTaskMemFree(data);
			data = NULL;
		}
		result.push_back(aRow);
		aRow.clear();
		iRow++;
	}
	std::vector<std::vector<std::vector<char*> > > something;
	something.push_back(axesInfo[0]);
	*results = createJson(result, something);
	deleteCharPtrs(valuePtrs);
	for (unsigned i = 0; i < 2; i++)
	{
		for (unsigned j = 0; j < axesInfo[i].size(); j++)
		{
			deleteCharPtrs(axesInfo[i].at(j));
		}
	}
}

bool ADDCALL odboValidHandle(int connectionNum)
{
	return openedConnections[connectionNum] != NULL;
}

void ADDCALL odboGetCatalogs(int connectionNum, char **results)
{
	getSchemaRowsetResults *m = new getSchemaRowsetResults(openedConnections[connectionNum]->GetDbSchemaRowset(), ::DBSCHEMA_CATALOGS, 0, NULL);
	std::vector<char *> catalogNamePtrs;

	m->execute();
	DBORDINAL catalogNameColumn = 0;

	for (DBORDINAL i = 0; i < m->cColumns; i++)
	{
		if (m->rgInfo[i].pwszName == NULL) continue;
		
		USES_CONVERSION;
		if (0 == strcmp("COLUMN_NAME", OLE2A(m->rgInfo[i].pwszName)))
		{
			catalogNameColumn = i;
			break;
		}
	}

	void *data = ::CoTaskMemAlloc(m->rowlength);

	void *catalogName;
	::HRESULT hr = E_FAIL;
	std::vector<std::vector<char *> > catalogs;
	std::vector<char *> aRow;
	for (::DBCOUNTITEM i = 0; i < m->cRowsObtained; i++)
	{
		hr = m->rowset->GetData(m->rghRows[i], m->hAccessor, data);
		if(FAILED(hr))
		{
			std::cerr << GetErrorMessage( std::string( "MiddleMan::getCatalogs()" ),
				std::string( "GetData() failed." ),
				m->rowset ) << std::endl;
		}

		catalogName = (::BYTE*) data + m->bindings[catalogNameColumn].obValue;
		wchar_t *wcatalogName = ((static_cast<wchar_t*>(catalogName)));
		char *ccatalogName = new char[wcslen(wcatalogName)+1];
		wcstombs_s(NULL, ccatalogName, wcslen(wcatalogName)+1, wcatalogName, wcslen(wcatalogName)+1);
		catalogNamePtrs.push_back(ccatalogName);
		aRow.push_back(ccatalogName);
		catalogs.push_back(aRow);
	}

	//TODO: create json, check if "catalogs" created properly
	deleteCharPtrs(catalogNamePtrs);
}

void ADDCALL odboGetCubes(int connectionNum, char *catalogName, char **results)
{
	//TODO
}

void ADDCALL odboGetDimensions(int connectionNum, char *cubeName, char **results)
{
	//TODO
}

void ADDCALL odboGetHierarchies(int connectionNum, char *dimensionUniqueName, char **results)
{
	//TODO
}

void ADDCALL odboGetLevels(int connectionNum, char *hierarchyUniqueName, char **results)
{
	//TODO
}

void ADDCALL odboGetMembers(int connectionNum, char *levelUniqueName, char **results)
{
	//TODO
}

void ADDCALL odboGetMembersChildren(int connectionNum, char *parentUniqueName, char **results)
{
	//TODO
}
