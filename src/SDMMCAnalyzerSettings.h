#ifndef __SDMMC_ANALYZER_SETTINGS
#define __SDMMC_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum SDMMCProtocol {
	PROTOCOL_MMC,
	PROTOCOL_SD,
};

enum SDMMCBusWidth {
    BUS_WIDTH_0,
    BUS_WIDTH_1,
    BUS_WIDTH_4,
    BUS_WIDTH_8
};

enum SDMMCSampleEdge {
	SAMPLE_EDGE_RISING,
	SAMPLE_EDGE_FALLING
};

class SDMMCAnalyzerSettings : public AnalyzerSettings
{
public:
	SDMMCAnalyzerSettings();
	virtual ~SDMMCAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();

	virtual void LoadSettings(const char *settings);
	virtual const char *SaveSettings();

	Channel mClockChannel;
	Channel mCommandChannel;
	Channel mDataChannel0;
	Channel mDataChannel1;
	Channel mDataChannel2;
	Channel mDataChannel3;
	Channel mDataChannel4;
	Channel mDataChannel5;
	Channel mDataChannel6;
	Channel mDataChannel7;
	enum SDMMCProtocol mProtocol;
	enum SDMMCBusWidth mBusWidth;
	enum SDMMCSampleEdge mSampleEdge;

protected:
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mClockChannelInterface;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mCommandChannelInterface;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface0;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface1;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface2;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface3;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface4;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface5;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface6;
	std::auto_ptr<AnalyzerSettingInterfaceChannel> mDataChannelInterface7;
	std::auto_ptr<AnalyzerSettingInterfaceNumberList> mProtocolInterface;
	std::auto_ptr<AnalyzerSettingInterfaceNumberList> mBusWidthInterface;
	std::auto_ptr<AnalyzerSettingInterfaceNumberList> mSampleEdgeInterface;
};

#endif
