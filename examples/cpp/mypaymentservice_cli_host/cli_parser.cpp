#include "mypaymentservice_cli_host/cli_parser.h"
#include <algorithm>

namespace mypaymentservice::cli_host {

CliParser::CliParser(int argc, char* argv[]) {
    if (argc < 2) {
        return;
    }

    command_ = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.substr(0, 2) == "--") {
            // Option with value: --amount 100
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                options_[arg.substr(2)] = argv[i + 1];
                ++i;
            } else {
                // Flag: --help
                flags_.push_back(arg.substr(2));
            }
        } else {
            // Positional argument
            positional_args_.push_back(arg);
        }
    }
}

std::string CliParser::GetCommand() const {
    return command_;
}

bool CliParser::HasFlag(const std::string& flag) const {
    return std::find(flags_.begin(), flags_.end(), flag) != flags_.end();
}

std::string CliParser::GetOption(const std::string& option) const {
    auto it = options_.find(option);
    return it != options_.end() ? it->second : "";
}

std::string CliParser::GetPositional(size_t index) const {
    return index < positional_args_.size() ? positional_args_[index] : "";
}

bool CliParser::IsValid() const {
    return !command_.empty();
}

} // namespace mypaymentservice::cli_host
