#pragma once

namespace FCUtility {

	#define AlreadyQuoted (b == '"' && e == '"')
	#define MakeDequoted arg.substr(1, s - 2)
	#define MakeQuoted ('"' + arg + '"')

	std::string Quoted(std::string arg) {
		auto s = arg.size();
		if (s < 2) return MakeQuoted;
		char b = arg[0];
		char e = arg[s - 1];
		return AlreadyQuoted ? arg : MakeQuoted;
	}

	std::string Dequoted(std::string arg) {
		auto s = arg.size();
		if (s < 2) return arg;
		char b = arg[0];
		char e = arg[s - 1];
		return AlreadyQuoted ? MakeDequoted : arg;
	}

	#undef AlreadyQuoted
	#undef MakeDequoted
	#undef MakeQuoted

	template <typename T>
	T* MakeCopy(T* source) {
		static std::vector<T> storage;
		if (!source) return nullptr;
		storage.push_back(*source);
		return &storage.back();
	}

	bool VerifyExtension(const std::filesystem::directory_entry& entry, const wchar_t* ext) {
		if (!ext) return false;
		auto cmp = entry.path().extension().wstring();
		return (wcsicmp(cmp.data(), ext) == 0);
	}

	namespace CIS {

		using CSR = const std::string&;
		using CWR = const std::wstring&;

		std::string WideToString(CWR wide) {
			static auto empty = "";
			std::string result = empty;
			return REX::UTF16_TO_UTF8(wide, result) ? result : empty;
		}

		std::wstring StringToWide(CSR str) {
			static auto empty = L"";
			std::wstring result = empty;
			return REX::UTF8_TO_UTF16(str, result) ? result : empty;
		}

		struct Hash {
			std::size_t operator()(CSR s) const noexcept {
				auto low = StringToWide(s);
				for (auto& wch : low) wch = std::towlower(wch);
				static std::hash<std::wstring> hasher;
				return hasher(low);
			}
		};

		struct Equal {
			bool operator()(CSR a, CSR b) const noexcept {
				if (a.size() != b.size()) return false;
				auto aw = StringToWide(a);
				auto bw = StringToWide(b);
				return std::ranges::equal(aw, bw, [](wchar_t x, wchar_t y) {
					return std::towlower(x) == std::towlower(y);
				});
			}
		};

	}

}
