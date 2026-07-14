namespace Logos.Payment.Service.Core.SharedKernel;

public class Money
{
    private readonly decimal _amount;
    private readonly string _currency;

    public Money(decimal amount, string currency)
    {
        if (amount < 0)
            throw new ArgumentException("Amount cannot be negative", nameof(amount));

        if (string.IsNullOrWhiteSpace(currency))
            throw new ArgumentException("Currency is required", nameof(currency));

        _amount = amount;
        _currency = currency.ToUpperInvariant();
    }

    public decimal GetAmount() => _amount;
    public string GetCurrency() => _currency;

    public bool Equals(Money? other)
    {
        if (other is null) return false;
        return _amount == other._amount && _currency == other._currency;
    }

    public override bool Equals(object? obj) => obj is Money money && Equals(money);
    public override int GetHashCode() => HashCode.Combine(_amount, _currency);

    public override string ToString() => $"{_amount:F2} {_currency}";
}
