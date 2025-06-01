//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include "ZNBKit/jni/signatures/klass_signature.hpp"

namespace znb_kit
{
    template <typename T>
    class method_signature
    {
    protected:
        JNIEnv *env;
        jmethodID identity;

        std::shared_ptr<klass_signature> owner;

        bool is_static;

        [[nodiscard]] jmethodID build_identity() const
        {

            if (owner == nullptr || env == nullptr)
            {
                throw std::runtime_error("method_signature::build_identity: owner or env is null for method '" + name + "' with signature '" + signature + "'");
            }

            const jclass klass = owner->get_owner();

            if (klass == nullptr)
            {
                throw std::runtime_error("method_signature::build_identity: owner->get_owner() returned null jclass for method '" + name + "' with signature '" + signature + "' (declaring class: unknown)");
            }

            const auto method_id_val = get_method_id(env, klass, name, signature, this->is_static);

            if (env->ExceptionCheck())
            {
                env->ExceptionDescribe();
                env->ExceptionClear();
                throw std::runtime_error("JNI exception occurred while getting method ID for '" + name + "' with signature '" + signature + "' in class (jclass was not null).");
            }

            if (method_id_val == nullptr)
            {
                throw std::runtime_error("Unable to get method ID (returned null) for '" + name + "' with signature '" + signature + "' in class (jclass was not null).");
            }

            return method_id_val;
        }
    public:

        const std::string name;
        const std::string signature;

        std::optional<std::vector<std::string>> parameters;

        virtual ~method_signature() = default;

        method_signature(JNIEnv *env, std::shared_ptr<klass_signature> owner, std::string name, std::string signature, std::optional<std::vector<std::string>> parameters, const bool is_static)
            :
            env(env),
            owner(std::move(owner)),
            is_static(is_static),
            name(std::move(name)),
            signature(std::move(signature)),
            parameters(std::move(parameters))
        {
            identity = build_identity();
        }

        explicit method_signature(nullptr_t) = delete;
        method_signature(const method_signature &) = delete;
        method_signature &operator=(const method_signature &) = delete;

        method_signature(method_signature &&other) noexcept
            :
            env(other.env),
            identity(other.identity),
            owner(std::move(other.owner)),
            is_static(other.is_static),
            name(other.name),
            signature(other.signature),
            parameters(std::move(other.parameters))
        {
        }

        method_signature &operator=(method_signature &&other) noexcept
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

        virtual T invoke(const jobject &instance, std::vector<jvalue> &parameters) = 0;

        [[nodiscard]] jclass get_owner() const
        {
            return owner->get_owner();
        }

        [[nodiscard]] jmethodID get_identity() const
        {
            return identity;
        }
    };
}