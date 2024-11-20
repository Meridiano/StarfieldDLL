#pragma once
#include "internal.hpp"

namespace PIMConsole {

	RE::ConsoleLog* conLog = nullptr;

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

	bool SafeToGrab(RE::SCRIPT_FUNCTION* func) {
		static REL::Relocation<void(*)> EmptyFunction{ REL::ID(72465) };
		return (func ? func->executeFunction == EmptyFunction.get() : false);
	}

	RE::SCRIPT_FUNCTION* LocateCommand(std::string fullName) {
		auto func = RE::Script::LocateConsoleCommand(fullName);
		return (SafeToGrab(func) ? func : nullptr);
	}

	bool ExecutePVFI(const RE::SCRIPT_PARAMETER* paramInfo, const char* stringData,
					 RE::TESObjectREFR* thisObj, RE::TESObjectREFR* objContainer,
					 RE::Script* script, RE::ScriptLocals* scriptLocals,
					 float* result, std::uint32_t* opcodeOffsetPtr) {
		auto arguments = GetCommandStringArguments(stringData);
		if (arguments.size() != 3) arguments.resize(3, space);
		std::string path = arguments[0];
		std::string section = arguments[1];
		std::string key = arguments[2];
		conLog->Log("{}Path = {}\n{}Section = {}\n{}Key = {}", tab, path, tab, section, tab, key);
		if (PIMInternal::IniDataExistsInternal(2, path, section, key)) {
			conLog->Log("{}{}Value = {}", tab, tab, PIMInternal::PullStringFromIniInternal(path, section, key, ""));
			return true;
		}
		conLog->Log("{}{}Could not find this path/section/key.", tab, tab);
		return false;
	}

	bool ExecutePVTI(const RE::SCRIPT_PARAMETER* paramInfo, const char* stringData,
					 RE::TESObjectREFR* thisObj, RE::TESObjectREFR* objContainer,
					 RE::Script* script, RE::ScriptLocals* scriptLocals,
					 float* result, std::uint32_t* opcodeOffsetPtr) {
		std::vector<std::string> arguments = GetCommandStringArguments(stringData);
		if (arguments.size() != 5) arguments.resize(5, space);
		std::string path = arguments[0];
		std::string section = arguments[1];
		std::string key = arguments[2];
		std::string value = arguments[3];
		bool force = PIMUtility::StringToBool(arguments[4], false);
		conLog->Log("{}Path = {}\n{}Section = {}\n{}Key = {}\n{}Value = {}\n{}Force = {}", tab, path, tab, section, tab, key, tab, value, tab, force);
		if (PIMInternal::IniDataExistsInternal(2, path, section, key) || force) {
			bool result = PIMInternal::PushStringToIniInternal(path, section, key, value, force);
			if (result) {
				conLog->Log("{}{}Value has been pushed successfully.", tab, tab);
				return true;
			}
			conLog->Log("{}{}Could not push value to this path/section/key.", tab, tab);
			return false;
		}
		conLog->Log("{}{}Could not find this path/section/key.\n{}{}Push without force canceled.", tab, tab, tab, tab);
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
		conLog = RE::ConsoleLog::GetSingleton();
		return (conLog ? RegisterPVFI() && RegisterPVTI() : false);
	}

}
