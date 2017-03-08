#include <AnalyzerHelpers.h>
#include "SDMMCAnalyzerSettings.h"

SDMMCAnalyzerSettings::SDMMCAnalyzerSettings()
:	mClockChannel(UNDEFINED_CHANNEL),
	mCommandChannel(UNDEFINED_CHANNEL),
	mDataChannel0(UNDEFINED_CHANNEL),
	mDataChannel1(UNDEFINED_CHANNEL),
	mDataChannel2(UNDEFINED_CHANNEL),
	mDataChannel3(UNDEFINED_CHANNEL),
	mProtocol(PROTOCOL_MMC),
	mSampleEdge(SAMPLE_EDGE_RISING)
{
	mClockChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mClockChannelInterface->SetTitleAndTooltip("Clock", "Clock (CLK)");
	mClockChannelInterface->SetChannel(mClockChannel);

	mCommandChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mCommandChannelInterface->SetTitleAndTooltip("Command", "Command (CMD)");
	mCommandChannelInterface->SetChannel(mCommandChannel);

	mProtocolInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mProtocolInterface->SetTitleAndTooltip("Protocol", "Protocol");
	mProtocolInterface->AddNumber(PROTOCOL_MMC, "MMC", "MMC protocol");
	mProtocolInterface->AddNumber(PROTOCOL_SD,	"SD",  "SD protocol");
	mProtocolInterface->SetNumber(mProtocol);

	mDataChannelInterface0.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface0->SetTitleAndTooltip("Data channel 0", "Data channel 0 (D0)");
	mDataChannelInterface0->SetChannel(mDataChannel0);
	mDataChannelInterface0->SetSelectionOfNoneIsAllowed(true);

	mDataChannelInterface1.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface1->SetTitleAndTooltip("Data channel 1", "Data channel 1 (D1)");
	mDataChannelInterface1->SetChannel(mDataChannel1);
	mDataChannelInterface1->SetSelectionOfNoneIsAllowed(true);

	mDataChannelInterface2.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface2->SetTitleAndTooltip("Data channel 2", "Data channel 2 (D2)");
	mDataChannelInterface2->SetChannel(mDataChannel2);
	mDataChannelInterface2->SetSelectionOfNoneIsAllowed(true);

	mDataChannelInterface3.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface3->SetTitleAndTooltip("Data channel 3", "Data channel 3 (D3)");
	mDataChannelInterface3->SetChannel(mDataChannel3);
	mDataChannelInterface3->SetSelectionOfNoneIsAllowed(true);

	mSampleEdgeInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mSampleEdgeInterface->SetTitleAndTooltip("Sample edge", "Clock sampling edge");
	mSampleEdgeInterface->AddNumber(SAMPLE_EDGE_RISING,  "Rising",  "Sample on rising edge");
	mSampleEdgeInterface->AddNumber(SAMPLE_EDGE_FALLING, "Falling", "Sample on falling edge");
	mSampleEdgeInterface->SetNumber(mSampleEdge);

	AddInterface(mClockChannelInterface.get());
	AddInterface(mCommandChannelInterface.get());
	AddInterface(mProtocolInterface.get());
	AddInterface(mSampleEdgeInterface.get());
	AddInterface(mDataChannelInterface0.get());
	AddInterface(mDataChannelInterface1.get());
	AddInterface(mDataChannelInterface2.get());
	AddInterface(mDataChannelInterface3.get());

	AddExportOption(0, "Export as text file");
	AddExportExtension(0, "text", "txt");

	ClearChannels();
	AddChannel(mClockChannel, "Clock", false);
	AddChannel(mCommandChannel, "Command", false);
	AddChannel(mDataChannel0, "Data channel 0", false);
	AddChannel(mDataChannel1, "Data channel 1", false);
	AddChannel(mDataChannel2, "Data channel 2", false);
	AddChannel(mDataChannel3, "Data channel 3", false);
}

SDMMCAnalyzerSettings::~SDMMCAnalyzerSettings()
{
}

bool SDMMCAnalyzerSettings::SetSettingsFromInterfaces()
{
	Channel clk = mClockChannelInterface->GetChannel();
	Channel cmd = mCommandChannelInterface->GetChannel();
	Channel data0 = mDataChannelInterface0->GetChannel();
	Channel data1 = mDataChannelInterface1->GetChannel();
	Channel data2 = mDataChannelInterface2->GetChannel();
	Channel data3 = mDataChannelInterface3->GetChannel();

	if (clk == cmd) {
		SetErrorText("Please select different channels for each input.");
		return false;
	}

	mClockChannel = clk;
	mCommandChannel = cmd;
	mDataChannel0 = data0;
	mDataChannel1 = data1;
	mDataChannel2 = data2;
	mDataChannel3 = data3;
	mProtocol = SDMMCProtocol((U32)mProtocolInterface->GetNumber());
	mSampleEdge = SDMMCSampleEdge((U32)mSampleEdgeInterface->GetNumber());

	ClearChannels();
	AddChannel(mClockChannel, "Clock", true);
	AddChannel(mCommandChannel, "Command", true);
	AddChannel(mDataChannel0, "Data channel 0", true);
	AddChannel(mDataChannel1, "Data channel 1", true);
	AddChannel(mDataChannel2, "Data channel 2", true);
	AddChannel(mDataChannel3, "Data channel 3", true);

	return true;
}

void SDMMCAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mClockChannelInterface->SetChannel(mClockChannel);
	mCommandChannelInterface->SetChannel(mCommandChannel);
	mDataChannelInterface0->SetChannel(mDataChannel0);
	mDataChannelInterface1->SetChannel(mDataChannel1);
	mDataChannelInterface2->SetChannel(mDataChannel2);
	mDataChannelInterface3->SetChannel(mDataChannel3);
	mProtocolInterface->SetNumber(mProtocol);
	mSampleEdgeInterface->SetNumber(mSampleEdge);
}

void SDMMCAnalyzerSettings::LoadSettings(const char *settings)
{
	SimpleArchive archive;
	U32 tmp;

	archive.SetString(settings);

	archive >> mClockChannel;
	archive >> mCommandChannel;
	archive >> mDataChannel0;
	archive >> mDataChannel1;
	archive >> mDataChannel2;
	archive >> mDataChannel3;
	archive >> tmp; mProtocol = SDMMCProtocol(tmp);
	archive >> tmp; mSampleEdge = SDMMCSampleEdge(tmp);

	ClearChannels();
	AddChannel(mClockChannel, "Clock", true);
	AddChannel(mCommandChannel, "Command", true);
	AddChannel(mDataChannel0, "Data channel 0", true);
	AddChannel(mDataChannel1, "Data channel 1", true);
	AddChannel(mDataChannel2, "Data channel 2", true);
	AddChannel(mDataChannel3, "Data channel 3", true);

	UpdateInterfacesFromSettings();
}

const char *SDMMCAnalyzerSettings::SaveSettings()
{
	SimpleArchive archive;

	archive << mClockChannel;
	archive << mCommandChannel;
	archive << mDataChannel0;
	archive << mDataChannel1;
	archive << mDataChannel2;
	archive << mDataChannel3;
	archive << mProtocol;
	archive << mSampleEdge;

	return SetReturnString(archive.GetString());
}
