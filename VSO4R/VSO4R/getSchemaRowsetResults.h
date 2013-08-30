#pragma once
#include "ODBOUtils.h"
#include "ODBOProviderProxy.h"
class getSchemaRowsetResults
{
public:
	getSchemaRowsetResults(
		::IDBSchemaRowset * schemaRowset,
		::IID				riid,
		::ULONG				cRestrictions,
		const VARIANT	*	rgRestrictions);
	void execute();
private:
	getSchemaRowsetResults();

public:
	::IDBSchemaRowset * sr;
	::IID				iid;
	::IRowset*			rowset;
	::DBORDINAL			cColumns;
	::DBCOLUMNINFO*		rgInfo;
	::OLECHAR*			pStringsBuffer;
	::DBCOUNTITEM		cRowsObtained;
	::HROW*				rghRows;
	::DBBYTEOFFSET		rowlength;
	::DBBINDING*		bindings;
	::HACCESSOR			hAccessor;
	::ULONG				cRestrictions;
	const ::VARIANT	*	rgRestrictions;
};
