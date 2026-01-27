namespace RCCUtility {

	class ExtraTextDisplayData : public RE::BSExtraData {
	public:
		static constexpr auto RTTI{ RE::RTTI::ExtraTextDisplayData };
		static constexpr auto VTABLE{ RE::VTABLE::ExtraTextDisplayData };
		static constexpr auto TYPE{ RE::ExtraDataType::kTextDisplayData };
		// members
		RE::BSFixedString name;
		RE::BGSMessage*   message;
		RE::TESQuest*     quest;
		void*             unk30;
		void*             unk38;
		void*             unk40;
		std::uint32_t     type;
		std::uint16_t     nameLength;
		std::uint16_t     pad4E;
	};
	static_assert(offsetof(ExtraTextDisplayData, name) == 0x18);
	static_assert(offsetof(ExtraTextDisplayData, message) == 0x20);
	static_assert(offsetof(ExtraTextDisplayData, quest) == 0x28);
	static_assert(offsetof(ExtraTextDisplayData, type) == 0x48);
	static_assert(offsetof(ExtraTextDisplayData, nameLength) == 0x4C);
	static_assert(sizeof(ExtraTextDisplayData) == 0x50);

	template <typename T>
	T GetValue(std::uint64_t address) {
		return *std::bit_cast<T*>(address);
	}

	std::vector<std::string> GetCommandStringArguments(const char* dataPointer) {
		static auto u16Size = sizeof(std::uint16_t);
		auto dataAddress = std::uint64_t(dataPointer);
		auto dataOffset = (GetValue<std::uint8_t>(dataAddress) == 28 ? 8 : 4);
		auto resultSizeAddress = dataAddress + dataOffset;
		auto resultSize = GetValue<std::uint16_t>(resultSizeAddress);
		std::vector<std::string> result(resultSize);
		auto bufferSizeAddress = resultSizeAddress + u16Size;
		for (std::uint16_t indexA = 0; indexA < resultSize; indexA++) {
			auto bufferSize = GetValue<std::uint16_t>(bufferSizeAddress);
			std::vector<char> buffer(bufferSize);
			for (std::uint16_t indexB = 0; indexB < bufferSize; indexB++) {
				auto symbolAddress = bufferSizeAddress + u16Size + indexB;
				auto symbol = GetValue<char>(symbolAddress);
				buffer[indexB] = symbol;
			}
			result[indexA] = std::string(buffer.data(), bufferSize);
			bufferSizeAddress += std::uint64_t(u16Size + bufferSize);
		}
		return result;
	}

	void LogTwice(std::string sText) {
		auto conLog = RE::ConsoleLog::GetSingleton();
		if (conLog) conLog->Log("{}", sText);
		REX::INFO("{}", sText);
	}

	bool SafeToGrab(RE::SCRIPT_FUNCTION* command) {
		static REL::Relocation<void(*)> EmptyFunction{ REL::ID(35677) };
		return (command ? command->executeFunction == EmptyFunction.get() : false);
	}

	RE::SCRIPT_FUNCTION* LocateCommand(std::string fullName) {
		auto nameSize = fullName.size();
		static REL::Relocation<std::uint32_t*> count{ REL::ID(7977), 0x1 };
		static REL::Relocation<RE::SCRIPT_FUNCTION*> first{ REL::ID(896666) };
		auto list = std::span<RE::SCRIPT_FUNCTION>(first.get(), *count.get());
		for (auto& command : list)
			if (auto name = command.functionName; name && std::strlen(name) == nameSize)
				if (strnicmp(name, fullName.data(), nameSize) == 0)
					if (auto result = std::addressof(command); SafeToGrab(result))
						return result;
		return nullptr;
	}

	bool CreateExtraTextDisplayData(RE::TESObjectREFR* reference) {
		if (reference) {
			auto message = RE::TESForm::LookupByID<RE::BGSMessage>(0x27DE89);
			if (message) {
				using type = void(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*, RE::BGSMessage*);
				static REL::Relocation<type> SetOverrideName{ REL::ID(118465) };
				SetOverrideName(NULL, NULL, reference, message);
				return true;
			}
		}
		return false;
	}

	ExtraTextDisplayData* GetExtraTextDisplayData(RE::TESObjectREFR* objectReference) {
		if (!objectReference) return nullptr;
		static auto type = RE::ExtraDataType::kTextDisplayData;
		auto extraDataList = objectReference->extraDataList.get();
		if (!extraDataList) {
			bool success = CreateExtraTextDisplayData(objectReference);
			if (success) extraDataList = objectReference->extraDataList.get();
			if (!extraDataList) return nullptr;
		}
		bool hasTextData = extraDataList->HasType(type);
		if (!hasTextData) {
			bool success = CreateExtraTextDisplayData(objectReference);
			if (success) hasTextData = extraDataList->HasType(type);
			if (!hasTextData) return nullptr;
		}
		auto extraData = extraDataList->GetByType(type);
		return static_cast<ExtraTextDisplayData*>(extraData);
	}

	bool SetDisplayFullName(RE::TESObjectREFR* refr, std::string name) {
		if (auto size64 = name.size(); refr && size64) {
			static constexpr std::uint16_t max16 = USHRT_MAX;
			std::uint16_t size16 = size64 > max16 ? 0 : size64 & max16;
			if (auto data = GetExtraTextDisplayData(refr); data && size16) {
				// unlock
				if (data->message) data->message = nullptr;
				if (data->quest) data->quest = nullptr;
				// set
				data->name = RE::BSFixedString(name);
				data->nameLength = size16;
				// done
				return (strnicmp(refr->GetDisplayFullName(), name.data(), size64) == 0);
			}
		}
		return false;
	}

}

namespace RCCSettings {

	std::string sCommandGet = "GetDebugText";
	std::string sCommandSet = "SetDebugText";

	std::string ConfigString(mINI::INIStructure ini, std::string section, std::string key, std::string fallback) {
		std::string result = fallback;
		if (auto map = ini.get(section); map.has(key)) result = map.get(key);
		else REX::INFO("Failed to read [{}]{} ini value", section, key);
		REX::INFO("String value [{}]{} is \"{}\"", section, key, result);
		return result;
	}

	void LoadSettings() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\RenameConsoleCommand.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
			// general
			sCommandGet = ConfigString(ini, "General", "sCommandGet", sCommandGet);
			sCommandSet = ConfigString(ini, "General", "sCommandSet", sCommandSet);
		} else REX::INFO("Config read error, all settings set to default");
	}

}

namespace RCCProcess {

	static bool GetNameExecute(const RE::SCRIPT_PARAMETER*, const char*,
							   RE::TESObjectREFR* thisObj, // i need only this
							   RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, float*, std::uint32_t*) {
		try {
			// get data
			auto formID = std::format("{:X}", thisObj->formID);
			auto newName = thisObj->GetDisplayFullName();
			// done
			std::string logRecord = std::format("Reference {} has name \"{}\"", formID, newName);
			RCCUtility::LogTwice(logRecord);
		} catch (...) {
			RCCUtility::LogTwice("Failed to get reference name");
		}
		return true;
	}

	static bool SetNameExecute(const RE::SCRIPT_PARAMETER*,
							   const char* unkString, RE::TESObjectREFR* thisObj, // i need only this
							   RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, float*, std::uint32_t*) {
		try {
			// get data
			auto formID = std::format("{:X}", thisObj->formID);
			auto newName = RCCUtility::GetCommandStringArguments(unkString)[0];
			// do rename
			bool result = RCCUtility::SetDisplayFullName(thisObj, newName);
			// done
			std::string logRecord = std::format("{} {} {} \"{}\"", result ? "Reference" : "Failed to rename reference", formID, result ? "renamed to" : "to", newName);
			RCCUtility::LogTwice(logRecord);
		} catch (...) {
			RCCUtility::LogTwice("Failed to set reference name");
		}
		return true;
	}

	bool InstallConsoleCommands() {
		// process get
		static auto commandGet = RCCUtility::LocateCommand(RCCSettings::sCommandGet);
		if (commandGet && RCCUtility::SafeToGrab(commandGet)) {
			// define params
			const std::uint16_t numParams = 0;
			static RE::SCRIPT_PARAMETER* newParams = nullptr;
			// set data
			commandGet->functionName = "GetName";
			commandGet->shortName = "GNam";
			commandGet->helpString = "Allows to get name of selected object reference";
			commandGet->referenceFunction = true;
			commandGet->numParams = numParams;
			commandGet->params = newParams;
			commandGet->executeFunction = GetNameExecute;
			commandGet->editorFilter = 0;
			commandGet->invalidatesCellList = 0;
			// done
		} else return false;
		// process set
		static auto commandSet = RCCUtility::LocateCommand(RCCSettings::sCommandSet);
		if (commandSet && RCCUtility::SafeToGrab(commandSet)) {
			// define params
			const std::uint16_t numParams = 1;
			static RE::SCRIPT_PARAMETER newParams[] = {
				{"Name (String)", 0, false} // required
			};
			// set data
			commandSet->functionName = "SetName";
			commandSet->shortName = "SNam";
			commandSet->helpString = "Allows to set name of selected object reference";
			commandSet->referenceFunction = true;
			commandSet->numParams = numParams;
			commandSet->params = newParams;
			commandSet->executeFunction = SetNameExecute;
			commandSet->editorFilter = 0;
			commandSet->invalidatesCellList = 0;
			// done
		} else return false;
		return true;
	}

	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept {
		if (a_msg->type == SFSE::MessagingInterface::kPostDataLoad) {
			if (InstallConsoleCommands()) REX::INFO("Console commands installed");
			else REX::FAIL("Console commands not installed");
		} else return;
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

	RCCSettings::LoadSettings();

	const auto SFSEMessagingInterface = SFSE::GetMessagingInterface();
	if (SFSEMessagingInterface && SFSEMessagingInterface->RegisterListener(RCCProcess::MessageCallback)) {
		REX::INFO("Message listener registered");
	} else {
		REX::FAIL("Message listener not registered");
	}
	return true;
}

