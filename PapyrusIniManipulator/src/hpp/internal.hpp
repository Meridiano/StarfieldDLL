#pragma once
#include "utility.hpp"

namespace PIMInternal {

	static bool usePrettyPrint = true;
	static bool useTranslation = true;

	/* I hope it will work some day
	std::string GetTranslation(std::string key) {
		std::string result = "";
		bool success = SFSE::Translation::Translate(key, result);
		return (success ? result : key);
	}
	*/

	bool IniDataExistsInternal(std::int32_t level, std::string path, std::string section, std::string key) {
		mINI::INIFile file(path);
		mINI::INIStructure ini;
		if (file.read(ini)) {
			if (level == 0) {
				return true;
			} else if (level == 1) {
				return ini.has(section);
			} else if (level == 2) {
				return (ini.has(section) && ini.get(section).has(key));
			}
			return false;
		}
		return false;
	}

	bool ClearIniDataInternal(std::int32_t level, std::string path, std::string section, std::string key) {
		mINI::INIFile file(path);
		mINI::INIStructure ini;
		if (file.read(ini)) {
			if (level == 0) {
				ini.clear();
				return file.write(ini, usePrettyPrint);
			} else if ((level == 1) && ini.has(section)) {
				ini[section].clear();
				return file.write(ini, usePrettyPrint);
			} else if ((level == 2) && ini.has(section) && ini.get(section).has(key)) {
				ini[section][key] = "";
				return file.write(ini, usePrettyPrint);
			}
			return false;
		}
		return false;
	}

	bool DestroyIniDataInternal(std::int32_t level, std::string path, std::string section, std::string key) {
		mINI::INIFile file(path);
		mINI::INIStructure ini;
		if (file.read(ini)) {
			if (level == 0) {
				fs::path target = path;
				return std::filesystem::remove(target);
			} else if ((level == 1) && ini.has(section)) {
				ini.remove(section);
				return file.write(ini, usePrettyPrint);
			} else if ((level == 2) && ini.has(section) && ini.get(section).has(key)) {
				ini[section].remove(key);
				return file.write(ini, usePrettyPrint);
			}
			return false;
		}
		return false;
	}

	std::vector<std::string> GetIniDataInternal(std::int32_t level, std::string path, std::string section, std::string key) {
		std::vector<std::string> result;
		mINI::INIFile file(path);
		mINI::INIStructure ini;
		if (file.read(ini)) {
			if (level == 0) {
				for (std::pair iterator : ini) {
					result.push_back(iterator.first);
				}
				return result;
			} else if ((level == 1) && ini.has(section)) {
				for (std::pair iterator : ini.get(section)) {
					result.push_back(iterator.first);
				}
				return result;
			} else if ((level == 2) && ini.has(section) && ini.get(section).has(key)) {
				/* I hope it will work some day
				std::string raw = ini.get(section).get(key);
				std::string str = (useTranslation ? GetTranslation(raw) : raw);
				*/
				std::string str = ini.get(section).get(key);
				return PIMUtility::StringToVector(str);
			}
			return result;
		}
		return result;
	}

	std::string PullStringFromIniInternal(std::string path, std::string section, std::string key, std::string fallback) {
		mINI::INIFile file(path);
		mINI::INIStructure ini;
		if (file.read(ini) && ini.has(section) && ini.get(section).has(key)) {
			/* I hope it will work some day
			std::string raw = ini.get(section).get(key);
			return (useTranslation ? GetTranslation(raw) : raw);
			*/
			return ini.get(section).get(key);
		}
		return fallback;
	}

	bool PullBoolFromIniInternal(std::string path, std::string section, std::string key, bool fallback) {
		std::string raw = PullStringFromIniInternal(path, section, key, PIMUtility::BoolToString(fallback));
		return PIMUtility::StringToBool(raw, fallback);
	}

	std::int32_t PullIntFromIniInternal(std::string path, std::string section, std::string key, std::int32_t fallback) {
		std::string raw = PullStringFromIniInternal(path, section, key, std::to_string(fallback));
		std::int32_t result;
		try {
			result = std::stol(raw, nullptr, 0);
		} catch (...) {
			return fallback;
		}
		return result;
	}

	float PullFloatFromIniInternal(std::string path, std::string section, std::string key, float fallback) {
		std::string raw = PullStringFromIniInternal(path, section, key, std::to_string(fallback));
		float result;
		try {
			result = std::stof(raw, nullptr);
		} catch (...) {
			return fallback;
		}
		return result;
	}

	bool PushStringToIniInternal(std::string path, std::string section, std::string key, std::string value, bool force) {
		if (!PIMUtility::FileExists(path, force)) return false;
		mINI::INIFile file(path);
		mINI::INIStructure ini;
		if (!file.read(ini)) // process file
		{
			if (force) file.generate(ini, usePrettyPrint);
			else return false;
		}
		if (section.empty() || section == space) // exit if bad section
		{
			return file.write(ini, usePrettyPrint);
		}
		if (!ini.has(section)) // process section
		{
			if (force) ini[section];
			else return false;
		}
		if (key.empty() || key == space) // exit if bad key
		{
			return file.write(ini, usePrettyPrint);
		}
		if (!ini.get(section).has(key)) // process key
		{
			if (force) ini[section][key];
			else return false;
		}
		if (value.empty() || value == space) // exit if bad value
		{
			return file.write(ini, usePrettyPrint);
		}
		// finally set new value
		ini[section][key] = value;
		return file.write(ini, usePrettyPrint);
	}

	bool PushBoolToIniInternal(std::string path, std::string section, std::string key, bool value, bool force) {
		return PushStringToIniInternal(path, section, key, PIMUtility::BoolToString(value), force);
	}

	bool PushIntToIniInternal(std::string path, std::string section, std::string key, std::int32_t value, bool force) {
		return PushStringToIniInternal(path, section, key, std::to_string(value), force);
	}

	bool PushFloatToIniInternal(std::string path, std::string section, std::string key, float value, bool force) {
		return PushStringToIniInternal(path, section, key, std::to_string(value), force);
	}

}