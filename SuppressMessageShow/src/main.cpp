namespace SMSUtility {

	template<typename Func>
	auto WriteFunctionHook(std::uint64_t id, std::size_t byteCopyCount, Func destination) {
		const REL::Relocation target{ REL::ID(id) };
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
				if (message && SMSForms::toSuppress.contains(message)) {
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
				if (message && SMSForms::toSuppress.contains(message)) {
					logs::info("Suppress Message.ShowAsHelpMessage >> {:X}", message->formID);
					return 0;
				} else if (gamepadMessage && SMSForms::toSuppress.contains(gamepadMessage)) {
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
			const REL::Relocation reloc{ REL::ID(148887), 0x17A8 };
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
