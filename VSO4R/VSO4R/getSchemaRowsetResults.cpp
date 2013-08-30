
#include "getSchemaRowsetResults.h"

getSchemaRowsetResults::getSchemaRowsetResults(){ ; }

getSchemaRowsetResults::getSchemaRowsetResults(
	::IDBSchemaRowset * schemaRowset,
	::IID				riid,
	::ULONG				in_cRestrictions,
	const VARIANT	*	in_rgRestrictions
):
cRestrictions(in_cRestrictions),
rgRestrictions(in_rgRestrictions),
sr(schemaRowset),
iid(riid),
rowset(NULL),
cColumns(NULL),
rgInfo(NULL),
pStringsBuffer(NULL),
cRowsObtained(0),
rghRows(NULL),
rowlength(0),
bindings(0),
hAccessor(0)
{
	;
}

void getSchemaRowsetResults::execute(void)
{

	::HRESULT hr = E_FAIL;

	Simba::Test::Utilities::_GetRowset(
		sr,
		NULL, 
		iid, 
		cRestrictions,		
		rgRestrictions, 
		::IID_IRowset, 
		0,		// not using properties
		NULL, 
		reinterpret_cast<::IUnknown **>(&rowset)
		);

	Simba::Test::Utilities::_GetColumnInfo(
		rowset, 
		&cColumns, 
		&rgInfo, 
		&pStringsBuffer, 
		::IID_IRowset);

	::DBROWCOUNT	cRows = 2000; //Arbitarily sets the number of rows to fetch to 2000 at a time

	hr = S_OK;
	hr = Simba::Test::Utilities::_GetNextRows(
		rowset, 
		DB_NULL_HCHAPTER, 
		0, 
		cRows, 
		&cRowsObtained, 
		&rghRows);

	::DBCOUNTITEM		tempcRowsObtained = 0;
	::HROW*				temprghRows = NULL;
	::DBCOUNTITEM		temp2cRowsObtained = 0;
	::HROW*				temp2rghRows = NULL;

	while(hr == S_OK)
	{
		hr = Simba::Test::Utilities::_GetNextRows(
			rowset, 
			DB_NULL_HCHAPTER, 
			0, 
			cRows, 
			&tempcRowsObtained, 
			&temprghRows);

		// Save rghRows and increment cRowsObtained
		temp2cRowsObtained = cRowsObtained;
		temp2rghRows = rghRows;
		cRowsObtained += tempcRowsObtained;
		rghRows = (HROW*)::CoTaskMemAlloc(sizeof(HROW*) * cRowsObtained); // allocate enough memory for all
		memset(rghRows, 0, sizeof(HROW*) * cRowsObtained);
		memcpy(rghRows, temp2rghRows, sizeof(HROW*) * temp2cRowsObtained); // copy first part
		memcpy(rghRows + temp2cRowsObtained, temprghRows, sizeof(HROW*) * tempcRowsObtained);

		// Clean up
		tempcRowsObtained = 0;
		::CoTaskMemFree(temprghRows);
		temprghRows = NULL;
		temp2cRowsObtained = 0;
		::CoTaskMemFree(temp2rghRows);
		temp2rghRows = NULL;
	}

	// Create bindings and accessor in order to call GetData...
	Simba::Test::Utilities::_CreateBinding(cColumns, rowlength, rgInfo, &bindings);

	Simba::Test::Utilities::_CreateAccessor(
		reinterpret_cast<::IUnknown*>(rowset), 
		hAccessor, 
		cColumns, 
		bindings);

	return;
}
