#pragma once
#include "internal.hpp"

namespace PIMConsole {

	RE::ConsoleLog* conLog = nullptr;

	std::vector<std::string> GetCommandStringArguments(const char* dataPointer) {
		static auto u16Size = sizeof(std::uint16_t);
		auto dataAddress = std::uint64_t(dataPointer);
		auto dataOffset = (GetU8(dataAddress) == 28 ? 8 : 4);
		auto resultSizeAddress = dataAddress + dataOffset;
		auto resultSize = GetU16(resultSizeAddress);
		std::vector<std::string> result(resultSize);
		auto bufferSizeAddress = resultSizeAddress + u16Size;
		for (std::uint16_t indexA = 0; indexA < resultSize; indexA++) {
			auto bufferSize = GetU16(bufferSizeAddress);
			std::vector<char> buffer(bufferSize);
			for (std::uint16_t indexB = 0; indexB < bufferSize; indexB++) {
				auto symbolAddress = bufferSizeAddress + u16Size + indexB;
				auto symbol = *std::bit_cast<char*>(symbolAddress);
				buffer[indexB] = symbol;
			}
			auto bufferData = std::string(buffer.data(), bufferSize);
			result[indexA] = bufferData;
			bufferSizeAddress += std::uint64_t(u16Size + bufferSize);
		}
		return result;
	}

	bool SafeToGrab(RE::SCRIPT_FUNCTION* func) {
		static REL::Relocation<void(*)> EmptyFunction{ REL::ID(35677) };
		return (func ? func->executeFunction == EmptyFunction.get() : false);
	}

	RE::SCRIPT_FUNCTION* LocateCommand(std::string fullName) {
		auto nameSize = fullName.size();
		auto nameData = fullName.data();
		static REL::Relocation<std::uint32_t*> count{ REL::ID(7977), 0x1 };
		static REL::Relocation<RE::SCRIPT_FUNCTION*> first{ REL::ID(896666) };
		auto list = std::span<RE::SCRIPT_FUNCTION>(first.get(), *count.get());
		for (auto& command : list)
			if (auto name = command.functionName; name && std::strlen(name) == nameSize)
				if (strnicmp(name, nameData, nameSize) == 0)
					if (auto result = std::addressof(command); SafeToGrab(result))
						return result;
		return nullptr;
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
			REX::INFO("PullValueFromIni and PVFI console commands registered");
			return true;
		}
		REX::INFO("PullValueFromIni and PVFI console commands not registered");
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
			REX::INFO("PushValueToIni and PVTI console commands registered");
			return true;
		}
		REX::INFO("PushValueToIni and PVTI console commands not registered");
		return false;
	}

	bool Register() {
		conLog = RE::ConsoleLog::GetSingleton();
		return (conLog ? RegisterPVFI() && RegisterPVTI() : false);
	}

}
