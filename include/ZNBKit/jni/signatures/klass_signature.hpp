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
        jni_global_ref<jclass> owner;

        const std::string klass_name;
    public:
        klass_signature(JNIEnv *jni, const std::string &klass_name): jni(jni), klass_name(klass_name)
        {
            const auto klass = wrapper::search_for_class(jni, klass_name);
            this->owner = wrapper::change_reference_policy<jni_local_ref<jclass>>(jni, wrapper::jni_reference_policy::GLOBAL, klass);
        }

        klass_signature(JNIEnv *jni, const jni_local_ref<jclass> &owner)
        {
            VAR_CHECK(jni);
            VAR_CHECK(owner);

            this->jni = jni;
            this->owner = wrapper::change_reference_policy<jni_local_ref<jclass>>(jni, wrapper::jni_reference_policy::GLOBAL, owner);
        }

        ~klass_signature()
        {
            if (owner != nullptr && jni != nullptr)
            {
                owner = nullptr;
            }
        }

        [[nodiscard]] jclass get_owner() const
        {
            return *owner;
        }

        [[nodiscard]] const std::string &get_klass_name() const
        {
            return klass_name;
        }
    };
}