#include <AnalyzerChannelData.h>
#include "SDMMCAnalyzer.h"
#include "SDMMCAnalyzerResults.h"
#include "SDMMCHelpers.h"

const char SDMMCAnalyzer::Name[] = "SDMMC";

SDMMCAnalyzer::SDMMCAnalyzer()
:	Analyzer(),
	mSettings(new SDMMCAnalyzerSettings()),
	mSimulationInitialized(false)
{
	SetAnalyzerSettings(mSettings.get());
}

SDMMCAnalyzer::~SDMMCAnalyzer()
{
	KillThread();
}

const char* SDMMCAnalyzer::GetAnalyzerName() const
{
	return Name;
}

void SDMMCAnalyzer::WorkerThread()
{
	mResults.reset(new SDMMCAnalyzerResults(this, mSettings.get()));
	SetAnalyzerResults(mResults.get());

	mResults->AddChannelBubblesWillAppearOn(mSettings->mCommandChannel);
	mResults->AddChannelBubblesWillAppearOn(mSettings->mDataChannel0);

	mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
	mCommand = GetAnalyzerChannelData(mSettings->mCommandChannel);
	mData0 = GetAnalyzerChannelData(mSettings->mDataChannel0);
	mData1 = GetAnalyzerChannelData(mSettings->mDataChannel1);
	mData2 = GetAnalyzerChannelData(mSettings->mDataChannel2);
	mData3 = GetAnalyzerChannelData(mSettings->mDataChannel3);
	mData4 = GetAnalyzerChannelData(mSettings->mDataChannel4);
	mData5 = GetAnalyzerChannelData(mSettings->mDataChannel5);
	mData6 = GetAnalyzerChannelData(mSettings->mDataChannel6);
	mData7 = GetAnalyzerChannelData(mSettings->mDataChannel7);

	while (true) {
		int cmdindex;

		ReportProgress(mClock->GetSampleNumber());
		CheckIfThreadShouldExit();
		AdvanceToNextClock();

		cmdindex = TryReadCommand();
		if (cmdindex < 0) {
			/* continue if parsing the command failed */
			continue;
		}

		if (mSettings->mProtocol == PROTOCOL_MMC) {
			struct MMCResponse response = SDMMCHelpers::MMCCommandResponse(cmdindex);
			if (response.mType != MMC_RSP_NONE)
				WaitForAndReadMMCResponse(response);
		} else {
			/* FIXME: implement SD response handling */
		}
	}
}

bool SDMMCAnalyzer::NeedsRerun()
{
	return false;
}

U32 SDMMCAnalyzer::GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	if (!mSimulationInitialized) {
	mDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
	mSimulationInitialized = true;
	}

	return mDataGenerator.GenerateSimulationData(newest_sample_requested, sample_rate, simulation_channels);
}

U32 SDMMCAnalyzer::GetMinimumSampleRateHz()
{
	return 400000 * 4;
}

void SDMMCAnalyzer::AdvanceToNextClock()
{
	enum BitState search = mSettings->mSampleEdge == SAMPLE_EDGE_RISING ? BIT_HIGH : BIT_LOW;

	do {
		mClock->AdvanceToNextEdge();
	} while (mClock->GetBitState() != search);

	mCommand->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData0->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData1->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData2->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData3->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData4->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData5->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData6->AdvanceToAbsPosition(mClock->GetSampleNumber());
	mData7->AdvanceToAbsPosition(mClock->GetSampleNumber());
}

int SDMMCAnalyzer::TryReadCommand()
{
	int index;

	/* check for start bit */
	if (mCommand->GetBitState() != BIT_LOW)
		return -1;

	mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::Start, mSettings->mCommandChannel);
	AdvanceToNextClock();

	/* transfer bit */
	if (mCommand->GetBitState() != BIT_HIGH) {
		/* if host is not transferring this is no command */
		mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::X, mSettings->mCommandChannel);
		return -1;
	}
	AdvanceToNextClock();

	/* command index and argument */
	{
		Frame frame;

		frame.mStartingSampleInclusive = mClock->GetSampleNumber();
		frame.mData1 = 0;
		frame.mData2 = 0;
		frame.mType = SDMMCAnalyzerResults::FRAMETYPE_COMMAND;

		for (int i = 0; i < 6; i++) {
			frame.mData1 = (frame.mData1 << 1) | mCommand->GetBitState();
			AdvanceToNextClock();
		}

		for (int i = 0; i < 32; i++) {
			frame.mData2 = (frame.mData2 << 1) | mCommand->GetBitState();
			AdvanceToNextClock();
		}

		frame.mEndingSampleInclusive = mClock->GetSampleNumber() - 1;
		mResults->AddFrame(frame);
		mResults->CommitResults();

		/* save index for returning */
		index = (int)frame.mData1;
	}

	/* crc */
	{
		Frame frame;

		frame.mStartingSampleInclusive = mClock->GetSampleNumber();
		frame.mData1 = 0;
		frame.mType = SDMMCAnalyzerResults::FRAMETYPE_CRC;

		for (int i = 0; i < 7; i++) {
			frame.mData1 = (frame.mData1 << 1) | mCommand->GetBitState();
			AdvanceToNextClock();
		}

		frame.mEndingSampleInclusive = mClock->GetSampleNumber() - 1;
		mResults->AddFrame(frame);
		mResults->CommitResults();
	}

	/* stop bit */
	mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mCommandChannel);

	mResults->CommitResults();
	ReportProgress(mClock->GetSampleNumber());

	return index;
}

int SDMMCAnalyzer::WaitForAndReadMMCResponse(struct MMCResponse response)
{
	int timeout = response.mTimeout + 3; // add some slack time

	while (timeout-- >= 0 && mCommand->GetBitState() != BIT_LOW)
		AdvanceToNextClock();

	if (timeout < 0)
		return -1;

	// Frame objects used by state machine functions - can't be created there
	// (lifetime)
	Frame respFrame;
	Frame dataFrame;
	// Start ReadResponse & ReadData state machine
	ResponseReadState respState = {RESP_INIT, response.mType, 0, response.mBits, 0};
	DataReadState dataState = {DATA_INIT, 0, 0};
	if (!response.hasDataBlock || mSettings->mBusWidth == BUS_WIDTH_0) {
		dataState.phase = DATA_END;
	}

	while (respState.phase != RESP_ERROR && dataState.phase != DATA_ERROR &&
			!(respState.phase == RESP_END && dataState.phase == DATA_END)) {
		ReadResponseBit(&respState, &respFrame);
		ReadDataBit(&dataState, &dataFrame);
		AdvanceToNextClock();
	}

	mResults->CommitResults();
	ReportProgress(mClock->GetSampleNumber());

	return 1;
}

void SDMMCAnalyzer::ReadResponseBit(ResponseReadState *state, struct Frame *frame) {
	switch(state->phase) {
		case RESP_INIT:
			mResults->AddMarker(mClock->GetSampleNumber(),
					AnalyzerResults::Start, mSettings->mCommandChannel);
			state->phase = RESP_READDIR;
			return;
		case RESP_READDIR:
			if (mCommand->GetBitState() != BIT_LOW) {
				/* if card is not transferring this is no response */
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::X, mSettings->mCommandChannel);
				state->phase = RESP_ERROR;
			} else {
				state->phase = RESP_IGNORED;
			}
			return;
		case RESP_IGNORED:
			if (state->ignore_cnt == 0)
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
			if (state->ignore_cnt == 5)
				state->phase = RESP_DATA;
			state->ignore_cnt++;
			return;
		case RESP_DATA:
			/* first data bit only */
			if (state->data_cnt == 0) {
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_RESPONSE;
				frame->mFlags = state->responseType;
			}
			if (state->data_cnt < 64) {
				frame->mData1 = (frame->mData1 << 1) | mCommand->GetBitState();
				/* bits 0 to 63 */
			} else {
				frame->mData2 = (frame->mData2 << 1) | mCommand->GetBitState();
				/* bits 64 to 127 */
			}
			/* last data bit only */
			if (state->data_cnt == state->data_bits) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				if (state->responseType != MMC_RSP_R2_CID &&
						state->responseType != MMC_RSP_R2_CSD) {
					state->phase = RESP_CRC;
				} else {
					state->phase = RESP_STOP;
				}
			}
			state->data_cnt++;
			return;
		case RESP_CRC:
			/* first crc bit only */
			if (state->crc_cnt == 0) {
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_CRC;
			}
			frame->mData1 = (frame->mData1 << 1) | mCommand->GetBitState();

			/* last crc bit only */
			if (state->crc_cnt == 6) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				state->phase = RESP_STOP;
			}
			state->crc_cnt++;
			return;
		case RESP_STOP:
			/* stop bit */
			mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mCommandChannel);
			state->phase = RESP_END;
			return;
		/* this method does not gets called for END and ERROR phases */
		case RESP_ERROR:
		case RESP_END:
			return;
	}

}

void SDMMCAnalyzer::ReadDataBit(DataReadState *state, struct Frame *frame) {
	switch(state->phase) {
		case DATA_INIT:
			if (mData0->GetBitState() == BIT_LOW) {
				/* other data lines must be at 0, too
				 * (depending on bus width (?)) */
				if (    /* BusWidth=4 or 8 */
						mSettings->mBusWidth != BUS_WIDTH_1 &&
						(mData1->GetBitState() != BIT_LOW ||
						 mData2->GetBitState() != BIT_LOW ||
						 mData3->GetBitState() != BIT_LOW) ||
						/* BusWidth=8 */
						mSettings->mBusWidth == BUS_WIDTH_8 &&
						(mData4->GetBitState() != BIT_LOW ||
						 mData5->GetBitState() != BIT_LOW ||
						 mData6->GetBitState() != BIT_LOW ||
						 mData7->GetBitState() != BIT_LOW)) {
					mResults->AddMarker(mClock->GetSampleNumber(),
							AnalyzerResults::X, mSettings->mDataChannel0);
					state->phase = DATA_ERROR;
				} else {
					mResults->AddMarker(mClock->GetSampleNumber(),
							AnalyzerResults::Start, mSettings->mDataChannel0);
					state->phase = DATA_DATA;
				}
			}
			return;
		case DATA_DATA:
			 /* Start frames if necessary*/
			if (mSettings->mBusWidth == BUS_WIDTH_8 ||
					(mSettings->mBusWidth == BUS_WIDTH_4 && state->data_cnt % 2 == 0) ||
					(mSettings->mBusWidth == BUS_WIDTH_1 && state->data_cnt % 8 == 0)) {
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_DATA_CONTENTS;
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mFlags = 0;
			}
			/* Store data */
			if (mSettings->mBusWidth == BUS_WIDTH_8) {
				frame->mData1 = (frame->mData1 << 1) | mData7->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData6->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData5->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData4->GetBitState();
			}
			if (mSettings->mBusWidth == BUS_WIDTH_8 ||
					mSettings->mBusWidth == BUS_WIDTH_4) {
				frame->mData1 = (frame->mData1 << 1) | mData3->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData2->GetBitState();
				frame->mData1 = (frame->mData1 << 1) | mData1->GetBitState();
			}
			frame->mData1 = (frame->mData1 << 1) | mData0->GetBitState();
			/* Commit results if necessary */
			if (mSettings->mBusWidth == BUS_WIDTH_8 ||
					(mSettings->mBusWidth == BUS_WIDTH_4 && state->data_cnt % 2 == 1) ||
					(mSettings->mBusWidth == BUS_WIDTH_1 && state->data_cnt % 8 == 7)
					) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
			}
			/* last data bit only - change state */
			if (state->data_cnt == 1023) {
				/* FIXME do not used fixed block width */
				state->phase = DATA_CRC;
			}
			state->data_cnt++;
			return;
		case DATA_CRC:
			if (state->crc_cnt == 0) {
				frame->mType = SDMMCAnalyzerResults::FRAMETYPE_DATA_CRC;
				frame->mStartingSampleInclusive = mClock->GetSampleNumber();
				frame->mData1 = 0;
				frame->mData2 = 0;
				frame->mFlags = 0;
			}
			frame->mData1 <<= 1;
			frame->mData1 |= (mData0->GetBitState());
			if (mSettings->mBusWidth != BUS_WIDTH_1) { /* BusWidth=4 or 8 */
				frame->mData1 |= (mData1->GetBitState() << 16);
				frame->mData1 |= (mData2->GetBitState() << 32);
				frame->mData1 |= (mData3->GetBitState() << 48);
			}
			if (mSettings->mBusWidth == BUS_WIDTH_8) { /* BusWidth=8 */
				frame->mData2 |= (mData4->GetBitState() << 16);
				frame->mData2 |= (mData5->GetBitState() << 16);
				frame->mData2 |= (mData6->GetBitState() << 32);
				frame->mData2 |= (mData7->GetBitState() << 48);
			}
			if (state->crc_cnt == 15) {
				frame->mEndingSampleInclusive = mClock->GetSampleNumber();
				mResults->AddFrame(*frame);
				mResults->CommitResults();
				state->phase = DATA_STOP;
			}
			state->crc_cnt++;
			return;
		case DATA_STOP:
			if (    /* BusWidth=4 or 8 */
					mSettings->mBusWidth != BUS_WIDTH_1 &&
					(mData0->GetBitState() == BIT_LOW ||
					 mData1->GetBitState() == BIT_LOW ||
					 mData2->GetBitState() == BIT_LOW ||
					 mData3->GetBitState() == BIT_LOW) ||
					/* BusWidth=8 */
					mSettings->mBusWidth == BUS_WIDTH_8 &&
					(mData4->GetBitState() == BIT_LOW ||
					 mData5->GetBitState() == BIT_LOW ||
					 mData6->GetBitState() == BIT_LOW ||
					 mData7->GetBitState() == BIT_LOW)) {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::X, mSettings->mDataChannel0);
				state->phase = DATA_ERROR;
			} else {
				mResults->AddMarker(mClock->GetSampleNumber(),
						AnalyzerResults::Stop, mSettings->mDataChannel0);
				state->phase = DATA_END;
			}
			return;
			/* this method does not gets called for END and ERROR phases */
		case DATA_ERROR:
		case DATA_END:
			return;
	}
}
/*
 * loader hooks
 */

const char* GetAnalyzerName()
{
	return SDMMCAnalyzer::Name;
}

Analyzer* CreateAnalyzer()
{
	return new SDMMCAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer)
{
	delete analyzer;
}
