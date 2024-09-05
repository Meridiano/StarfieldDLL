namespace EZCUtility {

	bool EqualStrings(std::string a, std::string b, bool noCase) {
		auto length = a.length();
		if (b.length() == length) {
			if (noCase) return (strnicmp(a.data(), b.data(), length) == 0);
			else return (strncmp(a.data(), b.data(), length) == 0);
		}
		return false;
	}

	// { success, value }
	std::pair<bool, bool> StringToBool(std::string str) {
		bool t = EqualStrings(str, "true", true) || EqualStrings(str, "1", true);
		if (t) return { t, true };
		bool f = EqualStrings(str, "false", true) || EqualStrings(str, "0", true);
		if (f) return { f, false };
		// bruh
		return { false, false };
	}

}

namespace EZCSettings {

	bool bCraftEnabled = false;
	bool bResearchEnabled = false;

	bool ConfigBool(mINI::INIStructure ini, std::string section, std::string key, bool fallback) {
		bool result = fallback;
		std::string raw = ini.get(section).get(key);
		auto boolPair = EZCUtility::StringToBool(raw);
		if (boolPair.first) result = boolPair.second;
		else logs::info("Failed to read [{}]{} ini value", section, key);
		logs::info("Bool value [{}]{} is {}", section, key, result);
		return result;
	}

	void LoadConfig() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\EasyCraft.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
			// craft
			bCraftEnabled = ConfigBool(ini, "General", "bCraftEnabled", false);
			// research
			bResearchEnabled = ConfigBool(ini, "General", "bResearchEnabled", false);
		} else logs::info("Config read error, all settings disabled");
	}

}

namespace EZCProcess {

	using comp_t = RE::BSTTuple3<RE::TESForm*, RE::BGSCurveForm*, RE::BGSTypedFormValuePair::SharedVal>;
	bool SetOneCredit(RE::BSTArray<comp_t>* comp) {
		if (comp && comp->size() > 0) {
			comp->clear();
			RE::BGSTypedFormValuePair::SharedVal shared = { 1 };
			comp_t credit = {
				RE::TESForm::LookupByID(0xF), nullptr, shared
			};
			comp->push_back(credit);
			return true;
		}
		return false;
	}

	std::uint32_t ProcessCOBJ(RE::TESDataHandler* tesDH) {
		std::uint32_t result = 0;
		if (tesDH) {
			auto& cobjArray = tesDH->formArrays[0x97];
			cobjArray.lock.lock_write();
			for (auto& niPtr : cobjArray.formArray) {
				if (auto cobj = niPtr.get()->As<RE::BGSConstructibleObject>(); cobj) {
					if (auto comp = cobj->components; SetOneCredit(comp)) result += 1;
				}
			}
			cobjArray.lock.unlock_write();
		}
		return result;
	}

	std::uint32_t ProcessRSPJ(RE::TESDataHandler* tesDH) {
		std::uint32_t result = 0;
		if (tesDH) {
			auto& rspjArray = tesDH->formArrays[0xC1];
			rspjArray.lock.lock_write();
			for (auto& niPtr : rspjArray.formArray) {
				if (auto rspj = niPtr.get()->As<RE::BGSResearchProjectForm>(); rspj) {
					if (auto comp = rspj->components; SetOneCredit(comp)) result += 1;
				}
			}
			rspjArray.lock.unlock_write();
		}
		return result;
	}

	void ProcessForms(const char* src) {
		logs::info("ProcessForms:{}", src);
		auto tesDataHandler = RE::TESDataHandler::GetSingleton();
		// do cobj
		if (EZCSettings::bCraftEnabled) {
			auto cobjCount = ProcessCOBJ(tesDataHandler);
			logs::info("COBJ / Total affected count = {}", cobjCount);
		}
		// do rspj
		if (EZCSettings::bResearchEnabled) {
			auto rspjCount = ProcessRSPJ(tesDataHandler);
			logs::info("RSPJ / Total affected count = {}", rspjCount);
		}
	}
	
	// ID 148887 + Offset 17XX = Call ID 148635
	class ReloadHook {
	private:
		struct ReloadMod {
			static std::int64_t thunk(std::int64_t in) {
				// original
				auto out = func(in);
				// custom
				ProcessForms("DataReloaded");
				// done
				return out;
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};
	public:
		static void Install() {
			const REL::Relocation<std::uintptr_t> reloc{ REL::ID(148887), 0x177E };
			SFSE::stl::write_thunk_call<ReloadMod>(reloc.address());
			logs::info("Reload hook installed");
		}
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			ReloadHook::Install();
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			ProcessForms("DataLoaded");
		}
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse);
	SFSE::AllocTrampoline(32);

	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);

	const auto sfseMessagingInterface = SFSE::GetMessagingInterface();
	if (sfseMessagingInterface && sfseMessagingInterface->RegisterListener(EZCProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}

	EZCSettings::LoadConfig();

	return true;
}
