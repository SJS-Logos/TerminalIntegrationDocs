#pragma once

#include <string>
#include <vector>
#include <map>

namespace mypaymentservice::cli_host {

/// @brief Command-line argument parser
class CliParser {
public:
    CliParser(int argc, char* argv[]);

    /// Get the command name (first argument)
    std::string GetCommand() const;

    /// Check if a flag exists (e.g., --help)
    bool HasFlag(const std::string& flag) const;

    /// Get value for an option (e.g., --amount 100)
    std::string GetOption(const std::string& option) const;

    /// Get positional argument by index
    std::string GetPositional(size_t index) const;

    /// Check if valid command was provided
    bool IsValid() const;

private:
    std::string command_;
    std::vector<std::string> positional_args_;
    std::map<std::string, std::string> options_;
    std::vector<std::string> flags_;
};

} // namespace mypaymentservice::cli_host
