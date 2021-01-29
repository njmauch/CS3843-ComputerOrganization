/**********************************************************************
cs3843p2.h
Purpose:
This defines the typedefs, prototypes, and constants used
for a program which demonstrates a hex dump function.
Notes:

**********************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1

#define FALSE           0
#define TRUE            1

/* program return codes */
#define ERR_COMMAND_LINE_SYNTAX     -1      // invalid command line syntax
#define USAGE_ONLY                  -2      // show usage only
#define ERR_EXIT                    -3      // error during processing

/*  Error Messages during command line processing */
#define ERR_MISSING_SWITCH          "missing switch"
#define ERR_EXPECTED_SWITCH         "expected switch, found"
#define ERR_MISSING_ARGUMENT        "missing argument for"

/* function return codes for errors */
#define ERR_INVALID_HEXDUMP_PARM    -5  // invalid hexDmp parm
#define ERR_ENCRYPT_KEY_SIZE        -6  // encrypt key is too big
#define ERR_INPUT_TOO_LARGE         -7  // input data is too large
#define ERR_KEYS_DO_NOT_MATCH       -8  // decrypted key doesn't match encrypt key
#define ERR_BUFF_LGTH               -9  // decrypted buffer length is bad

/* buffer sizes */
#define INC_IN_SIZE 100                 // incremental read size of input file
#define MAX_IN_BUF_SIZE 1000            // maximum input buffer size
#define B_PER_LINE 40                   // Bytes per line

// Driver functions you might use
int sumKey(char *pszKey);
void errExit(char szFmt[], ...);

// Student functions
int hexDump(char *psbBuffer, int iBufferLength, int iBytesPerLine);

int encrypt(char *psbInBuffer, int iInBufferLgth, char *pszKey, char *psbOutBuffer, int iOutMaxBufferLgth);
int decrypt(char *psbInBuffer, int iInBufferLgth, char *pszKey, char *psbOutBuffer, int iOutMaxBufferLgth);

unsigned int bitSwap(unsigned int uiOrig, int k);
void charSwap(char *psbBuf, int k, int iBuffLgth);
unsigned int rotateLeft(unsigned int uiOrig, int k);