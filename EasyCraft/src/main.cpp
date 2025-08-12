#include "hpp/hooks.hpp"

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
		if (EZCSettings::bCraftEnabled) EZCHooks::COBJ_Hook::Install();
		if (EZCSettings::bResearchEnabled) EZCHooks::RSPJ_Hook::Install();
		EZCHooks::ReloadHookA::Install();
		EZCHooks::ReloadHookB::Install();
	} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
		EZCProcess::ProcessForms("DataLoaded");
		EZCHooks::EventHandler::Install();
	}
}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 64
	};
	SFSE::Init(a_sfse, info);

	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);

	const auto messagingInterface = SFSE::GetMessagingInterface();
	if (messagingInterface && messagingInterface->RegisterListener(MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}

	EZCSettings::LoadConfig();

	return true;
}
