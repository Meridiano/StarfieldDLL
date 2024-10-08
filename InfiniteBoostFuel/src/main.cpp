namespace IBFUtility {

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

	RE::TESObjectREFR* GetFurnitureUsing(RE::Actor* arg) {
		using type = RE::TESObjectREFR*(*)(std::int64_t, std::int64_t, RE::Actor*);
		REL::Relocation<type> func{ REL::ID(170441) };
		return func(NULL, NULL, arg);
	}

	bool PlayerUsesVehicle(RE::PlayerCharacter* arg) {
		if (auto refr = GetFurnitureUsing(arg); refr) {
			if (auto bound = refr->GetBaseObject().get(); bound) {
				if (auto furn = bound->formType == RE::FormType::kFURN ? static_cast<RE::TESFurniture*>(bound) : nullptr; furn) {
					return furn->HasKeywordString("VehicleKeyword");
				}
			}
		}
		return false;
	}

	class GodMode {
	private:
		std::uintptr_t ptr = REL::ID(2242333).address();
	public:
		bool Get() {
			static auto get = REL::Relocation<bool*>(ptr).get();
			return *get;
		}
		void Set(bool val) {
			static auto set = REL::Relocation<bool*>(ptr).get();
			*set = val;
		}
	};

}

namespace IBFSettings {

	bool bBoostpack = true;
	bool bSpaceship = true;
	bool bVehicle = true;

	bool ConfigBool(mINI::INIStructure ini, std::string section, std::string key, bool fallback) {
		bool result = fallback;
		std::string raw = ini.get(section).get(key);
		auto StringToBool = [](std::string str) {
			auto StringToLower = [](std::string arg) {
				auto out = arg;
				std::transform(
					arg.begin(), arg.end(), out.begin(),
					[](unsigned char uch) { return std::tolower(uch); }
				);
				return out;
			};
			auto low = StringToLower(str);
			if (!low.compare("true") || !low.compare("1")) return true;
			if (!low.compare("false") || !low.compare("0")) return false;
			throw std::invalid_argument("non-boolean string argument");
		};
		try {
			result = StringToBool(raw);
		} catch (...) {
			logs::info("Failed to read [{}]{} ini value", section, key);
		}
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
			bVehicle = ConfigBool(ini, "General", "bVehicle", true);
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
	bool VehicleState = false;
	bool GodModeState = false;

	void ProcessVehicle() {
		bool NewVehicleState = IBFUtility::PlayerUsesVehicle(Player);
		if (NewVehicleState != VehicleState) {
			IBFUtility::GodMode tgm;
			if (NewVehicleState) {
				GodModeState = tgm.Get();
				tgm.Set(true);
			} else {
				tgm.Set(GodModeState);
			}
			VehicleState = NewVehicleState;
		}
	}

	void BoostThread(std::uint32_t sleepTime) {
		auto bDataLoaded = REL::Relocation<bool*>{ REL::ID(881028) }.get();
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
					if (IBFSettings::bVehicle) {
						ProcessVehicle();
					}
				}
			}
			Sleep(sleepTime);
		}
	}

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			logs::info("Data loaded, creating new thread");
			std::thread(BoostThread, 100).detach();
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
