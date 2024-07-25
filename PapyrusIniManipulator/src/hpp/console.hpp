#pragma once
#include "internal.hpp"

namespace PIMConsole {

	std::vector<std::string> GetCommandStringArguments(const char* dataPointer) {
		auto dataAddress = std::uint64_t(dataPointer);
		auto dataOffset = (*reinterpret_cast<std::uint8_t*>(dataAddress) == 28 ? 8 : 4);
		auto resultSizeAddress = dataAddress + dataOffset;
		auto resultSize = *reinterpret_cast<std::uint8_t*>(resultSizeAddress);
		std::vector<std::string> result(resultSize);
		auto bufferSizeAddress = resultSizeAddress + 2;
		for (std::uint8_t indexA = 0; indexA < resultSize; indexA++) {
			auto bufferSize = *reinterpret_cast<std::uint8_t*>(bufferSizeAddress);
			std::vector<char> buffer(bufferSize);
			for (std::uint8_t indexB = 0; indexB < bufferSize; indexB++) {
				auto symbolAddress = bufferSizeAddress + 2 + indexB;
				auto symbol = *reinterpret_cast<char*>(symbolAddress);
				buffer[indexB] = symbol;
			}
			auto bufferData = std::string(buffer.data(), bufferSize);
			result[indexA] = bufferData;
			bufferSizeAddress += std::uint64_t(2 + bufferSize);
		}
		return result;
	}

	void ConsolePrint(std::string text) {
		static auto conLog = RE::ConsoleLog::GetSingleton();
		if (conLog) conLog->Log("{}", text);
	}

	bool SafeToGrab(RE::Script::SCRIPT_FUNCTION* func) {
		static REL::Relocation<void(*)> EmptyFunction{ REL::ID(72465) };
		return (func ? func->executeFunction == EmptyFunction.get() : false);
	}

	RE::Script::SCRIPT_FUNCTION* LocateCommand(std::string fullName) {
		auto func = RE::Script::LocateConsoleCommand(fullName);
		return (SafeToGrab(func) ? func : nullptr);
	}

	bool ExecutePVFI(const RE::SCRIPT_PARAMETER* paramInfo, const char* stringData,
					 RE::TESObjectREFR* thisObj, RE::TESObjectREFR* containingObj,
					 RE::Script* script, RE::ScriptLocals* locals,
					 float* result, std::uint32_t* opcodeOffsetPtr) {
		std::vector<std::string> arguments = GetCommandStringArguments(stringData);
		std::size_t missing = 3 - arguments.size();
		if (missing > 0) {
			std::vector<std::string> append(missing, space);
			arguments.insert(arguments.end(), append.begin(), append.end());
		}
		std::string path = arguments[0];
		std::string section = arguments[1];
		std::string key = arguments[2];
		ConsolePrint(std::format("    Path = {}\n    Section = {}\n    Key = {}", path, section, key));
		if (PIMInternal::IniDataExistsInternal(2, path, section, key)) {
			ConsolePrint(std::format("        Value = {}", PIMInternal::PullStringFromIniInternal(path, section, key, "")));
			return true;
		}
		ConsolePrint("        Could not find this path/section/key.");
		return false;
	}

	bool ExecutePVTI(const RE::SCRIPT_PARAMETER* paramInfo, const char* stringData,
					 RE::TESObjectREFR* thisObj, RE::TESObjectREFR* containingObj,
					 RE::Script* script, RE::ScriptLocals* locals,
					 float* result, std::uint32_t* opcodeOffsetPtr) {
		std::vector<std::string> arguments = GetCommandStringArguments(stringData);
		std::size_t missing = 5 - arguments.size();
		if (missing > 0) {
			std::vector<std::string> append(missing, space);
			arguments.insert(arguments.end(), append.begin(), append.end());
		}
		std::string path = arguments[0];
		std::string section = arguments[1];
		std::string key = arguments[2];
		std::string value = arguments[3];
		bool force = PIMUtility::StringToBool(arguments[4], false);
		ConsolePrint(std::format("    Path = {}\n    Section = {}\n    Key = {}\n    Value = {}\n    Force = {}", path, section, key, value, force));
		if (PIMInternal::IniDataExistsInternal(2, path, section, key) || force) {
			bool result = PIMInternal::PushStringToIniInternal(path, section, key, value, force);
			if (result) {
				ConsolePrint("        Value has been pushed successfully.");
				return true;
			}
			ConsolePrint("        Could not push value to this path/section/key.");
			return false;
		}
		ConsolePrint("        Could not find this path/section/key.\n        Push without force canceled.");
		return false;
	}
	
	bool RegisterPVFI() {
		std::string toSteal = PIMInternal::PullStringFromIniInternal(PIMUtility::PluginConfigPath(), "Console", "sConsoleCommandToStealA", "StartTrackPlayerDoors");
		static auto commandGet = LocateCommand(toSteal);
		if (commandGet) {
			commandGet->functionName = "PullValueFromIni";
			commandGet->shortName = "PVFI";
			commandGet->helpString = "Pulls value from ini file";
			commandGet->referenceFunction = false;
			commandGet->numParams = 3;
			static RE::SCRIPT_PARAMETER newParams[] = {
				{ "Path (String)", 0, false },
				{ "Section (String)", 0, true },
				{ "Key (String)", 0, true }
			};
			commandGet->params = newParams;
			commandGet->executeFunction = ExecutePVFI;
			commandGet->editorFilter = 0;
			commandGet->invalidatesCellList = 0;
			// done
			logs::info("PullValueFromIni and PVFI console commands registered");
			return true;
		}
		logs::info("PullValueFromIni and PVFI console commands not registered");
		return false;
	}

	bool RegisterPVTI() {
		std::string toSteal = PIMInternal::PullStringFromIniInternal(PIMUtility::PluginConfigPath(), "Console", "sConsoleCommandToStealB", "StopTrackPlayerDoors");
		static auto commandSet = LocateCommand(toSteal);
		if (commandSet) {
			commandSet->functionName = "PushValueToIni";
			commandSet->shortName = "PVTI";
			commandSet->helpString = "Pushes value to ini file";
			commandSet->referenceFunction = false;
			commandSet->numParams = 5;
			static RE::SCRIPT_PARAMETER newParams[] = {
				{ "Path (String)", 0, false },
				{ "Section (String)", 0, true },
				{ "Key (String)", 0, true },
				{ "Value (String)", 0, true },
				{ "Force (Boolean)", 0, true }
			};
			commandSet->params = newParams;
			commandSet->executeFunction = ExecutePVTI;
			commandSet->editorFilter = 0;
			commandSet->invalidatesCellList = 0;
			// done
			logs::info("PushValueToIni and PVTI console commands registered");
			return true;
		}
		logs::info("PushValueToIni and PVTI console commands not registered");
		return false;
	}

	bool Register() {
		bool pvfi = RegisterPVFI();
		bool pvti = RegisterPVTI();
		return (pvfi && pvti);
	}

}
