#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>

namespace logos::payment_service::domain::value_objects {

/// Shared Value Object representing monetary amounts.
/// Used by both Domain logic and Application contracts (AP-004 §7).
/// 
/// Stores amount as integer minorUnits (smallest currency unit) to avoid
/// floating-point precision issues. Amount is stored as: actual_amount * 100
/// For example: kr. 10.50 is stored as 1050 minorUnits
class Money {
public:
    /// Construct from string representation (e.g., "10.50")
    Money(const std::string& amount_str, const std::string& currency);

    /// Construct from minorUnits directly (preferred for precision)
    static Money FromMinorUnits(int64_t minor_units, const std::string& currency);

    static Money Zero(const std::string& currency);

    Money Add(const Money& other) const;
    bool IsPositive() const;
    bool IsGreaterThan(const Money& other) const;

    /// Get raw minorUnits value (for calculations and persistence)
    int64_t GetMinorUnits() const { return minor_units_; }

    const std::string& GetCurrency() const { return currency_; }

    /// Format for display with proper separators and currency
    /// Danish default: comma decimal, period thousands, currency prefix
    /// e.g., "DKK 1.234,56"
    std::string ToString() const;

    /// Format with custom separators
    /// @param decimal_sep Decimal separator (e.g., '.' or ',')
    /// @param thousands_sep Thousands separator (e.g., ',' or '.' or ' ')
    /// @param show_currency If true, prepends currency code (Danish convention)
    std::string ToString(char decimal_sep, char thousands_sep, bool show_currency = true) const;

    bool operator==(const Money& other) const;
    bool operator!=(const Money& other) const;

private:
    Money(int64_t minor_units, const std::string& currency);

    int64_t minor_units_;      // Amount in smallest currency unit (e.g., øre for DKK)
    std::string currency_;
};

} // namespace logos::payment_service::domain::value_objects
