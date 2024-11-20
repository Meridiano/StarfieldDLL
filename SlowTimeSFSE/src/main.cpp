#include "hpp/process.hpp"

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {

	SFSE::Init(a_sfse, false);
	SFSE::AllocTrampoline(128);

	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);
	
	// read settings
	SlowTimeSettings::LoadSettings();

	// register stuff
	const auto messagingInterface = SFSE::GetMessagingInterface();
	if (messagingInterface && messagingInterface->RegisterListener(SlowTimeProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}
	return true;
}