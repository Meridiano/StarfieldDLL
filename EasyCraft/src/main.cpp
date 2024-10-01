namespace EZCUtility {

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (a_name.size() > 0 && a_offset > 0) {
			auto sName = RE::BSFixedString(a_name);
			auto iOffset = std::int32_t(a_offset);
			using type = RE::TESForm*(*)(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
			static REL::Relocation<type> func{ REL::ID(171055) };
			return func(NULL, NULL, NULL, iOffset, &sName);
		}
		return nullptr;
	}

}

namespace EZCSettings {

	bool bCraftEnabled = false;
	bool bResearchEnabled = false;
	std::string sCreditsPlugin = "";
	std::uint32_t iCreditsID = 0;

	bool ConfigBool(mINI::INIStructure ini, std::string section, std::string key, bool fallback) {
		bool result = fallback;
		std::string raw = ini.get(section).get(key);
		// lambda
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

	std::string ConfigString(mINI::INIStructure ini, std::string section, std::string key, std::string fallback) {
		auto result = fallback;
		if (auto map = ini.get(section); map.has(key)) result = map.get(key);
		else logs::info("Failed to read [{}]{} ini value", section, key);
		logs::info("String value [{}]{} is \"{}\"", section, key, result);
		return result;
	}

	std::uint32_t ConfigUInt32(mINI::INIStructure ini, std::string section, std::string key, std::uint32_t fallback) {
		auto result = fallback;
		std::string raw = ini.get(section).get(key);
		try {
			result = std::stoul(raw, nullptr, 0);
		} catch (...) {
			logs::info("Failed to read [{}]{} ini value", section, key);
		}
		logs::info("Integer value [{}]{} is {}", section, key, result);
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
			// credits
			sCreditsPlugin = ConfigString(ini, "General", "sCreditsPlugin", "");
			iCreditsID = ConfigUInt32(ini, "General", "iCreditsID", 0);
		} else logs::info("Config read error, all settings disabled");
	}

}

namespace EZCProcess {

	std::set<RE::TESForm*> exceptions;

	void SetupExceptions(RE::TESDataHandler* tesDH) {
		exceptions.clear();
		if (!tesDH) return;
		namespace fs = std::filesystem;
		fs::path dirPath = "Data/SFSE/Plugins";
		if (fs::exists(dirPath)) {
			std::string type = ".ini";
			for (fs::directory_entry fileEntry : fs::directory_iterator(dirPath)) {
				fs::path filePath = fileEntry.path();
				if (fs::is_regular_file(filePath) && filePath.extension() == type) {
					auto fileName = filePath.filename().string();
					if (fileName.length() > 24 && strnicmp(fileName.data(), "EasyCraft.Exception.", 20) == 0) {
						logs::info("Reading exception file >> {}", fileName);
						// read ini
						mINI::INIFile file(filePath.string());
						mINI::INIStructure ini;
						if (file.read(ini)) {
							for (auto sectionIterator : ini) {
								auto section = sectionIterator.first;
								for (auto keyIterator : sectionIterator.second) {
									auto key = keyIterator.first;
									auto value = keyIterator.second;
									std::uint32_t valueUInt32 = 0;
									try {
										valueUInt32 = std::stoul(value, nullptr, 0);
										if (auto form = EZCUtility::GetFormFromFile(section, valueUInt32); form) {
											exceptions.insert(form);
											logs::info("Exception added >> {}.{:X}", RE::FormTypeToString(form->GetFormType()), form->formID);
										}
									} catch (...) {
										logs::info("Bad value >> {}|{}|{}", section, key, value);
									}
								}
							}
						} else logs::info("Bad ini-file structure >> {}", fileName);
					}
				}
			}
		}
	}

	using comp_t = RE::BSTTuple3<RE::TESForm*, RE::BGSCurveForm*, RE::BGSTypedFormValuePair::SharedVal>;
	bool SetOneCredit(RE::BSTArray<comp_t>* comp) {
		if (comp && comp->size() > 0) {
			if (auto form = EZCUtility::GetFormFromFile(EZCSettings::sCreditsPlugin, EZCSettings::iCreditsID); form) {
				comp->clear();
				RE::BGSTypedFormValuePair::SharedVal shared = { 1 };
				comp_t credit = { form, nullptr, shared };
				comp->push_back(credit);
				return true;
			}
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
					if (exceptions.contains(cobj)) continue;
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
					if (exceptions.contains(rspj)) continue;
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
		SetupExceptions(tesDataHandler);
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
			const REL::Relocation<std::uintptr_t> reloc{ REL::ID(148887), 0x17A8 };
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
