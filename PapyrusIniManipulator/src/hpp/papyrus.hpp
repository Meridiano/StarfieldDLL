#pragma once
#include "internal.hpp"

namespace PIMPapyrus {

	void RegisterFunctions(RE::BSScript::IVirtualMachine**);
	using RF = decltype(RegisterFunctions);
	RF* OriginalRegisterFunctions;

	std::string GetVersion(std::monostate) {
		const auto plugin = SFSE::PluginVersionData::GetSingleton();
		return REL::Version::unpack(plugin->pluginVersion).string("-");
	}

	bool IniDataExists(std::monostate, std::int32_t level, std::string path, std::string section, std::string key) {
		return PIMInternal::IniDataExistsInternal(level, path, section, key);
	}

	bool ClearIniData(std::monostate, std::int32_t level, std::string path, std::string section, std::string key) {
		return PIMInternal::ClearIniDataInternal(level, path, section, key);
	}

	bool DestroyIniData(std::monostate, std::int32_t level, std::string path, std::string section, std::string key) {
		return PIMInternal::DestroyIniDataInternal(level, path, section, key);
	}

	std::vector<std::string> GetIniData(std::monostate, std::int32_t level, std::string path, std::string section, std::string key) {
		return PIMInternal::GetIniDataInternal(level, path, section, key);
	}

	std::string PullStringFromIni(std::monostate, std::string path, std::string section, std::string key, std::string defaultValue) {
		return PIMInternal::PullStringFromIniInternal(path, section, key, defaultValue);
	}

	bool PullBoolFromIni(std::monostate, std::string path, std::string section, std::string key, bool defaultValue) {
		return PIMInternal::PullBoolFromIniInternal(path, section, key, defaultValue);
	}

	std::int32_t PullIntFromIni(std::monostate, std::string path, std::string section, std::string key, std::int32_t defaultValue) {
		return PIMInternal::PullIntFromIniInternal(path, section, key, defaultValue);
	}

	float PullFloatFromIni(std::monostate, std::string path, std::string section, std::string key, float defaultValue) {
		return PIMInternal::PullFloatFromIniInternal(path, section, key, defaultValue);
	}

	bool PushStringToIni(std::monostate, std::string path, std::string section, std::string key, std::string value, bool force) {
		return PIMInternal::PushStringToIniInternal(path, section, key, value, force);
	}

	bool PushBoolToIni(std::monostate, std::string path, std::string section, std::string key, bool value, bool force) {
		return PIMInternal::PushBoolToIniInternal(path, section, key, value, force);
	}

	bool PushIntToIni(std::monostate, std::string path, std::string section, std::string key, std::int32_t value, bool force) {
		return PIMInternal::PushIntToIniInternal(path, section, key, value, force);
	}

	bool PushFloatToIni(std::monostate, std::string path, std::string section, std::string key, float value, bool force) {
		return PIMInternal::PushFloatToIniInternal(path, section, key, value, force);
	}

	void RegisterFunctions(RE::BSScript::IVirtualMachine** a_vm) {
		OriginalRegisterFunctions(a_vm);
		std::string className = "PapyrusIniManipulator";
		
		// general
		(*a_vm)->BindNativeMethod(className, "GetVersion", &GetVersion, true, false);

		// basic
		(*a_vm)->BindNativeMethod(className, "IniDataExists", &IniDataExists, true, false);
		(*a_vm)->BindNativeMethod(className, "ClearIniData", &ClearIniData, true, false);
		(*a_vm)->BindNativeMethod(className, "DestroyIniData", &DestroyIniData, true, false);
		(*a_vm)->BindNativeMethod(className, "GetIniData", &GetIniData, true, false);

		// pullers
		(*a_vm)->BindNativeMethod(className, "PullStringFromIni", &PullStringFromIni, true, false);
		(*a_vm)->BindNativeMethod(className, "PullBoolFromIni", &PullBoolFromIni, true, false);
		(*a_vm)->BindNativeMethod(className, "PullIntFromIni", &PullIntFromIni, true, false);
		(*a_vm)->BindNativeMethod(className, "PullFloatFromIni", &PullFloatFromIni, true, false);

		// pushers
		(*a_vm)->BindNativeMethod(className, "PushStringToIni", &PushStringToIni, true, false);
		(*a_vm)->BindNativeMethod(className, "PushBoolToIni", &PushBoolToIni, true, false);
		(*a_vm)->BindNativeMethod(className, "PushIntToIni", &PushIntToIni, true, false);
		(*a_vm)->BindNativeMethod(className, "PushFloatToIni", &PushFloatToIni, true, false);
	}

}