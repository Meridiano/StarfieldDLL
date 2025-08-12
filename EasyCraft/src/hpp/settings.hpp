#pragma once

#include "hpp/utility.hpp"

namespace EZCSettings {

	bool bCraftEnabled = false;
	bool bResearchEnabled = false;
	std::string sCreditsPlugin = "";
	std::uint32_t iCreditsID = 0;

	template<typename T>
	auto Config(mINI::INIStructure ini, std::string section, std::string key, T fallback) {
		T result = fallback;
		if (auto map = ini.get(section); map.has(key)) {
			std::string raw = map.get(key);
			if (auto temp = EZCUtility::ConvertTo<T>(raw); temp.first) result = temp.second;
			else REX::INFO("Failed to read [{}]{} config option", section, key);
		} else REX::INFO("Config option [{}]{} not found", section, key);
		REX::INFO("Config option [{}]{} = {}", section, key, result);
		return result;
	}

	void LoadConfig() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\EasyCraft.ini");
		if (mINI::INIStructure ini; file.read(ini)) {
			#define CONFIG(V, S, K) V = Config<decltype(V)>(ini, S, K, V)
			// craft
			CONFIG(bCraftEnabled, "General", "bCraftEnabled");
			// research
			CONFIG(bResearchEnabled, "General", "bResearchEnabled");
			// credits
			CONFIG(sCreditsPlugin, "General", "sCreditsPlugin");
			CONFIG(iCreditsID, "General", "iCreditsID");
			#undef CONFIG
		} else REX::INFO("Config read error, all settings disabled");
	}

}