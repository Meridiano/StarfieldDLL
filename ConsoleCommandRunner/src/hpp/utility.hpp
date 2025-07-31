#pragma once

static bool saveLoaded = false;

namespace CCRUtility {

    RE::TESObjectCELL* GetParentCell(RE::TESObjectREFR* arg) {
        using func_t = RE::TESObjectCELL*(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*);
        REL::Relocation<func_t> func{ REL::ID(118505) };
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
        auto reloc = REL::Relocation<T*>(address);
        return reloc.get();
    };

    RE::BSTEventSource<RE::MenuOpenCloseEvent>* GetMenuEventSource(RE::UI* ui) {
        using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
        return GetMember<type>(ui, 0x20);
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

    RE::TESFile* LookupPlugin(std::uint8_t type, std::uint32_t id) {
        if (static auto tesDH = RE::TESDataHandler::GetSingleton(); tesDH) {
            using list = RE::BSTArray<RE::TESFile*>;
            switch (type) {
                case 0: {
                    auto l1 = GetMember<list>(tesDH, 0x1550);
                    if (l1->size() > id) return l1->operator[](id);
                }   break;
                case 1: {
                    auto l2 = GetMember<list>(tesDH, 0x1560);
                    if (l2->size() > id) return l2->operator[](id);
                }   break;
                case 2: {
                    auto l3 = GetMember<list>(tesDH, 0x1570);
                    if (l3->size() > id) return l3->operator[](id);
                }   break;
            }
        }
        return nullptr;
    }

    std::string FormToString(RE::TESForm* form) {
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
                auto StringToLower = [](std::string str) {
                    std::string out = str;
                    std::transform(
                        str.begin(), str.end(), out.begin(),
                        [](unsigned char uch) { return std::tolower(uch); }
                    );
                    return out;
                };
                return std::format("{}:0x{:X}", StringToLower(plugin->fileName), id);
            }
        }
        return "";
    }

    auto GetAllStrings(RE::TESForm* form) {
        StrVec result;
        if (form) {
            auto AddNonEmpty = [](StrVec& v, std::string s) {
                if (s.size() > 0) v.push_back(s);
            };
            auto type = form->formType.get();
            switch (type) {

                #define ADD_STR(S) AddNonEmpty(result, S)
                #define ADD_FORM(F) ADD_STR(FormToString(F))
                #define ADD_EDID(F) ADD_STR(F->GetFormEditorID())
                #define ADD_STR_FORM(S,F) { ADD_STR(S); ADD_FORM(F); }
                #define ADD_EDID_FORM(F) { ADD_EDID(F); ADD_FORM(F); }
                #define CAST_ADD_KEYS(F,T) { auto obj = F->As<RE::##T>(); ADD_EDID(obj); for (auto key : obj->keywords) ADD_STR(key ? key->formEditorID.data() : ""); }

                case RE::FormType::kACHR: {
                    if (form->formID == 0x14) {
                        auto base = RE::TESForm::LookupByID(0x7);
                        ADD_STR_FORM("PlayerRef", form);
                        ADD_STR_FORM("Player", base);
                    } else {
                        auto actor = form->As<RE::Actor>();
                        ADD_EDID_FORM(actor);
                        if (auto bound = actor->GetBaseObject().get(); bound) {
                            auto npc = bound->As<RE::TESNPC>();
                            ADD_EDID_FORM(npc);
                            if (auto face = npc->faceNPC; face) {
                                ADD_EDID_FORM(face);
                            }
                        }
                    }
                }   break;
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

    StrVec SplitString(std::string input, char delim) {
        StrVec result;
        std::stringstream stream(input);
        std::string sub;
        while (std::getline(stream, sub, delim)) result.push_back(sub);
        return result;
    }

    std::string VectorToString(StrVec vec, std::string del = ";") {
        std::string str;
        auto len = vec.size();
        if (len > 0) {
            str = vec[0];
            for (std::size_t ind = 1; ind < len; ind++) str += std::format("{}{}", del, vec[ind]);
        }
        return std::format("[{}]", str);
    }

    bool GameIsFocused() {
        if (auto procID = GetCurrentProcessId(); procID) {
            std::set<HWND> hwndList;
            HWND hwnd = NULL;
            do {
                DWORD newProcID = 0;
                hwnd = FindWindowExA(NULL, hwnd, NULL, NULL);
                auto thread = GetWindowThreadProcessId(hwnd, &newProcID);
                if (thread && newProcID == procID) hwndList.insert(hwnd);
            } while (hwnd != NULL);
            if (hwndList.size() > 0) if (auto current = GetForegroundWindow(); current) {
                bool selected = false;
                for (auto element : hwndList) if (element == current) {
                    selected = true;
                    break;
                }
                if (selected) return (IsIconic(current) == 0);
            }
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
            std::int32_t gamepadID = []() {
                constexpr std::int32_t postLast = GLFW_JOYSTICK_LAST + 1;
                for (std::int32_t id = GLFW_JOYSTICK_1; id < postLast; id++)
                    if (glfwJoystickIsGamepad(id))
                        return id;
                return -1;
            }();
            if (gamepadID >= GLFW_JOYSTICK_1)
                if (GLFWgamepadstate state; glfwGetGamepadState(gamepadID, &state))
                    return state.buttons[button] == GLFW_PRESS;
        }
        return false;
    }

    bool IsKeyPressed(bool gamepad, bool secondary, std::int32_t value) {
        bool result = false;
        if (gamepad) {
            result = ValidGamepadButton(value) ? GamepadButtonPressed(value) : secondary;
        } else {
            result = ValidVirtualButton(value) ? (GetAsyncKeyState(value) & 0x8000) : secondary;
        }
        return result;
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
        auto error = "toml node error";
        throw std::exception(error);
        return error;
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
        auto data = SplitString(input, ':');
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
                        REX::INFO("Text replacement {{{}}} >> {}", inside, replacement);
                        output += replacement;
                        index = end + 1;
                    } else output += input[index++];
                } else output += input[index++];
            }
            return { true, output };
        }
        return { false, input };
    }

    void ConsoleExecute(std::string command) {
        // resolve
        if (auto resolved = TextReplacement(command); resolved.first) {
            command = resolved.second;
            REX::INFO("Resolved console command >> {}", command);
        }
        // execute
        std::thread([](std::string commandLambda) {
            static REL::Relocation<void*> manager{ REL::ID(949606) };
            static REL::Relocation<void(*)(void*, const char*)> function{ REL::ID(113576) };
            function(manager.get(), commandLambda.data());
        }, command).detach();
    }

    class VectorSearch {
    private:
        StrVec vec;
        StrVec::const_iterator vbeg;
        StrVec::const_iterator vend;
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