using Microsoft.EntityFrameworkCore;

namespace Logos.Payment.Service.Infrastructure.EntityFramework;

/// <summary>
/// Entity Framework <see cref="DbContext"/> confined to the persistence Adapter Unit.
/// The core never references it (AP-007 §9.1).
/// </summary>
public class PaymentDbContext : DbContext
{
    public PaymentDbContext(DbContextOptions<PaymentDbContext> options)
        : base(options)
    {
    }

    public DbSet<PaymentRow> Payments => Set<PaymentRow>();

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        var payment = modelBuilder.Entity<PaymentRow>();
        payment.ToTable("Payments");
        payment.HasKey(p => p.Id);
        payment.Property(p => p.Id).HasMaxLength(64);
        payment.Property(p => p.Amount).HasColumnType("TEXT");
        payment.Property(p => p.Currency).HasMaxLength(3).IsRequired();
        payment.Property(p => p.MerchantId).HasMaxLength(128).IsRequired();
        payment.Property(p => p.Status).HasMaxLength(16).IsRequired();
        payment.Property(p => p.DeclineReason).HasMaxLength(256);
    }
}
