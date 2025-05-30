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

	std::string StringToLower(std::string str) {
		std::string out = str;
		std::transform(
			str.begin(), str.end(), out.begin(),
			[](unsigned char uch) { return std::tolower(uch); }
		);
		return out;
	}

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		auto reloc = REL::Relocation<T*>(address);
		return reloc.get();
	}

	RE::TESFile* LookupPlugin(std::uint8_t type, std::uint32_t id) {
		if (static auto tesDH = RE::TESDataHandler::GetSingleton(); tesDH) {
			using list = RE::BSTArray<RE::TESFile*>;
			switch (type) {
				case 0: {
					auto full = GetMember<list>(tesDH, 0x1550);
					if (full->size() > id) return full->operator[](id);
				}   break;
				case 1: {
					auto small = GetMember<list>(tesDH, 0x1560);
					if (small->size() > id) return small->operator[](id);
				}   break;
				case 2: {
					auto medium = GetMember<list>(tesDH, 0x1570);
					if (medium->size() > id) return medium->operator[](id);
				}   break;
			}
		}
		return nullptr;
	}

	std::string FormToString(RE::TESForm* form) {
		if (form) {
			RE::TESFile* plugin = nullptr;
			auto id = form->formID;
			switch (id >> 24) {
				case 0xFF:
					break;
				case 0xFE:
					plugin = LookupPlugin(1, (id >> 12) & 0xFFF);
					id = id & 0xFFF;
					break;
				case 0xFD:
					plugin = LookupPlugin(2, (id >> 16) & 0xFF);
					id = id & 0xFFFF;
					break;
				default:
					plugin = LookupPlugin(0, id >> 24);
					id = id & 0xFFFFFF;
					break;
			}
			if (plugin) return std::format(FormString, StringToLower(plugin->fileName), id);
		}
		return "";
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

	std::set<std::string> exceptions;
	std::uint32_t creditsError = 0;

	void SetupExceptions() {
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
										auto element = std::format(FormString, section, valueUInt32);
										if (exceptions.count(element) == 0) {
											exceptions.insert(element);
											REX::INFO("Exception added >> {}", element);
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
			if (auto form = EZCUtility::GetFormFromFile(EZCSettings::sCreditsPlugin, EZCSettings::iCreditsID); form) {
				comp->clear();
				RE::BGSTypedFormValuePair::SharedVal shared{ 1 };
				comp_t credit{ form, nullptr, shared };
				comp->push_back(credit);
				return true;
			} else creditsError += 1;
		}
		return false;
	}

	std::uint32_t cobjCount = 0;
	std::uint32_t rspjCount = 0;
	std::uint32_t cobjCountExcept = 0;
	std::uint32_t rspjCountExcept = 0;

	bool ProcessSingleCOBJ(RE::BGSConstructibleObject* cobj) {
		auto str = EZCUtility::FormToString(cobj);
		if (exceptions.contains(str)) {
			cobjCountExcept += 1;
			return false;
		}
		return SetOneCredit(cobj->components);
	}

	bool ProcessSingleRSPJ(RE::BGSResearchProjectForm* rspj) {
		auto str = EZCUtility::FormToString(rspj);
		if (exceptions.contains(str)) {
			rspjCountExcept += 1;
			return false;
		}
		return SetOneCredit(rspj->components);
	}

	void ReportAndReset(const char* arg) {
		REX::INFO("Report:{}", arg);
		if (creditsError) REX::INFO("Credits form not found in {} attempt(s)", creditsError);
		REX::INFO("COBJ / Total {} affected / With {} exception(s)", cobjCount, cobjCountExcept);
		REX::INFO("RSPJ / Total {} affected / With {} exception(s)", rspjCount, rspjCountExcept);
		creditsError = cobjCount = rspjCount = cobjCountExcept = rspjCountExcept = 0;
	}

	class COBJ_Hook {
	private:
		struct Call {
			static void NEW(RE::BGSConstructibleObject* cobj) {
				OLD(cobj);
				cobjCount += ProcessSingleCOBJ(cobj);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(410228) };
			Call::OLD = reloc.write_vfunc(0x1F, Call::NEW);
			REX::INFO("COBJ hook installed");
		}
	};

	class RSPJ_Hook {
	private:
		struct Call {
			static void NEW(RE::BGSResearchProjectForm* rspj) {
				OLD(rspj);
				rspjCount += ProcessSingleRSPJ(rspj);
			}
			static inline REL::Relocation<decltype(NEW)> OLD;
		};
	public:
		static void Install() {
			REL::Relocation reloc{ REL::ID(405103) };
			Call::OLD = reloc.write_vfunc(0x1F, Call::NEW);
			REX::INFO("RSPJ hook installed");
		}
	};

	class ReloadHook {
	private:
		struct Call {
			static std::int64_t NEW() {
				ReportAndReset("DataReloaded");
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
			EZCProcess::SetupExceptions();
			if (EZCSettings::bCraftEnabled) COBJ_Hook::Install();
			if (EZCSettings::bResearchEnabled) RSPJ_Hook::Install();
			ReloadHook::Install();
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			ReportAndReset("DataLoaded");
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
