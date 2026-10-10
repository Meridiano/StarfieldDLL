#include "hpp/hooks.hpp"

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 128
	};
	SFSE::Init(a_sfse, info);
	auto gamePath = std::filesystem::path(REX::FModule::GetExecutingModule().GetFileName());
	auto gameName = gamePath.filename().replace_extension("").string();
	auto gameVersion = a_sfse->RuntimeVersion().string(".");
	REX::INFO("{} v{}", gameName, gameVersion);
	FCConfig::ParseConfigList();
	return true;
}
