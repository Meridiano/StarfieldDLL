#include "hpp/process.hpp"

struct DataReloadedHook {
	static std::int64_t NEW() {
		std::string event = "DataReloaded";
		UCProcess::FindCutters(event);
		UCProcess::TweakCutters(event);
		return OLD();
	}
	static inline REL::THook OLD{ REL::ID(99468), 0x1907, NEW };
};

void MessageListener(SFSE::MessagingInterface::Message* a_msg) {
	switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			UCSettings::LoadSettings();
			break;
		case SFSE::MessagingInterface::kPostDataLoad: {
			std::string event = "DataLoaded";
			UCProcess::FindCutters(event);
			UCProcess::TweakCutters(event);
		}   break;
	}
}

SFSE_PLUGIN_LOAD(SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info{
		.trampoline = true,
		.trampolineSize = 16
	};
	SFSE::Init(a_sfse, info);
	auto gameVersion = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameVersion);
	auto sfseMessaging = SFSE::GetMessagingInterface();
	return sfseMessaging && sfseMessaging->RegisterListener(MessageListener);
}