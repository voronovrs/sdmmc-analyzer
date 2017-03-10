#ifndef __SDMMC_ANALYZER_H
#define __SDMMC_ANALYZER_H

#include <Analyzer.h>
#include "SDMMCAnalyzerSettings.h"
#include "SDMMCAnalyzerResults.h"
#include "SDMMCSimulationDataGenerator.h"
#include "SDMMCAnalyzer.h"
#include "SDMMCHelpers.h"

class ANALYZER_EXPORT SDMMCAnalyzer : public Analyzer
{
public:
	static const char Name[];

public:
	SDMMCAnalyzer();
	virtual ~SDMMCAnalyzer();

	virtual const char *GetAnalyzerName() const;

	virtual void WorkerThread();
	virtual bool NeedsRerun();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

protected:
	void AdvanceToNextClock();
	void AdvanceToNextCommand();
	int TryReadCommand();
	int WaitForAndReadMMCResponse(struct MMCResponse response);
	void ReadResponseBit(ResponseReadState *state, struct Frame *respFrame);
	void ReadDataBit(DataReadState *state, struct Frame *dataFrame);

protected:
	std::auto_ptr<SDMMCAnalyzerSettings> mSettings;
	std::auto_ptr<SDMMCAnalyzerResults> mResults;

	SDMMCSimulationDataGenerator mDataGenerator;
	bool mSimulationInitialized;

	AnalyzerChannelData *mClock;
	AnalyzerChannelData *mCommand;
	AnalyzerChannelData *mData0;
	AnalyzerChannelData *mData1;
	AnalyzerChannelData *mData2;
	AnalyzerChannelData *mData3;
	AnalyzerChannelData *mData4;
	AnalyzerChannelData *mData5;
	AnalyzerChannelData *mData6;
	AnalyzerChannelData *mData7;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif
