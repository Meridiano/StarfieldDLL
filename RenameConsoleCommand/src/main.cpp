namespace RCCUtility {

	std::vector<std::string> GetCommandStringArguments(const char* dataPointer) {
		std::uint64_t dataAddress = std::uint64_t(dataPointer);
		std::uint64_t dataOffset = (*reinterpret_cast<std::uint8_t*>(dataAddress) == 28 ? 8 : 4);
		std::uint64_t resultSizeAddress = dataAddress + dataOffset;
		std::uint8_t resultSize = *reinterpret_cast<std::uint8_t*>(resultSizeAddress);
		std::vector<std::string> result(resultSize);
		std::uint64_t bufferSizeAddress = resultSizeAddress + 2;
		for (std::uint8_t indexA = 0; indexA < resultSize; indexA++) {
			std::uint8_t bufferSize = *reinterpret_cast<std::uint8_t*>(bufferSizeAddress);
			std::vector<char> buffer(bufferSize);
			for (std::uint8_t indexB = 0; indexB < bufferSize; indexB++) {
				std::uint64_t symbolAddress = bufferSizeAddress + 2 + indexB;
				char symbol = *reinterpret_cast<char*>(symbolAddress);
				buffer[indexB] = symbol;
			}
			std::string bufferData = std::string(buffer.data(), bufferSize);
			result[indexA] = bufferData;
			bufferSizeAddress += std::uint64_t(2 + bufferSize);
		}
		return result;
	}

	void LogTwice(std::string sText) {
		auto conLog = RE::ConsoleLog::GetSingleton();
		if (conLog) conLog->Log("{}", sText);
		logs::info("{}", sText);
	}

	bool SafeToGrab(RE::SCRIPT_FUNCTION* command) {
		static REL::Relocation<void(*)> EmptyFunction{ REL::ID(72465) };
		return (command ? command->executeFunction == EmptyFunction.get() : false);
	}

	bool ObjectHasScript(RE::TESObjectREFR* objectReference, const char* scriptName, bool attachMissing) {
		if (auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton(); vm) {
			if (auto hPolicy = &vm->GetObjectHandlePolicy(); hPolicy) {
				RE::BSTSmartPointer<RE::BSScript::Object> newObject;
				std::uint32_t type = objectReference->formType.underlying();
				if (auto handle = hPolicy->GetHandleForObject(type, objectReference); handle) {
	#define ATTACHED vm->FindBoundObject(handle, scriptName, true, newObject, true)
					if (ATTACHED) return true;
					if (attachMissing) {
						if (auto bPolicy = &vm->GetObjectBindPolicy(); bPolicy) {
							vm->CreateObject(scriptName, newObject);
							bPolicy->BindObject(newObject, handle);
							return ATTACHED;
						}
					}
	#undef ATTACHED
				}
			}
		}
		return false;
	}

	bool SetDisplayFullName(RE::TESObjectREFR* refr, std::string name) {
		if (refr && name.size() > 0) {
			std::uint32_t mesgID = 0x27DE89;
			if (auto mesg = RE::TESForm::LookupByID<RE::BGSMessage>(mesgID); mesg) {
				mesg->fullName = name;
				mesg->shortName = name;
				if (ObjectHasScript(refr, "ObjectReference", true)) {
					using type = std::int64_t(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*, RE::BGSMessage*);
					static REL::Relocation<type> func{ REL::ID(172570) };
					return func(NULL, NULL, refr, mesg);
				}
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
		else logs::info("Failed to read [{}]{} ini value", section, key);
		logs::info("String value [{}]{} is \"{}\"", section, key, result);
		return result;
	}

	void LoadSettings() {
		mINI::INIFile file("Data\\SFSE\\Plugins\\RenameConsoleCommand.ini");
		mINI::INIStructure ini;
		if (file.read(ini)) {
			// general
			sCommandGet = ConfigString(ini, "General", "sCommandGet", "GetDebugText");
			sCommandSet = ConfigString(ini, "General", "sCommandSet", "SetDebugText");
		} else logs::info("Config read error, all settings set to default");
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
		static auto commandGet = RE::Script::LocateConsoleCommand(RCCSettings::sCommandGet);
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
		static auto commandSet = RE::Script::LocateConsoleCommand(RCCSettings::sCommandSet);
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
			if (InstallConsoleCommands()) logs::info("Console commands installed");
			else SFSE::stl::report_and_fail("Console commands not installed");
		} else return;
	}

}

SFSEPluginLoad(const SFSE::LoadInterface* a_sfse) {
	SFSE::Init(a_sfse, false);
	SFSE::AllocTrampoline(64);

	logs::init();
	spdlog::set_pattern("%d.%m.%Y %H:%M:%S [%s:%#] %v");

	const auto pluginInfo = SFSE::PluginVersionData::GetSingleton();
	logs::info(
		"{} version {} is loading into Starfield {}",
		std::string(pluginInfo->pluginName),
		REL::Version::unpack(pluginInfo->pluginVersion).string("."),
		a_sfse->RuntimeVersion().string(".")
	);

	RCCSettings::LoadSettings();

	const auto SFSEMessagingInterface = SFSE::GetMessagingInterface();
	if (SFSEMessagingInterface && SFSEMessagingInterface->RegisterListener(RCCProcess::MessageCallback)) {
		logs::info("Message listener registered");
	} else {
		SFSE::stl::report_and_fail("Message listener not registered");
	}
	return true;
}
