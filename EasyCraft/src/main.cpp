#include "hpp/hooks.hpp"

void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
	if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
		if (EZCSettings::bCraftEnabled) EZCHooks::COBJ_Hook::Install();
		if (EZCSettings::bLegendaryEnabled) EZCHooks::LGDI_Hook::Install();
		if (EZCSettings::bResearchEnabled) EZCHooks::RSPJ_Hook::Install();
		EZCHooks::ReloadHookA::Install();
		EZCHooks::ReloadHookB::Install();
	} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
		EZCHooks::EventHandler::Install();
		EZCActions::OnDataLoaded();
	}
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 32
	};
	SFSE::Init(a_sfse, info);

	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);

	const auto messaging = SFSE::GetMessagingInterface();
	if (messaging && messaging->RegisterListener(MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}

	EZCSettings::LoadConfig();

	return true;
}