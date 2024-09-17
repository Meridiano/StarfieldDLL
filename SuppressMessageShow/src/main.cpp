namespace SMSUtility {

	template<typename Func>
	auto WriteFunctionHook(std::uint64_t id, std::size_t byteCopyCount, Func destination) {
		const REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
		struct XPatch: Xbyak::CodeGenerator {
			using ull = unsigned long long;
			using uch = unsigned char;
			XPatch(std::uintptr_t originalFuncAddr, ull originalByteLength, ull newByteLength):
			Xbyak::CodeGenerator(originalByteLength + newByteLength, TRAMPOLINE.allocate(originalByteLength + newByteLength)) {
				auto byteAddr = reinterpret_cast<uch*>(originalFuncAddr);
				for (ull i = 0; i < originalByteLength; i++) db(*byteAddr++);
				jmp(qword[rip]);
				dq(ull(byteAddr));
			}
		};
		XPatch patch(target.address(), byteCopyCount, 20);
		patch.ready();
		auto patchSize = patch.getSize();
		TRAMPOLINE.write_branch<5>(target.address(), destination);
		auto alloc = TRAMPOLINE.allocate(patchSize);
		memcpy(alloc, patch.getCode(), patchSize);
		return reinterpret_cast<std::uintptr_t>(alloc);
	}

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (auto tesDH = RE::TESDataHandler::GetSingleton(); tesDH) {
			// set lambdas
			using type = RE::BSTArray<RE::TESFile*>;
			auto GetArray = [](RE::TESDataHandler* base, std::ptrdiff_t offset) {
				auto address = std::uintptr_t(base) + offset;
				auto reloc = REL::Relocation<type*>(address);
				return reloc.get();
			};
			auto GetData = [](type* a1, std::string a2, std::uint32_t a3, std::uint8_t a4) {
				std::pair<std::uint32_t, std::uint8_t> result = { 0, 0 };
				for (auto file : *a1) {
					if (auto name = std::string(file->fileName); stricmp(a2.data(), name.data()) == 0) {
						result = { a3, a4 };
						break;
					}
					a3 += 1;
				}
				return result;
			};
			auto GetForm = [](std::pair<std::uint32_t, std::uint8_t> a1, std::uint32_t a2) {
				auto id = (a1.first << a1.second) + a2;
				return RE::TESForm::LookupByID(id);
			};
			// is full
			auto full = GetArray(tesDH, 0x1548);
			auto fullData = GetData(full, a_name, 0x00, 24);
			if (fullData.second != 0) return GetForm(fullData, a_offset);
			// is medium
			auto medium = GetArray(tesDH, 0x1568);
			auto mediumData = GetData(medium, a_name, 0xFD00, 16);
			if (mediumData.second != 0) return GetForm(mediumData, a_offset);
			// is small
			auto small = GetArray(tesDH, 0x1558);
			auto smallData = GetData(small, a_name, 0xFE000, 12);
			if (smallData.second != 0) return GetForm(smallData, a_offset);
		}
		return nullptr;
	}

}

namespace SMSForms {

	std::set<RE::BGSMessage*> toSuppress;

	void LoadForms(std::string src) {
		toSuppress.clear();
		if (RE::TESDataHandler::GetSingleton()) {
			namespace fs = std::filesystem;
			fs::path dirPath = "Data/SFSE/Plugins/SuppressMessageShow";
			if (fs::exists(dirPath)) {
				std::string type = ".ini";
				for (fs::directory_entry fileEntry : fs::directory_iterator(dirPath)) {
					fs::path filePath = fileEntry.path();
					if (fs::is_regular_file(filePath) && filePath.extension() == type) {
						auto fileName = filePath.filename().string();
						logs::info("Reading config file >> {}", fileName);
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
										auto form = SMSUtility::GetFormFromFile(section, valueUInt32);
										if (form && form->GetFormType() == RE::FormType::kMESG) {
											toSuppress.insert(reinterpret_cast<RE::BGSMessage*>(form));
											logs::info("Message added >> {:X}", form->formID);
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

}

namespace SMSHooks {

	class Show {
	private:
		struct Function {
			static std::int32_t Modified(std::int64_t a1, std::int64_t a2,
										 RE::BGSMessage* message,
										 float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8, float arg9) {
				if (SMSForms::toSuppress.contains(message)) {
					logs::info("Suppress Message.Show >> {:X}", message->formID);
					return 0;
				}
				return Original(a1, a2, message, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
			}
			static inline REL::Relocation<decltype(Modified)> Original;
		};
	public:
		static void Install() {
			// 48 8B C4 + 48 89 58 20
			Function::Original = SMSUtility::WriteFunctionHook(170694, 7, Function::Modified);
			logs::info("Show hook installed");
		}
	};

	class ShowAsHelpMessage {
	private:
		struct Function {
			static std::int64_t Modified(std::int64_t a1, std::int64_t a2,
										 RE::BGSMessage* message,
										 RE::BSFixedString* eventName, float duration, float interval, std::int32_t maxTimes, RE::BSFixedString* context, std::int32_t priority, RE::BGSMessage* gamepadMessage) {
				if (SMSForms::toSuppress.contains(message)) {
					logs::info("Suppress Message.ShowAsHelpMessage >> {:X}", message->formID);
					return 0;
				} else if (SMSForms::toSuppress.contains(gamepadMessage)) {
					logs::info("Suppress Message.ShowAsHelpMessage >> {:X}", gamepadMessage->formID);
					return 0;
				}
				return Original(a1, a2, message, eventName, duration, interval, maxTimes, context, priority, gamepadMessage);
			}
			static inline REL::Relocation<decltype(Modified)> Original;
		};
	public:
		static void Install() {
			// 48 83 EC 58 + 48 8B 84 24 A8 00 00 00
			Function::Original = SMSUtility::WriteFunctionHook(170696, 12, Function::Modified);
			logs::info("ShowAsHelpMessage hook installed");
		}
	};

	class DataReloaded {
	private:
		struct Call {
			static std::int64_t Modified(std::int64_t arg) {
				auto out = Original(arg);
				SMSForms::LoadForms("DataReloaded");
				return out;
			}
			static inline REL::Relocation<decltype(Modified)> Original;
		};
	public:
		static void Install() {
			// ID 148887 + Offset 17XX = Call ID 148635
			const REL::Relocation<std::uintptr_t> reloc{ REL::ID(148887), 0x177E };
			Call::Original = TRAMPOLINE.write_call<5>(reloc.address(), Call::Modified);
			logs::info("DataReloaded hook installed");
		}
	};

	void InstallHooks() {
		Show::Install();
		ShowAsHelpMessage::Install();
		DataReloaded::Install();
	}

}

namespace SMSProcess {

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostLoad) {
			logs::info("Plugin loaded, installing hooks");
			SMSHooks::InstallHooks();
		} else if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			logs::info("Data loaded, reading configs");
			SMSForms::LoadForms("DataLoaded");

		} else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse);
	SFSE::AllocTrampoline(256);

	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);

	const auto sfseMessage = SFSE::GetMessagingInterface();
	if (sfseMessage && sfseMessage->RegisterListener(SMSProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}

	return true;
}
