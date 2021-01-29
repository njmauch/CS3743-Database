/**********************************************************************
	cs3743p2.c by Nathan Mauch
Purpose:
	This program is designed to create a binary file and then will do
	various reads and writes of books to the file and place the books
	in the file based on the size of the record times the RBN value
	determined by hashing the book id and max primary key together.
	The program will also probe to adjacent spots up to max number 
	of probes to try and insert a record if it is a synonym of an 
	already existing record.  Program will also delete records and be
	able to insert new records into that deleted spot.
Command Parameters:
	cs3743p2Driver.c
	cs3743p2.h
Results:
	Creates a binary file book.dat with binary data written to it read
	in from P1Input.txt
Returns:
	#define RC_OK                  0
	#define RC_FILE_EXISTS         1   // file already exists
	#define RC_REC_EXISTS          2   // record already exists
	#define RC_REC_NOT_FOUND       3   // record not found
	#define RC_FILE_NOT_FOUND      4   // file not found
	#define RC_HEADER_NOT_FOUND    5   // Header record not found
	#define RC_BAD_REC_SIZE        6   // invalid record size in info record
	#define RC_LOC_NOT_FOUND       7   // Location not found for read
	#define RC_LOC_NOT_WRITTEN     8   // Location not written for write
	#define RC_NOT_IMPLEMENTED     10  // Not implemented
	#define RC_TOO_MANY_COLLISIONS 11  // probing found too many collisions
Notes:

**********************************************************************/
#define _CRT_SECURE_NO_WARNINGS 1
#include "cs3743p2.h"
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
	on function writeRec to write the record to that location.  Will probe next 
	spots up to max probes to try and insert book into file if there 
	are collisions, if it reaches end of file or max probes will break out because 
	of too many collisions
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
	RC_TOO_MANY_COLLISIONS 11  // probing found too many collisions
*************************************************************************/
int insertBook(HashFile *pHashFile, Book *pBook)
{
	int iRBN, rcFseek, iReadHashFile, rcInsert;
	int i, iTempRBN;
	//temporary book if book is found at location
	Book tempBook;
	
	//calling hash function to determine iRBN value
	iRBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
	iTempRBN = iRBN;
	//calling readRec to find it record is already stored in location
	//stores what is found in tempBook
	iReadHashFile = readRec(pHashFile, iRBN, &tempBook);

	//Loops through file up to max number of probes to find a spot in insert book
	for (i = 0; i < pHashFile->hashHeader.iMaxProbes; i++, iRBN++)
	{
		iReadHashFile = readRec(pHashFile, iRBN, &tempBook);
		//If max rbn is reached break out of loop
		if (iRBN >= pHashFile->hashHeader.iMaxOvRBN)
		{
			return RC_TOO_MANY_COLLISIONS;
		}
		//if it finds an empty string or file has not been written to yet try to 
		//insert the book
		else if (tempBook.szBookId[0] == '\0' || iReadHashFile == RC_LOC_NOT_FOUND)
		{
			//for extra credit, ensure that even if empty spot is found, check probe spots
			//for the book since it may still exist.
			if (tempBook.szBookId[0] == '\0')
			{
				//extra credit it is checking if book exist in a spot after a deleted book
				for (i = 1; i < pHashFile->hashHeader.iMaxProbes; i++, iTempRBN++)
				{
					//if book does exist return record already exist.
					iReadHashFile = readRec(pHashFile, iTempRBN, &tempBook);
					if (iReadHashFile == RC_OK && strcmp(pBook->szBookId, tempBook.szBookId) == 0)
					{
						return RC_REC_EXISTS;
					}
				}
			}
			//insert the book if able to find empty spot and not found in file
			rcInsert = writeRec(pHashFile, iRBN, pBook);
			return rcInsert;
		}
		//check if record exist in its first hash position
		else if (iReadHashFile == RC_OK && strcmp(pBook->szBookId, tempBook.szBookId) == 0)
		{
			return RC_REC_EXISTS;
		}
	}
	//return this if reaches too many probes with out finding empty slot to insert book
	return RC_TOO_MANY_COLLISIONS;
}
/***************************** readBook *******************************
int readBook(HashFile *pHashFile, Book *pBook, int *piRBN)
Purpose:
	Is passed a Book record and attempts to read that record from the file if it exist.
	Also verifies that the record at the location if found matches the book attempting to be
	read.  Will read up to maximum number of probes to check if book does exist.
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
	int i;
	Book tempBook;

	//calls hash function to get RBN value
	iRBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
	//probes through all possible spots book could have been entered
	for (i = 0; i < pHashFile->hashHeader.iMaxProbes; i++, iRBN++)
	{
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

	}
	return RC_REC_NOT_FOUND;
}
/***************************** updateBook *******************************
int updateBook(HashFile *pHashFile, Book *pBook)
Purpose:
	Will update book record if the book is found and matches the book id
Parameters:
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
	I	Book pBook					Book record to be stored in hash file if location
									is available.
Notes:
	Uses writeRec function to update record and insert updated information 
	into proper spot in the file.
Returns:
	RC_REC_NOT_FOUND       3   // record not found
*************************************************************************/
int updateBook(HashFile *pHashFile, Book *pBook)
{
	int iRBN, rcFseek, iReadHashFile, rcUpdate;
	int i;
	//temporary book if book is found at location
	Book tempBook;

	//calling hash function to determine iRBN value
	iRBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);

	//calling readRec to find it record is already stored in location
	//stores what is found in tempBook
	iReadHashFile = readRec(pHashFile, iRBN, &tempBook);
	//checks all possible locations book could be located up to max probes
	for (i = 0; i < pHashFile->hashHeader.iMaxProbes; i++, iRBN++)
	{
		iReadHashFile = readRec(pHashFile, iRBN, &tempBook);
		//compares the id of the book and the record found at location
		if (strcmp(pBook->szBookId, tempBook.szBookId) == 0)
		{
			//inserts the updated book information into file if found
			rcUpdate = writeRec(pHashFile, iRBN, pBook);
			return rcUpdate;
		}
	}
	return RC_REC_NOT_FOUND;
}
/***************************** deleteBook *******************************
int deleteBook(HashFile *pHashFile, Book *pBook)
Purpose:
	Deletes a book from the file and will set the entire record to '\0'
	to mark it is deleted
Parameters:
	I	HashFile pHashFile			Pointer to structer of the hash file and will
									store the pFile in structure.
	I	Book pBook					Book record to be stored in hash file if location
									is available.
Notes:
	Calls function writeRec to insert book with all bytes set to '\0' to 
	delete the book that is called to be deleted.
Returns:
	RC_REC_NOT_FOUND       3   // record not found
*************************************************************************/
int deleteBook(HashFile *pHashFile, Book *pBook)
{
	int iRBN, rcFseek, iReadHashFile, rcDelete;
	int i;
	//temporary book if book is found at location
	Book tempBook;
	//Book to be inserted as a deleted record as entire structure is filled with '\0'
	Book deleteBook;
	memset(&deleteBook, '\0', sizeof(pHashFile->hashHeader.iRecSize));


	//calling hash function to determine iRBN value
	iRBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);

	//calling readRec to find it record is already stored in location
	//stores what is found in tempBook
	iReadHashFile = readRec(pHashFile, iRBN, &tempBook);
	//Checks all possible positions record could be located up to max probes
	for (i = 0; i < pHashFile->hashHeader.iMaxProbes; i++, iRBN++)
	{
		iReadHashFile = readRec(pHashFile, iRBN, &tempBook);
		//compares the id of the book and the record found at location
		if (strcmp(pBook->szBookId, tempBook.szBookId) == 0)
		{
			//Inserts book full of '\0' to mark the desired book as deleted.
			rcDelete = writeRec(pHashFile, iRBN, &deleteBook);
			return rcDelete;
		}

	}
	return RC_REC_NOT_FOUND;
}