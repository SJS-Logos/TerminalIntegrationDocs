// Example demonstrating Money precision vs floating-point
// This is NOT part of the build - just for documentation

#include "logos_payment_service_domain/value_objects/money.h"
#include <iostream>
#include <iomanip>

using namespace logos::payment_service::domain::value_objects;

int main() {
    std::cout << std::fixed << std::setprecision(20);

    // Example 1: Classic floating-point problem
    std::cout << "=== Floating-Point Precision Problem ===\n\n";

    double double_a = 0.1;
    double double_b = 0.2;
    double double_sum = double_a + double_b;

    std::cout << "Using double:\n";
    std::cout << "  0.1 + 0.2 = " << double_sum << "\n";
    std::cout << "  Expected:   0.30000000000000000000\n";
    std::cout << "  Equal to 0.3? " << (double_sum == 0.3 ? "YES" : "NO") << "\n\n";

    // Example 2: Using Money with integer cents
    Money money_a(0.1, "USD");
    Money money_b(0.2, "USD");
    Money money_sum = money_a.Add(money_b);

    std::cout << "Using Money (integer cents):\n";
    std::cout << "  $0.10 + $0.20 = $" << money_sum.GetAmount() << "\n";
    std::cout << "  Internal cents: " << money_a.GetCents() << " + " 
              << money_b.GetCents() << " = " << money_sum.GetCents() << "\n";
    std::cout << "  Equal to $0.30? " << (money_sum.GetCents() == 30 ? "YES" : "NO") << "\n\n";

    // Example 3: Accumulation errors
    std::cout << "=== Accumulation Error Test ===\n\n";

    double double_total = 0.0;
    for (int i = 0; i < 10; i++) {
        double_total += 0.1;
    }

    std::cout << "Using double:\n";
    std::cout << "  0.1 * 10 = " << double_total << "\n";
    std::cout << "  Expected:  1.00000000000000000000\n";
    std::cout << "  Error:     " << (1.0 - double_total) << "\n\n";

    Money money_total = Money::Zero("USD");
    Money increment(0.1, "USD");
    for (int i = 0; i < 10; i++) {
        money_total = money_total.Add(increment);
    }

    std::cout << "Using Money:\n";
    std::cout << "  $0.10 * 10 = $" << money_total.GetAmount() << "\n";
    std::cout << "  Internal cents: " << money_total.GetCents() << "\n";
    std::cout << "  Exact? " << (money_total.GetCents() == 100 ? "YES" : "NO") << "\n\n";

    // Example 4: Large number of operations
    std::cout << "=== Large Scale Test (1000 operations) ===\n\n";

    double double_account = 1000.0;
    for (int i = 0; i < 1000; i++) {
        double_account -= 0.1;
    }

    std::cout << "Using double:\n";
    std::cout << "  $1000.00 - ($0.10 * 1000) = $" << double_account << "\n";
    std::cout << "  Expected: $900.00\n";
    std::cout << "  Error:    $" << (900.0 - double_account) << "\n\n";

    Money money_account(1000.0, "USD");
    Money debit(0.1, "USD");
    for (int i = 0; i < 1000; i++) {
        Money new_balance = Money::FromCents(
            money_account.GetCents() - debit.GetCents(), 
            "USD"
        );
        money_account = new_balance;
    }

    std::cout << "Using Money:\n";
    std::cout << "  $1000.00 - ($0.10 * 1000) = $" << money_account.GetAmount() << "\n";
    std::cout << "  Internal cents: " << money_account.GetCents() << "\n";
    std::cout << "  Exact? " << (money_account.GetCents() == 90000 ? "YES" : "NO") << "\n";

    return 0;
}
