#include "logos_payment_service_domain/value_objects/money.h"
#include "logos_payment_service_domain/value_objects/money.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace logos::payment_service::domain::value_objects {

Money::Money(int64_t cents, const std::string& currency)
    : cents_(cents), currency_(currency) {
}

Money::Money(const std::string& amount_str, const std::string& currency)
    : currency_(currency) {
    // Parse string like "10.50" or "10,50" to cents
    std::string cleaned = amount_str;

    // Replace comma with period for consistent parsing
    std::replace(cleaned.begin(), cleaned.end(), ',', '.');

    // Find decimal point
    size_t dot_pos = cleaned.find('.');

    if (dot_pos == std::string::npos) {
        // No decimal point - whole dollars
        cents_ = std::stoll(cleaned) * 100;
    } else {
        // Split into dollars and cents
        std::string dollars_str = cleaned.substr(0, dot_pos);
        std::string cents_str = cleaned.substr(dot_pos + 1);

        // Pad or truncate cents to 2 digits
        if (cents_str.length() == 1) {
            cents_str += "0";  // "1" becomes "10"
        } else if (cents_str.length() > 2) {
            cents_str = cents_str.substr(0, 2);  // Truncate to 2 digits
        }

        int64_t dollars = dollars_str.empty() ? 0 : std::stoll(dollars_str);
        int64_t cents_part = cents_str.empty() ? 0 : std::stoll(cents_str);

        cents_ = dollars * 100 + cents_part;

        // Handle negative amounts
        if (!dollars_str.empty() && dollars_str[0] == '-' && dollars == 0) {
            cents_ = -cents_part;  // Handle "-0.50"
        }
    }
}

Money Money::FromCents(int64_t cents, const std::string& currency) {
    return Money(cents, currency);
}

Money Money::Zero(const std::string& currency) {
    return Money(0LL, currency);
}

Money Money::Add(const Money& other) const {
    if (currency_ != other.currency_) {
        throw std::runtime_error("Cannot add money with different currencies");
    }
    return Money(cents_ + other.cents_, currency_);
}

bool Money::IsPositive() const {
    return cents_ > 0;
}

bool Money::IsGreaterThan(const Money& other) const {
    if (currency_ != other.currency_) {
        throw std::runtime_error("Cannot compare money with different currencies");
    }
    return cents_ > other.cents_;
}

std::string Money::ToString() const {
    return ToString('.', ',', true);
}

std::string Money::ToString(char decimal_sep, char thousands_sep, bool show_currency) const {
    std::ostringstream oss;

    // Handle sign
    bool is_negative = cents_ < 0;
    int64_t abs_cents = is_negative ? -cents_ : cents_;

    // Split into dollars and cents
    int64_t dollars = abs_cents / 100;
    int64_t cents_part = abs_cents % 100;

    if (is_negative) {
        oss << '-';
    }

    // Format dollars with thousands separator
    std::string dollars_str = std::to_string(dollars);
    int insert_pos = static_cast<int>(dollars_str.length()) - 3;

    while (insert_pos > 0) {
        dollars_str.insert(insert_pos, 1, thousands_sep);
        insert_pos -= 3;
    }

    oss << dollars_str;
    oss << decimal_sep;
    oss << std::setfill('0') << std::setw(2) << cents_part;

    if (show_currency) {
        oss << ' ' << currency_;
    }

    return oss.str();
}

bool Money::operator==(const Money& other) const {
    return cents_ == other.cents_ && currency_ == other.currency_;
}

bool Money::operator!=(const Money& other) const {
    return !(*this == other);
}

} // namespace logos::payment_service::domain::value_objects
