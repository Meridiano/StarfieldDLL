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
			else return (strncmp(a.data(), b.data(), length) == 0);
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
		fs::path target(path);
		if (fs::exists(target)) return true;
		if (create) {
			fs::path parent = target.parent_path();
			if (!parent.empty() && !fs::exists(parent)) try {
				bool result = fs::create_directories(parent);
				if (!result) throw std::exception("filesystem fail");
			} catch (...) {
				// failed to create required folders
				return false;
			}
			std::ofstream file(target);
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
