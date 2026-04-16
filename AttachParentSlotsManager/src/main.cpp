#include "hpp/papyrus.hpp"

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* sfse) {
	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 32
	};
	SFSE::Init(sfse, info);
	auto gameVersion = sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameVersion);
	return true;
}
