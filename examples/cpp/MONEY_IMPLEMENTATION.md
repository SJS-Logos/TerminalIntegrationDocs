# Money Value Object - Implementation Notes

## Why Integer Cents Instead of Floating-Point?

The `Money` value object in this C++ implementation uses **integer cents** (int64_t) instead of floating-point (double or float) to represent monetary amounts. This is a critical design decision for financial applications.

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
// Actual:   899.9999999999636 (off by ~36 cents after 1000 transactions!)
```

## Our Solution: Integer Cents

The `Money` class stores amounts as **integer cents** (smallest currency unit):

```cpp
class Money {
private:
    int64_t cents_;      // Amount in cents (e.g., 1050 for $10.50)
    std::string currency_;
};
```

### Benefits

1. **Exact Precision**: Integer arithmetic is always exact
2. **No Rounding Errors**: Addition, subtraction, and comparison are precise
3. **Predictable**: Same results on all platforms and compilers
4. **Performance**: Integer operations are typically faster than floating-point
5. **Large Range**: int64_t supports ±$92,233,720,368,547.75 (±2^63 cents)

## Usage Examples

### Construction

```cpp
// From string (flexible - accepts "10.50" or "10,50")
Money price("10.50", "USD");  // Internally stored as 1050 cents

// From cents directly (preferred for precision)
Money precise = Money::FromCents(1050, "USD");  // Exact: 1050 cents

// Zero amount
Money empty = Money::Zero("USD");  // 0 cents
```

### Formatting for Display

```cpp
Money amount = Money::FromCents(123456789, "USD");  // $1,234,567.89

// Default US format: period decimal, comma thousands, with currency
std::string us = amount.ToString();
// Output: "1,234,567.89 USD"

// European format: comma decimal, period thousands
std::string eu = amount.ToString(',', '.', true);
// Output: "1.234.567,89 USD"

// Swiss format: period decimal, apostrophe thousands
std::string ch = amount.ToString('.', '\'', true);
// Output: "1'234'567.89 USD"

// Without currency code
std::string no_curr = amount.ToString('.', ',', false);
// Output: "1,234,567.89"
```

### Operations

```cpp
Money item1(10.50, "USD");    // 1050 cents
Money item2(5.25, "USD");     // 525 cents

Money total = item1.Add(item2);  // 1575 cents = $15.75 (exact!)

bool positive = item1.IsPositive();      // true
bool greater = item1.IsGreaterThan(item2);  // true
```

### Accessing Values

```cpp
Money amount("10.50", "USD");

// For calculations: use cents (exact)
int64_t cents = amount.GetCents();      // 1050
int64_t doubled = cents * 2;             // 2100 (exact)
Money result = Money::FromCents(doubled, "USD");  // $21.00

// For display: use ToString() with proper formatting
std::string display = amount.ToString();     // "10.50 USD"
std::cout << display << "\n";                // Output: 10.50 USD

// International display
std::string euro_fmt = amount.ToString(',', '.', true);  // "10,50 USD"
```

### Internal Storage

```cpp
// When you create Money("10.50", "USD"), internally:
Money::Money(const std::string& amount_str, const std::string& currency) {
    // Parses "10.50" -> 1050 cents
    // Also handles "10,50" (European format) -> 1050 cents
}

// When you call ToString(), it formats back:
std::string ToString(char decimal_sep, char thousands_sep, bool show_currency) {
    // 1050 cents -> "10.50 USD" (or "10,50 USD" for European)
}
```

## Comparison with Other Approaches

### Approach 1: Double (AVOID)
```cpp
? double amount = 10.50;
   std::cout << amount << " USD\n";  // No formatting, no precision
   // Issues: Precision errors, rounding issues, inconsistent results, no formatting
```

### Approach 2: Integer Cents with ToString() (OUR CHOICE)
```cpp
? Money amount = Money::FromCents(1050, "USD");
   std::cout << amount.ToString() << "\n";  // "10.50 USD"
   // Benefits: Exact, fast, predictable, proper formatting, internationalization
```

### Approach 3: Decimal Library (Alternative)
```cpp
?? decimal::Decimal amount("10.50");
   std::cout << amount << " USD\n";  // Manual formatting needed
   // Benefits: Exact decimal arithmetic
   // Drawbacks: Requires external library, slower, more complex, manual formatting
```

## Edge Cases Handled

### Rounding During Construction

```cpp
// Input strings are parsed to nearest cent
Money m1("10.505", "USD");   // Truncated to 1050 cents ($10.50)
Money m2("10.50", "USD");    // Exact: 1050 cents ($10.50)

// Both European and US formats supported
Money us("1,234.56", "USD");   // 123456 cents
Money eu("1.234,56", "EUR");   // 123456 cents (comma is decimal separator)
```

### Very Large Amounts

```cpp
// int64_t supports huge values
Money large = Money::FromCents(9223372036854775807LL, "USD");
// Maximum: $92,233,720,368,547.75 (over 92 trillion dollars)
```

### Currency Validation

```cpp
Money usd(10.00, "USD");
Money eur(10.00, "EUR");

// Operations validate matching currencies
Money total = usd.Add(eur);  // Throws: "Cannot add different currencies"
```

## Best Practices

### DO ?

```cpp
// Use FromCents when you have exact cent values
Money precise = Money::FromCents(1050, "USD");

// Use GetCents for calculations
int64_t subtotal = price.GetCents();
int64_t tax = subtotal * 7 / 100;  // Integer division (exact)
Money total = Money::FromCents(subtotal + tax, "USD");

// Use ToString() for display with proper formatting
std::cout << total.ToString() << "\n";  // "10.50 USD"

// Store as cents in databases
int64_t db_value = amount.GetCents();

// Format for different locales
std::string us_fmt = amount.ToString('.', ',', true);   // "1,234.56 USD"
std::string eu_fmt = amount.ToString(',', '.', true);   // "1.234,56 USD"
```

### DON'T ?

```cpp
// Don't use double anywhere for money
double bad = 10.50;  // Wrong abstraction!

// Don't manually format money strings
std::cout << price.GetCents() / 100.0 << " USD";  // NO! Use ToString()

// Don't multiply Money by floating-point
double multiplier = 1.5;  // Use integer math instead
// Bad:  result = amount * multiplier;
// Good: result = Money::FromCents(amount.GetCents() * 3 / 2, "USD");
```

## Database Storage

When persisting to a database, store the cents as a BIGINT:

```sql
CREATE TABLE payments (
    id VARCHAR(50) PRIMARY KEY,
    amount_cents BIGINT NOT NULL,     -- Store as integer cents
    currency VARCHAR(3) NOT NULL,
    ...
);

-- Query example:
INSERT INTO payments (id, amount_cents, currency)
VALUES ('PAY-001', 1050, 'USD');      -- $10.50 as 1050 cents
```

## API/JSON Serialization

When serializing to JSON, you have options:

```json
{
  "payment_id": "PAY-001",
  "amount_cents": 1050,
  "currency": "USD",
  "amount_display": "10.50 USD"
}
```

Or for international APIs:

```json
{
  "payment_id": "PAY-001",
  "amount_cents": 1050,
  "currency": "USD",
  "formatted": {
    "us": "10.50 USD",
    "eu": "10,50 USD"
  }
}
```

Always store `amount_cents` for precision; add display formats for convenience.

## Testing

The integer cents approach makes testing exact and predictable:

```cpp
TEST(MoneyTest, Addition_Is_Exact) {
    Money a = Money::FromCents(10, "USD");   // $0.10
    Money b = Money::FromCents(20, "USD");   // $0.20

    Money sum = a.Add(b);

    EXPECT_EQ(sum.GetCents(), 30);  // Always exactly 30, never 29 or 31
    EXPECT_EQ(sum.ToString(), "0.30 USD");
}

TEST(MoneyTest, No_Floating_Point_Errors) {
    Money total = Money::Zero("USD");
    Money increment = Money::FromCents(10, "USD");  // $0.10

    for (int i = 0; i < 1000; i++) {
        total = total.Add(increment);
    }

    EXPECT_EQ(total.GetCents(), 10000);  // Exactly $100.00
    EXPECT_EQ(total.ToString(), "100.00 USD");
}

TEST(MoneyTest, Formatting_With_Thousands_Separator) {
    Money large = Money::FromCents(123456789, "USD");

    EXPECT_EQ(large.ToString('.', ',', true), "1,234,567.89 USD");
    EXPECT_EQ(large.ToString(',', '.', true), "1.234.567,89 USD");
    EXPECT_EQ(large.ToString('.', '\'', true), "1'234'567.89 USD");
}
```

## References

- [Martin Fowler - Money Pattern](https://martinfowler.com/eaaCatalog/money.html)
- [C++ Core Guidelines - Arithmetic](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es-expressions-and-statements)
- [What Every Computer Scientist Should Know About Floating-Point Arithmetic](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html)

## Summary

Using integer cents for money with proper formatting:
- ? Eliminates floating-point precision errors
- ? Provides exact arithmetic for financial calculations
- ? Ensures consistent results across platforms
- ? Follows financial industry best practices
- ? Maintains acceptable performance
- ? Supports very large monetary values
- ? Provides proper display formatting with thousands separators
- ? Supports international number formats
- ? Eliminates double from the public API (correct abstraction)

This implementation prioritizes **correctness, precision, and proper formatting** over convenience, which is essential for financial applications.
