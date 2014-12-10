#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "foo.h"

foo::foo()
{
	//memset(bytes, 0, 8);
}

foo::~foo()
{
}

void foo::print()
{
	for(int i = 0; i < 8; i++) {
		printf("%02x ", bytes[i]);
	}
}
