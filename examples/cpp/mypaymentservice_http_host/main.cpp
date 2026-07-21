#include "mypaymentservice_http_host/configuration/http_server.h"
#include "mypaymentservice_core/application/container/service_container.h"
#include "mypaymentservice_core/application/use_cases/authorize_payment_use_case.h"
#include "mypaymentservice_core/application/use_cases/get_payment_use_case.h"
#include "mypaymentservice_core/domain/services/payment_authorization_service.h"
#include "mypaymentservice_core/shared_kernel/money.h"
#include "mypaymentservice_infrastructure/in_memory_payment_repository.h"
#include "mypaymentservice_infrastructure/simple_fraud_detection_service.h"

#include <QCoreApplication>
#include <iostream>
#include <memory>

using namespace mypaymentservice;

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    // Create service container.
    core::application::container::ServiceContainer container;

    // Register infrastructure (capability implementations).
    container.Register<core::capabilities::IPaymentRepository>(
        std::make_unique<infrastructure::InMemoryPaymentRepository>());

    container.Register<core::capabilities::IFraudDetectionService>(
        std::make_unique<infrastructure::SimpleFraudDetectionService>());

    // Register domain service with configuration.
    auto maximum_amount = core::shared_kernel::Money::FromMinorUnits(1000000, "USD");  // $10,000.00
    container.Register<core::domain::services::PaymentAuthorizationService>(
        std::make_unique<core::domain::services::PaymentAuthorizationService>(
            container.Resolve<core::capabilities::IFraudDetectionService>(),
            maximum_amount));

    // Create use cases with resolved dependencies.
    core::application::use_cases::AuthorizePaymentUseCase authorize_use_case(
        container.Resolve<core::domain::services::PaymentAuthorizationService>(),
        container.Resolve<core::capabilities::IPaymentRepository>());

    core::application::use_cases::GetPaymentUseCase get_use_case(
        container.Resolve<core::capabilities::IPaymentRepository>());

    // Start HTTP server.
    const quint16 port = 8080;
    http_host::configuration::HttpServer server(&authorize_use_case, &get_use_case);
    if (!server.Start(port)) {
        std::cerr << "Failed to start HTTP server on port " << port << "\n";
        return 1;
    }

    std::cout << "Payment HTTP host listening on port " << port << "\n";
    std::cout << "  POST /api/payments/authorize\n";
    std::cout << "  GET  /api/payments/{id}\n";

    return app.exec();
}
