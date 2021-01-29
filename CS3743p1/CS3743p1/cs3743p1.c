/**********************************************************************
	cs3743p1.c by Nathan Mauch
Purpose:
	This program is designed to create a binary file and then will do 
	various reads and writes of books to the file and place the books
	in the file based on the size of the record times the RBN value
	determined by hashing the book id and max primary key together.
Command Parameters:
	cs3743p1Driver.c
	cs3743p1.h
Results:
	Creates a binary file book.dat with binary data written to it read 
	in from P1Input.txt
Returns:
	RC_OK                  0
	RC_FILE_EXISTS         1   // file already exists
	RC_REC_EXISTS          2   // record already exists
	RC_REC_NOT_FOUND       3   // record not found
	RC_FILE_NOT_FOUND      4   // file not found
	RC_HEADER_NOT_FOUND    5   // Header record not found
	RC_BAD_REC_SIZE        6   // invalid record size in info record
	RC_LOC_NOT_FOUND       7   // Location not found for read
	RC_LOC_NOT_WRITTEN     8   // Location not written for write
	RC_SYNONYM             9   // Synonym exists at the specified location
	RC_NOT_IMPLEMENTED     10  // Not implemented
	RC_TOO_MANY_COLLISIONS 11  // probing found too many collisions
Notes:

**********************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1
#include "cs3743p1.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
/***************************** hashCreate *******************************
int hashCreate(char szFileNm[], HashHeader *pHashHeader)
Purpose:
	Checks to see if hash file exist and if it does not exist it will create
	the file book.dat then will write the hash header to the file
Parameters:
	I	char szFileNm				The name of the hash file
	I	HashHeader pHashHeader		Header record to be inserted at beginning of
									file
Notes:
	After writing header to file will close the file.
Returns:
	RC_OK                  0
	RC_FILE_EXISTS         1   // file already exists
*************************************************************************/
int hashCreate(char szFileNm[], HashHeader *pHashHeader)
{
	//opens file with just read to check for existence
	FILE *pHashFile = fopen(szFileNm, "rb");
	int rcWrite, rcFseek;

	//if file doesn't exist enter if statement and create file
	if (pHashFile == NULL)
	{
		//creates file to be read from and modified
		pHashFile = fopen(szFileNm, "wb+");
		if (pHashFile == NULL)
		{
			//Some unforseen error where file was not created
			errExit("Could not create hash file");
		}
		//writes header to the file
		rcWrite = fwrite(pHashHeader, pHashHeader->iRecSize, 1L, pHashFile);
		assert(rcWrite == 1);
		fclose(pHashFile);
		return RC_OK;
	}
	return RC_FILE_EXISTS;
}
/***************************** hashOpen *******************************
int hashOpen(char szFileNm[], HashFile *pHashFile)
Purpose:
	Opens an already existing hash file to be opened and remains open during 
	the remainder of the run time for the program.
Parameters:
	I	char szFileNm				The name of the hash file
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
Notes:
	The hash header is stored in the structure in pHashFile->hashHeader.  The file
	remains open for the duration of the runtime of the program.
Returns:
	RC_OK                  0
	RC_FILE_NOT_FOUND      4   // file not found
	RC_HEADER_NOT_FOUND    5   // Header record not found
*************************************************************************/
int hashOpen(char szFileNm[], HashFile *pHashFile)
{
	int iReadHeader, rcFseek;
	//Opens the has file and stores the file in the structure
	pHashFile->pFile = fopen(szFileNm, "rb+");

	//checking if file was found
	if (pHashFile->pFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}

	//Reads the hash header into the structure
	iReadHeader = fread(&pHashFile->hashHeader, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile);

	//checks if the header was found or not
	if (iReadHeader != 1)
	{
		return RC_HEADER_NOT_FOUND;
	}
	return RC_OK;
}
/***************************** readRec *******************************
int readRec(HashFile *pHashFile, int iRBN, void *pRecord)
Purpose:
	Takes the RBN value to determine where to read the book record from in the 
	hash file. Will then store the record in pRecord
Parameters:
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
	I	int iRBN					Hashed value to determine place in file of record
	O	void pRecord				Void pointer that is used to store record that is found
									at the location searched.
Notes:
	N/A
Returns:
	RC_OK                  0
	RC_LOC_NOT_FOUND       7   // Location not found for read
*************************************************************************/
int readRec(HashFile *pHashFile, int iRBN, void *pRecord)
{
	//calculation to find the place in file to search for record
	long lRBA = iRBN * pHashFile->hashHeader.iRecSize;
	int iReadRecord, rcFseek;

	//seeks to the place in the file for the record
	rcFseek = fseek(pHashFile->pFile, lRBA, SEEK_SET);
	assert(rcFseek == 0);

	//Reads the record at that location and stores it in pRecord
	iReadRecord = fread(pRecord, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile);

	//Checks if read was successful
	if (iReadRecord == 1)
	{
		return RC_OK;
	}
	return RC_LOC_NOT_FOUND;
}
/***************************** writeRec *******************************
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord)
Purpose:
	Takes the RBN value to determine where to write the book record from in the
	hash file.
Parameters:
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
	I	int iRBN					Hashed value to determine place in file of record
	O	void pRecord				Void pointer that is used to store record to be 
									written to the file
Notes:
	N/A
Returns:
	RC_OK                  0
	RC_LOC_NOT_WRITTEN     8   // Location not written for write
*************************************************************************/
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord)
{
	//calculation to determine spot in file to write the record
	long lRBA = pHashFile->hashHeader.iRecSize * iRBN;
	int iWriteRecord, rcFseek;
	
	//Seeks forward to that location to write the record
	rcFseek = fseek(pHashFile->pFile, lRBA, SEEK_SET);
	iWriteRecord = fwrite(pRecord, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile);

	//checks if the write was successful or not
	if (iWriteRecord != 1)
	{
		return RC_LOC_NOT_WRITTEN;
	}
	return RC_OK;
}
/***************************** insertBook *******************************
int insertBook(HashFile *pHashFile, Book *pBook)
Purpose:
	To check if the location based on hash value and size of the record is 
	available to have a record written to that location and if so will call
	on function writeRec to write the record to that location
Parameters:
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
	I	Book pBook					Book record to be stored in hash file if location
									is available.
Notes:
	Calls function readRec to ensure that location is available and not already in use.
	Calls function writeRec to write the book to the file. Calls function hash to 
	calculate the iRBN value.
Returns:
	RC_REC_EXISTS          2   // record already exists
	RC_LOC_NOT_FOUND       7   // Location not found for read
	RC_SYNONYM             9   // Synonym exists at the specified location
*************************************************************************/
int insertBook(HashFile *pHashFile, Book *pBook)
{
	int iRBN, rcFseek, iReadHashFile, rcInsert;
	//temporary book if book is found at location
	Book tempBook;

	//calling hash function to determine iRBN value
	iRBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
	
	//calling readRec to find it record is already stored in location
	//stores what is found in tempBook
	iReadHashFile = readRec(pHashFile, iRBN, &tempBook);

	//if location was skipped over or not found will write record to that location
	if (tempBook.szBookId[0] = "\0" || iReadHashFile == RC_LOC_NOT_FOUND)
	{
		rcInsert = writeRec(pHashFile, iRBN, pBook);
		return rcInsert;
	}
	//if record is in that location will return record exists
	else if (iReadHashFile == RC_OK && strcmp(pBook->szBookId, tempBook.szBookId) == 0)
	{
		return RC_REC_EXISTS;
	}
	//for program 2
	else
	{
		return RC_SYNONYM;
	}
}
/***************************** readBook *******************************
int readBook(HashFile *pHashFile, Book *pBook, int *piRBN)
Purpose:
	Is passed a Book record and attempts to read that record from the file if it exist.
	Also verifies that the record at the location if found matches the book attempting to be 
	read
Parameters:
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
	I/O	Book pBook					Book record to be read from the hash file and if 
									record is found and Book Id's match will store
									that record in Book
	O	int piRBN					Return value of the RBN of the book if it is found and
									matches id

Notes:
	Calls on readRec function to determine if record exist in the file
	at the location of RBN times the size of a record.
Returns:
	RC_OK                  0
	RC_REC_NOT_FOUND       3   // record not found
*************************************************************************/
int readBook(HashFile *pHashFile, Book *pBook, int *piRBN)
{
	int iRBN, rcFseek, iReadHashFile;
	Book tempBook;

	//calls hash function to get RBN value
	iRBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);

	//calls readRec to see if that a record exist at location
	//stores that record in tempBook
	iReadHashFile = readRec(pHashFile, iRBN, &tempBook);

	//compares the id of the book and the record found at location
	if (strcmp(pBook->szBookId, tempBook.szBookId) == 0)
	{
		//if id's match store record found in pBook and the iRBN in piRBN
		//to be returned to driver
		*pBook = tempBook;
		*piRBN = iRBN;
		return RC_OK;
	}
	else
	{
		//if not found or doesn't match id's return record not found.
		return RC_REC_NOT_FOUND;
	}
}
