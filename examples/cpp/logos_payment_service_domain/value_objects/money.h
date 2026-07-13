#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>

namespace logos::payment_service::domain::value_objects {

/// Shared Value Object representing monetary amounts.
/// Used by both Domain logic and Application contracts (AP-004 §7).
/// 
/// Stores amount as integer cents (smallest currency unit) to avoid
/// floating-point precision issues. Amount is stored as: actual_amount * 100
/// For example: $10.50 is stored as 1050 cents
class Money {
public:
    /// Construct from string representation (e.g., "10.50")
    Money(const std::string& amount_str, const std::string& currency);

    /// Construct from cents directly (preferred for precision)
    static Money FromCents(int64_t cents, const std::string& currency);

    static Money Zero(const std::string& currency);

    Money Add(const Money& other) const;
    bool IsPositive() const;
    bool IsGreaterThan(const Money& other) const;

    /// Get raw cents value (for calculations and persistence)
    int64_t GetCents() const { return cents_; }

    const std::string& GetCurrency() const { return currency_; }

    /// Format for display with proper separators and currency
    /// Uses period as decimal separator and comma as thousands separator
    /// e.g., "1,234.56 USD"
    std::string ToString() const;

    /// Format with custom separators
    /// @param decimal_sep Decimal separator (e.g., '.' or ',')
    /// @param thousands_sep Thousands separator (e.g., ',' or '.' or ' ')
    /// @param show_currency If true, appends currency code
    std::string ToString(char decimal_sep, char thousands_sep, bool show_currency = true) const;

    bool operator==(const Money& other) const;
    bool operator!=(const Money& other) const;

private:
    Money(int64_t cents, const std::string& currency);

    int64_t cents_;      // Amount in smallest currency unit (cents)
    std::string currency_;
};

} // namespace logos::payment_service::domain::value_objects
