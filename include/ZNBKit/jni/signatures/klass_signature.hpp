//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

#include "ZNBKit/jni/internal/wrapper.hpp"
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
            const auto klass = wrapper::search_for_class(env, klass_name);
            owner = reinterpret_cast<jclass>(wrapper::add_global_ref(env, klass));
            wrapper::remove_local_ref(env, klass);
        }

        klass_signature(JNIEnv *env, jclass owner)
        {
            if (env == nullptr || owner == nullptr)
            {
                throw std::invalid_argument("JNIEnv or class is invalid");
            }

            this->env = env;
            this->owner = reinterpret_cast<jclass>(wrapper::add_global_ref(env, owner));
        }

        ~klass_signature()
        {
            if (owner != nullptr && env != nullptr)
            {
                wrapper::remove_global_ref(env, owner);
                owner = nullptr;
            }
        }

        klass_signature(const klass_signature& other) : env(other.env) {
            if (other.owner) {
                owner = reinterpret_cast<jclass>(wrapper::add_global_ref(env, other.owner));
            } else {
                owner = nullptr;
            }
        }

        klass_signature& operator=(const klass_signature& other) {
            if (this != &other) {
                if (owner && env) {
                    wrapper::remove_global_ref(env, owner);
                }

                env = other.env;
                if (other.owner) {
                    owner = reinterpret_cast<jclass>(wrapper::add_global_ref(env, other.owner));
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