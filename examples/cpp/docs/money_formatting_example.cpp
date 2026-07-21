// Example demonstrating Money formatting with different locales
// This is NOT part of the build - just for documentation

#include "mypaymentservice_domain/value_objects/money.h"
#include <iostream>

using namespace mypaymentservice::domain::value_objects;

int main() {
    // Create various amounts
    Money small("10.50", "USD");
    Money medium("1234.56", "EUR");
    Money large = Money::FromCents(123456789, "USD");  // $1,234,567.89

    std::cout << "=== US Format (period as decimal, comma as thousands) ===\n";
    std::cout << "Small:  " << small.ToString('.', ',', true) << "\n";
    std::cout << "Medium: " << medium.ToString('.', ',', true) << "\n";
    std::cout << "Large:  " << large.ToString('.', ',', true) << "\n";
    std::cout << "\n";

    std::cout << "=== European Format (comma as decimal, period as thousands) ===\n";
    std::cout << "Small:  " << small.ToString(',', '.', true) << "\n";
    std::cout << "Medium: " << medium.ToString(',', '.', true) << "\n";
    std::cout << "Large:  " << large.ToString(',', '.', true) << "\n";
    std::cout << "\n";

    std::cout << "=== Swiss Format (period as decimal, apostrophe as thousands) ===\n";
    std::cout << "Small:  " << small.ToString('.', '\'', true) << "\n";
    std::cout << "Medium: " << medium.ToString('.', '\'', true) << "\n";
    std::cout << "Large:  " << large.ToString('.', '\'', true) << "\n";
    std::cout << "\n";

    std::cout << "=== Without Currency Code ===\n";
    std::cout << "Small:  " << small.ToString('.', ',', false) << "\n";
    std::cout << "Medium: " << medium.ToString('.', ',', false) << "\n";
    std::cout << "Large:  " << large.ToString('.', ',', false) << "\n";
    std::cout << "\n";

    std::cout << "=== Using Default (US Format) ===\n";
    std::cout << "Small:  " << small.ToString() << "\n";
    std::cout << "Medium: " << medium.ToString() << "\n";
    std::cout << "Large:  " << large.ToString() << "\n";
    std::cout << "\n";

    std::cout << "=== Input Flexibility ===\n";
    Money from_us("1,234.56", "USD");      // Comma ignored, period is decimal
    Money from_eu("1.234,56", "EUR");      // Period ignored, comma is decimal
    Money from_plain("1234.56", "GBP");    // Simple format

    std::cout << "US input (1,234.56):   " << from_us.ToString() << " = " << from_us.GetCents() << " cents\n";
    std::cout << "EU input (1.234,56):   " << from_eu.ToString() << " = " << from_eu.GetCents() << " cents\n";
    std::cout << "Plain (1234.56):       " << from_plain.ToString() << " = " << from_plain.GetCents() << " cents\n";

    return 0;
}

/* Expected Output:

=== US Format (period as decimal, comma as thousands) ===
Small:  10.50 USD
Medium: 1,234.56 EUR
Large:  1,234,567.89 USD

=== European Format (comma as decimal, period as thousands) ===
Small:  10,50 USD
Medium: 1.234,56 EUR
Large:  1.234.567,89 USD

=== Swiss Format (period as decimal, apostrophe as thousands) ===
Small:  10.50 USD
Medium: 1'234.56 EUR
Large:  1'234'567.89 USD

=== Without Currency Code ===
Small:  10.50
Medium: 1,234.56
Large:  1,234,567.89

=== Using Default (US Format) ===
Small:  10.50 USD
Medium: 1,234.56 EUR
Large:  1,234,567.89 USD

=== Input Flexibility ===
US input (1,234.56):   1,234.56 USD = 123456 cents
EU input (1.234,56):   1,234.56 EUR = 123456 cents
Plain (1234.56):       1,234.56 GBP = 123456 cents

*/
