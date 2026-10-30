#include "hpp/hooks.hpp"

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] [T:%t] %v",
		.trampoline = false
	};
	SFSE::Init(a_sfse, info);
	auto gamePath = std::filesystem::path(REX::FModule::GetExecutingModule().GetFileName());
	auto gameName = gamePath.filename().replace_extension("").string();
	auto gameVersion = a_sfse->RuntimeVersion().string(".");
	REX::INFO("{} v{}", gameName, gameVersion);
	SPN_Utility::SetupStrings();
	SPN_Config::Load(true);
	SPN_Hooks::Install();
	return true;
}
