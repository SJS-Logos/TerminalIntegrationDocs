#pragma once

#include <QHttpServer>
#include <QTcpServer>
#include <memory>

#include "mypaymentservice_http_host/controllers/payments_controller.h"
#include "mypaymentservice_core/application/use_cases/authorize_payment_use_case.h"
#include "mypaymentservice_core/application/use_cases/get_payment_use_case.h"

namespace mypaymentservice::http_host::configuration {

/// @brief HTTP server for the payment service.
/// @details Configures routes and starts the Qt HTTP server. This is purely
///          mechanical wiring (AP-003 �5.3); it contains no business logic.
class HttpServer {
public:
    HttpServer(
        core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case,
        core::application::use_cases::GetPaymentUseCase* get_use_case);

    /// Start the server on the specified port. Returns true on success.
    bool Start(quint16 port);

private:
    void SetupRoutes();

    QHttpServer server_;
    QTcpServer tcp_server_;
    std::unique_ptr<controllers::PaymentsController> payments_controller_;
};

} // namespace mypaymentservice::http_host::configuration
