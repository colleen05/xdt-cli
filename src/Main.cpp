#include <libxdt.hpp>
#include <map>
#include <algorithm>
#include <filesystem>

enum class ParseState {
    InFilename,
    InAction
};

enum class ToolError {
    FileOpenError,
    XDTReadError,
    UnkownAction,
    NoGivenAction,
    InvalidSyntax,
    NoGivenArguments,
    InvalidTypename,
    Unknown
};

void PrintErr(ToolError errcode, std::vector<std::string> args = std::vector<std::string>()) {
    std::cerr << "\e[1;31merror: \e[0m";
    switch(errcode) {
        case ToolError::FileOpenError:
            std::cerr << "could not open file: \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::XDTReadError:
            std::cerr << "could not read file as XDT: \"" << args[0] << "\"." << std::endl;
            std::cerr << args[1] << std::endl;
            break;
        case ToolError::UnkownAction:
            std::cerr << "unkown action: \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::NoGivenAction:
            std::cerr << "no given action for file \"" << args[0] << "\"." << std::endl;
            break;
        case ToolError::InvalidSyntax:
            if(args.empty()) {
                std::cerr << "invalid syntax." << std::endl;
            }else {
                std::cerr << "invalid syntax: " << args[0] << std::endl;
            }
            break;
        case ToolError::NoGivenArguments:
            std::cerr << "no given arguments." << std::endl;
            break;
        case ToolError::InvalidTypename:
            std::cerr << "invalid type name: " << args[0] << std::endl;
            break;
        default:
            std::cerr << "UNKOWN ERROR. THIS IS A BUG.";
            break;
    }

    std::cout << "use option '--help' or '-h' for help with using XDT CLI." << std::endl;
}

void PrintVersion() {
    std::cout
        << "XDT CLI v1.0.0 - December, 2022\n"
        << "by Colleen (@colleen05), and GitHub contributors.\n"
        << "\n"
        << "This software is distributed under the zlib license.\n"
        << "For more information, see the 'LICENSE' file provided, or visit:\n"
        << "https://choosealicense.com/licenses/zlib/\n"
        << std::endl;
}

void PrintHelp() {
    std::cout
        << "Usage: xdt <file> <action> [args...] [options...]\n"
        << "\n"
        << "Actions:\n"
        << "    list                            List all items in file.\n"
        << "    get     <names...>              Get value(s) in file.\n"
        << "    set     <setters...>            Set value(s) in file, by setters.\n"
        << "    mutate  <mutators...>           Cast type of value to new type, by mutators.\n"
        << "    remove  <names...>              Remove value(s) in file.\n"
        << "    dump    <directory> [items...]  Dump items to directory. (All items by default.)\n"
        << "\n"
        << "Setter:     <type>:<name>=<value>\n"
        << "Mutator:    <name>:<type>\n"
        << "\n"
        << "Options:\n"
        << "    --version or -v     Display version information.\n"
        << "    --help    or -h     Display this help information.\n"
        << "    --backup  or -b     Create backup file before modification.\n"
        << "\n"
        << "Types:\n"
        << "  - byte            Byte.\n"
        << "  - bool            Boolean.\n"
        << "  - int16           16-bit integer (signed).\n"
        << "  - uint16          16-bit integer (unsigned).\n"
        << "  - int32           32-bit integer (signed).\n"
        << "  - uint32          32-bit integer (unsigned).\n"
        << "  - int64           64-bit integer (signed).\n"
        << "  - uint64          64-bit integer (unsigned).\n"
        << "  - float           Float.\n"
        << "  - double          Double.\n"
        << "  - time            Timestamp.\n"
        << "  - longtime        Long timestamp (64-bit).\n"
        << "  - string          ASCII string.\n"
        << "  - utf8string      UTF-8 string.\n"
        << "  - file            File data.\n"
        << "  - bin             Binary data.\n"
        << std::endl;

    PrintVersion();
}

void PrintItem(xdt::Item &item, const std::string &name) {
    std::cout << "(" << xdt::GetTypeString(item.type) << ") \"" << name << "\": ";
    
    switch(item.type) {
        case xdt::ItemType::ASCIIString:
        case xdt::ItemType::UTF8String:
            std::cout << "\"" << item.GetString() << "\"" << std::endl;
            break;

        case xdt::ItemType::File:
        case xdt::ItemType::Raw:
            std::cout << "<" << item.data.size() << " bytes>" << std::endl;
            break;

        default:
            std::cout << item.GetString() << std::endl;
        break;
    }
}

bool ApplySetter(xdt::Table &table, const std::string &setterStr) {
    // Get strings
    auto colon_pos = setterStr.find_first_of(":");
    auto equals_pos = setterStr.find_first_of("=");

    if((colon_pos == std::string::npos) || (equals_pos == std::string::npos)) {
        PrintErr(ToolError::InvalidSyntax, {setterStr});
        return false;
    }

    auto typestr    = setterStr.substr(0, colon_pos);
    auto name       = setterStr.substr(colon_pos + 1, equals_pos - colon_pos - 1);
    auto valuestr   = setterStr.substr(equals_pos + 1, setterStr.size() - equals_pos);

    // Test strings
    if(typestr.empty() || name.empty() || valuestr.empty()) {
        PrintErr(ToolError::InvalidSyntax, {setterStr});
        return false;
    }

    // Apply setter
    if(typestr == "byte") {
        table.SetByte(name, std::stoi(valuestr), true);
    }else if(typestr == "bool") {
        bool val = std::stoi(valuestr);
        val |= (valuestr == "true") || (valuestr == "TRUE");

        table.SetBool(name, val, true);
    }else if(typestr == "int16") {
        table.SetInt16(name, std::stoi(valuestr), true);
    }else if(typestr == "uint16") {
        table.SetUint16(name, std::stoul(valuestr), true);
    }else if(typestr == "int32") {
        table.SetInt32(name, std::stoi(valuestr), true);
    }else if(typestr == "uint32") {
        table.SetUint32(name, std::stoul(valuestr), true);
    }else if(typestr == "int64") {
        table.SetInt64(name, std::stol(valuestr), true);
    }else if(typestr == "uint64") {
        table.SetUint64(name, std::stoul(valuestr), true);
    }else if(typestr == "float") {
        table.SetFloat(name, std::stof(valuestr), true);
    }else if(typestr == "double") {
        table.SetDouble(name, std::stod(valuestr), true);
    }else if(typestr == "time") {
        table.SetTimestamp(name, std::chrono::seconds(std::stoi(valuestr)), false, true);
    }else if(typestr == "longtime") {
        table.SetTimestamp(name, std::chrono::seconds(std::stol(valuestr)), true, true);
    }else if(typestr == "string") {
        table.SetString(name, valuestr, false, true);
    }else if(typestr == "utf8string") {
        table.SetString(name, valuestr, true, true);
    }else if(typestr == "file") {
        std::ifstream f(valuestr, std::ios::binary);

        auto inBytes = std::vector<uint8_t>(
            std::istreambuf_iterator<char>(f),
            std::istreambuf_iterator<char>()
        );

        table.SetBytes(name, inBytes, true, true);
    }else if(typestr == "byte") {
        std::ifstream f(valuestr, std::ios::binary);

        auto inBytes = std::vector<uint8_t>(
            std::istreambuf_iterator<char>(f),
            std::istreambuf_iterator<char>()
        );

        table.SetBytes(name, inBytes, false, true);
    }else {
        PrintErr(ToolError::InvalidTypename, {typestr});
        return false;
    }

    // Print output
    PrintItem(*table.GetItem(name), name);

    return true;
}

bool ApplyMutator(xdt::Table &table, const std::string &mutatorStr) {
    // Get strings
    auto colon_pos = mutatorStr.find_first_of(":");

    if(colon_pos == std::string::npos) {
        PrintErr(ToolError::InvalidSyntax, {mutatorStr});
        return false;
    }

    auto name = mutatorStr.substr(0, colon_pos);
    auto typestr = mutatorStr.substr(colon_pos + 1, mutatorStr.size() - colon_pos);

    // Test strings
    if(typestr.empty() || name.empty()) {
        PrintErr(ToolError::InvalidSyntax, {mutatorStr});
        return false;
    }

    // Apply setter
    if(typestr == "byte") {
        table.SetByte(name, table.GetByte(name), true);
    }else if(typestr == "bool") {
        table.SetBool(name, table.GetBool(name), true);
    }else if(typestr == "int16") {
        table.SetInt16(name, table.GetInt16(name), true);
    }else if(typestr == "uint16") {
        table.SetUint16(name, table.GetUint16(name), true);
    }else if(typestr == "int32") {
        table.SetInt32(name, table.GetInt32(name), true);
    }else if(typestr == "uint32") {
        table.SetUint32(name, table.GetUint32(name), true);
    }else if(typestr == "int64") {
        table.SetInt64(name, table.GetInt64(name), true);
    }else if(typestr == "uint64") {
        table.SetUint64(name, table.GetUint64(name), true);
    }else if(typestr == "float") {
        table.SetFloat(name, table.GetFloat(name), true);
    }else if(typestr == "double") {
        table.SetDouble(name, table.GetDouble(name), true);
    }else if(typestr == "time") {
        table.SetTimestamp(name, table.GetTimestamp(name), false, true);
    }else if(typestr == "longtime") {
        table.SetTimestamp(name, table.GetTimestamp(name), true, true);
    }else if(typestr == "string") {
        table.SetString(name, table.GetString(name), false, true);
    }else if(typestr == "utf8string") {
        table.SetString(name, table.GetString(name), true, true);
    }else if(typestr == "file") {
        table.SetBytes(name, table.GetBytes(name), false, true);
    }else if(typestr == "byte") {
        table.SetBytes(name, table.GetBytes(name), true, true);
    }else {
        PrintErr(ToolError::InvalidTypename, {typestr});
        return false;
    }

    // Print output
    PrintItem(*table.GetItem(name), name);

    return true;
}

void ListItems(xdt::Table &table, const std::string &filename) {
    int keycount = table.directory.size();

    std::cout << "File \"" << filename << "\" (" << std::to_string(keycount) << " items):" << std::endl;

    for(auto &[name, item] : table.directory) {
        PrintItem(item, name);
    }
}

void GetItems(xdt::Table &table, const std::string &filename, const std::vector<std::string> &itemNames) {
    if(itemNames.empty()) {
        PrintErr(ToolError::NoGivenArguments);
        return;
    }

    std::vector<std::string> notFound;

    for(auto &name : itemNames) {
        if(table.ItemExists(name)) {
            PrintItem(*table.GetItem(name), name);
        }else {
            notFound.push_back(name);
        }
    }

    if(!notFound.empty()) {
        if(notFound.size() != itemNames.size()) std::cout << std::endl;

        std::cout << notFound.size() << " items not found: ";

        for(auto i = 0; i < notFound.size(); i++) {
            std::cout << "\"" << notFound[i] << "\"" << ((i == notFound.size() - 1) ? "." : ", ");
        }

        std::cout << std::endl;
    }
}

bool SetItems(xdt::Table &table, const std::string &filename, const std::vector<std::string> &setters) {
    if(setters.empty()) {
        PrintErr(ToolError::NoGivenArguments);
        return false;
    }

    for(auto &s : setters) {
        if(!ApplySetter(table, s)) return false;
    }

    return true;
}

bool MutateItems(xdt::Table &table, const std::string &filename, const std::vector<std::string> &mutators) {
    if(mutators.empty()) {
        PrintErr(ToolError::NoGivenArguments);
        return false;
    }

    for(auto &m : mutators) {
        if(!ApplyMutator(table, m)) return false;
    }

    return true;
}

bool RemoveItems(xdt::Table &table, const std::vector<std::string> &itemNames) {
    if(itemNames.empty()) {
        PrintErr(ToolError::NoGivenArguments);
        return false;
    }

    std::vector<std::string> notFound;

    // Remove items
    for(auto &name : itemNames) {
        if(table.ItemExists(name)) {
            table.DeleteItem(name);
            std::cout << "Deleted item \"" << name << "\"." << std::endl;
        }else {
            notFound.push_back(name);
        }
    }

    // List not found items
    if(!notFound.empty()) {
        if(notFound.size() != itemNames.size()) std::cout << std::endl;

        std::cout << notFound.size() << " items not found: ";

        for(auto i = 0; i < notFound.size(); i++) {
            std::cout << "\"" << notFound[i] << "\"" << ((i == notFound.size() - 1) ? "." : ", ");
        }

        std::cout << std::endl;
    }

    return true;
}

bool DumpTable(xdt::Table &table, const std::string &directory, const std::vector<std::string> &itemNames) {
    std::map<std::string, xdt::Item> output;
    std::vector<std::string> notFound;

    // Get output list
    if(itemNames.empty()) {
        for(auto &[name, item] : table.directory) {
            output.insert(std::make_pair(directory + name, item));
        }
    }else {
        for(auto &name : itemNames) {
            const auto item = table.GetItem(name);

            if(item != nullptr) {
                output.insert(std::make_pair(directory + name, *item));
            }else {
                notFound.push_back(name);
            }
        }
    }

    // Dump files
    for(auto &[name, item] : output) {
        std::cout << "Dumping: \"" << name << "\". ";

        if(!(std::filesystem::exists(directory) && std::filesystem::is_directory(directory))) {
            std::filesystem::create_directory(directory);
        }

        std::ofstream f(name, std::ios::binary);

        if(f.good()) {
            f.write(reinterpret_cast<const char*>(item.data.data()), item.data.size());
            std::cout << "DONE." << std::endl;
        }else {
            std::cout << "FAILED." << std::endl;
        }

        f.close();
    }

    // List not found items
    if(!notFound.empty()) {
        if(notFound.size() != itemNames.size()) std::cout << std::endl;

        std::cout << notFound.size() << " items not found: ";

        for(auto i = 0; i < notFound.size(); i++) {
            std::cout << "\"" << notFound[i] << "\"" << ((i == notFound.size() - 1) ? "." : ", ");
        }

        std::cout << std::endl;
    }

    return true;
}

void MakeBackup(const std::string &filename) {
    std::filesystem::copy_file(filename, filename + ".bak", std::filesystem::copy_options::overwrite_existing);
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv+argc);

    bool backupFile = false;

    // Check command validity
    if( (args.empty()) ||
        (std::find(args.begin(), args.end(), "-h") != args.end()) ||
        (std::find(args.begin(), args.end(), "--help") != args.end())
    ) {
        PrintHelp();
        return 0;
    }else if(
        (std::find(args.begin(), args.end(), "-v") != args.end()) ||
        (std::find(args.begin(), args.end(), "--version") != args.end())
    ) {
        PrintVersion();
        return 0;
    }else {
        backupFile |= (std::find(args.begin(), args.end(), "-b") != args.end());
        backupFile |= (std::find(args.begin(), args.end(), "--backup") != args.end());
        std::remove(args.begin(), args.end(), "-b");
        std::remove(args.begin(), args.end(), "--backup");
    }

    // Parse
    xdt::Table table;

    std::string filename;
    std::ifstream file;
    std::vector<uint8_t> file_bytes;

    ParseState state = ParseState::InFilename;

    for(auto i = 0; i < args.size(); i++) {
        auto &arg = args[i];

        switch(state) {
            case ParseState::InFilename:
                filename = arg;
                file.open(filename);

                // Load XDT if file exists. Create new XDT if file does not.
                if(args.size() >= 2) {
                    // Does file open?
                    if(file.good()) {
                        // Check for XDT read error
                        if(!table.Load(filename)) {
                            PrintErr(ToolError::XDTReadError, {arg, table.errorStatus});
                            file.close();
                            return 1;
                        }
                    }else {
                        if(args[1] != "set") {
                            PrintErr(ToolError::FileOpenError, {arg});
                            file.close();
                            return 1;
                        }
                    }
                }else {
                    // Does file not open?
                    if(!file.good()) {
                        PrintErr(ToolError::FileOpenError, {arg});
                        file.close();
                        return 1;
                    }
                }

                // Make sure an action is actually given
                if(i == args.size() - 1) {
                    PrintErr(ToolError::NoGivenAction, {arg});
                    file.close();
                    return 1;
                }

                // Update state & close file
                state = ParseState::InAction;
                file.close();

                break;
            case ParseState::InAction:
                if(arg == "list") {
                    ListItems(table, filename);
                    return 0;

                }else if(arg == "get") {
                    GetItems(table, filename, std::vector(args.begin() + 2, args.end()));
                    return 0;

                }else if(arg == "set") {
                    if(backupFile) MakeBackup(filename);
                    if(SetItems(table, filename, std::vector(args.begin() + 2, args.end()))) {
                        table.Save(filename);
                    }
                    return 0;

                }else if(arg == "mutate") {
                    if(backupFile) MakeBackup(filename);
                    if(MutateItems(table, filename, std::vector(args.begin() + 2, args.end()))) {
                        table.Save(filename);
                    }
                    return 0;

                }else if(arg == "remove") {
                    if(backupFile) MakeBackup(filename);
                    if(RemoveItems(table, std::vector(args.begin() + 2, args.end()))) {
                        table.Save(filename);
                    }
                    return 0;

                }else if(arg == "dump") {
                    const auto dumpArgs = std::vector(args.begin() + 2, args.end());

                    if(dumpArgs.empty()) {
                        PrintErr(ToolError::NoGivenArguments);
                        return 1;
                    }

                    const auto dir = dumpArgs[0];
                    const auto itemNames = std::vector(dumpArgs.begin() + 1, dumpArgs.end());

                    DumpTable(table, dir, itemNames);
                    return 0;

                }else {
                    PrintErr(ToolError::UnkownAction, {arg});
                    return 1;
                }
                break;
            default:
                PrintErr(ToolError::Unknown);
                break;
        }
    }

    return 0;
}