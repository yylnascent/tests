
int http_MakeMessage(
	/* [in,out] Buffer with the contents of the message. */
	INOUT membuffer* buf, 
	/* [in] HTTP major version. */
	IN int http_major_version,
	/* [in] HTTP minor version. */
	IN int http_minor_version,
	/* [in] Pattern format. */
	IN const char* fmt,
	/* [in] Format arguments. */
	... );

int http_SendMessage(
	/* [in] Socket information object. */
	SOCKINFO *info,
	/* [in,out] Time out value. */
	int* timeout_secs, 
	/* [in] Pattern format to take actions upon. */
	const char* fmt,
	/* [in] Variable parameter list. */
	...);

