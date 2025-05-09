//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <memory>
#include <string>
#include <vector>

#include "jni/proxies/KlassProxy.hpp"

class MethodProxy
{
    JNIEnv *env;
    jmethodID identity;

    std::unique_ptr<KlassProxy> owner;

    std::string name;
    std::string signature;
    std::vector<std::string> parameters;
    bool is_static;

    [[nodiscard]] jmethodID build_identity() const
    {
        const auto identity = is_static ? env->GetStaticMethodID(owner->get_klass(), name.c_str(), signature.c_str())
                                        : env->GetMethodID(owner->get_klass(), name.c_str(), signature.c_str());

        if (identity == nullptr)
        {
            throw std::runtime_error("Unable to construct method identity for '" + name + "' with signature '" + signature + "'");
        }

        return identity;
    }
public:

    MethodProxy(JNIEnv *env, std::unique_ptr<KlassProxy> owner, std::string &name, std::string &signature, std::vector<std::string> parameters, const bool is_static)
        :
        env(env),
        owner(std::move(owner)),
        name(std::move(name)),
        signature(std::move(signature)),
        parameters(std::move(parameters)),
        is_static(is_static)
    {
        identity = build_identity();
    }

    explicit MethodProxy(nullptr_t) = delete;

    MethodProxy(const MethodProxy &) = delete;
    MethodProxy &operator=(const MethodProxy &) = delete;

    MethodProxy(MethodProxy &&other) noexcept
        :
        env(other.env),
        identity(other.identity),
        owner(std::move(other.owner)),
        name(std::move(other.name)),
        signature(std::move(other.signature)),
        parameters(std::move(other.parameters)),
        is_static(other.is_static)
    {
    }

    MethodProxy &operator=(MethodProxy &&other) noexcept
    {
        if (this != &other)
        {
            identity = other.identity;
            owner = std::move(other.owner);
            name = std::move(other.name);
            signature = std::move(other.signature);
            parameters = std::move(other.parameters);
            is_static = other.is_static;
        }
        return *this;
    }

};
