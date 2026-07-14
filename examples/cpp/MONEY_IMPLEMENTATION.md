# Money Value Object - Implementation Notes

> **Note on Danish Currency Formatting**: This document uses Danish conventions where:
> - The minor unit for DKK (Danish Krone) is **øre** (1 DKK = 100 øre)
> - Currency code appears **before** the amount: "DKK 10,50" (not "10,50 DKK")

## Why Integer Minor Units Instead of Floating-Point?

The `Money` value object in this C++ implementation uses **integer minor units** (int64_t) instead of floating-point (double or float) to represent monetary amounts. This is a critical design decision for financial applications.

## The Problem with Floating-Point

Floating-point numbers cannot precisely represent many decimal values due to their binary representation:

```cpp
// BAD: Floating-point precision errors
double a = 0.1;
double b = 0.2;
double sum = a + b;  // Result: 0.30000000000000004 (not exactly 0.3!)

// This can lead to serious issues in financial calculations:
double price = 0.1;
double total = 0.0;
for (int i = 0; i < 10; i++) {
    total += price;  // Expected: 1.0, Actual: 0.9999999999999999
}
```

### Real-World Impact

```cpp
// Example: Compounding errors in a payment system
double account_balance = 1000.00;
double transaction = 0.10;

for (int i = 0; i < 1000; i++) {
    account_balance -= transaction;  // Accumulates rounding errors
}
// Expected: 900.00
// Actual:   899.9999999999636 (off by ~0.36 DKK after 1000 transactions!)
```

## Our Solution: Integer Minor Units

The `Money` class stores amounts as **integer minor units** (smallest currency unit):

```cpp
class Money {
private:
    int64_t minor_units_;      // Amount in minor units (e.g., 1050 øre for DKK 10,50)
    std::string currency_;
};
```

### Benefits

1. **Exact Precision**: Integer arithmetic is always exact
2. **No Rounding Errors**: Addition, subtraction, and comparison are precise
3. **Predictable**: Same results on all platforms and compilers
4. **Performance**: Integer operations are typically faster than floating-point
5. **Large Range**: int64_t supports ±92,233,720,368,547.75 DKK (±2^63 minor units)

## Usage Examples

### Construction

```cpp
// From string (flexible - accepts "10.50" or "10,50")
Money price("10.50", "DKK");  // Internally stored as 1050 øre

// From minor units directly (preferred for precision)
Money precise = Money::FromMinorUnits(1050, "DKK");  // Exact: 1050 øre

// Zero amount
Money empty = Money::Zero("DKK");  // 0 øre
```

### Formatting for Display

```cpp
Money amount = Money::FromMinorUnits(123456789, "DKK");  // DKK 1.234.567,89

// Danish format: comma decimal, period thousands, currency prefix
std::string dk = amount.ToString(',', '.', true);
// Output: "DKK 1.234.567,89"

// Alternative: period decimal, comma thousands
std::string alt = amount.ToString('.', ',', true);
// Output: "DKK 1,234,567.89"

// Swiss format: period decimal, apostrophe thousands
std::string ch = amount.ToString('.', '\'', true);
// Output: "DKK 1'234'567.89"

// Without currency code
std::string no_curr = amount.ToString(',', '.', false);
// Output: "1.234.567,89"
```

### Operations

```cpp
Money item1("10.50", "DKK");    // 1050 øre
Money item2("5.25", "DKK");     // 525 øre

Money total = item1.Add(item2);  // 1575 øre = DKK 15,75 (exact!)

bool positive = item1.IsPositive();      // true
bool greater = item1.IsGreaterThan(item2);  // true
```

### Accessing Values

```cpp
Money amount("10.50", "DKK");

// For calculations: use minor units/øre (exact)
int64_t ore = amount.GetMinorUnits();      // 1050 øre
int64_t doubled = ore * 2;                  // 2100 øre (exact)
Money result = Money::FromMinorUnits(doubled, "DKK");  // DKK 21,00

// For display: use ToString() with proper formatting
std::string display = amount.ToString(',', '.', true);     // "DKK 10,50"
std::cout << display << "\n";                              // Output: DKK 10,50

// Alternative format
std::string alt_fmt = amount.ToString('.', ',', true);  // "DKK 10.50"
```

### Internal Storage

```cpp
// When you create Money("10.50", "DKK"), internally:
Money::Money(const std::string& amount_str, const std::string& currency) {
    // Parses "10.50" -> 1050 øre
    // Also handles "10,50" (Danish format) -> 1050 øre
}

// When you call ToString(), it formats back:
std::string ToString(char decimal_sep, char thousands_sep, bool show_currency) {
    // 1050 øre -> "DKK 10,50" (Danish) or "DKK 10.50" (alternative)
}
```

## Comparison with Other Approaches

### Approach 1: Double (AVOID)
```cpp
? double amount = 10.50;
   std::cout << amount << " DKK\n";  // No formatting, no precision
   // Issues: Precision errors, rounding issues, inconsistent results, no formatting
```

### Approach 2: Integer Minor Units with ToString() (OUR CHOICE)
```cpp
✓ Money amount = Money::FromMinorUnits(1050, "DKK");
   std::cout << amount.ToString(',', '.', true) << "\n";  // "DKK 10,50"
   // Benefits: Exact, fast, predictable, proper formatting, internationalization
```

### Approach 3: Decimal Library (Alternative)
```cpp
?? decimal::Decimal amount("10.50");
   std::cout << amount << " DKK\n";  // Manual formatting needed
   // Benefits: Exact decimal arithmetic
   // Drawbacks: Requires external library, slower, more complex, manual formatting
```

## Edge Cases Handled

### Rounding During Construction

```cpp
// Input strings are parsed to nearest øre
Money m1("10.505", "DKK");   // Truncated to 1050 øre (DKK 10,50)
Money m2("10,50", "DKK");    // Exact: 1050 øre (DKK 10,50)

// Both Danish and alternative formats supported
Money dk("1.234,56", "DKK");   // 123456 øre (comma decimal)
Money alt("1,234.56", "DKK");  // 123456 øre (period decimal)
```

### Very Large Amounts

```cpp
// int64_t supports huge values
Money large = Money::FromMinorUnits(9223372036854775807LL, "DKK");
// Maximum: 92.233.720.368.547,75 DKK (over 92 trillion kroner)
```

### Currency Validation

```cpp
Money dkk("10.00", "DKK");
Money eur("10.00", "EUR");

// Operations validate matching currencies
Money total = dkk.Add(eur);  // Throws: "Cannot add different currencies"
```

## Best Practices

### DO ?

```cpp
// Use FromMinorUnits when you have exact minor unit values
Money precise = Money::FromMinorUnits(1050, "DKK");

// Use GetMinorUnits for calculations
int64_t subtotal = price.GetMinorUnits();
int64_t tax = subtotal * 25 / 100;  // 25% Danish VAT (integer division, exact)
Money total = Money::FromMinorUnits(subtotal + tax, "DKK");

// Use ToString() for display with proper formatting
std::cout << total.ToString(',', '.', true) << "\n";  // "10,50 DKK"

// Store as minor units in databases
int64_t db_value = amount.GetMinorUnits();

// Format for different locales
std::string dk_fmt = amount.ToString(',', '.', true);   // "1.234,56 DKK"
std::string alt_fmt = amount.ToString('.', ',', true);  // "1,234.56 DKK"
```

### DON'T ?

```cpp
// Don't use double anywhere for money
double bad = 10.50;  // Wrong abstraction!

// Don't manually format money strings
std::cout << price.GetMinorUnits() / 100.0 << " DKK";  // NO! Use ToString()

// Don't multiply Money by floating-point
double multiplier = 1.5;  // Use integer math instead
// Bad:  result = amount * multiplier;
// Good: result = Money::FromMinorUnits(amount.GetMinorUnits() * 3 / 2, "DKK");
```

## Database Storage

When persisting to a database, store the minor units (øre for DKK) as a BIGINT:

```sql
CREATE TABLE payments (
    id VARCHAR(50) PRIMARY KEY,
    amount_minor_units BIGINT NOT NULL,     -- Store as integer øre for DKK
    currency VARCHAR(3) NOT NULL,
    ...
);

-- Query example:
INSERT INTO payments (id, amount_minor_units, currency)
VALUES ('PAY-001', 1050, 'DKK');      -- DKK 10,50 stored as 1050 øre
```

## API/JSON Serialization

When serializing to JSON, you have options:

```json
{
  "payment_id": "PAY-001",
  "amount_minor_units": 1050,
  "currency": "DKK",
  "amount_display": "DKK 10,50"
}
```

Or for international APIs:

```json
{
  "payment_id": "PAY-001",
  "amount_minor_units": 1050,
  "currency": "DKK",
  "formatted": {
    "dk": "DKK 10,50",
    "alt": "DKK 10.50"
  }
}
```

Always store `amount_minor_units` for precision; add display formats for convenience.

## Testing

The integer minor units (øre) approach makes testing exact and predictable:

```cpp
TEST(MoneyTest, Addition_Is_Exact) {
    Money a = Money::FromMinorUnits(10, "DKK");   // 10 øre = DKK 0,10
    Money b = Money::FromMinorUnits(20, "DKK");   // 20 øre = DKK 0,20

    Money sum = a.Add(b);

    EXPECT_EQ(sum.GetMinorUnits(), 30);  // Always exactly 30 øre, never 29 or 31
    EXPECT_EQ(sum.ToString(',', '.', true), "DKK 0,30");
}

TEST(MoneyTest, No_Floating_Point_Errors) {
    Money total = Money::Zero("DKK");
    Money increment = Money::FromMinorUnits(10, "DKK");  // 10 øre

    for (int i = 0; i < 1000; i++) {
        total = total.Add(increment);
    }

    EXPECT_EQ(total.GetMinorUnits(), 10000);  // Exactly 10000 øre = DKK 100,00
    EXPECT_EQ(total.ToString(',', '.', true), "DKK 100,00");
}

TEST(MoneyTest, Formatting_With_Thousands_Separator) {
    Money large = Money::FromMinorUnits(123456789, "DKK");

    EXPECT_EQ(large.ToString(',', '.', true), "DKK 1.234.567,89");
    EXPECT_EQ(large.ToString('.', ',', true), "DKK 1,234,567.89");
    EXPECT_EQ(large.ToString('.', '\'', true), "DKK 1'234'567.89");
}
```

## References

- [Martin Fowler - Money Pattern](https://martinfowler.com/eaaCatalog/money.html)
- [C++ Core Guidelines - Arithmetic](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es-expressions-and-statements)
- [What Every Computer Scientist Should Know About Floating-Point Arithmetic](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html)

## Summary

Using integer minor units (øre for DKK) for money with proper formatting:
- ✓ Eliminates floating-point precision errors
- ✓ Provides exact arithmetic for financial calculations
- ✓ Ensures consistent results across platforms
- ✓ Follows financial industry best practices
- ✓ Maintains acceptable performance
- ✓ Supports very large monetary values
- ✓ Provides proper display formatting with thousands separators
- ✓ Supports Danish number formats (currency before amount: "DKK 10,50")
- ✓ Eliminates double from the public API (correct abstraction)

This implementation prioritizes **correctness, precision, and proper formatting** over convenience, which is essential for financial applications.
