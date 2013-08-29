odboConnect <- function(provider, parameters)
{
	connection <- .Call("RODBOConnect", as.character(provider), as.character(parameters))
	if (class(connection) == "integer")
		class(connection) <- "ODBO"
	connection
}

odboClose <- function(handle)
{
	.Call("RODBOClose", handle)
	invisible()
}

odboCloseAll <- function()
{
	.Call("RODBOCloseAll")
	invisible()
}

odboExecute <- function(handle, query)
{
	result <- .Call("RODBOExecute", handle, as.character(query))
	if(class(result) == "list")
		resultDF <- data.frame(result, check.names=FALSE)
	else result
}

print.ODBO <- function(x, ...)
{
    cat("Connection ", x[1], "\nProvider: ", attr(x, "Provider"), "\nConnection string: ", attr(x, "ConString"), "\n")
    invisible(x)
}

odboValidateHandle <- function(handle)
{
	inherits(handle, "ODBO") && is.integer(handle) && .Call("RXMLAValidHandle", handle)
}