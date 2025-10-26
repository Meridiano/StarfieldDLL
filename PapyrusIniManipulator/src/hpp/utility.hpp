#pragma once

namespace PIMUtility {

	std::string PluginConfigPath() {
		auto pluginName = SFSE::GetPluginName();
		return std::format("Data\\SFSE\\Plugins\\{}.ini", pluginName);
	}

	bool EqualStrings(std::string a, std::string b, bool noCase) {
		auto length = a.length();
		if (b.length() == length) {
			if (noCase) return (strnicmp(a.data(), b.data(), length) == 0);
			else return (std::strncmp(a.data(), b.data(), length) == 0);
		}
		return false;
	}

	std::string BoolToString(bool value) {
		return (value ? "true" : "false");
	}

	std::vector<std::string> StringToVector(std::string value) {
		std::vector<std::string> result;
		if (std::wstring ws; REX::UTF8_TO_UTF16(value, ws)) {
			for (wchar_t wc : ws) {
				std::wstring element(1, wc);
				if (std::string str; REX::UTF16_TO_UTF8(element, str)) {
					result.push_back(str);
				}
			}
		}
		return result;
	}

	bool StringToBool(std::string value, bool fallback) {
		bool t = EqualStrings(value, "true", true) || EqualStrings(value, "1", true);
		if (t) return true;
		bool f = EqualStrings(value, "false", true) || EqualStrings(value, "0", true);
		if (f) return false;
		return fallback;
	}

	bool FileExists(std::string path, bool create) {
		fs::path target = path;
		if (fs::exists(target)) return true;
		if (create) {
			fs::path target_root = target.root_path();
			fs::path target_parent = target.parent_path();
			if (target_root.compare(target_parent) != 0) // need to create folders
			{
				if (!target_root.string().empty() && !fs::exists(target_root)) // path is absolute but local drive not found
				{
					return false;
				}
				if (!fs::create_directories(target_parent)) // could not create folders
				{
					return false;
				}
			}
			std::ofstream file(path);
			file << "";
			file.close();
			return fs::exists(target);
		}
		return false;
	}

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		auto reloc = REL::Relocation<T*>(address);
		return reloc.get();
	};

}
