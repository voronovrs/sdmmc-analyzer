#include <AnalyzerHelpers.h>
#include "SDMMCAnalyzerSettings.h"

SDMMCAnalyzerSettings::SDMMCAnalyzerSettings()
:	mClockChannel(UNDEFINED_CHANNEL),
	mCommandChannel(UNDEFINED_CHANNEL),
	mDataChannel0(UNDEFINED_CHANNEL),
	mDataChannel1(UNDEFINED_CHANNEL),
	mDataChannel2(UNDEFINED_CHANNEL),
	mDataChannel3(UNDEFINED_CHANNEL),
	mDataChannel4(UNDEFINED_CHANNEL),
	mDataChannel5(UNDEFINED_CHANNEL),
	mDataChannel6(UNDEFINED_CHANNEL),
	mDataChannel7(UNDEFINED_CHANNEL),
	mProtocol(PROTOCOL_MMC),
	mBusWidth(BUS_WIDTH_0),
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

	mBusWidthInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mBusWidthInterface->SetTitleAndTooltip("Bus Width", "Bus Width");
	mBusWidthInterface->AddNumber(BUS_WIDTH_0, "0 (no data lines)", "0 (no data lines)");
	mBusWidthInterface->AddNumber(BUS_WIDTH_1,	"1 (D0 only)",  "1 (D0 only)");
	mBusWidthInterface->AddNumber(BUS_WIDTH_4,	"4 (D0-D3 only)",  "4 (D0-D3 only)");
	mBusWidthInterface->AddNumber(BUS_WIDTH_8,	"8 (D0-D8)",  "8 (D0-D8)");
	mBusWidthInterface->SetNumber(mBusWidth);

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
	mDataChannelInterface3->SetChannel(mDataChannel3);
	mDataChannelInterface3->SetTitleAndTooltip("Data channel 3", "Data channel 3 (D3)");
	mDataChannelInterface3->SetSelectionOfNoneIsAllowed(true);
	mDataChannelInterface4.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface4->SetTitleAndTooltip("Data channel 4", "Data channel 4 (D4)");
	mDataChannelInterface4->SetChannel(mDataChannel4);
	mDataChannelInterface4->SetSelectionOfNoneIsAllowed(true);
	mDataChannelInterface5.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface5->SetTitleAndTooltip("Data channel 5", "Data channel 5 (D5)");
	mDataChannelInterface5->SetChannel(mDataChannel5);
	mDataChannelInterface5->SetSelectionOfNoneIsAllowed(true);
	mDataChannelInterface6.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface6->SetTitleAndTooltip("Data channel 6", "Data channel 6 (D6)");
	mDataChannelInterface6->SetChannel(mDataChannel6);
	mDataChannelInterface6->SetSelectionOfNoneIsAllowed(true);
	mDataChannelInterface7.reset(new AnalyzerSettingInterfaceChannel());
	mDataChannelInterface7->SetTitleAndTooltip("Data channel 7", "Data channel 7 (D7)");
	mDataChannelInterface7->SetChannel(mDataChannel7);
	mDataChannelInterface7->SetSelectionOfNoneIsAllowed(true);

	mSampleEdgeInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mSampleEdgeInterface->SetTitleAndTooltip("Sample edge", "Clock sampling edge");
	mSampleEdgeInterface->AddNumber(SAMPLE_EDGE_RISING,  "Rising",  "Sample on rising edge");
	mSampleEdgeInterface->AddNumber(SAMPLE_EDGE_FALLING, "Falling", "Sample on falling edge");
	mSampleEdgeInterface->SetNumber(mSampleEdge);

	AddInterface(mClockChannelInterface.get());
	AddInterface(mCommandChannelInterface.get());
	AddInterface(mProtocolInterface.get());
	AddInterface(mBusWidthInterface.get());
	AddInterface(mSampleEdgeInterface.get());
	AddInterface(mDataChannelInterface0.get());
	AddInterface(mDataChannelInterface1.get());
	AddInterface(mDataChannelInterface2.get());
	AddInterface(mDataChannelInterface3.get());
	AddInterface(mDataChannelInterface4.get());
	AddInterface(mDataChannelInterface5.get());
	AddInterface(mDataChannelInterface6.get());
	AddInterface(mDataChannelInterface7.get());

	AddExportOption(0, "Export as text file");
	AddExportExtension(0, "text", "txt");

	ClearChannels();
	AddChannel(mClockChannel, "Clock", false);
	AddChannel(mCommandChannel, "Command", false);
	AddChannel(mDataChannel0, "Data channel 0", false);
	AddChannel(mDataChannel1, "Data channel 1", false);
	AddChannel(mDataChannel2, "Data channel 2", false);
	AddChannel(mDataChannel3, "Data channel 3", false);
	AddChannel(mDataChannel4, "Data channel 4", false);
	AddChannel(mDataChannel5, "Data channel 5", false);
	AddChannel(mDataChannel6, "Data channel 6", false);
	AddChannel(mDataChannel7, "Data channel 7", false);
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
	Channel data4 = mDataChannelInterface4->GetChannel();
	Channel data5 = mDataChannelInterface5->GetChannel();
	Channel data6 = mDataChannelInterface6->GetChannel();
	Channel data7 = mDataChannelInterface7->GetChannel();

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
	mDataChannel4 = data4;
	mDataChannel5 = data5;
	mDataChannel6 = data6;
	mDataChannel7 = data7;
	mProtocol = SDMMCProtocol((U32)mProtocolInterface->GetNumber());
	mBusWidth = SDMMCBusWidth((U32)mBusWidthInterface->GetNumber());
	mSampleEdge = SDMMCSampleEdge((U32)mSampleEdgeInterface->GetNumber());

	if (mBusWidth == BUS_WIDTH_1 && mDataChannel0 == UNDEFINED_CHANNEL) {
		SetErrorText("Please select a channel for D0 or set bus width to 0.");
		return false;
	}
	if (mBusWidth == BUS_WIDTH_4 &&
			(mDataChannel0 == UNDEFINED_CHANNEL ||
			 mDataChannel1 == UNDEFINED_CHANNEL ||
			 mDataChannel2 == UNDEFINED_CHANNEL ||
			 mDataChannel3 == UNDEFINED_CHANNEL)) {
		SetErrorText("Please select a channel for D0, D1, D2 and D3 or change bus width.");
		return false;
	}
	if (mBusWidth == BUS_WIDTH_8 &&
			(mDataChannel0 == UNDEFINED_CHANNEL ||
			 mDataChannel1 == UNDEFINED_CHANNEL ||
			 mDataChannel2 == UNDEFINED_CHANNEL ||
			 mDataChannel3 == UNDEFINED_CHANNEL ||
			 mDataChannel4 == UNDEFINED_CHANNEL ||
			 mDataChannel5 == UNDEFINED_CHANNEL ||
			 mDataChannel6 == UNDEFINED_CHANNEL ||
			 mDataChannel7 == UNDEFINED_CHANNEL)) {
		SetErrorText("Please select a channel for all data channels (D*) or change bus width.");
		return false;
	}

	ClearChannels();
	AddChannel(mClockChannel, "Clock", true);
	AddChannel(mCommandChannel, "Command", true);
	AddChannel(mDataChannel0, "Data channel 0", true);
	AddChannel(mDataChannel1, "Data channel 1", true);
	AddChannel(mDataChannel2, "Data channel 2", true);
	AddChannel(mDataChannel3, "Data channel 3", true);
	AddChannel(mDataChannel4, "Data channel 4", true);
	AddChannel(mDataChannel5, "Data channel 5", true);
	AddChannel(mDataChannel6, "Data channel 6", true);
	AddChannel(mDataChannel7, "Data channel 7", true);

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
	mDataChannelInterface4->SetChannel(mDataChannel4);
	mDataChannelInterface5->SetChannel(mDataChannel5);
	mDataChannelInterface6->SetChannel(mDataChannel6);
	mDataChannelInterface7->SetChannel(mDataChannel7);
	mProtocolInterface->SetNumber(mProtocol);
	mBusWidthInterface->SetNumber(mBusWidth);
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
	archive >> mDataChannel4;
	archive >> mDataChannel5;
	archive >> mDataChannel6;
	archive >> mDataChannel7;
	archive >> tmp; mProtocol = SDMMCProtocol(tmp);
	archive >> tmp; mBusWidth = SDMMCBusWidth(tmp);
	archive >> tmp; mSampleEdge = SDMMCSampleEdge(tmp);

	ClearChannels();
	AddChannel(mClockChannel, "Clock", true);
	AddChannel(mCommandChannel, "Command", true);
	AddChannel(mDataChannel0, "Data channel 0", true);
	AddChannel(mDataChannel1, "Data channel 1", true);
	AddChannel(mDataChannel2, "Data channel 2", true);
	AddChannel(mDataChannel3, "Data channel 3", true);
	AddChannel(mDataChannel4, "Data channel 3", true);
	AddChannel(mDataChannel5, "Data channel 3", true);
	AddChannel(mDataChannel6, "Data channel 3", true);
	AddChannel(mDataChannel7, "Data channel 3", true);

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
	archive << mDataChannel4;
	archive << mDataChannel5;
	archive << mDataChannel6;
	archive << mDataChannel7;
	archive << mProtocol;
	archive << mBusWidth;
	archive << mSampleEdge;

	return SetReturnString(archive.GetString());
}
