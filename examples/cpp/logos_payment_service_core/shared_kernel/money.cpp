#include "logos_payment_service_core/shared_kernel/money.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace logos::payment::service::core::shared_kernel {

Money::Money(int64_t minor_units, const std::string& currency)
    : minor_units_(minor_units), currency_(currency) {
}

Money::Money(const std::string& amount_str, const std::string& currency)
    : currency_(currency) {
    // Parse string like "10.50" or "10,50" to minorUnits
    std::string cleaned = amount_str;

    // Replace comma with period for consistent parsing
    std::replace(cleaned.begin(), cleaned.end(), ',', '.');

    // Find decimal point
    size_t dot_pos = cleaned.find('.');

    if (dot_pos == std::string::npos) {
        // No decimal point - whole majorUnits
        minor_units_ = std::stoll(cleaned) * 100;
    } else {
        // Split into majorUnits and minorUnits
        std::string major_units_str = cleaned.substr(0, dot_pos);
        std::string minor_units_str = cleaned.substr(dot_pos + 1);

        // Pad or truncate minorUnits to 2 digits
        if (minor_units_str.length() == 1) {
            minor_units_str += "0";  // "1" becomes "10"
        } else if (minor_units_str.length() > 2) {
            minor_units_str = minor_units_str.substr(0, 2);  // Truncate to 2 digits
        }

        int64_t major_units = major_units_str.empty() ? 0 : std::stoll(major_units_str);
        int64_t minor_units_part = minor_units_str.empty() ? 0 : std::stoll(minor_units_str);

        minor_units_ = major_units * 100 + minor_units_part;

        // Handle negative amounts
        if (!major_units_str.empty() && major_units_str[0] == '-' && major_units == 0) {
            minor_units_ = -minor_units_part;  // Handle "-0.50"
        }
    }
}

Money Money::FromMinorUnits(int64_t minor_units, const std::string& currency) {
    return Money(minor_units, currency);
}

Money Money::Zero(const std::string& currency) {
    return Money(0LL, currency);
}

Money Money::Add(const Money& other) const {
    if (currency_ != other.currency_) {
        throw std::runtime_error("Cannot add money with different currencies");
    }
    return Money(minor_units_ + other.minor_units_, currency_);
}

bool Money::IsPositive() const {
    return minor_units_ > 0;
}

bool Money::IsGreaterThan(const Money& other) const {
    if (currency_ != other.currency_) {
        throw std::runtime_error("Cannot compare money with different currencies");
    }
    return minor_units_ > other.minor_units_;
}

std::string Money::ToString() const {
    // Danish default: comma decimal, period thousands, currency prefix
    return ToString(',', '.', true);
}

std::string Money::ToString(char decimal_sep, char thousands_sep, bool show_currency) const {
    std::ostringstream oss;

    // Danish convention: Currency code before amount (e.g., "DKK 10,50")
    if (show_currency) {
        oss << currency_ << ' ';
    }

    // Handle sign
    bool is_negative = minor_units_ < 0;
    int64_t abs_minor_units = is_negative ? -minor_units_ : minor_units_;

    // Split into majorUnits and minorUnits
    int64_t major_units = abs_minor_units / 100;
    int64_t minor_units_part = abs_minor_units % 100;

    if (is_negative) {
        oss << '-';
    }

    // Format majorUnits with thousands separator
    std::string major_units_str = std::to_string(major_units);
    int insert_pos = static_cast<int>(major_units_str.length()) - 3;

    while (insert_pos > 0) {
        major_units_str.insert(insert_pos, 1, thousands_sep);
        insert_pos -= 3;
    }

    oss << major_units_str;
    oss << decimal_sep;
    oss << std::setfill('0') << std::setw(2) << minor_units_part;

    return oss.str();
}

bool Money::operator==(const Money& other) const {
    return minor_units_ == other.minor_units_ && currency_ == other.currency_;
}

bool Money::operator!=(const Money& other) const {
    return !(*this == other);
}

} // namespace logos::payment::service::core::shared_kernel
