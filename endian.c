#include <stdio.h>
#include <stdint.h>

void main(void)
{
	union Endian {
		uint16_t value;
		char byte[2];
	};

	union Endian tmp;
	
	tmp.value = 0x1234;

	if (tmp.byte[0] == 0x12 && tmp.byte[1] == 0x34) {
		printf("Little Endian\n");
		return;
	}

	if (tmp.byte[1] == 0x12 && tmp.byte[0] == 0x34) {
		printf("Big Endian\n");
		return;
	}
	
	return;
}
