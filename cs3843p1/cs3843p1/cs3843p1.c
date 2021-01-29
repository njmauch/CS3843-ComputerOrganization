/*****************************************************************************
cs3841p1.c by Nathan Mauch
Purpose:
	This program reads from the file p1Input.txt and converts the information into
	hex, creating a hex dump.
Command Paramaters:
	p1 -i inputFile > outputFile
		The input file contains commands which tells the driver what to
		use when testing the hexDump function.
Input:
	Standard input file with customer records, inventory records, and 100 records.
	Sample data:
		CREC 0 petem@xyz.net,Pete Moss
		AREC 0 123 Boggy Lane,New Orleans,LA,70112
		IREC 0 SBB001 300 14.95 Snuggie Brown
		NREC 1 L 100
Results:
	For the different records it creates a hex dump of the records.
Returns:
	Amount of lines in the hexDump
Notes:
	1. This program is also passed the amount of bytes per line to be printed in
	the dump.
	2. If the character is not printable a "." will be printed.
******************************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1
//define of if statement to determine if characters are printable or not
#define PRINTABLE(c) ((c >= ' ' && c <= '~') ? 1 : 0)

#include "cs3843p1.h"

int hexDump(char *psbBuffer, int iBufferLength, int iBytesPerLine)
{
	//Declaration of variables
	int i, j;
	int iLines = 0;

	//Prints out the number of bytes in dump and starting address
	printf("hexDump of %d bytes beginning at 0x%p\n", iBufferLength, &psbBuffer);

	/*
	For loop to go through buffer and display record in plain text and in hex
	i = iLines * iBytesPerLine is to make sure that the location of where to start
	the new line isn't lost
	*/
	for (i = iLines * iBytesPerLine; i <= iBufferLength; i++)
	{
		//prints out the bit location this line is starting at
		if (i % iBytesPerLine == 0)
		{
			printf("  %04X ", i);
		}
		/*
		checks if the plain text character is printable and if yes will 
		print that character to the screen, if not then prints "."
		*/
		if (PRINTABLE(psbBuffer[i]))
		{
			printf("%c ", psbBuffer[i]);
		}
		else
		{
			printf("%c ", '.');
		}
		//Every 4 bytes prints a " "
		if (((i + 1) % 4) == 0 && i != 0)
		{
			printf(" ");
		}
		//i != 0 is to prevent going into next line for hex values on the first passthrough
		if (i != 0)
		{
			/*
			Whenever the number of bytes per line is reached or reaches the end of 
			the buffer, will create a new line and start printing the hex value
			of the plain text record just printed
			*/
			if (((i + 1) % iBytesPerLine) == 0 || i == iBufferLength)
			{
				printf("\n");
				printf("       ");
				for (j = iLines * iBytesPerLine; j <= iBufferLength; j++)
				{
					/*
					Prints the value of the byte in Hex and uses bitwise &
					with 0x00FF to avoid any chance of propagating a negative sign
					*/
					printf("%02X", psbBuffer[j] & 0x00FF);
					if ((j + 1) % 4 == 0 && j != 0)
					{
						printf(" ");
					}
					/*
					Creates a new line and then goes back to print next line of 
					plain text record
					*/
					if ((j + 1) % iBytesPerLine == 0 && (j + 1) != 0)
					{
						printf("\n");
						//counts the number of lines printed in hex dump
						iLines++;
						break;
					}
				}
			}
		}
	}
	printf("\n");
	return iLines + 1;
}