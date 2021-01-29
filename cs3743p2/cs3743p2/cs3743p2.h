/**********************************************************************
	cs3743p2.h
	Copyright 2020 Larry Clark,  this code cannot be copied to any other website
Purpose:
	Defines constants for
		return codes
		maximum record size
	Defines typedefs for
		HashHeader - the first record of the HashFile will contain information
			   describing the contents.
		HashFile - contains the file pointer and hashHeader for the hash file.
		Book - describes the contents of a book record
	Prototypes
		functions defined in the driver
		functions the students must code
Notes:

**********************************************************************/

#include <stdio.h>
#define TRUE                   1
#define FALSE                  0
// Return code values (some are not used for pgm#1)
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
#define RC_INVALID_PARM        99

#define MAX_REC_SIZE        500

// HashHeader structure for record at RBN 0. 
// Notes:
//     - do NOT write the sizeof this record.  Instead,
//       use the iRecSize.
//     - it contains filler on the end to make certain
//     - it is as big as a hash record.
typedef struct
{
	int  iMaxPrimary;       // Number of records defined for Primary 
							// area.  (This isn't the number of
							// records currently in the Primary Area.)
	int  iRecSize;          // Size in bytes for each record in the
							// hash file
	int  iMaxOvRBN;         // Maximum RBN in overflow.  (This isn't
							// the highest RBN used so far.) (Pgm 2)
	int  iMaxProbes;        // Max number of probes for a collision (Pgm 2)

	char sbFiller[MAX_REC_SIZE];     // This should be set to zeros.  It is 
							// here to make certain the Info record is 
							// at least as big as the data.
} HashHeader;

// HashFile structure containing a FILE pointer and HashHeader
// record for the hash file.
typedef struct
{
	FILE *pFile;            // FILE pointer to the hash file
	HashHeader hashHeader;  // the header record contents for a hash file
} HashFile;

// Book structure represents one book.
typedef struct
{
	char szBookId[12];      // Individual Book Id (not ISBN since
							// multiple copies have the same value)
	char szTitle[40];       // Title
	char szPublisher[12];   // Book publisher
	char szSubject[12];     // Subject (e.g., PgmLang, Database)
	int  iPages;            // Number of pages in the book
} Book;

// Driver functions that the student can use
int printAll(char szFileNm[]);
int hash(char szKey[], int iMaxPrimary);
void printBook(Book *pBook, HashHeader *pHashHeader);
void errExit(const char szFmt[], ...);

// Functions written by the student
int hashCreate(char szFileNm[], HashHeader *pHashHeader);
int hashOpen(char szFileNm[], HashFile *pHashFile);
int readRec(HashFile *pHashFile, int iRBN, void *pRecord);
int writeRec(HashFile *pHashFile, int iRBN, void *pRecord);
int insertBook(HashFile *pHashFile, Book *pBook);
int readBook(HashFile *pHashFile, Book *pBook, int *piRBN);
int updateBook(HashFile *pHashFile, Book *pBook);
int deleteBook(HashFile *pHashFile, Book *pBook);