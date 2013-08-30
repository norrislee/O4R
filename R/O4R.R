odboConnect <- function(provider, parameters)
{
	connection <- .Call("RODBOConnect", as.character(provider), as.character(parameters))
	if (class(connection) == "integer")
		class(connection) <- "ODBO"
	connection
}

odboClose <- function(handle)
{
	if(!odboValidHandle(handle))
		stop("argument is not an open ODBO handle")
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
	if(!odboValidHandle(handle))
		stop("first argument is not an open ODBO handle")
	result <- .Call("RODBOExecute", handle, as.character(query))
	if(class(result) == "list")
		resultDF <- data.frame(result, check.names=FALSE)
	else result
}

odboDiscover <- function(handle, request)
{
	if(!odboValidHandle(handle))
		stop("first argument is not an open ODBO handle")
	result <- .Call("RODBODiscover", handle, as.character(request))
	if(class(result) == "list")
		resultDF <- data.frame(result, check.names=FALSE)
	else result
}

print.ODBO <- function(x, ...)
{
	conStringList <- strsplit(attr(x, "ConString"), ";")
	conString <- paste0(conStringList[[1]], collapse="\n")
    cat("Connection ", x[1], "\nProvider=", attr(x, "Provider"), "\n", conString, sep = "")
    invisible(x)
}

odboValidHandle <- function(handle)
{
	inherits(handle, "ODBO") && is.integer(handle) && .Call("RODBOValidHandle", handle)
}