namespace IBFUtility {

	unsigned char CharToLower(unsigned char ch) {
		return std::tolower(ch);
	}

	std::string StringToLower(std::string toConvert) {
		std::string lower = toConvert;
		std::transform(toConvert.begin(), toConvert.end(), lower.begin(), CharToLower);
		return lower;
	}
	
	RE::ActorValueInfo* LocateActorValue(std::string edid) {
		auto form = RE::TESForm::LookupByEditorID(edid);
		if (form && form->formType == RE::FormType::kAVIF) {
			return static_cast<RE::ActorValueInfo*>(form);
		}
		return nullptr;
	}

	void RestoreBaseAV(RE::ActorValueOwner* avo, RE::ActorValueInfo* avi, float low) {
		if (avo && avi) {
			auto base = avo->GetBaseActorValue(*avi);
			auto value = avo->GetActorValue(*avi);
			if (base > 0.0F && base > value && value / base < low) {
				avo->RestoreActorValue(*avi, base);
			}
		}
	}

}

namespace IBFSettings {

	bool bBoostpack = true;
	bool bSpaceship = true;

	bool ConfigBool(mINI::INIStructure ini, std::string section, std::string key, bool fallback) {
		bool result = fallback;
		std::string raw = ini.get(section).get(key);
		std::string lower = IBFUtility::StringToLower(raw);
		if (lower.compare("true") == 0 || lower.compare("1") == 0) result = true;
		else if (lower.compare("false") == 0 || lower.compare("0") == 0) result = false;
		else logs::info("Failed to read [{}]{} ini value", section, key);
		logs::info("Bool value [{}]{} is {}", section, key, result);
		return result;
	}

	void LoadSettings() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\InfiniteBoostFuel.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
			// general
			bBoostpack = ConfigBool(ini, "General", "bBoostpack", true);
			bSpaceship = ConfigBool(ini, "General", "bSpaceship", true);
		} else {
			logs::info("Config read error, all settings set to default");
		}
	}

}

namespace IBFProcess {

	std::string BFuelID = "BoostpackFuel";
	std::string SFuelID = "SpaceshipBoostFuel";
	RE::ActorValueInfo* BFuelAV = nullptr;
	RE::ActorValueInfo* SFuelAV = nullptr;
	RE::PlayerCharacter* Player = nullptr;

	void BoostThread(std::uint32_t sleepTime) {
		auto bDataLoadedREL = REL::Relocation<bool*>{ REL::ID(881028) };
		auto bDataLoaded = bDataLoadedREL.get();
		while (true) {
			if (*bDataLoaded) {
				BFuelAV = IBFUtility::LocateActorValue(BFuelID);
				SFuelAV = IBFUtility::LocateActorValue(SFuelID);
				Player = RE::PlayerCharacter::GetSingleton();
				if (BFuelAV && SFuelAV && Player) {
					if (IBFSettings::bBoostpack) {
						IBFUtility::RestoreBaseAV(Player, BFuelAV, 0.7F);
					}
					if (IBFSettings::bSpaceship) {
						IBFUtility::RestoreBaseAV(Player->GetSpaceship(true), SFuelAV, 0.7F);
					}
				}
			}
			Sleep(sleepTime);
		}
	}

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			logs::info("Data loaded, creating new thread");
			std::thread boost(BoostThread, 100);
			boost.detach();
		} else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse);
	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);

	// read settings
	IBFSettings::LoadSettings();

	// register stuff
	const auto sfseMessagingInterface = SFSE::GetMessagingInterface();
	if (sfseMessagingInterface && sfseMessagingInterface->RegisterListener(IBFProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}
	return true;
}
