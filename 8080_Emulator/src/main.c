#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "8080Disassembler.h"

int main(int argc, char** argv)
{
	FILE *file = fopen(argv[1], "rb");

	if (file == NULL)
	{
		printf("Error: Could not open file - %s\n", argv[1]);
		exit(1);
	}

	fseek(file, 0L, SEEK_END);
	int fileSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	unsigned char *buffer = malloc(fileSize);

	fread(buffer, fileSize, 1, file);
	fclose(file);

	int programCounter = 0;

	while (programCounter < fileSize)
	{
		programCounter += Disassemble8080Opcode(buffer, programCounter);
	}

	getchar();

	return 0;
}