#include "mypaymentservice_http_host/configuration/http_server.h"

#include <QHostAddress>

namespace mypaymentservice::http_host::configuration {

HttpServer::HttpServer(
    core::application::use_cases::AuthorizePaymentUseCase* authorize_use_case,
    core::application::use_cases::GetPaymentUseCase* get_use_case) {
    payments_controller_ = std::make_unique<controllers::PaymentsController>(
        authorize_use_case, get_use_case);
    SetupRoutes();
}

void HttpServer::SetupRoutes() {
    // POST /api/payments/authorize
    server_.route("/api/payments/authorize", QHttpServerRequest::Method::Post,
        [this](const QHttpServerRequest& request) {
            return payments_controller_->AuthorizePayment(request);
        });

    // GET /api/payments/{id}
    server_.route("/api/payments/<arg>", QHttpServerRequest::Method::Get,
        [this](const QString& id) {
            return payments_controller_->GetPayment(id);
        });
}

bool HttpServer::Start(quint16 port) {
    if (!tcp_server_.listen(QHostAddress::Any, port)) {
        return false;
    }
    return server_.bind(&tcp_server_);
}

} // namespace mypaymentservice::http_host::configuration
