#pragma once
#include "hpp/utility.hpp"

namespace UCSettings {

	std::string sCutterForms = "Starfield.esm:0x16758|Starfield.esm:0x1F9662";
	bool bChangeAmmoCapacity = true;
	std::uint32_t iNewAmmoCapacity = 65535;
	bool bChangeEnergyDamage = false;
	std::uint32_t iNewEnergyDamage = 4000;
	bool bChangeMaxRange = true;
	float fNewMaxRange = 999.0F;

	template<typename T>
	auto Config(mINI::INIStructure ini, std::string section, std::string key, T fallback) {
		T result = fallback;
		if (auto map = ini.get(section); map.has(key)) {
			std::string raw = map.get(key);
			if (auto temp = UCUtility::ConvertTo<T>(raw); temp.first) result = temp.second;
			else REX::INFO("Failed to read [{}]{} config option", section, key);
		} else REX::INFO("Config option [{}]{} not found", section, key);
		REX::INFO("Config option [{}]{} = {}", section, key, result);
		return result;
	}

	void LoadSettings() {
		auto configPath = std::format("Data/SFSE/Plugins/{}.ini", SFSE::GetPluginName());
		mINI::INIFile file(configPath);
		if (mINI::INIStructure ini; file.read(ini)) {
			#define CONFIG(V, S, K) V = Config<decltype(V)>(ini, S, K, V)
			// general
			CONFIG(sCutterForms, "General", "sCutterForms");
			// ammo capacity
			CONFIG(bChangeAmmoCapacity, "AmmoCapacity", "bChangeAmmoCapacity");
			CONFIG(iNewAmmoCapacity, "AmmoCapacity", "iNewAmmoCapacity");
			// energy damage
			CONFIG(bChangeEnergyDamage, "EnergyDamage", "bChangeEnergyDamage");
			CONFIG(iNewEnergyDamage, "EnergyDamage", "iNewEnergyDamage");
			// max range
			CONFIG(bChangeMaxRange, "MaxRange", "bChangeMaxRange");
			CONFIG(fNewMaxRange, "MaxRange", "fNewMaxRange");
			#undef CONFIG
		} else REX::INFO("Config reading error, all settings remain default");
	}

}