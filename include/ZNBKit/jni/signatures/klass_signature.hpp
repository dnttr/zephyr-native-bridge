//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <string>

#include "ZNBKit/jni/internal/wrapper.hpp"

namespace znb_kit
{
    class klass_signature
    {
        JNIEnv *jni;
        jclass owner;

        const std::string klass_name;
    public:
        klass_signature(JNIEnv *jni, const std::string &klass_name): jni(jni), klass_name(klass_name)
        {
            const auto klass = wrapper::search_for_class(jni, klass_name);
            owner = reinterpret_cast<jclass>(wrapper::add_global_ref(jni, klass));
            wrapper::remove_local_ref(jni, klass);
        }

        klass_signature(JNIEnv *jni, const jclass &owner)
        {
            VAR_CHECK(jni);
            VAR_CHECK(owner);

            this->jni = jni;
            this->owner = reinterpret_cast<jclass>(wrapper::add_global_ref(jni, owner));
        }

        ~klass_signature()
        {
            if (owner != nullptr && jni != nullptr)
            {
                wrapper::remove_global_ref(jni, owner);
                owner = nullptr;
            }
        }

        klass_signature(const klass_signature& other) : jni(other.jni) {
            if (other.owner) {
                owner = reinterpret_cast<jclass>(wrapper::add_global_ref(jni, other.owner));
            } else {
                owner = nullptr;
            }
        }

        klass_signature& operator=(const klass_signature& other) {
            if (this != &other) {
                if (owner && jni) {
                    wrapper::remove_global_ref(jni, owner);
                }

                jni = other.jni;
                if (other.owner) {
                    owner = reinterpret_cast<jclass>(wrapper::add_global_ref(jni, other.owner));
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