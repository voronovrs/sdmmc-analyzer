#include "SDMMCAnalyzerResults.h"
#include "SDMMCAnalyzer.h"
#include "SDMMCAnalyzerSettings.h"
#include "SDMMCHelpers.h"

#include <AnalyzerHelpers.h>
#include <iostream>
#include <fstream>

SDMMCAnalyzerResults::SDMMCAnalyzerResults(SDMMCAnalyzer* analyzer, SDMMCAnalyzerSettings* settings)
:	AnalyzerResults(),
	mSettings(settings),
	mAnalyzer(analyzer)
{
}

SDMMCAnalyzerResults::~SDMMCAnalyzerResults()
{
}

void SDMMCAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)
{
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);

	switch (frame.mType) {
	case FRAMETYPE_HEADER:
		if (frame.mData1 == 1)
			AddResultString("Host sending");
		else
			AddResultString("Card sending");
		break;

	case FRAMETYPE_COMMAND:
	{
		char str_cmd[4];
		char str_arg[33];

		AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, 6, str_cmd, sizeof(str_cmd));
		AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 32, str_arg, sizeof(str_arg));

		AddResultString("CMD");
		AddResultString("CMD", str_cmd);
		AddResultString("CMD", str_cmd, ", arg=", str_arg);
		break;
	}

	case FRAMETYPE_RESPONSE:
	{
		char str_32[33];

		switch (frame.mFlags) {
		case MMC_RSP_R1:
		{
			const char *str_state = "reserved";
			std::string str_flags("");

			switch ((frame.mData1 >> 9) & 0xf) {
			case 0: str_state = "Idle"; break;
			case 1: str_state = "Ready"; break;
			case 2: str_state = "Ident"; break;
			case 3: str_state = "Stby"; break;
			case 4: str_state = "Tran"; break;
			case 5: str_state = "Data"; break;
			case 6: str_state = "Rcv"; break;
			case 7: str_state = "Prg"; break;
			case 8: str_state = "Dis"; break;
			case 9: str_state = "Btst"; break;
			case 10: str_state = "Slp "; break;
			}

			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, str_32, sizeof(str_32));
			
			AddResultString("R1");
			AddResultString("R1, ", str_state);
			AddResultString("R1, ", str_state, ", rsp=", str_32);

			if (frame.mData1 & (1 << 31))
					str_flags += " ADDRESS_OUT_OF_RANGE";
			if (frame.mData1 & (1 << 30))
					str_flags += " ADDRESS_MISALIGN";
			if (frame.mData1 & (1 << 29))
					str_flags += " BLOCK_LEN_ERROR";
			if (frame.mData1 & (1 << 28))
					str_flags += " ERASE_SEQ_ERROR";
			if (frame.mData1 & (1 << 27))
					str_flags += " ERASE_PARAM";
			if (frame.mData1 & (1 << 26))
					str_flags += " WP_VIOLATION";
			if (frame.mData1 & (1 << 25))
					str_flags += " CARD_IS_LOCKED";
			if (frame.mData1 & (1 << 24))
					str_flags += " LOCK_UNLOCK_FAILED";
			if (frame.mData1 & (1 << 23))
					str_flags += " COM_CRC_ERROR";
			if (frame.mData1 & (1 << 22))
					str_flags += " ILLEGAL_COMMAND";
			if (frame.mData1 & (1 << 21))
					str_flags += " CARD_ECC_FAILED";
			if (frame.mData1 & (1 << 20))
					str_flags += " CC_ERROR";
			if (frame.mData1 & (1 << 19))
					str_flags += " ERROR";
			if (frame.mData1 & (1 << 18))
					str_flags += " UNDERRUN";
			if (frame.mData1 & (1 << 17))
					str_flags += " OVERRUN";
			if (frame.mData1 & (1 << 16))
					str_flags += " CID/CSD_OVERWRITE";
			if (frame.mData1 & (1 << 15))
					str_flags += " WP_ERASE_SKIP";
			if (frame.mData1 & (1 << 13))
					str_flags += " ERASE_RESET";
			if (frame.mData1 & (1 << 8))
					str_flags += " READY_FOR_DATA";
			if (frame.mData1 & (1 << 7))
					str_flags += " SWITCH_ERROR";
			if (frame.mData1 & (1 << 5))
					str_flags += " APP_CMD";

			if (str_flags.length() > 0)
				AddResultString("R1, ", str_state, ", rsp=", str_32, str_flags.c_str());

			break;
		}
		case MMC_RSP_R2_CID:
		{
			std::string res("R2");
			char pname[7], prv_str[4], psn_str[12];
			char rsp_str[64];

			AddResultString(res.c_str());

			res += " [CID]";
			AddResultString(res.c_str());

			res += " rsp=";
			AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			AddResultString(res.c_str());

			pname[0] = (frame.mData1 >> 32) & 0xff;
			pname[1] = (frame.mData1 >> 24) & 0xff;
			pname[2] = (frame.mData1 >> 16) & 0xff;
			pname[3] = (frame.mData1 >>  8) & 0xff;
			pname[4] = (frame.mData1 >>  0) & 0xff;
			pname[5] = (frame.mData2 >> 56) & 0xff;
			pname[6] = 0;

			unsigned prv = (unsigned)((frame.mData2 >> 48) & 0xff);
			prv_str[0] = '0' + ((prv >> 4) & 0x0f);
			prv_str[1] = '.';
			prv_str[2] = '0' + (prv & 0x0f);
			prv_str[3] = 0;

			unsigned psn = (unsigned)((frame.mData2 >> 16) & 0xfffffffful);
			AnalyzerHelpers::GetNumberString(psn, Decimal, 32, psn_str, sizeof(psn_str));

			res += " pnm='";
			res += pname;
			res += "' prv=";
			res += prv_str;
			res += " psn=";
			res += psn_str;
			AddResultString(res.c_str());

			break;
		}
		case MMC_RSP_R2_CSD:
		{
			std::string res("R2");
			char rsp_str[64];

			AddResultString(res.c_str());

			res += " [CSD]";
			AddResultString(res.c_str());

			res += " rsp=";
			AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			res += " ";
			AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
			res += rsp_str;
			AddResultString(res.c_str());

			break;
		}
		case MMC_RSP_R3:
			AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 32, str_32, sizeof(str_32));
			AddResultString("R3");
			AddResultString("R3, ocr=", str_32);
			break;
		case MMC_RSP_R4:
			AddResultString("R4");
			break;
		case MMC_RSP_R5:
			AddResultString("R5");
			break;
		}
		break;
	}

	case FRAMETYPE_CRC:
	{
		char str_crc[8];

		AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 7, str_crc, sizeof(str_crc));

		AddResultString("CRC");
		AddResultString("CRC=", str_crc);
		break;
	}

	default:
		AddResultString("error");
	}
}

void SDMMCAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 export_type_user_id)
{
	std::ofstream file_stream(file, std::ios::out);
	
	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();
	
	U8 lastCmd = 0;
	
	U64 num_frames = GetNumFrames();
	for  (U32 i = 0; i < num_frames; i++)
	{
		Frame frame = GetFrame(i);
		
		switch (frame.mType) {
		case FRAMETYPE_COMMAND:
		{
			char str_cmd[4];
			char str_arg[33];

			AnalyzerHelpers::GetNumberString(frame.mData1, Decimal, 6, str_cmd, sizeof(str_cmd));
			AnalyzerHelpers::GetNumberString(frame.mData2, display_base, 32, str_arg, sizeof(str_arg));

			file_stream << "CMD" << str_cmd << ", arg=" << str_arg << " ";
			break;
		}

		case FRAMETYPE_RESPONSE:
		{
			char str_32[33];

			switch (frame.mFlags) {
			case MMC_RSP_R1:
			{
				const char *str_state = "reserved";
				std::string str_flags("");

				switch ((frame.mData1 >> 9) & 0xf) {
				case 0: str_state = "Idle"; break;
				case 1: str_state = "Ready"; break;
				case 2: str_state = "Ident"; break;
				case 3: str_state = "Stby"; break;
				case 4: str_state = "Tran"; break;
				case 5: str_state = "Data"; break;
				case 6: str_state = "Rcv"; break;
				case 7: str_state = "Prg"; break;
				case 8: str_state = "Dis"; break;
				case 9: str_state = "Btst"; break;
				case 10: str_state = "Slp "; break;
				}

				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 32, str_32, sizeof(str_32));

				if (frame.mData1 & (1 << 31))
						str_flags += " ADDRESS_OUT_OF_RANGE";
				if (frame.mData1 & (1 << 30))
						str_flags += " ADDRESS_MISALIGN";
				if (frame.mData1 & (1 << 29))
						str_flags += " BLOCK_LEN_ERROR";
				if (frame.mData1 & (1 << 28))
						str_flags += " ERASE_SEQ_ERROR";
				if (frame.mData1 & (1 << 27))
						str_flags += " ERASE_PARAM";
				if (frame.mData1 & (1 << 26))
						str_flags += " WP_VIOLATION";
				if (frame.mData1 & (1 << 25))
						str_flags += " CARD_IS_LOCKED";
				if (frame.mData1 & (1 << 24))
						str_flags += " LOCK_UNLOCK_FAILED";
				if (frame.mData1 & (1 << 23))
						str_flags += " COM_CRC_ERROR";
				if (frame.mData1 & (1 << 22))
						str_flags += " ILLEGAL_COMMAND";
				if (frame.mData1 & (1 << 21))
						str_flags += " CARD_ECC_FAILED";
				if (frame.mData1 & (1 << 20))
						str_flags += " CC_ERROR";
				if (frame.mData1 & (1 << 19))
						str_flags += " ERROR";
				if (frame.mData1 & (1 << 18))
						str_flags += " UNDERRUN";
				if (frame.mData1 & (1 << 17))
						str_flags += " OVERRUN";
				if (frame.mData1 & (1 << 16))
						str_flags += " CID/CSD_OVERWRITE";
				if (frame.mData1 & (1 << 15))
						str_flags += " WP_ERASE_SKIP";
				if (frame.mData1 & (1 << 13))
						str_flags += " ERASE_RESET";
				if (frame.mData1 & (1 << 8))
						str_flags += " READY_FOR_DATA";
				if (frame.mData1 & (1 << 7))
						str_flags += " SWITCH_ERROR";
				if (frame.mData1 & (1 << 5))
						str_flags += " APP_CMD";

				file_stream << "R1, " << str_state << ", rsp=" << str_32;

				if (str_flags.length() > 0)
					file_stream << str_flags.c_str();

				file_stream << " ";

				break;
			}
			case MMC_RSP_R2_CID:
			{
				std::string res("R2");
				char pname[7], prv_str[4], psn_str[12];
				char rsp_str[64];

				res += " [CID]";
				res += " rsp=";
				AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;

				pname[0] = (frame.mData1 >> 32) & 0xff;
				pname[1] = (frame.mData1 >> 24) & 0xff;
				pname[2] = (frame.mData1 >> 16) & 0xff;
				pname[3] = (frame.mData1 >>  8) & 0xff;
				pname[4] = (frame.mData1 >>  0) & 0xff;
				pname[5] = (frame.mData2 >> 56) & 0xff;
				pname[6] = 0;

				unsigned prv = (unsigned)((frame.mData2 >> 48) & 0xff);
				prv_str[0] = '0' + ((prv >> 4) & 0x0f);
				prv_str[1] = '.';
				prv_str[2] = '0' + (prv & 0x0f);
				prv_str[3] = 0;

				unsigned psn = (unsigned)((frame.mData2 >> 16) & 0xfffffffful);
				AnalyzerHelpers::GetNumberString(psn, Decimal, 32, psn_str, sizeof(psn_str));

				res += " pnm='";
				res += pname;
				res += "' prv=";
				res += prv_str;
				res += " psn=";
				res += psn_str;
				file_stream << res.c_str() << std::endl;

				break;
			}
			case MMC_RSP_R2_CSD:
			{
				std::string res("R2");
				char rsp_str[64];

				res += " [CSD]";

				res += " rsp=";
				AnalyzerHelpers::GetNumberString(frame.mData1 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData1 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 >> 32, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				res += " ";
				AnalyzerHelpers::GetNumberString(frame.mData2 & 0xffffffffull, display_base, 32, rsp_str, sizeof(rsp_str));
				res += rsp_str;
				file_stream << res.c_str() << std::endl;

				break;
			}
			case MMC_RSP_R3:
				AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 32, str_32, sizeof(str_32));
				file_stream << "R3, ocr=" << str_32 << std::endl;
				break;
			case MMC_RSP_R4:
				file_stream << "R4 ";
				break;
			case MMC_RSP_R5:
				file_stream << "R5 ";
				break;
			}
			break;
		}

		case FRAMETYPE_CRC:
		{
			if (i == 0)
			{
				file_stream << "CRC Error" << std::endl;
				break;
			}
			
			Frame dFrame = GetFrame(i-1);
			char str_crc[8];
			char str_c[128];
			
			U8 data[6];
			U8 calcCrc;
			bool crc = 1;
			
			switch (dFrame.mType) {
			case FRAMETYPE_COMMAND:
			{
				lastCmd = dFrame.mData1;
				data[0] = 0x40 + (U8)(dFrame.mData1);
				data[1] = dFrame.mData2 >> 24;
				data[2] = dFrame.mData2 >> 16;
				data[3] = dFrame.mData2 >>  8;
				data[4] = dFrame.mData2;
			
				calcCrc = SDMMCHelpers::crc7(data, 5);
				break;
			}

			case FRAMETYPE_RESPONSE:
			{
				switch (frame.mFlags) {
				case MMC_RSP_R1:
				case MMC_RSP_R4:
				case MMC_RSP_R5:
					data[0] = (U8)(lastCmd);
					data[1] = dFrame.mData1 >> 24;
					data[2] = dFrame.mData1 >> 16;
					data[3] = dFrame.mData1 >>  8;
					data[4] = dFrame.mData1;
			
					calcCrc = SDMMCHelpers::crc7(data, 5);
					break;
				case MMC_RSP_R2_CID:
				case MMC_RSP_R2_CSD:
				case MMC_RSP_R3:
					crc = 0;
					break;
				}

				break;
			}
			}
			
			if (!crc) break;
			
			AnalyzerHelpers::GetNumberString(frame.mData1, Hexadecimal, 8, str_crc, sizeof(str_crc));

			file_stream << "CRC=" << str_crc;
			
			if (calcCrc != (U8)(frame.mData1)) {
				AnalyzerHelpers::GetNumberString(calcCrc, Hexadecimal, 8, str_crc, sizeof(str_crc));
				file_stream << ", ERROR=" << str_crc;
			}
			file_stream << std::endl;
			break;
		}

		default:
			file_stream << "error";
		}
		
		//file_stream << ""; //std::endl;
		
		
		// check for cancel flag
		if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true)
		{
			file_stream.close();
			return;
		}
	}
	file_stream.close();
}

void SDMMCAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SDMMCAnalyzerResults::GeneratePacketTabularText(U64 packet_id, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}

void SDMMCAnalyzerResults::GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base)
{
	ClearResultStrings();
	AddResultString("not supported");
}
