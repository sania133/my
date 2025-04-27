#include "print.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <format> <number>\n", argv[0]);
		return 1;
	}

	char out_buf[256];
	print(out_buf, argv[1], argv[2]);
	puts(out_buf);

	return 0;
}
