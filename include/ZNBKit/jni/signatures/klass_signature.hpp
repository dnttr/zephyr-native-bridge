//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

#include "ZNBKit/jni/utils/util.hpp"

namespace znb_kit
{
    class klass_signature
    {
        JNIEnv *env;
        jclass owner;

    public:
        klass_signature(JNIEnv *env, const std::string &klass_name): env(env)
        {
            if (env == nullptr || klass_name.empty())
            {
                throw std::invalid_argument("JNIEnv or class is invalid");
            }

            const auto klass = get_klass(env, klass_name);
            if (klass == nullptr) {
                throw std::runtime_error("Unable to find class " + klass_name);
            }

            owner = reinterpret_cast<jclass>(trackNewGlobalRef(env, klass));
            trackDeleteLocalRef(env, klass);
        }

        klass_signature(JNIEnv *env, jclass owner)
        {
            if (env == nullptr || owner == nullptr)
            {
                throw std::invalid_argument("JNIEnv or class is invalid");
            }

            this->env = env;
            this->owner = reinterpret_cast<jclass>(trackNewGlobalRef(env, owner));
        }

        ~klass_signature()
        {
            if (owner != nullptr && env != nullptr)
            {
                trackDeleteGlobalRef(env, owner);
                owner = nullptr;
            }
        }

        klass_signature(const klass_signature& other) : env(other.env) {
            if (other.owner) {
                owner = reinterpret_cast<jclass>(trackNewGlobalRef(env, other.owner));
            } else {
                owner = nullptr;
            }
        }

        klass_signature& operator=(const klass_signature& other) {
            if (this != &other) {
                if (owner && env) {
                    trackDeleteGlobalRef(env, owner);
                }

                env = other.env;
                if (other.owner) {
                    owner = reinterpret_cast<jclass>(trackNewGlobalRef(env, other.owner));
                } else {
                    owner = nullptr;
                }
            }
            return *this;
        }

        [[nodiscard]] jclass get_owner() const
        {
            return owner;
        }
    };
}