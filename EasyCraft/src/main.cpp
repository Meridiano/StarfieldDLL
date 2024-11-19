namespace EZCUtility {

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (a_name.size() && a_offset) {
			auto sName = RE::BSFixedString(a_name);
			auto iOffset = std::int32_t(a_offset & 0xFFFFFF);
			using type = RE::TESForm*(*)(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
			static REL::Relocation<type> func{ REL::ID(171055) };
			return func(NULL, NULL, NULL, iOffset, &sName);
		}
		return nullptr;
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

}

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
			else logs::info("Failed to read [{}]{} config option", section, key);
		} else logs::info("Config option [{}]{} not found", section, key);
		logs::info("Config option [{}]{} = {}", section, key, result);
		return result;
	}

	void LoadConfig() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\EasyCraft.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
	#define CONFIG(V, S, K) V = Config<decltype(V)>(ini, S, K, V)
			// craft
			CONFIG(bCraftEnabled, "General", "bCraftEnabled");
			// research
			CONFIG(bResearchEnabled, "General", "bResearchEnabled");
			// credits
			CONFIG(sCreditsPlugin, "General", "sCreditsPlugin");
			CONFIG(iCreditsID, "General", "iCreditsID");
	#undef CONFIG
		} else logs::info("Config read error, all settings disabled");
	}

}

namespace EZCProcess {

	std::set<RE::TESForm*> exceptions;

	void SetupExceptions(RE::TESDataHandler* tesDH) {
		exceptions.clear();
		if (!tesDH) return;
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
									bool badValue = true;
									if (auto temp = EZCUtility::ConvertTo<std::uint32_t>(value); temp.first) {
										std::uint32_t valueUInt32 = temp.second;
										if (auto form = EZCUtility::GetFormFromFile(section, valueUInt32); form) {
											exceptions.insert(form);
											logs::info("Exception added >> {}.{:X}", RE::FormTypeToString(form->GetFormType()), form->formID);
											badValue = false;
										}
									}
									if (badValue) logs::info("Bad value >> {}|{}|{}", section, key, value);
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
		struct ReloadCall {
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
			const REL::Relocation target{ REL::ID(148887), 0x17A8 };
			SFSE::stl::write_thunk_call<ReloadCall>(target.address());
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
	SFSE::Init(a_sfse, false);
	SFSE::AllocTrampoline(32);

	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

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
