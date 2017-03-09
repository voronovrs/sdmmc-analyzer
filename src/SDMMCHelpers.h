#ifndef __SDMMC_HELPERS
#define __SDMMC_HELPERS

#include <Analyzer.h>

enum MMCResponseType {
	MMC_RSP_NONE,
	MMC_RSP_R1,
	MMC_RSP_R2_CID,
	MMC_RSP_R2_CSD,
	MMC_RSP_R3,
	MMC_RSP_R4,
	MMC_RSP_R5,
};

enum ResponseReadPhase {
	RESP_INIT, // Command is still at 1
	RESP_READDIR, // Direction bit being read
	RESP_IGNORED, // 6 bits - "command index" or "check bits"
	RESP_DATA,
	RESP_CRC, // 0 or 7 bits
	RESP_STOP,
	RESP_ERROR,
	RESP_END
};

enum DataReadPhase {
	DATA_INIT, // Before start bit
	DATA_DATA, // Data being read
	DATA_CRC,  // 16 bits
	DATA_STOP, // Stop bit
	DATA_ERROR, // Decoding error
	DATA_END
};

struct ResponseReadState {
	enum ResponseReadPhase phase;
	enum MMCResponseType responseType;
	U8 ignore_cnt;
	U8 data_bits; // data length, depends only on command index
	U8 data_cnt;
	U8 crc_cnt;
};

struct DataReadState { // FIXME: take "block len" into account
	enum DataReadPhase phase;
	U16 data_cnt;
	U8 crc_cnt;
};

struct MMCResponse
{
	enum MMCResponseType mType;
	unsigned int mBits;
	int mTimeout;
	bool mBusy;
	bool hasDataBlock;
};

struct MMCCommand
{
	char *desc;
};

class SDMMCHelpers
{
public:

public:
	static U8 crc7(const U8 *data, unsigned int size);
	static struct MMCResponse MMCCommandResponse(unsigned int index);
	static const char * MMCCommandDescription(unsigned int index, unsigned int args);

private:
	static U8 __crc7(U8 crc, U8 data);
	static U8 __crc7_finalize(U8 crc);
};

#endif
