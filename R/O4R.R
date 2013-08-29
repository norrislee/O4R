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
    cat("Connection ", x[1], "\nURL: ", attr(x, "URL"), "\nSessionID: ", attr(x, "Session"), "\nUsername: ", attr(x, "Username"), "\nPassword: ", attr(x, "Password"), "\n")
    invisible(x)
}

odboValidateHandle <- function(handle)
{

}