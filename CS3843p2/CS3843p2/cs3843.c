/*****************************************************************************
cs3841p2.c by Nathan Mauch
Purpose:
	This program is meant to encrypt the file that is read, either p2One.txt or
	p2Two.txt, by using a char swap, a left rotate, and a bit swap. If given the -d
	command paramater the program will then decrypt the encrypted file. Also prints the
	hex dump of the original input and the output, whether it be encrypted or decrypted.
	If debug mode is on, will print hex dump of inputBuffer after each step of encryption
	or decryption.
Command Paramaters:
	    p2 -l           This causes the driver to invoke some low level functions
                        to help confirm correctness. (That is an el not a one.)
        p2 -e inFileNm -k key -o outFileNm
                        The driver will read the specified inFileNm and encrypt it 
                        using the specified key.  The output is written to the
                        specified outFileNm.
        p2 -d encFileNm -k key -o outFileNm
                        This reads the specified encrypted file, decrypts it and writes
                        the results to the specified outFileNm.
        p2 -b inFileNm -k key -o outFileNm
                        The driver will read the specified inFileNm and encrypt it 
                        using the specified key.  The output is written to the
                        specified outFileNm.  It will also read that file, decrypt it, and
                        confirm that it is the same as the input.
        You can only specify one of -l, -e, -d, -b.

Input:
	Input from file to be encrypted or decrypted and encryption key.
Results:
	Encrypted or decrypted value after running methods.
Returns:
	Amount of bytes in the encrypted or decrypted value.
Notes:
	1. This program is also passed the amount of bytes per line to be printed in
	the dump.
	2. If the character is not printable a "." will be printed.
	3. Encryption key is read from command line when program is ran
	4. If Debug is on will show hex dump of each step of encryption or decryption.
******************************************************************************/
//define of if statement to determine if characters are printable or not
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PRINTABLE(c) ((c >= ' ' && c <= '~') ? 1 : 0)
#define DEBUG_ON
#include "cs3843p2.h"

int hexDump(char *psbBuffer, int iBufferLength, int iBytesPerLine)
{
	//Declaration of variables
	int i, j;
	int iNum;
	//Prints out the number of bytes in dump and starting address
	printf("hexDump of %d bytes\n", iBufferLength);

	for (i = 0; i < iBufferLength; i = i + iBytesPerLine)
	{
		//prints out the bit location this line is starting at
		printf("   %04X ", i);
		iNum = i;

		while (iNum < i + iBytesPerLine)
		{
			//check if the value is printable, if not print "."
			if (PRINTABLE(psbBuffer[iNum]))
			{
				printf("%c ", psbBuffer[iNum]);
			}
			else
			{
				printf("%c ", '.');
			}
			//Every 4 bytes prints a " "
			if ((iNum - 3) % 4 == 0)
			{
				printf(" ");
			}
			iNum++;
			if (iNum == iBufferLength)
			{
				break;
			}
		}

		printf("\n        ");
		j = i;

		while (j < i + iBytesPerLine)
		{
			//prints hex values of character in buffer
			if (PRINTABLE(psbBuffer[j]))
			{
				printf("%02X", psbBuffer[j] & 0x00FF);
			}
			else
			{
				printf("%02X", 0 & 0x00FF);
			}
			if ((j - 3) % 4 == 0)
			{
				printf(" ");
			}
			j++;
			if (j == iBufferLength)
			{
				break;
			}
		}
		printf("\n");
	}
	return iBufferLength / iBytesPerLine + 1;
}
/**************** encrypt *************************************
int encrypt(char *psbInBuffer, int iInBufferLgth, char *pszKey, char *psbOutBuffer, int iOutMaxBufferLgth)
Purpose:
	Is passed in a char buffer, an encryption key, and the length of the buffer.  Encrypts 
	the buffer by doing a char swap, left rotate, and a bit swap.
Paramaters:
	I char *psbInBuffer     Input buffer to be encrypted.  This might not be a zero 
							terminated string.
	I int iInBufferLgth     Number of bytes in psbInBuffer
	I char *pszKey          A zero terminated string which is the encryption key.
	O char *psbOutBuffer    The encrypted result is returned via this parameter 
							which has a maximum of iOutMaxBufferLgth bytes.
	I int iOutMaxBufferLgth The maximum bytes allowed in psbOutBuffer
Returns:
	Amount of bytes that were encrypted
Notes:
	1. If debug is on will print out hex dump after each step of encryption.
***************************************************************/
int encrypt(char *psbInBuffer, int iInBufferLgth, char *pszKey, char *psbOutBuffer, int iOutMaxBufferLgth)
{
	int iOutputBufferLgth;
	int iPadCount;
	char *pcI, *pcL, *pcM;
	int iNumber;

	if (strlen(pszKey) > 64)
	{
		return ERR_ENCRYPT_KEY_SIZE;
	}
	if (strlen(psbInBuffer) > iOutMaxBufferLgth)
	{
		return ERR_INPUT_TOO_LARGE;
	}
	//Set up of the output buffer by adding the encryption key, number of bytes in psbInputBuffer,
	//the psbInBuffer, and then padding at the end of the output buffer which is null bytes.
	strcpy(psbOutBuffer, pszKey);
	pcL = psbOutBuffer + strlen(pszKey);
	*((int *)pcL) = iInBufferLgth;
	pcI = pcL + 4;
	memcpy(pcI, psbInBuffer, iInBufferLgth);
	//getting the length of the output buffer
	//Padding is to make output buffer length a multiple of 4
	iOutputBufferLgth = strlen(pszKey) + 4 + iInBufferLgth;
	iPadCount = 4 - (iOutputBufferLgth % 4);

	pcM = pcI + iInBufferLgth;
	//sets padding to null bytes
	memset(pcM, '\0', iPadCount);

	iOutputBufferLgth += iPadCount;
//If debug is on will print how many bytes were needed for padding.
#ifdef DEBUG_ON
	printf("padded with %d bytes for a total of %d bytes\n", iPadCount, iOutputBufferLgth);
#endif
	char *pCh;
	int i;
	int k;
	int iSum = sumKey(pszKey);
	//Find k value to determine what every amount of chars to swap.
	k = (iSum % (64 - strlen(pszKey))) + 1;
	//calls charswap function
	charSwap(psbOutBuffer, k, iOutputBufferLgth);

//If debug is one will print out hexdump of outbuffer and print how often chars were swapped.
#ifdef DEBUG_ON
	iNumber = hexDump(psbOutBuffer, iOutputBufferLgth, B_PER_LINE);
	printf("swapping every %d bytes, size = %d bytes\n", k, iOutputBufferLgth);
#endif

	//Loops through every 32 its of output buffer
	for (pCh = psbOutBuffer, i = 0; i < iOutputBufferLgth; pCh += 4, i += 4)
	{
		//sets value grabbed from buffer to unsigned int
		unsigned int uiValA = *((unsigned int *)pCh);

		unsigned int uiResA;
		//K value detmerined to figure out how many bits to roate left
		k = iSum % 17 + 1;
		uiResA = rotateLeft(uiValA, k);
		*((unsigned int *)pCh) = uiResA;
	}

//if debug is one prints out hex dump after rotate left is done and how many bits were rotated
#ifdef DEBUG_ON
	iNumber = hexDump(psbOutBuffer, iOutputBufferLgth, B_PER_LINE);
	printf("left rotational shift by %d bits, size = %d\n", k, iOutputBufferLgth);
#endif

	//Loops through every 32 bits to swap bits 
	for (pCh = psbOutBuffer, i = 0; i < iOutputBufferLgth; pCh += 4, i += 4)
	{
		//sets value grabbed from buffer to unsigned int
		unsigned int uiValB = *((unsigned int *)pCh);

		unsigned int uiResB;
		//K value determined to figure out how often to swap bits
		k = iSum % 7 + 1;
		uiResB = bitSwap(uiValB, k);
		*((unsigned int *)pCh) = uiResB;
	}

//if debug is one prints out hex dump of the output buffer after bit swap
//and how often bits were swapped
#ifdef DEBUG_ON
	iNumber = hexDump(psbOutBuffer, iOutputBufferLgth, B_PER_LINE);
	printf("swap every %d bits, size = %d\n", k, iOutputBufferLgth);
#endif

	return iOutputBufferLgth;
}
/**************** decrypt *************************************
int decrypt(char *psbInBuffer, int iInBufferLgth, char *pszKey, char *psbOutBuffer, int iOutMaxBufferLgth)
Purpose:
	Is passed in a char buffer, an encryption key, and the length of the buffer.  Decrypts the
	already encrypted value
Paramaters:
	I char *psbInBuffer     Input buffer to be encrypted.  This might not be a zero
							terminated string.
	I int iInBufferLgth     Number of bytes in psbInBuffer
	I char *pszKey          A zero terminated string which is the encryption key.
	O char *psbOutBuffer    The encrypted result is returned via this parameter
							which has a maximum of iOutMaxBufferLgth bytes.
	I int iOutMaxBufferLgth The maximum bytes allowed in psbOutBuffer
Returns:
	Amount of bytes that were in the original encrypted value
Notes:
	1. If debug is on will print out hex dump after each step of decryption.
***************************************************************/
int decrypt(char *psbInBuffer, int iInBufferLgth, char *pszKey, char *psbOutBuffer, int iOutMaxBufferLgth)
{
	if (strlen(pszKey) > 64)
	{
		return ERR_ENCRYPT_KEY_SIZE;
	}
	if (strlen(psbInBuffer) > iOutMaxBufferLgth)
	{
		return ERR_INPUT_TOO_LARGE;
	}
	memcpy(psbOutBuffer, psbInBuffer, iInBufferLgth);
	int iNumber, i, k;
	int iSum = sumKey(pszKey);

	char *pCh = psbOutBuffer;
	//Loops through every 32 bits swapping every k bits
	for (pCh = psbOutBuffer, i = 0; i < iInBufferLgth; pCh += 4, i += 4)
	{
		unsigned int uiValue = *((unsigned int *)pCh);

		unsigned int uiResult;

		k = iSum % 7 + 1;
		uiResult = bitSwap(uiValue, k);
		*((unsigned int *)pCh) = uiResult;
	}

//if debug is on will show output buffer after bit swap
#ifdef DEBUG_ON
	printf("swap every %d bits, size = %d\n", k, iInBufferLgth);
	iNumber = hexDump(psbOutBuffer, iInBufferLgth, B_PER_LINE);
#endif

	//Loops through every 32 bits rotating right k amount of bits
	for (pCh = psbOutBuffer, i = 0; i < iInBufferLgth; pCh += 4, i += 4)
	{
		unsigned int uiValue = *((unsigned int *)pCh);

		unsigned int uiResult;

		k = iSum % 17 + 1;
		uiResult = rotateRight(uiValue, k);
		*((unsigned int *)pCh) = uiResult;
	}

//if debug is on prints hex dump of buffer after rotating right k bits
#ifdef DEBUG_ON
	iNumber = hexDump(psbOutBuffer, iInBufferLgth, B_PER_LINE);
	printf("left rotational shift by %d bits, size = %d\n", k, iInBufferLgth);
#endif

	//Char swaps entire buffer
	k = (iSum % (64 - strlen(pszKey))) + 1;
	charSwap(psbOutBuffer, k, iInBufferLgth);

//if debug is on will print hex dump of buffer after character swap
#ifdef DEBUG_ON
	iNumber = hexDump(psbOutBuffer, iInBufferLgth, B_PER_LINE);
	printf("swapping every %d bytes, size = %d bytes\n", k, iInBufferLgth);
#endif

	//checks if the keys match after decryption is done
	if (memcmp(pszKey, psbOutBuffer, strlen(pszKey)) == 0)
	{
		printf("Keys match!!!\n");
	}
	else
	{
		return ERR_KEYS_DO_NOT_MATCH;
	}
	//moves the buffer over to make the original value of buffer length next
	memmove(psbOutBuffer - strlen(pszKey), psbOutBuffer, iInBufferLgth);
	//gets the original value of input buffer from the buffer
	char *iInBufferLgthOrig;
	memcpy(iInBufferLgthOrig, psbOutBuffer, 4);
	//leaves only the final decrypted value in the buffer
	memmove(psbOutBuffer - 4, psbOutBuffer, *(int*)iInBufferLgthOrig + 4);
	return *(int *)iInBufferLgthOrig;
}
/*********************** bitSwap *****************************************
unsigned int bitSwap(unsigned int uiOrig, int k)
Purpose:
	Swaps bits within the four byte unsigned int begging from the right side,
	returning the result as a functional value.
Paramaters:
	I	unsigned int uiOrig		Original 4 byte value
	I	int k					Swap bits every k bits
Returns:
	unsigned int of the original value but with the bits swapped.
**************************************************************************/
unsigned int bitSwap(unsigned int uiOrig, int k)
{
	int i, j;
	unsigned int uiBiti, uiBitj;

	//Loops through the 4 bytes usnigned int swapping every k bytes
	for (i = 0, j = k; j < 32; i += k + 1, j += k + 1)
	{
		//Grabs the value at that bit
		uiBiti = (uiOrig & 1 << i);
		uiBitj = (uiOrig & 1 << j);
		//if bit is on then swaps to the other bit it is on.
		if (uiBiti)
		{
			uiOrig |= 1 << j;
		}
		//if bit is off swaps with the other bit to be off
		else
		{
			uiOrig &= ~(1 << j);
		}
		if (uiBitj)
		{
			uiOrig |= 1 << i;
		}
		else
		{
			uiOrig &= ~(1 << i);
		}
	}
	return uiOrig;
}
/*********************** charSwap *****************************************
void charSwap(char *psbBuf, int k, int iBuffLgth)
Purpose:
	Swaps characters in the buffer every k characters beginning from the left side
Paramaters:
	I	char *psbBuf			The original buffer to have the charsSwapped
	I	int k					Swap chars every k characters
	I	int iBuffLgth			Length of the buffer
Returns:
	unsigned int of the original value but with the bits swapped.
**************************************************************************/
void charSwap(char *psbBuf, int k, int iBuffLgth)
{
	char *psbA = psbBuf + k;
	char *psbB = psbBuf;
	char temp;
	//swaps every k characters
	while (psbA < psbBuf + iBuffLgth)
	{
		temp = *psbA;
		*psbA = *psbB;
		*psbB = temp;

		psbB = psbA + 1;
		psbA = psbA + k + 1;
	}
}
/*********************** rotateLeft *****************************************
unsigned int rotateLeft(unsigned int uiOrig, int k)
Purpose:
	Rotates the 4 byte unsigned int value k bits from the left of the value 
	to be at the right of the value.
Paramaters:
	I	unsigned int uiOrig		Original 4 byte value
	I	int k					Rotate k bits
Returns:
	unsigned int of the original value with k bits rotated left
**************************************************************************/
unsigned int rotateLeft(unsigned int uiOrig, int k)
{
	unsigned int uiSave = uiOrig;
	//moves the original value over k bits
	uiOrig = (uiOrig << k);
	//stores the left k bits to the right of the unsigned it
	uiSave = (uiSave >> (32 - k));
	//returns the or of the 2 values which is the final rotated left value
	return uiOrig | uiSave;
}
/*********************** rotateRight *****************************************
unsigned int rotateRight(unsigned int uiOrig, int k)
Purpose:
	Rotates the 4 byte unsigned int value k bits from the right of the value
	to be at the left of the value.
Paramaters:
	I	unsigned int uiOrig		Original 4 byte value
	I	int k					Rotate k bits
Returns:
	unsigned int of the original value with k bits rotated right
**************************************************************************/
unsigned int rotateRight(unsigned int uiOrig, int k)
{
	unsigned int uiSave = uiOrig;
	//moves the original value over k bits
	uiOrig = (uiOrig >> k);
	//stores the left k bits to the left of the unsigned it
	uiSave = (uiSave << (32 - k));
	//returns the or of the 2 values which is the final rotated right value
	return uiOrig | uiSave;
}