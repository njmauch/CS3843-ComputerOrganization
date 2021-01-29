/**********************************************************************
	cs3843p2Driver.c by Larry Clark
Purpose:
	This program demonstrates the use of bit level manipulation in C.
	It encrypts and decrypts text using a key.
Command Parameters:
	p2 -l -e inFileNm -d encFileNm -b inFileNm -k key -o outFileNm
		There are several different switches that can be specified:
		p2 -l           This causes the driver to invoke some low level functions
						to help confirm correctness.
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
	The specified input file is read assuming there might not be \n on each line.
	For the encrypted file, the data is binary data and isn't necessarily printable characters.

Results:
	All driver's print output is written to stdout.
	It also may write encrypted/decrypted data to/from the specified files

Returns:
	0       normal
	-1      invalid command line syntax
	-2      usage only
	99      exit error
Notes:
	p2 -?   will provide the usage information.  In some shells,
			you will have to type p2 -\?
**********************************************************************/
// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable : 4996)

// Low level IO on Microsoft Visual studio needs io.h which doesn't exist on Linux.
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "cs3843p2.h"

/* constants*/
#define MAX_TOKEN_SIZE 50       // largest token size for tokenizer
#define MAX_BUFFER_SZ 100       // size of input buffer


/* files */
FILE *pfileCommand;         // Command input text file

/* prototypes for the driver*/
void processCommandSwitches(int argc, char *argv[]
	, char **ppszInfile
	, char **ppszOutFile
	, char **ppszKey
	, char *cFuncFlag
);
void lowLevel();
void driverEncrypt(char *pszInFileNm, char *pszKey, char *pszOutFileNm);
void driverDecrypt(char *pszInFileNm, char *pszKey, char *pszOutFileNm);
int readFromFile(char *pszInFileNm, char *psbInBuff);
void writeToFile(char *psbOutBuf, int iSize, char *pszOutFileNm);
void errRcExit(char* pszBeginMsg, int iRc);
void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo);

int main(int argc, char *argv[])
{
	char        *pszInFileNm = NULL;         // input file name
	char        *pszOutFileNm = NULL;
	char        *pszKey = NULL;              // encrypt/decrypt key
	char        cFunc = 'x';                 // default to no func specified
	char        *pszTempFileNm = "tempDecrypt.dat";
	printf("PROGRAM 2 Output:\n");
	processCommandSwitches(argc, argv, &pszInFileNm, &pszOutFileNm, &pszKey, &cFunc);

	// determine which capability to invoke
	switch (cFunc)
	{
	case 'e':           // encrypt only
		if (pszInFileNm == NULL)
			errExit("Input file not specfied, but required to encrypt");
		if (pszKey == NULL)
			errExit("Encryption key not specfied, but required for encrypting");
		if (pszOutFileNm == NULL)
			errExit("Output file not specfied, but required for result of encryption");
		printf("*** Encryption Only\n");
		driverEncrypt(pszInFileNm, pszKey, pszOutFileNm);
		break;
	case 'd':           // decrypt only
		if (pszInFileNm == NULL)
			errExit("Input file not specfied, but required to decrypt");
		if (pszKey == NULL)
			errExit("Decryption key not specfied, but required for decrypting");
		if (pszOutFileNm == NULL)
			errExit("Output file not specfied, but required to decrypt");
		printf("*** Decryption Only\n");
		driverDecrypt(pszInFileNm, pszKey, pszOutFileNm);
		break;
	case 'b':           // encrypt and decrypt
		if (pszInFileNm == NULL)
			errExit("Input file not specfied, but required to encrypt");
		if (pszKey == NULL)
			errExit("Encryption key not specfied, but required for encrypting");
		if (pszOutFileNm == NULL)
			errExit("Output file not specfied, but required to encrypt");
		printf("*** Both Encryption and Decryption\n");
		driverEncrypt(pszInFileNm, pszKey, pszOutFileNm);
		driverDecrypt(pszOutFileNm, pszKey, pszTempFileNm);
		break;
	case 'l':
		printf("*** Low Level Test\n");
		lowLevel();
		break;
	default:
		errExit("function not specified");
	}
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	exit(0);
}
/******************** lowLevel ********************************************
void lowLevel()
Purpose:
	This exercises some of the students low level manipulation functions.
Parameters:
	n/a
Notes:
	This invokes:
		charSwap   - swaps every k characters in a buffer of characters
		rotateLeft - rotates a four byte unsigned int to the left by
					  kShiftBits
		bitSwap    - swaps bits in a four byte unsigned int, swapping
					 every kSwapBits bits
Return Value:
	n/a
**********************************************************************/
void lowLevel()
{
	char *pszStr = "abcdefghijklmnopqrstuvwzyz Computer Science is the best major in the world.  We have fun! ";
	char* pszSwp = "ebcdajghifolmnktqrspyvwzum CozrutepeSci ice ne thstbes omaj   inrwhe t.rldo  We  aveh un!f";
	int iStrLgth = strlen(pszStr);
	char sbStr[200];
	unsigned int uiOrig = 0xB5DE1FAA;
	unsigned int uiSave = uiOrig;
	unsigned int uiRes;
	strcpy(sbStr, pszStr);
	printf(">>> Testing character swap of every 4th character (0<->4, 5<->9, 10<->14, ...)\n");
	printf("    origStr=\"%s\"\n", sbStr);
	charSwap(sbStr, 4, iStrLgth);
	printf("    resStr=\"%s\"", sbStr);
	if (memcmp(sbStr, pszSwp, iStrLgth) == 0)
		printf(" CORRECT\n");
	else
		printf(" WRONG\n");

	printf(">>> Testing six bit left rotate of bits (not entire buffer)\n");
	printf("    Orig 4 bytes=%08X\n", uiOrig);
	uiRes = rotateLeft(uiOrig, 6);
	printf("    Res 4 bytes=%08X", uiRes);
	if (uiRes == 0x7787EAAD)
		printf(" CORRECT\n");
	else
		printf(" WRONG\n");

	uiOrig = uiSave;
	printf(">>> Testing bit swap of every 4 bits in four bytes (not entire buffer)\n");
	printf("    Orig 4 bytes=%08X\n", uiOrig);
	uiRes = bitSwap(uiOrig, 4);
	printf("    Res 4 bytes=%08X", uiRes);
	if (uiRes == 0x97D6DBAA)
		printf(" CORRECT\n");
	else
		printf(" WRONG\n");
}
/******************** driverEncrypt ********************************************
void driverEncrypt(char *pszInFileNm, char *pszKey, char *pszOutFileNm)
Purpose:
	Reads the specified input file, encrypts the data, and writes it to the
	specified output file.
Parameters:
	I char *pszInFileNm  - input file containing data to be encrypted
	I char *pszKey       - encryption key
	I char *pszOutFileNm - output file which will be written and will contain
						   the encrypted data
Notes:
	1. The input and output files can contain binary data and do not have
	   to have \n terminated text lines.
	2. The encryption key must have less than 64 bytes.
	3. If the student provided encrypt function returns a negative size value,
	   it is assumed to be a return code and we exit.
	4. If the encrypt return value is abnormally large, we exit with an
	   error message.
Return Value:
	n/a
**********************************************************************/
void driverEncrypt(char *pszInFileNm, char *pszKey, char *pszOutFileNm)
{
	char sbBigInBuf[MAX_IN_BUF_SIZE];
	char sbOutBuf[MAX_IN_BUF_SIZE + 100];
	int iRc;

	// read the input and hexDump it
	int iSize = readFromFile(pszInFileNm, sbBigInBuf);
	printf(">>> Hexdump of Original Data, %d bytes\n", iSize);
	iRc = hexDump(sbBigInBuf, iSize, B_PER_LINE);
	if (iRc < 0)
		errRcExit("hexDump returned ", iRc);

	// encrypt it
	int iRcSize = encrypt(sbBigInBuf, iSize, pszKey, sbOutBuf, sizeof(sbOutBuf));

	// check for encrpyt returning an error and for a really bad return value
	if (iRcSize < 0)
		errRcExit("encrypt returned ", iRcSize);
	if (iRcSize > MAX_IN_BUF_SIZE + 100)
		errExit("encrypt returned a very large size of %d", iRcSize);
	printf(">>> Hexdump of Encrypted Data, %d bytes\n", iRcSize);
	iRc = hexDump(sbOutBuf, iRcSize, B_PER_LINE);
	if (iRc < 0)
		errRcExit("hexDump returned ", iRc);

	// write the encrypted data to the output file
	writeToFile(sbOutBuf, iRcSize, pszOutFileNm);
}
/******************** driverDecrypt ********************************************
void driverDecrypt(char *pszInFileNm, char *pszKey, char *pszOutFileNm)
Purpose:
	Reads the specified input file, decrypts the data, and writes it to the
	specified output file.
Parameters:
	I char *pszInFileNm  - input file containing data to be decrypted
	I char *pszKey       - encryption key
	I char *pszOutFileNm - output file which will be written and will contain
						   the decrypted data
Notes:
	1. The input and output files can contain binary data and do not have
	   to have \n terminated text lines.
	2. The encryption key must have less than 64 bytes.
	3. If the student provided decrypt function returns a negative size value,
	   it is assumed to be a return code and we exit.
	4. If the decrypt return value is abnormally large, we exit with an
	   error message.
Return Value:
	n/a
**********************************************************************/
void driverDecrypt(char *pszInFileNm, char *pszKey, char *pszOutFileNm)
{
	char sbBigInBuf[MAX_IN_BUF_SIZE];
	char sbOutBuf[MAX_IN_BUF_SIZE + 100];
	int iRc;

	// read the input and hexDump it
	int iSize = readFromFile(pszInFileNm, sbBigInBuf);
	printf(">>> Hexdump of Encrypted Data, %d bytes\n", iSize);
	iRc = hexDump(sbBigInBuf, iSize, B_PER_LINE);
	if (iRc < 0)
		errRcExit("hexDump returned ", iRc);

	// decrypt the data
	int iRcSize = decrypt(sbBigInBuf, iSize, pszKey, sbOutBuf, sizeof(sbOutBuf));

	// check for decrypt returning an error and for it returning a strange size
	if (iRcSize < 0)
		errRcExit("decrypt returned:", iRcSize);
	if (iRcSize > MAX_IN_BUF_SIZE + 100)
		errExit("encrypt returned a very large size of %d", iRcSize);

	// hexDump it
	printf(">>> Hexdump of Decrypted Data, %d bytes\n", iRcSize);
	iRc = hexDump(sbOutBuf, iRcSize, B_PER_LINE);
	if (iRc < 0)
		errRcExit("hexDump returned ", iRc);

	// write the decrypted data
	writeToFile(sbOutBuf, iRcSize, pszOutFileNm);
}
/******************** readFromFile ********************************************
int readFromFile(char *pszInFileNm, char *psbInBuff)
Purpose:
	Reads the specified input fileinto the spcified buffer.
Parameters:
	I char *pszInFileNm  - input file containing data to be read
	O char *psbInBuf     - buffer to return the data

Notes:
	1. The input file can contain binary data and do not have
	   to have \n terminated text lines.
	2. The buffer is limited to MAX_IN_BUF_SIZE bytes.  If it has more than
	   that, it does an errExit.
	3. This function opens and closes the file.
Return Value:
	Size (in bytes) of the file.
**********************************************************************/
int readFromFile(char *pszInFileNm, char *psbInBuff)
{
	char sbInBuf[INC_IN_SIZE];
	int iSize = 0;
	int iFdRd;
	int iRead;
	iFdRd = open(pszInFileNm, O_RDONLY
		// If on Microsoft, we must tell the low level IO that it is BINARY
#if defined(_WIN32) || defined(_WIN64)
		| O_BINARY
#endif
	);
	if (iFdRd < 0)
		errExit("Open file for read: %s", strerror(errno));

	// read the file until EOF
	while ((iRead = read(iFdRd, sbInBuf, INC_IN_SIZE)) > 0)
	{
		if (iSize + iRead > MAX_IN_BUF_SIZE)
			errExit("Input file is too large (> %d)", MAX_IN_BUF_SIZE);
		memcpy(psbInBuff + iSize, sbInBuf, iRead);
		iSize += iRead;
	}
	close(iFdRd);
	return iSize;
}
/******************** writeToFile ********************************************
void writeToFile(char *psbOutBuf, int iSize, char *pszOutFileNm)
Purpose:
	Writes the specified buffer to the specified output file.
Parameters:
	I char *psbOutBuf    - buffer containing the data to be written.
	I int  iSize         - the size of the buffer.
	I char *pszOutFileNm - the output file to be written
Notes:
	1. The output file can contain binary data.
	2. The buffer is iSize bytes.
	3. This function opens and closes the file.
Return Value:
	n/a
**********************************************************************/
void writeToFile(char *psbOutBuf, int iSize, char *pszOutFileNm)
{
	int iCursor = 0;
	int iFdWr;
	int iWrite;
	// On Linux, if the file already exists and the existing file
	// is bigger than the new one, it won't necessarily reduce the
	// size to the size written.  To avoid this issue, we are simpy
	// removing the old file.
	remove(pszOutFileNm);

	// Open the file for create
	iFdWr = open(pszOutFileNm, O_WRONLY | O_CREAT
		// If on Microsoft, we must tell the low level IO that it is BINARY
#if defined(_WIN32) || defined(_WIN64)
		| O_BINARY, S_IREAD | S_IWRITE
#else
		, S_IRUSR | S_IWUSR
#endif
	);
	if (iFdWr < 0)
		errExit("Open file for write: %s", strerror(errno));

	// write the data
	iWrite = write(iFdWr, psbOutBuf, iSize);
	if (iWrite < 0)
		errExit("could not write encrypted data to %s: %s", pszOutFileNm, strerror(errno));
	close(iFdWr);
}
/*
** This Hex Dump can be used instead of calling the real
** hexDump to make certain that all input is working
** properly.
*/
int dumbHexDump(char *sbBuffer, int iBufferLength, int iBytesPerLine)
{
	printf("HEXDUMP:\n");
	printf("\t buffer length = %d\n", iBufferLength);
	printf("\t bytes per line = %d\n", iBytesPerLine);
	return 1;	// arbitrary value and meaningless in this case
}
/******************** sumKey **************************************
  int sumKey (char szKey[])
Purpose:
	Sums the characters in the encryption key
Parameters:
	I   char szKey[]           encryption key
Returns:
	Returns the absolute value of the sum of the characters in the
	encryption key
**************************************************************************/
int sumKey(char *pszKey)
{
	int iSum = 0;
	int i;
	char *pCh;
	int iL = strlen(pszKey);
	// sum the bytes in the key
	for (i = 0, pCh = pszKey; i < iL; i += 1, pCh++)
	{
		iSum += (int)*pCh;
	}
	return abs(iSum);
}


/******************** getSimpleToken **************************************
 int getSimpleToken(char szInput[], int *piBufferPosition, char szToken[])
 Purpose:
	Returns the next token in a string.  The delimiter for the token is a
	space, a newline, carriage return or the end of the string.  To help
	with a subsequent call, it also returns the next position in the buffer.
Parameters:
	I   char szInput[]          input buffer to be parsed
	I/O int *piBufferPosition   Position to begin looking for the next token.
								This is also used to return the next
								position to look for a token (which will
								follow the delimiter).
	O   char szToken[]          Returned token.
Returns:
	Functionally:
		TRUE - a token is returned
		FALSE - no more tokens
	iBufferPosition parm - the next position for parsing
	szToken parm - the returned token.  If not found, it will be an empty string.
Notes:
	- If the token is larger than the szToken parm, we return a truncated value.
**************************************************************************/

int getSimpleToken(char szInput[], int *piBufferPosition, char szToken[])
{
	int iDelimPos;                      // found position of delim
	int iCopy;                          // number of characters to copy
	char szDelims[20] = " \n\r";        // delimiters

	// check for past the end of the string
	if (*piBufferPosition >= (int)strlen(szInput))
	{
		szToken[0] = '\0';              // mark it empty
		return FALSE;                   // no more tokens
	}

	// get the position of the first delim, relative to the iBufferPosition
	iDelimPos = strcspn(&szInput[*piBufferPosition], szDelims);

	// see if we have more characters than target token, if so, trunc
	if (iDelimPos > MAX_TOKEN_SIZE)
		iCopy = MAX_TOKEN_SIZE;             // truncated size
	else
		iCopy = iDelimPos;

	// copy the token into the target token variable
	memcpy(szToken, &szInput[*piBufferPosition], iCopy);
	szToken[iCopy] = '\0';              // null terminate

	// advance the position
	*piBufferPosition += iDelimPos + 1;
	return TRUE;
}


/******************** processCommandSwitches *****************************
void processCommandSwitches(int argc, char *argv[]
	, char **ppszInfile
	, char **ppszOutFile
	, char **ppszKey
	, char *pcFuncFlag
	);
Purpose:
	Checks the syntax of command line arguments and returns the filenames
	and the type of function requested.
Parameters:
	I   int argc               count of command line arguments
	I   char *argv[]           array of command line arguments
	O   char **ppszInfile      input file name
	O   char **ppszOutFile     output file name
	O   char **ppszKey         encryption key
	O   char *pcFuncFlag       the function.  If valid, it should be one of
							   'e' - encrypt
							   'd' - decrypt
							   'b' - both encrypt and decrypt
							   'l' - low level manipulation function test
							   'x' - function was not set by the switches
Notes:
	1. If a -? switch is passed, the usage is printed and the program exits
	   with USAGE_ONLY.
	2. If a syntax error is encountered (e.g., unknown switch), the program
	   prints a message to stderr and exits with ERR_COMMAND_LINE_SYNTAX.
	3. The possible combinations of switches:
		p2 -l           This causes the driver to invoke some low level functions
						to help confirm correctness.
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
**************************************************************************/
void processCommandSwitches(int argc, char *argv[]
	, char **ppszInFile
	, char **ppszOutFile
	, char **ppszKey
	, char *pcFuncFlag
)
{
	int i;
	int rc = 0;
	int bShowCommandHelp = FALSE;
	*pcFuncFlag = 'x';
	for (i = 1; i < argc; i++)
	{
		// check for a switch
		if (argv[i][0] != '-')
			exitUsage(i, ERR_EXPECTED_SWITCH, argv[i]);
		// determine which switch it is
		switch (argv[i][1])
		{
		case 'e':                   // encrypt the in file
			if (++i >= argc)
				exitUsage(i, ERR_MISSING_ARGUMENT, argv[i - 1]);
			*ppszInFile = argv[i];
			*pcFuncFlag = 'e';
			break;
		case 'b':                   // encrypt the in file and decrypt it
			if (++i >= argc)
				exitUsage(i, ERR_MISSING_ARGUMENT, argv[i - 1]);
			*ppszInFile = argv[i];
			*pcFuncFlag = 'b';
			break;
		case 'd':                   // decrypt the infile
			if (++i >= argc)
				exitUsage(i, ERR_MISSING_ARGUMENT, argv[i - 1]);
			*ppszInFile = argv[i];
			*pcFuncFlag = 'd';
			break;
		case 'o':                   // output file
			if (++i >= argc)
				exitUsage(i, ERR_MISSING_ARGUMENT, argv[i - 1]);
			*ppszOutFile = argv[i];
			break;
		case 'l':                   // debug low level func
			*pcFuncFlag = 'l';
			break;
		case 'k':                   // output file
			if (++i >= argc)
				exitUsage(i, ERR_MISSING_ARGUMENT, argv[i - 1]);
			*ppszKey = argv[i];
			break;
		case '?':
			exitUsage(USAGE_ONLY, "", "");
			break;
		default:
			exitUsage(i, ERR_EXPECTED_SWITCH, argv[i]);
		}
	}
}

/******************** errExit **************************************
	void errExit(char szFmt[], ... )
Purpose:
	Prints an error message defined by the printf-like szFmt and the
	corresponding arguments to that function.  The number of
	arguments after szFmt varies dependent on the format codes in
	szFmt.
	It also exits the program, returning ERR_EXIT.
Parameters:
	I   char szFmt[]            This contains the message to be printed
								and format codes (just like printf) for
								values that we want to print.
	I   ...                     A variable-number of additional arguments
								which correspond to what is needed
								by the format codes in szFmt.
Returns:
	Returns a program exit return code:  the value of ERR_EXIT.
Notes:
	- Prints "ERROR: " followed by the formatted error message specified
	  in szFmt.
	- Prints the file path and file name of the program having the error.
	  This is the file that contains this routine.
	- Requires including <stdarg.h>
**************************************************************************/
void errExit(char szFmt[], ...)
{
	va_list args;               // This is the standard C variable argument list type
	va_start(args, szFmt);      // This tells the compiler where the variable arguments
								// begins.  They begin after szFmt.
	printf("ERROR: ");
	vprintf(szFmt, args);       // vprintf receives a printf format string and  a
								// va_list argument
	va_end(args);               // let the C environment know we are finished with the
								// va_list argument
	printf("\n\tEncountered in file %s\n", __FILE__);  // this 2nd arg is filled in by
								// the pre-compiler
	exit(ERR_EXIT);
}
/******************** errRcExit **************************************
	void errRcExit (char *pszBeginMsg, int iRc)
Purpose:
	Prints an error message for the specific iRC value and eexits.
Parameters:
	I   char *pszBeginMsg  This contains the beginnng
						   message to be printed.
	I   int  iRc           A return code to be converted to a message.
Returns:
	Returns a program exit return code as iRc.
Notes:
	- Prints "ERROR: ", followed by the pszBeginMsg, followed by a
	  message for the particular iRc.
**************************************************************************/
void errRcExit(char* pszBeginMsg, int iRc)
{
	char* pMsg;
	char szMsg[100];
	printf("ERROR: %s ", pszBeginMsg);
	switch (iRc)
	{
	case ERR_ENCRYPT_KEY_SIZE:
		pMsg = "encrypt key is too big";
		break;
	case ERR_INPUT_TOO_LARGE:
		pMsg = "input data is too large";
		break;
	case ERR_KEYS_DO_NOT_MATCH:
		pMsg = "decrypted key doesn't match encrypt key";
		break;
	case ERR_BUFF_LGTH:
		pMsg = "decrypted buffer length is bad";
		break;
	case ERR_INVALID_HEXDUMP_PARM:
		pMsg = "invalid hexDmp parm";
		break;
	default:
		sprintf(szMsg, "unknown error rc = %d", iRc);
		pMsg = szMsg;
	}
	printf("%s\n", pMsg);
	exit(iRc);
}
/******************** exitUsage *****************************
	void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo)
Purpose:
	If this is an argument error (iArg >= 0), it prints a formatted message
	showing which argument was in error, the specified message, and
	supplemental diagnostic information.  It also shows the usage. It exits
	with ERR_COMMAND_LINE_SYNTAX.

	If this is just asking for usage (iArg will be -1), the usage is shown.
	It exits with USAGE_ONLY.
Parameters:
	I int iArg                      command argument subscript
	I char *pszMessage              error message to print
	I char *pszDiagnosticInfo       supplemental diagnostic information
Notes:
	This routine causes the program to exit.
**************************************************************************/
void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo)
{
	if (iArg >= 0)
		fprintf(stderr, "Error: bad argument #%d.  %s %s\n"
			, iArg
			, pszMessage
			, pszDiagnosticInfo);
	fprintf(stderr,
		"p2 -l -e inFileNm -d encFileNm -b inFileNm -k key -o outFileNm \n");
	fprintf(stderr,
		"There are several different switches that can be specified:\n");
	fprintf(stderr,
		"p2 -l           This causes the driver to invoke some low level functions\n");
	fprintf(stderr,
		"                to help confirm correctness.\n");
	fprintf(stderr,
		"p2 -e inFileNm -k key -o outFileNm\n");
	fprintf(stderr,
		"                The driver will read the specified inFileNm and encrypt it \n");
	fprintf(stderr,
		"                using the specified key.  The output is written to the\n");
	fprintf(stderr,
		"                specified outFileNm.\n");
	fprintf(stderr,
		"p2 -d encFileNm -k key -o outFileNm\n");
	fprintf(stderr,
		"                This reads the specified encrypted file, decrypts it and writes\n");
	fprintf(stderr,
		"                the results to the specified outFileNm.\n");
	fprintf(stderr,
		"p2 -b inFileNm -k key -o outFileNm\n");
	fprintf(stderr,
		"                The driver will read the specified inFileNm and encrypt it \n");
	fprintf(stderr,
		"                using the specified key.  The output is written to the\n");
	fprintf(stderr,
		"                specified outFileNm.  It will also read that file, decrypt it, and\n");
	fprintf(stderr,
		"                confirm that it is the same as the input.\n");
	fprintf(stderr,
		"You can only specify one of -l, -e, -d, -b.\n");

	if (iArg >= 0)
		exit(ERR_COMMAND_LINE_SYNTAX);
	else
		exit(USAGE_ONLY);
}