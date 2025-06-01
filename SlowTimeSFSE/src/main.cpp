#include "hpp/process.hpp"

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {

	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 128
	};
	SFSE::Init(a_sfse, info);
	
	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);
	
	// read settings
	SlowTimeSettings::LoadSettings();

	// register stuff
	const auto messagingInterface = SFSE::GetMessagingInterface();
	if (messagingInterface && messagingInterface->RegisterListener(SlowTimeProcess::MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}
	return true;
}