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
			auto base = avo->GetPermanentActorValue(*avi);
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

	template<typename T>
	auto ConvertTo(std::string raw) {
		auto StringToBool = [](std::string str) {
			switch (str.length()) {
				case 1:
					if (!str.compare("1")) return true;
					if (!str.compare("0")) return false;
					break;
				case 4:
					if (!strnicmp(str.data(), "true", 4)) return true;
					break;
				case 5:
					if (!strnicmp(str.data(), "false", 5)) return false;
					break;
			}
			throw std::exception("non-boolean string argument");
		};
		T val = T{};
		bool suc = true;
		while (suc) try {
	#define TRY_TYPE(TYPE, FUNC) if constexpr (std::is_same<T, TYPE>::value) { val = FUNC; break; }
			TRY_TYPE(bool, StringToBool(raw));
			TRY_TYPE(std::int64_t, std::stoll(raw, nullptr, 0));
			TRY_TYPE(std::uint64_t, std::stoull(raw, nullptr, 0));
			TRY_TYPE(std::int32_t, std::stol(raw, nullptr, 0));
			TRY_TYPE(std::uint32_t, std::stoul(raw, nullptr, 0));
			TRY_TYPE(std::int16_t, std::stol(raw, nullptr, 0) & 0xFFFF);
			TRY_TYPE(std::uint16_t, std::stoul(raw, nullptr, 0) & 0xFFFF);
			TRY_TYPE(std::int8_t, std::stol(raw, nullptr, 0) & 0xFF);
			TRY_TYPE(std::uint8_t, std::stoul(raw, nullptr, 0) & 0xFF);
			TRY_TYPE(float, std::stof(raw, nullptr));
			TRY_TYPE(double, std::stod(raw, nullptr));
			TRY_TYPE(std::string, raw);
	#undef TRY_TYPE
			throw std::exception("unknown template type");
		} catch (...) { suc = false; }
		return std::pair(suc, val);
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

	template<typename T>
	auto Config(mINI::INIStructure ini, std::string section, std::string key, T fallback) {
		T result = fallback;
		if (auto map = ini.get(section); map.has(key)) {
			std::string raw = map.get(key);
			if (auto temp = IBFUtility::ConvertTo<T>(raw); temp.first) result = temp.second;
			else logs::info("Failed to read [{}]{} config option", section, key);
		} else logs::info("Config option [{}]{} not found", section, key);
		logs::info("Config option [{}]{} = {}", section, key, result);
		return result;
	}

	void LoadSettings() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\InfiniteBoostFuel.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
	#define CONFIG(V, S, K) V = Config<decltype(V)>(ini, S, K, V)
			// general
			CONFIG(bBoostpack, "General", "bBoostpack");
			CONFIG(bSpaceship, "General", "bSpaceship");
			CONFIG(bVehicle, "General", "bVehicle");
	#undef CONFIG
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
		auto bDataLoaded = REL::Relocation<bool*>(REL::ID(881028)).get();
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
	SFSE::Init(a_sfse, false);
	
	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

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
