#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>

namespace mypaymentservice::core::application::container {

/// Simple service container for dependency injection.
/// Owns service instances and provides type-safe access.
class ServiceContainer {
public:
    template<typename TInterface>
    void Register(std::unique_ptr<TInterface> service) {
        services_[std::type_index(typeid(TInterface))] =
            std::unique_ptr<ServiceHolder>(new TypedHolder<TInterface>(std::move(service)));
    }

    template<typename TInterface>
    TInterface* Resolve() {
        auto type = std::type_index(typeid(TInterface));
        auto it = services_.find(type);
        if (it == services_.end()) {
            throw std::runtime_error("Service not registered");
        }
        return static_cast<TypedHolder<TInterface>*>(it->second.get())->instance.get();
    }

private:
    struct ServiceHolder {
        virtual ~ServiceHolder() = default;
    };

    template<typename T>
    struct TypedHolder : ServiceHolder {
        std::unique_ptr<T> instance;
        explicit TypedHolder(std::unique_ptr<T> inst) : instance(std::move(inst)) {}
    };

    std::unordered_map<std::type_index, std::unique_ptr<ServiceHolder>> services_;
};

} // namespace mypaymentservice::core::application::container
