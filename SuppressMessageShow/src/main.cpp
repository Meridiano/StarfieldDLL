namespace SMSUtility {

	template<typename Func>
	auto WriteFunctionHook(std::uint64_t id, std::size_t copyCount, Func destination) {
		const auto target = REL::ID(id).address();
		if (REL::Pattern<"E8">().match(target)) return TRAMPOLINE.write_call<5>(target, destination);
		if (REL::Pattern<"E9">().match(target)) return TRAMPOLINE.write_branch<5>(target, destination);
		struct XPatch: Xbyak::CodeGenerator {
			using ull = unsigned long long;
			using uch = unsigned char;
			uch workspace[64];
			XPatch(std::uintptr_t baseAddress, ull bytesCount): Xbyak::CodeGenerator(bytesCount + 14, workspace) {
				auto bytePtr = reinterpret_cast<uch*>(baseAddress);
				for (ull i = 0; i < bytesCount; i++) db(*bytePtr++);
				jmp(qword[rip]);
				dq(ull(bytePtr));
			}
		};
		XPatch patch(target, copyCount);
		patch.ready();
		auto patchSize = patch.getSize();
		TRAMPOLINE.write_branch<5>(target, destination);
		auto alloc = TRAMPOLINE.allocate(patchSize);
		std::memcpy(alloc, patch.getCode(), patchSize);
		return reinterpret_cast<std::uintptr_t>(alloc);
	}

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (a_name.size() && a_offset) {
			auto name = RE::BSFixedString(a_name);
			auto offset = std::int32_t(a_offset & 0xFFFFFF);
			using type = RE::TESForm*(*)(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
			static REL::Relocation<type> func{ REL::ID(171055) };
			return func(NULL, NULL, NULL, offset, &name);
		}
		return nullptr;
	}

}

namespace SMSForms {

	std::set<RE::BGSMessage*> toSuppress;

	void LoadForms(std::string src) {
		logs::info("LoadForms:{}", src);
		toSuppress.clear();
		if (auto tesDH = RE::TESDataHandler::GetSingleton(); tesDH) {
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
									try {
										auto valueUInt32 = std::stoul(value, nullptr, 0);
										auto form = SMSUtility::GetFormFromFile(section, valueUInt32);
										if (form && form->GetFormType() == RE::FormType::kMESG) {
											toSuppress.insert(static_cast<RE::BGSMessage*>(form));
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
	SFSE::Init(a_sfse, false);
	SFSE::AllocTrampoline(256);

	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

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
