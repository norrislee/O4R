#ifndef _ODBO_UTILS_INCLUDE_
#define _ODBO_UTILS_INCLUDE_

#include <oledb.h>
#define MAX_COLUMN_SIZE 5000			// Maximum number of columns in a rowset.
#ifdef WIN64
	#define DBORDINAL_MAX ULLONG_MAX
#else
	#define DBORDINAL_MAX ULONG_MAX
#endif

namespace Simba
{

namespace Test
{

namespace Utilities
{
	DBLENGTH GetColumnLength( DBTYPE type, DBLENGTH columnSize, bool isFixedLength );
	void _GetColumnInfo(
		IUnknown	*pUnknown,	// [in] pointer to IRowset or IMDDataset
		DBORDINAL	*pcColumns,
		DBCOLUMNINFO **prgInfo,
		OLECHAR      **ppStringsBuffer,
		::IID		RowsetORdataset	// [in] this is either IID_IRowset or IID_IMDDataset
		);
	void _CreateBinding(
		DBORDINAL		cColumns,
		DBBYTEOFFSET	&rowlength,
		DBCOLUMNINFO *	rgColumnInfo,
		DBBINDING**		bindings
		);
	void _CreateAccessor(
		IUnknown	*pDataSet,
		HACCESSOR	&hAccessor, 
		DBORDINAL	cColumns, 
		DBBINDING	*bindings
		);
	void _FreeAxisInfo(IMDDataset	*pDataSet,
		DBCOUNTITEM		cAxes,
		MDAXISINFO		*rgAxisInfo
		);
	void _CreateAccessorPlusPrerequisite(
		::IUnknown * pUnknown, 
		::HACCESSOR &hAccessor, 
		::DBBYTEOFFSET &rowlength,
		::IID RowsetORdataset
		);
	void _CreateInstance(
		std::wstring		provider, // [in]
		::IDBInitialize*	&dbInitialize //[out]
		);
	void _CreateCommand(
		::IDBCreateCommand * dbCreateCommand, //[in]
		const IID iid, //[in]
		::IUnknown ** iUnknown //[out]
		);
	void _freeDbPropSet(
		ULONG cPropertySets, 
		::DBPROPSET * &rgPropertySets
		);
	void _freeDbPropInfoSet(
		ULONG cPropertyInfoSets, 
		::DBPROPINFOSET * &rgPropertyInfoSets
		);

	//IDBSchemaRowset
	HRESULT _GetRowset(
		IDBSchemaRowset *schemaRowset,
		IUnknown       *pUnkOuter,
		REFGUID         rguidSchema,
		ULONG           cRestrictions,
		const VARIANT   rgRestrictions[],
		REFIID          riid,
		ULONG           cPropertySets,
		DBPROPSET       rgPropertySets[],
		IUnknown      **ppRowset);

	//IRowset
	HRESULT _GetNextRows (
		IRowset			*rowset,
		HCHAPTER		hChapter,
		DBROWOFFSET		lRowsOffset,
		DBROWCOUNT		cRows,
		DBCOUNTITEM		*pcRowsObtained,
		HROW			**prghRows);

} // end namespace Utilities

} // end namespace Test

} // end namespace Simba
#endif
