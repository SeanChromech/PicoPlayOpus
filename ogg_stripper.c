#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ogg_stripper.h"

static oggPageHeader_t currentPageHeader;
static oggIDHeader_t currentIDHeader;
static oggCommentHeader_t currentCommentHeader;

#ifdef OGG_STRIP_FILE
    static FILE * oggFile = NULL;
#elif defined(OGG_STRIP_MEMORY)
    static const char * oggFile = NULL;
    static size_t oggFilePointer = 0;
    static size_t oggFileLength = 0;
#endif


// Generic function to read bytes from the source.
// Assumes the source is already set and opened.
// Returns the number of bytes read, or an error code.
static inline int ReadBytes (void * destination, size_t length) {
#ifdef OGG_STRIP_FILE
    if (oggFile == NULL)
        return OGG_STRIP_NULL_SOURCE;
    else
        return (int)fread(destination, 1, length, oggFile);
#elif defined(OGG_STRIP_MEMORY)
    if (oggFile == NULL) {
        return OGG_STRIP_NULL_SOURCE;
    } else {
        if (oggFilePointer + length > oggFileLength)
            length = oggFileLength - oggFilePointer;
        if (length)
            memcpy(destination, oggFile + oggFilePointer, length);
        else
            return OGG_STRIP_EOF;
        oggFilePointer += length;
        return (int)length;
    }
#endif
}


// Seek the source by a number of bytes.
static inline void SeekBytes (long length) {
#ifdef OGG_STRIP_FILE
    if (oggFile != NULL)
        fseek(oggFile, length, SEEK_CUR);
#elif defined(OGG_STRIP_MEMORY)
    if (oggFile != NULL)
        oggFilePointer += length;
#endif
}


// Rewind the source to the beginning.
static inline void Rewind (void) {
#ifdef OGG_STRIP_FILE
    if (oggFile != NULL)
        fseek(oggFile, 0, SEEK_SET);
#elif defined(OGG_STRIP_MEMORY)
    if (oggFile != NULL)
        oggFilePointer = 0;
#endif
}


// Set the source to read from.
// The source is assumed to be open and ready to read.
void OggSetSource (const void * source, size_t length) {
#ifdef OGG_STRIP_FILE
    oggFile = (FILE *)source;
#elif defined(OGG_STRIP_MEMORY)
    oggFile = (const char *)source;
    oggFilePointer = 0;
    oggFileLength = length;
#endif
}


// Parse the page header into a struct.
// Expect to be at the beginning of the page.
// Return the length of the data in the page.
// Seek to the beginning of the data when finished.
int OggReadPageHeader (oggPageHeader_t * header) {
    size_t i;
    if ( ReadBytes( (char *)header, 27 ) == 27 ) {
        if (header->Signature == OGGS_MAGIC) {
            if (header->Segments) {
                // Read in the segment table.
                printf("Segments: %d\n", header->Segments);
                ReadBytes( (char *)header->SegmentTable, header->Segments );
                header->DataLength = 0;
                for (i = 0; i < header->Segments; i++)
                    header->DataLength += header->SegmentTable[i];

                return (int)header->DataLength;
            } else {
                printf("No segments.\n");
                return OGG_STRIP_NO_SEGS;
            }
        } else {
            printf("Bad magic: %X\n", header->Signature);
            return OGG_STRIP_BAD_MAGIC;
        }
    } else {
        printf("EOF.\n");
        return OGG_STRIP_EOF;
    }
}


// Grab the next page's content into destination.
// This will pull the ENTIRE page, which is probably not as useful as the packet implementation below.
// We assume we're at the beginning of the page (i.e. on OggS).
// So, we need to get the page header first to figure out how much data is actually
// available in this page.
int OggGetNextDataPage (uint8_t * destination, size_t maxLength) {
    int dataLen = OggReadPageHeader(&currentPageHeader);
    if (dataLen > 0) {
        // The page header is good and dataLen is the number of available bytes in the page.
        // Note: Since we made sure dataLen > 0, casting to unsigned is safe.
        if ((unsigned)dataLen > maxLength)
            dataLen = (int)maxLength;

        if ( ReadBytes(destination, dataLen) == (unsigned)dataLen ) {
            return dataLen;
        } else {
            return OGG_STRIP_EOF;
        }
    } else {
        return dataLen; // This contains the error code from OggReadPageHeader.
    }
}


// Grab the next packet's content into destination.
// This is probably audio data.
// We assume we're at the beginning of a packet if currentPacket is nonzero.
// If it's zero, we're probably at the beginning of a page, so we should grab the page
// header and fast forward to the start of the content before pulling anything.
int OggGetNextPacket (uint8_t * destination, size_t maxLength) {
    static size_t currentPacket = 0;
    static int dataLen = 0;
    size_t packetLen;

    // If we're done with the previous page and need a new one.
    if (currentPacket >= currentPageHeader.Segments)
        currentPacket = 0;

    if (!currentPacket)
        dataLen = OggReadPageHeader(&currentPageHeader);
   
    if (dataLen > 0) {
        // The page header was pulled successfully, and we're cue'd up.
        // Note: Since we made sure dataLen > 0, casting to unsigned is safe.
        packetLen = ReadBytes(destination, currentPageHeader.SegmentTable[currentPacket]);

        if ( packetLen == currentPageHeader.SegmentTable[currentPacket++] )
            return (int)packetLen;
        else
            return OGG_STRIP_EOF;
    } else {
        printf("ERR! Couldn't read page header: %d.\r\n");
        return dataLen; // This contains the error code from OggReadPageHeader.
    }
}


oggPageHeader_t* OggGetLastPageHeader(void) {
    return &currentPageHeader;
}


// We should be at the start of the ID header data section.  Read it in.
// At the end of this thing, we should have advanced dataLen.
// Return an error code if something goes wrong, or OGG_STRIP_OK if everything's fine.
int OggGetIDHeader (oggIDHeader_t * destination, int dataLen) {
    int extraBytes = dataLen - 19;
    // If dataLen exceeds the length of the ID header (like if there's a channel mapping table)
    // just read in the ID stuff, and skip to the end.
    if (dataLen >= 19) {
        if ( ReadBytes( (char *)destination, 19 ) == 19 ) {
            // Advance any excess bytes.
            if (extraBytes > 0)
                SeekBytes(extraBytes);
            
            if (destination->Signature == OPUSHEAD_MAGIC)
                return OGG_STRIP_OK;
            else {
                printf("ERR! Bad magic string.\r\n");
                return OGG_STRIP_BAD_MAGIC;
            }
        } else {
            printf("ERR! Couldn't read enough bytes.\r\n");
            return OGG_STRIP_EOF;
        }
    } else {
        printf("ERR! ID Header Data length too short.\r\n");
        return OGG_STRIP_LEN_SHORT;
    }
}


// We should be at the start of the comment header data section.
// As of now, we don't need to parse this crap.  Just skip it all for now.
// Return an error code if something goes wrong, or OGG_STRIP_OK if everything's fine.
int OggGetCommentHeader (oggCommentHeader_t * destination, int dataLen) {
    int extraBytes = dataLen - 12;
    // If dataLen exceeds the length of the comment header (like if there's a custom comment)
    // just read in the fixed comment stuff, and skip to the end.
    if (dataLen >= 12) {
        if ( ReadBytes( (char *)destination, 12 ) == 12 ) {
            // Advance any excess bytes.
            if (extraBytes > 0)
                SeekBytes(extraBytes);
            
            if (destination->Signature == OPUSTAGS_MAGIC)
                return OGG_STRIP_OK;
            else {
                printf("ERR! Bad magic string.\r\n");
                return OGG_STRIP_BAD_MAGIC;
            }
        } else {
            printf("ERR! Couldn't read enough bytes.\r\n");
            return OGG_STRIP_EOF;
        }
    } else {
        printf("ERR! Comment Header Data length too short.\r\n");
        return OGG_STRIP_LEN_SHORT;
    }
}


// Start the file at the beginning.  If it's valid, read the info.
// Finally, seek to the beginning of the first data page.
// This function should be called first, before GetNextDataPage.
// Return the data length pulled from the page header.
bool OggPrepareFile (void) {
    int dataLen = 0;
    Rewind(); // Seek to the beginning.

    // Read in the ID header.
    dataLen = OggReadPageHeader(&currentPageHeader);
    if ( OggGetIDHeader(&currentIDHeader, dataLen) == OGG_STRIP_OK ) {
        printf("Got ID Header!\r\n");
    }

    // Read in the comment header.
    dataLen = OggReadPageHeader(&currentPageHeader);
    if ( OggGetCommentHeader(&currentCommentHeader, dataLen) == OGG_STRIP_OK ) {
        printf("Got Comment Header!\r\n");
    }

    if (dataLen > 0)
        return true;
    else
        return false;
}
