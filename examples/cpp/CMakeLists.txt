cmake_minimum_required(VERSION 3.20)
project(LogosPaymentService VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core library (Domain + SharedKernel + Capabilities + Application)
add_library(logos_payment_service_core STATIC
    logos_payment_service_core/shared_kernel/money.cpp
    logos_payment_service_core/shared_kernel/money.h
    logos_payment_service_core/shared_kernel/payment_status.h
    logos_payment_service_core/shared_kernel/payment_record.h
    logos_payment_service_core/domain/services/payment_authorization_service.cpp
    logos_payment_service_core/domain/services/payment_authorization_service.h
    logos_payment_service_core/capabilities/fraud_detection_service.h
    logos_payment_service_core/capabilities/payment_repository.h
    logos_payment_service_core/application/use_cases/authorize_payment_use_case.cpp
    logos_payment_service_core/application/use_cases/authorize_payment_use_case.h
    logos_payment_service_core/application/use_cases/get_payment_use_case.cpp
    logos_payment_service_core/application/use_cases/get_payment_use_case.h
    logos_payment_service_core/application/contracts/authorize_payment_request.h
    logos_payment_service_core/application/contracts/authorize_payment_response.h
    logos_payment_service_core/application/contracts/get_payment_response.h
    logos_payment_service_core/application/container/service_container.h
)

target_include_directories(logos_payment_service_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Infrastructure library (capability implementations)
add_library(logos_payment_service_infrastructure STATIC
    logos_payment_service_infrastructure/in_memory_payment_repository.cpp
    logos_payment_service_infrastructure/in_memory_payment_repository.h
    logos_payment_service_infrastructure/simple_fraud_detection_service.cpp
    logos_payment_service_infrastructure/simple_fraud_detection_service.h
)

target_link_libraries(logos_payment_service_infrastructure PUBLIC
    logos_payment_service_core
)

target_include_directories(logos_payment_service_infrastructure PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# CLI Host executable
add_executable(payment_cli
    logos_payment_service_cli_host/main.cpp
    logos_payment_service_cli_host/cli_parser.cpp
    logos_payment_service_cli_host/cli_parser.h
    logos_payment_service_cli_host/commands/authorize_command.cpp
    logos_payment_service_cli_host/commands/authorize_command.h
    logos_payment_service_cli_host/commands/get_payment_command.cpp
    logos_payment_service_cli_host/commands/get_payment_command.h
)

target_link_libraries(payment_cli PRIVATE
    logos_payment_service_core
    logos_payment_service_infrastructure
)

target_include_directories(payment_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# HTTP Host executable (Qt-based, AP-003 Incoming Implementation)
# Built only when Qt6 with the HttpServer module is available, so the default
# build (CLI host) remains functional without Qt installed.
find_package(Qt6 COMPONENTS Core Network HttpServer QUIET)

if(Qt6_FOUND)
    set(CMAKE_AUTOMOC ON)

    add_executable(payment_http_host
        logos_payment_service_http_host/main.cpp
        logos_payment_service_http_host/configuration/http_server.cpp
        logos_payment_service_http_host/configuration/http_server.h
        logos_payment_service_http_host/controllers/payments_controller.cpp
        logos_payment_service_http_host/controllers/payments_controller.h
        logos_payment_service_http_host/mappings/payment_dto.cpp
        logos_payment_service_http_host/mappings/payment_dto.h
    )

    target_link_libraries(payment_http_host PRIVATE
        logos_payment_service_core
        logos_payment_service_infrastructure
        Qt6::Core
        Qt6::Network
        Qt6::HttpServer
    )

    target_include_directories(payment_http_host PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
else()
    message(STATUS "Qt6 (Core, Network, HttpServer) not found - skipping payment_http_host target.")
endif()
