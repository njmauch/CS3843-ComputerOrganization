/**********************************************************************
	cs3843p1Driver.c by Larry Clark
Purpose:
	This program demonstrates the use of the hexDump function.
Command Parameters:
	p1 -i inputFile > outputFile
		The input file contains commands which tells the driver what to
		use when testing the hexDump function.
Input:
	The stream input file contains commands.  There are major commands
	which specify comments, number of bytes per line, and what needs to be
	sent to hexDump:

	* comment
		This is just a comment which helps explain what is being tested.
	BYTESPERLINE number
		This sets the bytes per line used by hexDump
	INVENTORY numberToSend
		This tells the driver to assume we are sending inventory records
		to hexDump.  It is followed by a variable number of IREC records
		which is then terminated by an END record.
	CUSTOMER numberToSend
		This tells the driver to assume we are sending customer records
		to hexDump.  It is followed by a variable number of CREC records
		and AREC records which is then terminated by an END record.
	NUMERIC numberToSend
		This tells the driver to assume we are sending numeric data
		to hexDump.  It is followed by a variable number of NREC records
		which is then terminated by an END record.

	Minor commands:

	IREC subscript stockNr stockQty unitPrice stockNm
		This places an inventory record in the inventory array at the
		specified subscript.
		Format: %d          %6s      %8ld      %10lf      %30[^\n]
			  subscript   stockNr  stockQty  unitPrice  stockNm
	CREC subscript email,fullName
		Specifies the subscript in the customer array followed by
		email address and full name (separated by commas)
	AREC subscript szStreetAddress,szCity,szStateCd,szZipCd
		Specifies the subscript in the customer array followed by
		the address for a customer (separated by commas)
	NREC subscript dataType value
		This places an entry in the numeric array at the specified
		subscript.  The dataType specifies L (long), F (float),
		D (double).  The value is dependent on the dataType.
	END
		marks the end of the the records within a major command
Results:
	All output is written to stdout.
	Prints each of the commands and their command parameters.  For each of
	the major commands when the END is encountered, it invokes hexDump
	to print the contents of the specified buffer.
Returns:
	0       normal
	-1      invalid command line syntax
	-2      show usage only
	99      error during processing, see stderr for more information

Notes:
	p1 -?   will provide the usage information.  In some shells,
			you will have to type p1 -\?
**********************************************************************/
// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include "cs3843p1.h"

/* constants*/
#define MAX_TOKEN_SIZE 50		// largest token size for tokenizer
#define MAX_CUSTOMER 100		// customer array size
#define MAX_INVENTORY 100		// inventory array size
#define MAX_NUMERIC 200			// numeric array size
#define MAX_BUFFER_SZ 100       // size of input buffer

/* files */
FILE *pfileCommand;         // Command input text file

/* prototypes */
void processCommandSwitches(int argc, char *argv[], char **ppszCommandFileName);
void processCommands(FILE *pfileCommand);
int getSimpleToken(char szInput[], int *piBufferPosition, char szToken[]);
void getInventoryRecs(FILE *pfileCommand, Inventory inventoryM[]);
void getCustomerRecs(FILE *pfileCommand, Customer customerM[]);
void getNumericRecs(FILE *pfileCommand, long lNumericM[]);
int getRecCommand(FILE *pfileCommand, char szCommand[], char szInputBuffer[], int *piBufferPosition, int *piSubscript, int iMaxSubscript);

int main(int argc, char *argv[])
{
	char        *pszCommandFileName = NULL;         // Command file name

	// get the filenames and comand type from the command switches
	processCommandSwitches(argc, argv, &pszCommandFileName);

	//Open the command input stream file
	if (pszCommandFileName == NULL)
		errExit(ERR_MISSING_SWITCH, "-i");

	pfileCommand = fopen(pszCommandFileName, "r");
	if (pfileCommand == NULL)
		errExit(ERR_COMMAND_FILENAME, pszCommandFileName);


	// process the commands in the command file
	processCommands(pfileCommand);
	fclose(pfileCommand);
	return 0;
}


/******************** processCommands **************************************
	void processCommands(FILE *pfileCommand)
Purpose:
	Reads the Command file to process commands.  There are several types of
	records (see the program header for more information).
Parameters:
	I FILE *pfileCommand    command stream input
Notes:
	This calls hexdump to dump the contents of the buffer specified in the
	commands in the command file.
**************************************************************************/
void processCommands(FILE *pfileCommand)
{
	// variables for command processing
	char szInputBuffer[MAX_BUFFER_SZ + 1];    // input buffer for a single text line
	char szCommand[MAX_TOKEN_SIZE + 1];     // command
	int bGotToken;                          // TRUE if getSimpleToken got a token
	int iBufferPosition;                    // This is used by getSimpleToken to 
											// track parsing position within input buffer

	// variables used for the buffer passed to hexdump 
	Customer customerM[MAX_CUSTOMER];       // array of Customer records
	long lNumericM[MAX_NUMERIC];            // array of numeric values.
	Inventory inventoryM[MAX_INVENTORY];    // array of Inventory records
	int iBytesPerLine = 50;                 // number of bytes to be displayed per line
											// by hexDump
	int iNumberRecords;                     // number of records to send to hexDump
	int iScanfCnt;                          // number of values returned by sscanf
	char *psbBuffer = NULL;                        // pointer to the buffer sent to hexDump
	int iBytesToSend = 0;                   // number of bytes sent to hexDump
	int iLines = 0;                         // number of lines returned from hexDump

	// Initialize the buffers we send to hexDump
	memset(customerM, '\0', sizeof(customerM));
	memset(inventoryM, '\0', sizeof(inventoryM));
	memset(lNumericM, '\0', sizeof(lNumericM));

	/* get command data until EOF
	** fgets returns null when EOF is reached.
	*/
	while (fgets(szInputBuffer, MAX_BUFFER_SZ, pfileCommand) != NULL)
	{
		// if the line is just a line feed, ignore it
		if (szInputBuffer[0] == '\n')
			continue;
		iBufferPosition = 0;                // reset buffer position

		// get the command
		bGotToken = getSimpleToken(szInputBuffer, &iBufferPosition, szCommand);
		if (!bGotToken)
			errExit(ERR_INVALID_COMMAND, szInputBuffer);

		// see if the command is a comment
		if (strcmp(szCommand, "*") == 0)
		{
			printf("%s", szInputBuffer);
			continue;       // it was just a comment
		}

		printf(">>> %s", szInputBuffer);

		//  The major commands need the number of records
		iScanfCnt = sscanf(&szInputBuffer[iBufferPosition], "%d", &iNumberRecords);
		if (iScanfCnt != 1)
			errExit(ERR_BAD_NR_OF_REC);

		// Note that we intentionally did not check to see if the number of records is past the
		// end of the array.  This lets us get a hexDump beyond our regular data in the array.

		if (strcmp(szCommand, "INVENTORY") == 0)
		{   // INVENTORY command
			psbBuffer = (char *)&inventoryM[0];                // address of buffer to pass
																// to hexdump
			iBytesToSend = iNumberRecords * sizeof(Inventory);
			getInventoryRecs(pfileCommand, inventoryM);
		}
		else if (strcmp(szCommand, "CUSTOMER") == 0)
		{   // CUSTOMER command
			psbBuffer = (char *)&customerM[0];                  // address of buffer to pass
																// to hexdump
			iBytesToSend = iNumberRecords * sizeof(Customer);
			getCustomerRecs(pfileCommand, customerM);
		}
		else if (strcmp(szCommand, "NUMERIC") == 0)
		{   // NUMERIC command
			psbBuffer = (char *)&lNumericM[0];                  // address of buffer to pass
																// to hexdump
			iBytesToSend = iNumberRecords * sizeof(long);
			getNumericRecs(pfileCommand, lNumericM);
		}
		else if (strcmp(szCommand, "BYTESPERLINE") == 0)
		{
			iBytesPerLine = iNumberRecords;  // we read into that variable for coding ease
			continue;
		}
		else
			errExit(ERR_INVALID_COMMAND, szCommand);

		// invoke hexDump
		iLines = hexDump(psbBuffer, iBytesToSend, iBytesPerLine);
		printf(">>> hexDump returned %d lines\n", iLines);
	}
	;
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

/******************** getInventoryRecs **************************************
	void getInventoryRecs(FILE *pfileCommand, Inventory inventoryM[])
Purpose:
	Populates the Inventory array based on IREC commands.
Parameters:
	I FILE *pfileCommand        command stream input
	I/O Inventory inventoryM[]  Inventory array receives entries
								based on the IREC commands
Notes:
	This does not remove existing entries in the array.  If an IREC
	command happens to specify an existing subscript, the item is replaced.

	IREC subscript stockNr stockQty unitPrice stockNm
		This places an inventory record in the inventory array at the
		specified subscript.
		Format: %d          %6s      %8ld      %10lf      %30[^\n]
			  subscript   stockNr  stockQty  unitPrice  stockNm

**************************************************************************/
void getInventoryRecs(FILE *pfileCommand, Inventory inventoryM[])
{
	char szCommand[MAX_TOKEN_SIZE + 1];     // command (IREC) from command file 
	int iBufferPosition;                    // buffer position in the input buffer
	int iSubscript;                         // subscript of where to put value
	int iScanfCnt;                          // count from scanf of successes
	char szInputBuffer[100];                // command file input buffer
	char *pszRemainingInput;                // points to the remaining input after 
											// removing command from the buffer 

	// Read the command file until an END command is found.
	// getRecCommand() returns FALSE when it encounters an END
	while (getRecCommand(pfileCommand
		, szCommand
		, szInputBuffer
		, &iBufferPosition
		, &iSubscript
		, MAX_INVENTORY
	) == TRUE)
	{
		pszRemainingInput = &szInputBuffer[iBufferPosition];

		// If the command wasn't IREC, it is an error
		if (strcmp(szCommand, "IREC") != 0)
			errExit(ERR_INVALID_COMMAND, szInputBuffer);

		// get the Inventory info from the buffer and place it in the array
		iScanfCnt = sscanf(pszRemainingInput, "%6s %8d %10lf %30[^\n]\n"
			, inventoryM[iSubscript].szStockNumber
			, &(inventoryM[iSubscript].iStockQty)
			, &(inventoryM[iSubscript].dUnitPrice)
			, inventoryM[iSubscript].szStockName);
		if (iScanfCnt < 4)
			errExit(ERR_BAD_INVENTORY_DATA, szInputBuffer);
	}
}

/******************** getCustomerRecs **************************************
	void getCustomerRecs(FILE *pfileCommand, Customer customerM[])
Purpose:
	Populates the Customer array based on CREC and AREC commands.
Parameters:
	I FILE *pfileCommand        command stream input
	I/O Customer customerM[]    Customer array receives entries
								based on the CREC and AREC commands
Notes:
	This does not remove existing entries in the array.  If either
	command happens to specify an existing subscript, the item is replaced.

	CREC commands get email and name.  AREC commands get postal addresses.

	CREC subscript email,fullName
		Specifies the subscript in the customer array followed by
		email address and full name (separated by commas)
	AREC subscript szStreetAddress,szCity,szStateCd,szZipCd
		Specifies the subscript in the customer array followed by
		the address for a customer (separated by commas)
**************************************************************************/
void getCustomerRecs(FILE *pfileCommand, Customer customerM[])
{
	char szCommand[MAX_TOKEN_SIZE + 1];     // command (IREC) from command file 
	int iBufferPosition;                    // buffer position in the input buffer
	int iSubscript;                         // subscript of where to put value
	int iScanfCnt;                          // count from scanf of successes
	char szInputBuffer[100];                // command file input buffer
	char *pszRemainingInput;                // points to the remaining input after 
											// removing command from the buffer 

	// Read the command file until an END command is found
	// getRecCommand() returns FALSE when it encounters an END
	while (getRecCommand(pfileCommand
		, szCommand
		, szInputBuffer
		, &iBufferPosition
		, &iSubscript
		, MAX_CUSTOMER) == TRUE)
	{
		pszRemainingInput = &szInputBuffer[iBufferPosition];

		// Check for CREC or AREC
		if (strcmp(szCommand, "CREC") == 0)
		{   // get the Customer Identification Information
			iScanfCnt = sscanf(pszRemainingInput, "%50[^,],%30[^\n]\n"
				, customerM[iSubscript].szEmailAddr
				, customerM[iSubscript].szFullName);
			if (iScanfCnt < 2)
				errExit(ERR_BAD_CUSTOMER_DATA, pszRemainingInput);
		}
		else if (strcmp(szCommand, "AREC") == 0)
		{   // get the address information
			iScanfCnt = sscanf(pszRemainingInput, "%30[^,],%20[^,],%2[^,],%5s"
				, customerM[iSubscript].szStreetAddress
				, customerM[iSubscript].szCity
				, customerM[iSubscript].szStateCd
				, customerM[iSubscript].szZipCd);
			if (iScanfCnt < 4)
				errExit(ERR_BAD_CUSTOMER_DATA, pszRemainingInput);
		}
		else
			errExit(ERR_INVALID_COMMAND, szInputBuffer);
	}
}

/******************** getNumericRecs **************************************
	void getNumericRecs(FILE *pfileCommand, long lNumericM[])
Purpose:
	Populates the numeric array based on NREC commands.
Parameters:
	I FILE *pfileCommand        command stream input
	I/O long lNumericM[]        numeric array receives entries
								based on the NREC commands
Notes:
	This does not remove existing entries in the array.  If an NREC
	command happens to specify an existing subscript, the item is replaced.

	NREC subscript dataType value
		This places an entry in the numeric array at the specified
		subscript.  The dataType specifies L (long), F (float),
		D (double).  The value is dependent on the dataType.
**************************************************************************/
void getNumericRecs(FILE *pfileCommand, long lNumericM[])
{
	char szCommand[MAX_TOKEN_SIZE + 1];     // command from command file
	int iBufferPosition;                    // buffer position in the input buffer
	int iSubscript;                         // subscript of where to put value
	int iScanfCnt;                          // count from scanf of successes
	char szInputBuffer[100];                // command file input buffer
	char *pszRemainingInput;                // points to the remaining input after 
											// removing command.  
	char szDatatype[MAX_TOKEN_SIZE + 1];      // data type for numeric data

	// Read the command file until an END command is found
	// getRecCommand() returns FALSE when it encounters an END
	while (getRecCommand(pfileCommand
		, szCommand
		, szInputBuffer
		, &iBufferPosition
		, &iSubscript
		, MAX_NUMERIC) == TRUE)
	{
		// check for NREC command
		if (strcmp(szCommand, "NREC") != 0)
			errExit(ERR_INVALID_COMMAND, szInputBuffer);

		// get the datatype
		getSimpleToken(szInputBuffer, &iBufferPosition, szDatatype);
		pszRemainingInput = &szInputBuffer[iBufferPosition];

		// the datatype command value determines the datatype for the value.
		switch (szDatatype[0])
		{
		case 'L':    // long value
			iScanfCnt = sscanf(pszRemainingInput, "%ld",
				&lNumericM[iSubscript]);
			break;
		case 'D':    // double value
			iScanfCnt = sscanf(pszRemainingInput, "%lf", (double *)
				&lNumericM[iSubscript]);    // storing a double in this location
											// will actually overwrite 2 items
			break;
		case 'F':    // floating point value
			iScanfCnt = sscanf(pszRemainingInput, "%f", (float *)
				&lNumericM[iSubscript]);    // this stores a float value
			break;
		default:
			errExit(ERR_BAD_NUMERIC_COMMAND, szInputBuffer);
		}
		if (iScanfCnt < 1)
			errExit(ERR_BAD_NUMERIC_COMMAND, szInputBuffer);
	}
}
/******************** getRecCommand **************************************
	int getRecCommand(FILE *pfileCommand, char szCommand[]
		, char szInputBuffer[], int *piBufferPosition
		, int *piSubscript, int iMaxSubscript)
Purpose:
	Gets the particualr REC command (IREC, CREC, AREC, NREC) and the
	subscript.
Parameters:
	I FILE *pfileCommand        command stream input
	O char szCommand[]          returns the command
	I char szInputBuffer[]      input buffer from the command stream
	O int *piBufferPosition     returns the position in the input buffer
								after the delimiter that follows the
								subscript
	O int *piSubscript          returns the subscript
	I int iMaxSubscript         the maximum allowable subscript is used
								to validate the subscript
Notes:
	1. Returns the command and the subscript.
	2. Returns the position after the subscript.
	3. Validates the subscript to make certain is is in range.
	4. If it encounters EOF, it raises an exit error.
Returns Functinally:
	TRUE    it found a record
	FALSE   it found an END command
**************************************************************************/
int getRecCommand(FILE *pfileCommand, char szCommand[]
	, char szInputBuffer[], int *piBufferPosition
	, int *piSubscript, int iMaxSubscript)
{
	int bGotOne;                        // boolean for getSimpleToken func result
	char szSubscript[MAX_TOKEN_SIZE + 1]; // subscript token
	int iScanfCnt;                      // count from scanf of successes

	/* get command data
	** fgets returns null when EOF is reached.
	*/
	if (fgets(szInputBuffer, 100, pfileCommand) != NULL)
	{
		*piBufferPosition = 0;
		// get the command
		bGotOne = getSimpleToken(szInputBuffer, piBufferPosition, szCommand);
		if (!bGotOne)
			errExit(ERR_INVALID_COMMAND, szInputBuffer);
		// if the command is END, return FALSE
		if (strcmp(szCommand, "END") == 0)
			return FALSE;

		printf("\t>>>%s", szInputBuffer);

		// The other commands all require a subscript
		bGotOne = getSimpleToken(szInputBuffer, piBufferPosition, szSubscript);
		if (!bGotOne)
			errExit(ERR_INVALID_COMMAND, szInputBuffer);
		// convert token to integer
		iScanfCnt = sscanf(szSubscript, "%d", piSubscript);
		if (iScanfCnt < 1)
			errExit(ERR_INVALID_COMMAND, szInputBuffer);

		// check for bad subscript
		if ((*piSubscript) >= iMaxSubscript || (*piSubscript) < 0)
			errExit(ERR_BAD_SUBSCRIPT, szInputBuffer);
		return TRUE;
	}
	// when EOF encountered, we must have missed an END
	errExit(ERR_MISSING_END);
	return FALSE;  // this wont be executed
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
	, char **ppszCommandFileName
	);
Purpose:
	Checks the syntax of command line arguments and returns the filenames.
	If any switches are unknown, it exits with an error.
Parameters:
	I   int argc                        count of command line arguments
	I   char *argv[]                    array of command line arguments
	O   char **ppszCommandFileName      command file name
Notes:
	If a -? switch is passed, the usage is printed and the program exits
	with USAGE_ONLY.
	If a syntax error is encountered (e.g., unknown switch), the program
	prints a message to stderr and exits with ERR_COMMAND_LINE_SYNTAX.
**************************************************************************/
void processCommandSwitches(int argc, char *argv[]
	, char **ppszCommandFileName)
{
	int i;
	int rc = 0;
	int bShowCommandHelp = FALSE;

	for (i = 1; i < argc; i++)
	{
		// check for a switch
		if (argv[i][0] != '-')
			exitUsage(i, ERR_EXPECTED_SWITCH, argv[i]);
		// determine which switch it is
		switch (argv[i][1])
		{
		case 'i':                   // Command Input File Name
			if (++i >= argc)
				exitUsage(i, ERR_MISSING_ARGUMENT, argv[i - 1]);
			// check for too long of a file anme
			else
				*ppszCommandFileName = argv[i];
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
	exit(ERROR_PROCESSING);
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
	fprintf(stderr, "p1 -i commandInputFileName\n");
	if (iArg >= 0)
		exit(ERR_COMMAND_LINE_SYNTAX);
	else
		exit(USAGE_ONLY);
}