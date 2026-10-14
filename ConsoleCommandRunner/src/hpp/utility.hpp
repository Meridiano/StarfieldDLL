#pragma once

#include "hpp/redata.hpp"

static bool saveLoaded = false;

namespace CCRUtility {

    template<typename Func>
    auto WriteFunctionHook(auto reloc, std::size_t copyCount, Func destination) {
        auto target = reloc.address();
        if (GET_VALUE(std::uint8_t, target) == 0xE8) return reloc.write_call<5>(destination);
        if (GET_VALUE(std::uint8_t, target) == 0xE9) return reloc.write_jmp<5>(destination);
        struct XPatch : Xbyak::CodeGenerator {
            using ull = unsigned long long;
            using uch = unsigned char;
            uch workspace[64];
            XPatch(std::uintptr_t baseAddress, ull bytesCount) : Xbyak::CodeGenerator(bytesCount + 14, workspace) {
                auto bytePtr = reinterpret_cast<uch*>(baseAddress);
                for (ull i = 0; i < bytesCount; i++) db(*bytePtr++);
                jmp(qword[rip]);
                dq(ull(bytePtr));
            }
        };
        XPatch patch(target, copyCount);
        patch.ready();
        auto patchSize = patch.getSize();
        TRAMPOLINE.write_jmp<5>(target, destination);
        auto alloc = TRAMPOLINE.allocate(patchSize);
        std::memcpy(alloc, patch.getCode(), patchSize);
        return reinterpret_cast<std::uintptr_t>(alloc);
    }

    RE::TESObjectCELL* GetParentCell(RE::TESObjectREFR* arg) {
        using func_t = RE::TESObjectCELL*(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*);
        static REL::Relocation<func_t> func{ REL::ID(118505) };
        return func(NULL, NULL, arg);
    }

    std::uint32_t PlayerCellFormID() {
        if (auto pc = RE::PlayerCharacter::GetSingleton(); pc)
            if (auto cell = GetParentCell(pc); cell)
                return (cell->formID);
        return 0;
    }

    std::string PlayerCellEditorID() {
        if (auto pc = RE::PlayerCharacter::GetSingleton(); pc)
            if (auto cell = GetParentCell(pc); cell)
                return cell->GetFormEditorID();
        return "";
    }

    StrVec TomlArrayToVector(toml::array* arr) {
        StrVec out;
        if (arr) for (toml::node& node : *arr) {
            auto str = node.as_string();
            if (str) out.push_back(str->get());
        }
        return out;
    }

    int GameLoadType() {
        if (saveLoaded) return 2;
        if (PlayerCellFormID() > 0) {
            auto gameDaysPassed = RE::TESForm::LookupByID<RE::TESGlobal>(0x39);
            if (gameDaysPassed && gameDaysPassed->GetValue() < 0.334F) return 1;
        }
        return 0;
    }

    template <typename T>
    T* GetMember(const void* base, std::ptrdiff_t offset) {
        auto address = std::uintptr_t(base) + offset;
        REL::Relocation<T*> reloc{ address };
        return reloc.get();
    }

    auto GetMenuEventSource(RE::UI* ui) {
        using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
        return GetMember<type>(ui, 0x20);
    }

    bool IsBlacklistMenuOpen(StrVec list) {
        static auto ui = RE::UI::GetSingleton();
        static auto main = RE::Main::GetSingleton();
        if (ui && main) for (auto menuName : list) {
            if (menuName == "PAUSED") {
                if (main->isGameMenuPaused) return true;
            } else {
                if (ui->IsMenuOpen(menuName)) return true;
            }
        }
        return false;
    }

    RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
        if (a_name.size() && a_offset) {
            auto sName = RE::BSFixedString(a_name);
            auto iOffset = std::int32_t(a_offset & 0xFFFFFF);
            using type = RE::TESForm*(*)(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
            static REL::Relocation<type> func{ REL::ID(117382) };
            return func(NULL, NULL, NULL, iOffset, &sName);
        }
        return nullptr;
    }

    RE::TESNPC* GetActorBase(RE::Actor* actor) {
        auto ValidBase = [](RE::TESNPC* npc) { return npc ? npc->formID < 0xFF000000 : false; };
        if (actor) {
            auto base = actor->GetBaseObject()->As<RE::TESNPC>();
            if (ValidBase(base)) return base;
            if (auto extraList = actor->extraDataList.get(); extraList) {
                if (auto extraData = extraList->GetByType<CCRExtra::ExtraLeveledCreature>(); extraData) {
                    auto extraBase = extraData->originalBase;
                    if (ValidBase(extraBase)) return extraBase;
                }
            }
        }
        return nullptr;
    }

    RE::TESFile* LookupPlugin(std::uint8_t type, std::uint32_t id) {
        if (static auto tesDH = RE::TESDataHandler::GetSingleton(); tesDH) {
            using list = RE::BSTArray<RE::TESFile*>;
            switch (type) {
                case 0: {
                    auto& l1 = *GetMember<list>(tesDH, 0x1580);
                    if (l1.size() > id) return l1[id];
                }   break;
                case 1: {
                    auto& l2 = *GetMember<list>(tesDH, 0x1590);
                    if (l2.size() > id) return l2[id];
                }   break;
                case 2: {
                    auto& l3 = *GetMember<list>(tesDH, 0x15A0);
                    if (l3.size() > id) return l3[id];
                }   break;
            }
        }
        return nullptr;
    }

    std::string StringToLower(std::string str) {
        std::string out = str;
        std::transform(
            str.begin(), str.end(), out.begin(),
            [](unsigned char uch) { return std::tolower(uch); }
        );
        return out;
    }

    std::pair<std::string, std::uint32_t> FormToPair(RE::TESForm* form) {
        std::pair<std::string, std::uint32_t> result{ "", 0 };
        if (form) {
            RE::TESFile* plugin = nullptr;
            auto id = form->formID;
            switch (id >> 24) {
                case 0xFF:
                    break;
                case 0xFE:
                    plugin = LookupPlugin(1, (id >> 12) & 0xFFF);
                    id = id & 0xFFF;
                    break;
                case 0xFD:
                    plugin = LookupPlugin(2, (id >> 16) & 0xFF);
                    id = id & 0xFFFF;
                    break;
                default:
                    plugin = LookupPlugin(0, id >> 24);
                    id = id & 0xFFFFFF;
                    break;
            }
            if (plugin) {
                result.first = StringToLower(plugin->fileName);
                result.second = id;
            }
        }
        return result;
    }

    std::string FormToString(RE::TESForm* form) {
        auto [plugin, id] = FormToPair(form);
        return (plugin.size() && id) ? std::format("{}:0x{:X}", plugin, id) : "";
    }

    auto GetAllStrings(RE::TESForm* form) {
        StrVec result;
        if (form) {
            auto AddNonEmpty = [](StrVec& v, std::string s) { if (s.size() > 0) v.push_back(s); };
            auto type = form->formType.get();
            switch (type) {

                #define ADD_STR(S) AddNonEmpty(result, S)
                #define ADD_FORM(F) ADD_STR(FormToString(F))
                #define ADD_EDID(F) ADD_STR(F->GetFormEditorID())
                #define ADD_STR_FORM(S,F) { ADD_STR(S); ADD_FORM(F); }
                #define ADD_EDID_FORM(F) { ADD_EDID(F); ADD_FORM(F); }
                #define CAST_ADD_KEYS(F,T) { auto obj = F->As<RE::T>(); ADD_EDID(obj); for (auto key : obj->keywords) ADD_STR(key ? key->formEditorID.data() : ""); }

                case RE::FormType::kACHR:
                    if (form->formID == 0x14) {
                        auto base = RE::TESForm::LookupByID(0x7);
                        ADD_STR_FORM("PlayerRef", form);
                        ADD_STR_FORM("Player", base);
                    } else {
                        auto actor = form->As<RE::Actor>();
                        if (actor) ADD_EDID_FORM(actor);
                        auto base = GetActorBase(actor);
                        if (base) ADD_EDID_FORM(base);
                    }
                    break;
                case RE::FormType::kWEAP:
                    CAST_ADD_KEYS(form, TESObjectWEAP);
                    break;
                case RE::FormType::kARMO:
                    CAST_ADD_KEYS(form, TESObjectARMO);
                    break;
                case RE::FormType::kAMMO:
                    CAST_ADD_KEYS(form, TESAmmo);
                    break;
                case RE::FormType::kALCH:
                    CAST_ADD_KEYS(form, AlchemyItem);
                    break;
                case RE::FormType::kBOOK:
                    CAST_ADD_KEYS(form, TESObjectBOOK);
                    break;
                case RE::FormType::kSPEL:
                    CAST_ADD_KEYS(form, SpellItem);
                    break;

                #undef ADD_STR
                #undef ADD_FORM
                #undef ADD_EDID
                #undef ADD_STR_FORM
                #undef ADD_EDID_FORM
                #undef CAST_ADD_KEYS
            }
        }
        return result;
    }

    StrVec SplitString(std::string input, char delim, std::size_t maxSize) {
        StrVec result;
        std::stringstream stream(input);
        std::string sub;
        while (std::getline(stream, sub, delim)) result.push_back(sub);
        if (auto resultSize = result.size(); resultSize > maxSize) {
            auto lastIndex = maxSize - 1;
            for (auto index = maxSize; index < resultSize; index++) {
                result[lastIndex] += delim + result[index];
            }
            result.resize(maxSize);
        }
        return result;
    }

    std::string VectorToString(StrVec vec, std::string del = ";") {
        std::string str;
        auto len = vec.size();
        if (len > 0) {
            str = vec[0];
            for (std::size_t ind = 1; ind < len; ind++) str += del + vec[ind];
        }
        return std::format("[{}]", str);
    }

    bool GameIsFocused() {
        auto window = GetForegroundWindow();
		auto procID = GetCurrentProcessId();
		if (window && procID) {
			DWORD newProcID = NULL;
			auto thread = GetWindowThreadProcessId(window, &newProcID);
			if (thread && newProcID == procID) return (IsIconic(window) == 0);
		}
		return false;
    }

    bool ValidGamepadButton(std::int32_t button) {
        if (button < GLFW_GAMEPAD_BUTTON_A) return false;
        if (button > GLFW_GAMEPAD_BUTTON_LAST) return false;
        return true;
    }

    bool ValidVirtualButton(std::int32_t button) {
        if (button < VK_LBUTTON) return false;
        if (button > VK_OEM_CLEAR) return false;
        return true;
    }

    bool GamepadButtonPressed(std::int32_t button) {
        if (static bool ready = glfwInit(); ready) {
            glfwPollEvents();
            static GLFWgamepadstate state;
            static std::int32_t postLast = GLFW_JOYSTICK_LAST + 1;
            for (std::int32_t id = GLFW_JOYSTICK_1; id < postLast; id++)
                if (glfwJoystickIsGamepad(id) && glfwGetGamepadState(id, &state))
                    if (state.buttons[button] == GLFW_PRESS) return true;
        } else REX::FAIL("Failed to initialize GLFW");
        return false;
    }

    bool IsKeyPressed(bool gamepad, bool secondary, std::int32_t value) {
        if (gamepad) return (ValidGamepadButton(value) ? GamepadButtonPressed(value) : secondary);
        else return (ValidVirtualButton(value) ? (GetAsyncKeyState(value) & 0x8000) : secondary);
    }

    std::string TomlNodeToString(toml::node* node) {
        if (node) switch (node->type()) {
            case toml::node_type::string:
                return node->as_string()->get();
            case toml::node_type::integer:
                return std::format("{}", node->as_integer()->get());
            case toml::node_type::floating_point:
                return std::format("{}", node->as_floating_point()->get());
            case toml::node_type::boolean:
                return (node->as_boolean()->get() ? "1" : "0");
        }
        throw std::exception("toml node error");
    }

    std::string TomlReadString(std::string path, std::string section, std::string key, std::string fallback) {
        auto result = fallback;
        auto fsPath = fs::path(path);
        if (fs::exists(fsPath) && fs::is_regular_file(fsPath) && fsPath.extension() == ".toml") {
            try {
                toml::parse_result data;
                data = toml::parse_file(path);
                auto table = data[section].as_table();
                if (table) {
                    auto node = table->get(key);
                    result = TomlNodeToString(node);
                } else throw std::exception("toml table error");
            } catch (...) {
                REX::INFO("Parsing error, file path >> {}", path);
            }
        }
        return result;
    }

    std::string TextReplacementData(std::string input) {
        auto data = SplitString(input, ':', 4);
        input = "{" + input + "}";
        if (data.size() == 4) {
            auto mode = data[0];
            if (mode == "Form") {
                try {
                    auto id = std::stoul(data[2], nullptr, 0);
                    auto form = GetFormFromFile(data[1], id);
                    if (form) return std::format("{:08X}", form->formID);
                    throw std::exception("nullptr form");
                } catch (...) {
                    return data[3];
                }
            } else return TomlReadString(mode, data[1], data[2], data[3]);
        } else return input;
    }

    std::pair<bool, std::string> TextReplacement(std::string input) {
        if (input.size() > 1 && input[0] == '$') {
            input.erase(0, 1);
            std::string output;
            std::size_t index = 0;
            std::size_t length = input.size();
            while (index < length) {
                if (input[index] == '{') {
                    std::size_t end = input.find('}', index + 1);
                    if (end != std::string::npos) {
                        auto inside = input.substr(index + 1, end - index - 1);
                        auto replacement = TextReplacementData(inside);
                        REX::INFO("Text replacement {}{}{} >> {}", '{', inside, '}', replacement);
                        output += replacement;
                        index = end + 1;
                    } else output += input[index++];
                } else output += input[index++];
            }
            return { true, output };
        }
        return { false, input };
    }

    SFSE_EXPORT bool TextReplacementAPI(const char* input, char** output) {
        if (input && output) {
            auto [valid, result] = TextReplacement(input);
            if (valid) {
                auto data = result.data();
                auto size = result.size() + 1;
                char* buffer = static_cast<char*>(std::malloc(size));
                if (buffer) {
                    std::memcpy(buffer, data, size);
                    *output = buffer;
                    return true;
                }
            }
        }
        return false;
    }

    void SetDataLoaded(bool arg) {
        REL::Relocation<bool*> target{ REL::ID(883582) };
        if (*target != arg) *target = arg;
    }

    void ConsoleExecute(std::string command) {
        // resolve
        if (auto resolved = TextReplacement(command); resolved.first) {
            command = resolved.second;
            REX::INFO("Resolved console command >> {}", command);
        }
        // execute
        using typeM = void*;
        using typeF = void(typeM, const char*);
        static REL::Relocation<typeM*> manager{ REL::ID(938528) };
        static REL::Relocation<typeF*> function{ REL::ID(113576) };
        function(*manager, command.data());
    }

    class VectorSearch {
    private:
        StrVec vec;
        StrVec::iterator vbeg;
        StrVec::iterator vend;
    public:
        VectorSearch(StrVec arg) {
            vec = arg;
            vbeg = vec.begin();
            vend = vec.end();
        }
        bool Contains(std::string arg) {
            return (std::find(vbeg, vend, arg) != vend);
        }
    };

    template <typename T1, typename T2, typename T3>
    struct Triplet {
        T1 first;
        T2 second;
        T3 third;
        auto operator<=>(const Triplet&) const = default;
    };

}