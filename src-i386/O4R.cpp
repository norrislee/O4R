#include "rapidjson/document.h"
#include <Rcpp.h>
#include <iostream>
#include <string>
#include "VSO4R.h"

RcppExport SEXP RODBOConnect(SEXP rprovider, SEXP rparameters)
{
	const char *provider = Rcpp::as<const char *>(rprovider);
	const char *parameters = Rcpp::as<const char *>(rparameters);
	Rcpp::IntegerVector handle;
	handle.push_back(odboConnect(provider, parameters));
	return handle;
}

RcppExport SEXP RODBOClose(SEXP rhandle)
{
	Rcpp::IntegerVector handle(rhandle);
	odboDisconnect(handle[0]);
	return R_NilValue;
}

RcppExport SEXP RODBOExecute(SEXP rhandle, SEXP rquery)
{
	Rcpp::IntegerVector handle(rhandle);
	const char *query = Rcpp::as<const char *>(rquery);
	char* results;
	
	if(odboExecute(handle[0], query))
	{
		odboGetDataset(&results);
		rapidjson::Document doc;
		doc.Parse<0>(results);
		std::cout << results << std::endl;
		unsigned numRows = doc["rowNames"].Size();
		unsigned numCols = doc["colNames"].Size();
		Rcpp::CharacterVector colNames;
		Rcpp::CharacterVector rowNames;
		Rcpp::NumericMatrix data(numRows, numCols);
		
		for (unsigned i = 0; i < numCols; i++)
		{
			colNames.push_back(doc["colNames"][rapidjson::SizeType(i)].GetString());
		}
		for (unsigned i = 0; i < numRows; i++)
		{
			rowNames.push_back(doc["rowNames"][rapidjson::SizeType(i)].GetString());
		}
		
		for (unsigned row = 0; row < numRows; row++)
		{
			char *rowNum = new char[10];
			itoa(row, rowNum, 10);
			for (unsigned col = 0; col < numCols; col++)
			{
				std::string svalue = doc["rowData"][rowNum][rapidjson::SizeType(col)].GetString();
				svalue.erase(std::remove(svalue.begin(), svalue.end(), ','), svalue.end());
				data(row, col) =strtod(svalue.c_str(), NULL);
			}
			delete[] rowNum;
		}

		colNames.push_front("Row Names");
		Rcpp::DataFrame resultDataFrame(data);
		resultDataFrame.push_front(rowNames);
		resultDataFrame.attr("names") = colNames;
		return resultDataFrame;
	}
	return R_NilValue;
}