/**********************************************************************
	cs3743p2Driver.c by Larry Clark
	Copyright 2020 Larry Clark, this code may not be copied to any
	other website.
Purpose:
	This program demonstrates a Hash File.
	It uses a stream input command file to test the various
	functions written by the students.  The command file comes from stdin.
Command Parameters:
	p1 < infile > outfile
	The stream input file contains commands.  There are commands
	which specify create, insert, read, delete, update, and printall.

	Commands in the input file:

	* comment
		This is just a comment which helps explain what is being tested.

	CREATE BOOK fileName maxPrimary maxOvr maxProbes
		Create the specified hash file if it doesn't already exist
		using hashCreate.  This only writes the header record at
		record 0.  This closes the created file.
		The size of each record is the size of a Book structure.
	OPEN BOOK fileName
		Opens the specified file using hashOpen.  Place the returned
		FILE pointer in pHashFile->pFile.
	INSERT BOOK bookId,title,publisher,subject,pages
		Uses sscanf to get those attributes and populate a Book structure.
		It invokes insertBook to insert the Book into the hash file.
	READ BOOK bookId
		Invokes readBook to read the Book from the hash file and (if found)
		prints that Book.
	PRINTALL BOOK fileName
		Prints the contents of the specified file.
		- opens the file
		- reads and prints each record
	UPDATE BOOK bookId,title,publisher,subject,pages
		Uses sscanf to get those attributes and populate a Book structure.
		It invokes iupdateBook to update the Book into the hash file.
	DELETE BOOK bookId
		Invokes deleteBook to delete the Book leaving a record of all zeroes.
		The deletion is for extra credit and causes changes in
			readBook
			insertBook
			upDateBook
	NUKE BOOK fileName
		Removes the specified file.
Results:
	All output is written to stdout.
	Prints each of the commands and their command parameters.  Some of the commands
	print information:
		CREATE - prints the record size
		INSERT - prints the hashed RBN (record block number)
		READ   - prints (if found):
				 -- Actual RBN
				 -- the Book information
				 -- original hash RBN
		PRINTALL- prints the file's contents
	If the command caused an error, it prints the error.
Returns:
	0       Normal
	99      Processing Error
Notes:

**********************************************************************/
// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "cs3743p2.h"

#define MAX_TOKEN_SIZE 50		// largest token size for tokenizer
#define MAX_BUFFER_SZ 200       // size of input buffer for command file
#define ERROR_PROCESSING 99

// prototypes only used by the driver
void processCommands(FILE *pfileCommand);
void printRC(int rc);

int main(int argc, char *argv[])
{
	// Process commands for manipulating hash files
	processCommands(stdin);
	printf("\n");
	return 0;
}

/******************** processCommands **************************************
	void processCommands(FILE *pfileCommand)
Purpose:
	Reads the Command file to process commands.  There are several types of
	records (see the program header for more information).
Parameters:
	I FILE *pfileCommand    command stream input file
Notes:
	This calls functions inside:
		cs3723p1
**************************************************************************/
void processCommands(FILE *pfileCommand)
{
	// variables for command processing
	char szInputBuffer[MAX_BUFFER_SZ + 1];    // input buffer for a single text line
	char szCommand[MAX_TOKEN_SIZE + 1];     // command
	char szRest[MAX_BUFFER_SZ + 1];           // rest of input buffer after command
	int iScanfCnt;                          // number of values returned by sscanf
	int rc;                                 // return code from functions
	Book book;                              // Book structure used by functions
	HashFile hashFile;                      // hash file 
	char szHashContent[11];                 // content type for hash file (e.g., Book)
	char szFileNm[31];                      // Hash File Nm
	int iRBN;                               // RBN of a read Book
	int bSkipCRCheck = FALSE;               // When FALSE, it checks input for carriage ret

	// Initialize the hash File structure with null information.
	hashFile.pFile = NULL;
	memset(&hashFile.hashHeader, 0, sizeof(HashHeader));

	//  get command data until EOF
	while (fgets(szInputBuffer, MAX_BUFFER_SZ, pfileCommand) != NULL)
	{
		// if the line is just a line feed, ignore it
		if (szInputBuffer[0] == '\n')
			continue;

		// See if student didn't remove \r 
		if (bSkipCRCheck == FALSE && strstr(szInputBuffer, "\r") != NULL)
			errExit("Your input contains carriage return characters.  Use dos2unix to remove.\n");
		else
			bSkipCRCheck = TRUE;

		// get the command and the rest of the line
		iScanfCnt = sscanf(szInputBuffer, "%30s %[^\n]", szCommand, szRest);
		if (iScanfCnt != 2 && szInputBuffer[0] != '*')
			errExit("Invalid stream input record:\n%s", szInputBuffer);

		// see if the command is a comment
		if (szCommand[0] == '*')
		{
			printf("%s", szInputBuffer);
			continue;       // it was just a comment
		}
		printf(" >> %s %s\n", szCommand, szRest);

		// Process each type of input command
		if (strcmp(szCommand, "CREATE") == 0)
		{ // CREATE BOOK fileName maxPrimary maxOvRBN maxProbes 
			memset(hashFile.hashHeader.sbFiller, 0
				, sizeof(hashFile.hashHeader.sbFiller));
			iScanfCnt = sscanf(szRest
				, "%10s %30s %d %d %d"
				, szHashContent
				, szFileNm
				, &hashFile.hashHeader.iMaxPrimary
				, &hashFile.hashHeader.iMaxOvRBN
				, &hashFile.hashHeader.iMaxProbes
			);
			if (iScanfCnt < 5)
				errExit("Invalid input:\n%s", szInputBuffer);
			hashFile.hashHeader.iRecSize = sizeof(Book);
			printf("    Record size is %d\n", hashFile.hashHeader.iRecSize);
			rc = hashCreate(szFileNm, &hashFile.hashHeader);
			if (rc != 0)
				printRC(rc);
		}
		else if (strcmp(szCommand, "OPEN") == 0)
		{ // OPEN BOOK fileName 
			iScanfCnt = sscanf(szRest
				, "%10s %30s"
				, szHashContent
				, szFileNm);
			if (iScanfCnt < 2)
				errExit("Invalid input:\n%s", szInputBuffer);
			hashFile.hashHeader.iRecSize = sizeof(Book);
			rc = hashOpen(szFileNm, &hashFile);
			if (rc != 0)
				printRC(rc);
		}
		else if (strcmp(szCommand, "INSERT") == 0)
		{ // INSERT BOOK bookId,title,publisher,subject,pages
			iScanfCnt = sscanf(szRest
				, "%10s %11[^,],%39[^,],%11[^,],%11[^,],%d"
				, szHashContent
				, book.szBookId
				, book.szTitle
				, book.szPublisher
				, book.szSubject
				, &book.iPages);
			if (iScanfCnt < 6)
				errExit("Invalid input:\n%s", szInputBuffer);
			printf("            Hash RBN is %d, id='%s'\n", hash(book.szBookId
				, hashFile.hashHeader.iMaxPrimary), book.szBookId);
			rc = insertBook(&hashFile, &book);
			if (rc != 0)
				printRC(rc);
		}
		else if (strcmp(szCommand, "UPDATE") == 0)
		{ // UPDATE BOOK bookId,title,publisher,subject,pages
			iScanfCnt = sscanf(szRest
				, "%10s %11[^,],%39[^,],%11[^,],%11[^,],%d"
				, szHashContent
				, book.szBookId
				, book.szTitle
				, book.szPublisher
				, book.szSubject
				, &book.iPages);
			if (iScanfCnt < 6)
				errExit("Invalid input:\n%s", szInputBuffer);
			printf("            Hash RBN is %d, id='%s'\n", hash(book.szBookId
				, hashFile.hashHeader.iMaxPrimary), book.szBookId);
			rc = updateBook(&hashFile, &book);
			if (rc != 0)
				printRC(rc);
		}
		else if (strcmp(szCommand, "READ") == 0)
		{ // READ BOOK bookId
			memset(&book, 0, sizeof(book));
			iScanfCnt = sscanf(szRest
				, "%10s %11s"
				, szHashContent
				, book.szBookId);
			if (iScanfCnt < 2)
				errExit("Invalid input:\n%s", szInputBuffer);
			printf("            Hash RBN is %d, id='%s'\n", hash(book.szBookId
				, hashFile.hashHeader.iMaxPrimary), book.szBookId);
			rc = readBook(&hashFile, &book, &iRBN);
			if (rc != 0)
				printRC(rc);
			else
			{
				printf("    %2d", iRBN);  //indent like the PRINTALL command
				printBook(&book, &hashFile.hashHeader);
			}
		}
		else if (strcmp(szCommand, "DELETE") == 0)
		{ // DELETE BOOK bookId
			memset(&book, 0, sizeof(book));
			iScanfCnt = sscanf(szRest
				, "%10s %11s"
				, szHashContent
				, book.szBookId);
			if (iScanfCnt < 2)
				errExit("Invalid input:\n%s", szInputBuffer);
			printf("            Hash RBN is %d, id='%s'\n", hash(book.szBookId
				, hashFile.hashHeader.iMaxPrimary), book.szBookId);
			rc = deleteBook(&hashFile, &book);
			if (rc != 0)
				printRC(rc);
		}
		else if (strcmp(szCommand, "PRINTALL") == 0)
		{ // PRINTALL BOOK fileName
			iScanfCnt = sscanf(szRest
				, "%10s %30s"
				, szHashContent
				, szFileNm);
			if (iScanfCnt < 2)
				errExit("Invalid input:\n%s", szInputBuffer);
			// Flush the Hash File so that printAll can see it
			fflush(hashFile.pFile);
			rc = printAll(szFileNm);
			if (rc != 0)
				printRC(rc);
		}
		else if (strcmp(szCommand, "NUKE") == 0)
		{ // NUKE BOOK fileName
			iScanfCnt = sscanf(szRest
				, "%10s %30s"
				, szHashContent
				, szFileNm);
			if (iScanfCnt < 2)
				errExit("Invalid input:\n%s", szInputBuffer);
			rc = remove(szFileNm);   // Ignore the rc
		}
		else if (strcmp(szCommand, "HALT") == 0)
		{ // HALT
			break;
		}
		else
			printf("   *** Unknown command in input: '%s'\n", szCommand);
	}
}
/******************** hash **************************************
  int hash(char szKey[],int iMaxPrimary)
Purpose:
	Hashes a key to return an RBN between 1 and iMaxPrimary
	(inclusive).
Parameters:
	I   char szKey[]           key to be hashed
	I   iMaxPrimary            number of primary records
Returns:
	Returns a hash value into the primary area.  This value is between 1
	and iMaxPrimary
Notes:
	- hash area of the hash table has subscripts from 1 to
	  iMaxPrimary.
	- The hash function is average at spreading the values.
**************************************************************************/
int hash(char szKey[], int iMaxPrimary)
{
	int iHash = 0;
	int i;
	if (iMaxPrimary <= 0)
		errExit("hash function received an invalid iMaxPrimary value: %d\n"
			, iMaxPrimary);
	for (i = 0; i < (int)strlen(szKey); i++)
	{
		iHash += szKey[i];
	}
	// restrict it to the hash area
	iHash = abs(iHash) % iMaxPrimary + 1;
	return iHash;
}

/******************** printAll **************************************
  int printAll(char szFileNm[]))
Purpose:
	Prints the contents of the specified hash file.
Parameters:
	I   char szFileNm[]               hash file name
Returns:
	RC_OK - normal
	RC_FILE_NOT_FOUND - file not found
	RC_HEADER_NOT_FOUND - header record not found
	RC_BAD_REC_SIZE - bad record size
Notes:
	- hash area of the hash table has subscripts from 1 to
	  hashHeader.iMaxPrimary.
	- opens te file using hashOpen, reads and prints the records, and
	  closes the file.
**************************************************************************/
int printAll(char szFileNm[])
{
	int rc;                                 // return code
	HashFile hashFile;                      // hash file structure containing
											// a FILE pointer and header.
	struct Record
	{
		char sbBuffer[MAX_REC_SIZE];
	} record;

	long lRBA;
	int iRecNum;
	int rcFseek;
	// open the hash file
	memset(&hashFile.hashHeader, 0, sizeof(HashHeader));
	hashFile.hashHeader.iRecSize = sizeof(Book);
	rc = hashOpen(szFileNm, &hashFile);
	if (rc != 0)
		return rc;

	// print the Hash Header
	printf("    MaxPrim=%d, RecSize=%d, MaxOv=%d, MaxProbes=%d\n"
		, hashFile.hashHeader.iMaxPrimary
		, hashFile.hashHeader.iRecSize
		, hashFile.hashHeader.iMaxOvRBN
		, hashFile.hashHeader.iMaxProbes);

	// iRecSize returned should be the same as the sizeof(Book)
	if (hashFile.hashHeader.iRecSize < sizeof(Book))
		return RC_BAD_REC_SIZE;

	// locate the first record
	iRecNum = 1;
	lRBA = iRecNum * hashFile.hashHeader.iRecSize;
	rcFseek = fseek(hashFile.pFile, lRBA, SEEK_SET);
	assert(rcFseek == 0);

	// read each record until EOF
	while (fread(&record, hashFile.hashHeader.iRecSize
		, 1L, hashFile.pFile) == 1)
	{
		if (record.sbBuffer[0] != '\0')
		{   // It contains a book
			printf("    %2d", iRecNum);
			printBook((Book *)&record, &hashFile.hashHeader);
		}
		iRecNum += 1;
	}
	fclose(hashFile.pFile);
	return RC_OK;
}
/******************** printBook **************************************
	void printBook(Book *pBook, int iMaxPrimary)
Purpose:
	Prints the Book information for one Book and its original
	hash value.
Parameters:
	I Book *pBook       // Book info
	I int iMaxPrimary            // maximum hash value
**************************************************************************/
void printBook(Book *pBook, HashHeader *pHashHeader)
{
	printf(" %s %-11s %-11s %4d %s Hash=%d\n"
		, pBook->szBookId
		, pBook->szPublisher
		, pBook->szSubject
		, pBook->iPages
		, pBook->szTitle
		, hash(pBook->szBookId, pHashHeader->iMaxPrimary)
	);
}

/******************** printRC **************************************
	void printRC(int rc)
Purpose:
	For a non-zero RC, it prints an appropriate message
Parameters:
	I int rc;           // return code value which is an error if
						// non-zero

**************************************************************************/
void printRC(int rc)
{
	char *pszMsg;           // pointer to an error message
	char szBuf[100];        // buffer for building an error message
	switch (rc)
	{
	case RC_OK:
		return;
	case RC_FILE_EXISTS:
		pszMsg = "file already exists";
		break;
	case RC_REC_EXISTS:
		pszMsg = "record already exists";
		break;
	case RC_REC_NOT_FOUND:
		pszMsg = "record not found";
		break;
	case RC_INVALID_PARM:
		pszMsg = "invalid parameter";
		break;
	case RC_FILE_NOT_FOUND:
		pszMsg = "file not found or invalid header record";
		break;
	case RC_HEADER_NOT_FOUND:
		pszMsg = "header record not found";
		break;
	case RC_BAD_REC_SIZE:
		pszMsg = "invalid record size in header recordd";
		break;
	case RC_LOC_NOT_FOUND:
		pszMsg = "Location not found for read";
		break;
	case RC_LOC_NOT_WRITTEN:
		pszMsg = "Location not written for write";
		break;
	case RC_NOT_IMPLEMENTED:
		pszMsg = "Function not implemented";
		break;
	case RC_TOO_MANY_COLLISIONS:
		pszMsg = "Too Many Collisions to write record";
		break;
	default:
		sprintf(szBuf, "unknown return code: %d", rc);
		pszMsg = szBuf;
	}
	printf("    **** ERROR: %s\n", pszMsg);
}


/******************** errExit **************************************
  void errExit(const char szFmt[], ... )
Purpose:
	Prints an error message defined by the printf-like szFmt and the
	corresponding arguments to that function.  The number of
	arguments after szFmt varies dependent on the format codes in
	szFmt.
	It also exits the program.
Parameters:
	I   const char szFmt[]      This contains the message to be printed
								and format codes (just like printf) for
								values that we want to print.
	I   ...                     A variable-number of additional arguments
								which correspond to what is needed
								by the format codes in szFmt.
Returns:
	Exits the program with a return code of ERROR_PROCESSING (99).
Notes:
	- Prints "ERROR: " followed by the formatted error message specified
	  in szFmt.
	- Requires including <stdarg.h>
**************************************************************************/
void errExit(const char szFmt[], ...)
{
	va_list args;               // This is the standard C variable argument list type
	va_start(args, szFmt);      // This tells the compiler where the variable arguments
								// begins.  They begin after szFmt.
	printf("ERROR: ");
	vprintf(szFmt, args);       // vprintf receives a printf format string and  a
								// va_list argument
	va_end(args);               // let the C environment know we are finished with the
								// va_list argument
	printf("\n");
	exit(ERROR_PROCESSING);
}