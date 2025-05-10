//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <memory>
#include <string>
#include <vector>

#include "jni/proxies/klass_proxy.hpp"

class method_proxy
{
    JNIEnv *env;
    jmethodID identity;

    std::string name;
    std::string signature;

    std::unique_ptr<klass_proxy> owner;
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

    method_proxy(JNIEnv *env, std::unique_ptr<klass_proxy> owner, std::string &name, std::string &signature, std::vector<std::string> parameters, const bool is_static)
        :
        env(env),
        name(std::move(name)),
        signature(std::move(signature)),
        owner(std::move(owner)),
        parameters(std::move(parameters)),
        is_static(is_static)
    {
        identity = build_identity();
    }

    explicit method_proxy(nullptr_t) = delete;

    method_proxy(const method_proxy &) = delete;
    method_proxy &operator=(const method_proxy &) = delete;

    method_proxy(method_proxy &&other) noexcept
        :
        env(other.env),
        identity(other.identity),
        name(std::move(other.name)),
        signature(std::move(other.signature)),
        owner(std::move(other.owner)),
        parameters(std::move(other.parameters)),
        is_static(other.is_static)
    {
    }

    method_proxy &operator=(method_proxy &&other) noexcept
    {
        if (this != &other)
        {
            env = other.env;
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
