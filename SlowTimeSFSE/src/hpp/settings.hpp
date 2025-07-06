#pragma once
#include "hpp/utility.hpp"

namespace SlowTimeSettings {

	// general
	float fGlobalMult = 0.1F;
	float fPlayerMult = 1.0F;
	float fMouseMult = 1.0F;
	std::string sBlacklist = "DialogueMenu|LoadingMenu";
	// hotkey
	bool bGamepadMode = false;
	std::int32_t iHotkey = 112;
	std::int32_t iModifier = 0;
	// actor value
	bool bEnableAV = false;
	std::string sEditorID = "Oxygen";
	std::pair<bool, float> pLowValue = { true, 0.1F };
	std::pair<bool, float> pDamageValue = { false, 7.5F };
	// message
	std::string sMessageOn = "Slow-Time Enabled";
	std::string sMessageOff = "Slow-Time Disabled";

	template<typename T>
	auto Config(mINI::INIStructure ini, std::string section, std::string key, T fallback) {
		T result = fallback;
		if (auto map = ini.get(section); map.has(key)) {
			std::string raw = map.get(key);
			if (auto temp = SlowTimeUtility::ConvertTo<T>(raw); temp.first) result = temp.second;
			else REX::INFO("Failed to read [{}]{} config option", section, key);
		} else REX::INFO("Config option [{}]{} not found", section, key);
		REX::INFO("Config option [{}]{} = {}", section, key, result);
		return result;
	}

	template<typename T1, typename T2>
	auto ConfigPair(mINI::INIStructure ini, std::string section, std::string key, std::pair<T1, T2> fallback) {
		auto result = fallback;
		if (auto map = ini.get(section); map.has(key)) {
			std::string raw = map.get(key);
			if (auto v = SlowTimeUtility::Split(raw, ':'); v.size() == 2) {
				auto tempF = SlowTimeUtility::ConvertTo<T1>(v[0]);
				auto tempS = SlowTimeUtility::ConvertTo<T2>(v[1]);
				if (tempF.first && tempS.first) result = { tempF.second, tempS.second };
				else REX::INFO("Failed to read [{}]{} config option", section, key);
			} else REX::INFO("Config option [{}]{} is not a pair", section, key);
		} else REX::INFO("Config option [{}]{} not found", section, key);
		REX::INFO("Config option [{}]{} = {}:{}", section, key, result.first, result.second);
		return result;
	}

	void LoadSettings() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\SlowTimeSFSE.ini");
		if (mINI::INIStructure ini; file.read(ini)) {
			#define CONFIG(V, S, K) V = Config<decltype(V)>(ini, S, K, V)
			#define CONFIG_PAIR(V, S, K) V = ConfigPair<decltype(V.first), decltype(V.second)>(ini, S, K, V)
			// general
			CONFIG(fGlobalMult, "General", "fGlobalMult");
			CONFIG(fPlayerMult, "General", "fPlayerMult");
			CONFIG(fMouseMult, "General", "fMouseMult");
			CONFIG(sBlacklist, "General", "sBlacklist");
			// hotkey
			CONFIG(bGamepadMode, "Hotkey", "bGamepadMode");
			CONFIG(iHotkey, "Hotkey", "iHotkey");
			CONFIG(iModifier, "Hotkey", "iModifier");
			// actor value
			CONFIG(bEnableAV, "ActorValue", "bEnableAV");
			CONFIG(sEditorID, "ActorValue", "sEditorID");
			CONFIG_PAIR(pLowValue, "ActorValue", "pLowValue");
			CONFIG_PAIR(pDamageValue, "ActorValue", "pDamageValue");
			// message
			CONFIG(sMessageOn, "Message", "sMessageOn");
			CONFIG(sMessageOff, "Message", "sMessageOff");
			#undef CONFIG
			#undef CONFIG_PAIR
		} else REX::INFO("Config read error, all settings set to default");
	}

}