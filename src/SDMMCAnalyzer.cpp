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

	mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
	mCommand = GetAnalyzerChannelData(mSettings->mCommandChannel);

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

	// Start ReadResponse state machine
	Frame frame;
	ResponseReadState respState = {RESP_INIT, response.mType, 0, response.mBits, 0};

	while (respState.phase != RESP_END && respState.phase != RESP_ERROR) {
		ReadResponseBit(&respState, &frame);
		AdvanceToNextClock();
	}

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
				mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::X, mSettings->mCommandChannel);
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
