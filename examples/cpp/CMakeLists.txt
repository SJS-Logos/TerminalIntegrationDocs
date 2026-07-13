cmake_minimum_required(VERSION 3.20)
project(LogosPaymentService VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Domain library
add_library(logos_payment_service_domain STATIC
    logos_payment_service_domain/value_objects/money.cpp
    logos_payment_service_domain/value_objects/money.h
    logos_payment_service_domain/value_objects/payment_status.h
    logos_payment_service_domain/value_objects/payment_record.h
    logos_payment_service_domain/services/payment_authorization_service.cpp
    logos_payment_service_domain/services/payment_authorization_service.h
    logos_payment_service_domain/abstractions/fraud_detection_service.h
    logos_payment_service_domain/abstractions/payment_repository.h
)

target_include_directories(logos_payment_service_domain PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Application library
add_library(logos_payment_service_application STATIC
    logos_payment_service_application/use_cases/authorize_payment_use_case.cpp
    logos_payment_service_application/use_cases/authorize_payment_use_case.h
    logos_payment_service_application/use_cases/get_payment_use_case.cpp
    logos_payment_service_application/use_cases/get_payment_use_case.h
    logos_payment_service_application/contracts/authorize_payment_request.h
    logos_payment_service_application/contracts/authorize_payment_response.h
    logos_payment_service_application/contracts/get_payment_response.h
    logos_payment_service_application/container/service_container.h
)

target_link_libraries(logos_payment_service_application PUBLIC
    logos_payment_service_domain
)

target_include_directories(logos_payment_service_application PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Adapters library
add_library(logos_payment_service_adapters STATIC
    logos_payment_service_adapters/in_memory_payment_repository.cpp
    logos_payment_service_adapters/in_memory_payment_repository.h
    logos_payment_service_adapters/simple_fraud_detection_service.cpp
    logos_payment_service_adapters/simple_fraud_detection_service.h
)

target_link_libraries(logos_payment_service_adapters PUBLIC
    logos_payment_service_domain
)

target_include_directories(logos_payment_service_adapters PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# CLI executable
add_executable(payment_cli
    logos_payment_service_cli/main.cpp
    logos_payment_service_cli/cli_parser.cpp
    logos_payment_service_cli/cli_parser.h
    logos_payment_service_cli/commands/authorize_command.cpp
    logos_payment_service_cli/commands/authorize_command.h
    logos_payment_service_cli/commands/get_payment_command.cpp
    logos_payment_service_cli/commands/get_payment_command.h
)

target_link_libraries(payment_cli PRIVATE
    logos_payment_service_application
    logos_payment_service_adapters
)

target_include_directories(payment_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
