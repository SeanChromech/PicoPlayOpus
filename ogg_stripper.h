// Ogg Stripper Header File
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef OGG_STRIPPER_H

#define OGGS_MAGIC     0x5367674F // "OggS" NOTE: Might change due to endianness?
#define OPUSHEAD_MAGIC 0x646165487375704F // "OpusHead"
#define OPUSTAGS_MAGIC 0x736761547375704F // "OpusTags"

typedef struct __attribute((packed)) {
    uint32_t Signature;
    uint8_t Version;
    uint8_t Flags;
    uint64_t GranulePosition;
    uint32_t SerialNumber;
    uint32_t SequenceNumber;
    uint32_t Checksum;
    uint8_t Segments;
    uint8_t SegmentTable[255];
    uint32_t DataLength;
} oggPageHeader_t;

typedef struct __attribute((packed)) {
    uint64_t Signature;
    uint8_t Version;
    uint8_t ChannelCount;
    uint16_t PreSkip;
    uint32_t InputSampleRate;
    uint16_t OutputGain;
    uint8_t MappingFamily;
} oggIDHeader_t;

typedef struct __attribute((packed)) {
    uint64_t Signature;
    uint32_t VendorStringLength;
} oggCommentHeader_t;

enum {
    OGG_STRIP_OK = 0,
    OGG_STRIP_ERR_UNKNOWN = -1,
    OGG_STRIP_EOF = -2,
    OGG_STRIP_BAD_MAGIC = -3,
    OGG_STRIP_NO_SEGS = -4,
    OGG_STRIP_LEN_SHORT = -5
};

int OggReadPageHeader (FILE * oggFile, oggPageHeader_t * header);
int OggGetNextDataPage (FILE * oggFile, uint8_t * destination, size_t maxLength);
int OggGetNextPacket (FILE * oggFile, uint8_t * destination, size_t maxLength);
oggPageHeader_t* OggGetLastPageHeader(void);
int OggGetIDHeader (FILE * oggFile, oggIDHeader_t * destination, int dataLen);
int OggGetCommentHeader (FILE * oggFile, oggCommentHeader_t * destination, int dataLen);
bool OggPrepareFile (FILE * oggFile);


#endif