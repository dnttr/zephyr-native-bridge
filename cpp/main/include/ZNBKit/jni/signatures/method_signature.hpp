//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <optional>
#include <string>
#include <vector>

#include "ZNBKit/jni/signatures/klass_signature.hpp"

namespace znb_kit
{
    template <typename T>
    class method_signature
    {
    protected:
        JNIEnv *env;
        jmethodID identity;

        std::string name;
        std::string signature;

        std::optional<std::vector<std::string>> parameters;
        klass_signature *owner;

        bool is_static;

        [[nodiscard]] jmethodID build_identity() const
        {
            const auto identity = get_method_id(env, owner->get_owner(), name, signature, is_static);

            if (identity == nullptr)
            {
                throw std::runtime_error("Unable to construct method identity for '" + name + "' with signature '" + signature + "'");
            }

            return identity;
        }
    public:
        virtual ~method_signature() = default;

        method_signature(JNIEnv *env, klass_signature *owner, std::string &name, std::string &signature, std::optional<std::vector<std::string>> parameters, const bool is_static)
            :
            env(env),
            name(std::move(name)),
            signature(std::move(signature)),
            parameters(std::move(parameters)),
            is_static(is_static)
        {
            this->owner = owner;
            identity = build_identity();
        }

        explicit method_signature(nullptr_t) = delete;
        method_signature(const method_signature &) = delete;
        method_signature &operator=(const method_signature &) = delete;

        method_signature(method_signature &&other) noexcept
            :
            env(other.env),
            identity(other.identity),
            name(std::move(other.name)),
            signature(std::move(other.signature)),
            parameters(std::move(other.parameters)),
            is_static(other.is_static)
        {
            owner = other.owner;
        }

        method_signature &operator=(method_signature &&other) noexcept
        {
            if (this != &other)
            {
                env = other.env;
                identity = other.identity;
                owner = other.owner,
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