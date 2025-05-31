namespace EZCUtility {

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (a_name.size() && a_offset) {
			auto sName = RE::BSFixedString(a_name);
			auto iOffset = std::int32_t(a_offset & 0xFFFFFF);
			using type = RE::TESForm*(*)(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
			static REL::Relocation<type> func{ REL::ID(117382) };
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

	std::string CustomFormType(RE::TESForm* form) {
		if (form) switch (form->formType.underlying()) {
			case COBJ:
				return "COBJ";
			case RSPJ:
				return "RSPJ";
			default:
				return "FORM";
		}
		return "NONE";
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

namespace EZCProcess {

	std::mutex hookLock;
	std::set<RE::TESForm*> exceptions;
	std::set<RE::BGSConstructibleObject*> cobjList;
	std::set<RE::BGSResearchProjectForm*> rspjList;
	RE::TESForm* credits = nullptr;

	void SetupExceptions(RE::TESDataHandler* tesDH) {
		exceptions.clear();
		if (!tesDH) return;
		fs::path dirPath = "Data/SFSE/Plugins";
		if (fs::exists(dirPath)) {
			std::string type = ".ini";
			for (fs::directory_entry fileEntry : fs::directory_iterator(dirPath)) {
				fs::path filePath = fileEntry.path();
				if (fileEntry.is_regular_file() && filePath.extension() == type) {
					auto fileName = filePath.filename().string();
					if (fileName.length() > 24 && strnicmp(fileName.data(), "EasyCraft.Exception.", 20) == 0) {
						REX::INFO("Reading exception file >> {}", fileName);
						// read ini
						mINI::INIFile file(filePath);
						if (mINI::INIStructure ini; file.read(ini)) {
							for (auto sectionIterator : ini) {
								auto section = sectionIterator.first;
								for (auto keyIterator : sectionIterator.second) {
									auto key = keyIterator.first;
									auto value = keyIterator.second;
									bool badValue = true;
									if (auto temp = EZCUtility::ConvertTo<std::uint32_t>(value); temp.first) {
										std::uint32_t valueUInt32 = temp.second;
										auto form = EZCUtility::GetFormFromFile(section, valueUInt32);
										if (form && exceptions.count(form) == 0) {
											exceptions.insert(form);
											REX::INFO("Exception added >> {}.{:X}", EZCUtility::CustomFormType(form), form->formID);
											badValue = false;
										}
									}
									if (badValue) REX::INFO("Bad value >> {}|{}|{}", section, key, value);
								}
							}
						} else REX::INFO("Bad ini-file structure >> {}", fileName);
					}
				}
			}
		}
	}

	using comp_t = RE::BSTTuple3<RE::TESForm*, RE::BGSCurveForm*, RE::BGSTypedFormValuePair::SharedVal>;
	bool SetOneCredit(RE::BSTArray<comp_t>* comp) {
		if (comp && comp->size() > 0) {
			comp->clear();
			RE::BGSTypedFormValuePair::SharedVal shared{ 1 };
			comp_t data{ credits, nullptr, shared };
			comp->push_back(data);
			return true;
		}
		return false;
	}

	std::uint32_t cobjCount = 0;
	std::uint32_t rspjCount = 0;
	std::uint32_t cobjCountExcept = 0;
	std::uint32_t rspjCountExcept = 0;

	bool ProcessSingleCOBJ(RE::BGSConstructibleObject* cobj) {
		if (exceptions.contains(cobj)) {
			cobjCountExcept += 1;
			return false;
		}
		return SetOneCredit(cobj->components);
	}

	bool ProcessSingleRSPJ(RE::BGSResearchProjectForm* rspj) {
		if (exceptions.contains(rspj)) {
			rspjCountExcept += 1;
			return false;
		}
		return SetOneCredit(rspj->components);
	}

	void ProcessForms(std::string info) {
		REX::INFO("ProcessForms:{}", info);
		auto tesDH = RE::TESDataHandler::GetSingleton();
		SetupExceptions(tesDH);
		if (tesDH) {
			if (credits = EZCUtility::GetFormFromFile(EZCSettings::sCreditsPlugin, EZCSettings::iCreditsID); credits) {
				for (auto cobj : cobjList) if (cobj) cobjCount += ProcessSingleCOBJ(cobj);
				REX::INFO("COBJ / Total {} affected / With {} exception(s)", cobjCount, cobjCountExcept);
				for (auto rspj : rspjList) if (rspj) rspjCount += ProcessSingleRSPJ(rspj);
				REX::INFO("RSPJ / Total {} affected / With {} exception(s)", rspjCount, rspjCountExcept);
			} else REX::INFO("Credits form not found");
		} else REX::INFO("TESDataHandler not found");
		cobjCount = rspjCount = cobjCountExcept = rspjCountExcept = 0;
		cobjList.clear(); rspjList.clear();
		credits = nullptr;
	}

	class COBJ_Hook {
	private:
		struct Virtual {
			static void NEW(RE::BGSConstructibleObject* cobj) {
				OLD(cobj);
				std::lock_guard<std::mutex> protect(hookLock);
				if (cobj && cobjList.count(cobj) == 0) cobjList.insert(cobj);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(410228) };
			Virtual::OLD = reloc.write_vfunc(0x1F, Virtual::NEW);
			REX::INFO("COBJ hook installed");
		}
	};

	class RSPJ_Hook {
	private:
		struct Virtual {
			static void NEW(RE::BGSResearchProjectForm* rspj) {
				OLD(rspj);
				std::lock_guard<std::mutex> protect(hookLock);
				if (rspj && rspjList.count(rspj) == 0) rspjList.insert(rspj);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(405103) };
			Virtual::OLD = reloc.write_vfunc(0x1F, Virtual::NEW);
			REX::INFO("RSPJ hook installed");
		}
	};

	class ReloadHook {
	private:
		struct Call {
			static std::int64_t NEW() {
				ProcessForms("DataReloaded");
				return OLD();
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(99468), 0x1907 };
			Call::OLD = reloc.write_call<5>(Call::NEW);
			REX::INFO("Reload hook installed");
		}
	};

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			if (EZCSettings::bCraftEnabled) COBJ_Hook::Install();
			if (EZCSettings::bResearchEnabled) RSPJ_Hook::Install();
			ReloadHook::Install();
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			ProcessForms("DataLoaded");
		}
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::InitInfo info{
		.logPattern = "%d.%m.%Y %H:%M:%S [%s:%#] %v",
		.trampoline = true,
		.trampolineSize = 64
	};
	SFSE::Init(a_sfse, info);

	const auto gameInfo = a_sfse->RuntimeVersion().string(".");
	REX::INFO("Starfield v{}", gameInfo);

	const auto messagingInterface = SFSE::GetMessagingInterface();
	if (messagingInterface && messagingInterface->RegisterListener(EZCProcess::MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}

	EZCSettings::LoadConfig();

	return true;
}
