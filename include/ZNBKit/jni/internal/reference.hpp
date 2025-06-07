//
// Created by Damian Netter on 05/06/2025.
//

#pragma once

#include <memory>

#include "internal.hpp"

namespace internal
{
    class jni_string_reference
    {
        std::string value;

    public:
        jni_string_reference(JNIEnv *jni, const jstring &ref)
        {
            VAR_CHECK(jni);
            VAR_CHECK(ref);

            const char *string_utf_chars = jni->GetStringUTFChars(ref, nullptr);
            VAR_CHECK(string_utf_chars);
            EXCEPT_CHECK(jni);

            const std::string str(string_utf_chars);
            jni->ReleaseStringUTFChars(ref, string_utf_chars);

            value = str;
        }

        [[nodiscard]] const std::string &get() const { return value; }
    };

    class policy
    {
    public:
        struct local_policy
        {
            void operator()(JNIEnv *jni, const jobject &ref) const {
                if (ref)
                {
                    tracker_manager::local_refs.erase(ref);
                    tracker_manager::local_ref_sources.erase(ref);

                    jni->DeleteLocalRef(ref);
                }
            }
        };

        struct global_policy
        {
            void operator()(JNIEnv *jni, const jobject &ref) const {
                if (ref)
                {
                    global_tracker::remove(ref);

                    jni->DeleteGlobalRef(ref);
                }
            }
        };

        template <typename T, typename policy>
        struct deleter
        {
            JNIEnv *jni;

            void operator()(T ref)
            {
                if (ref)
                {
                    policy{}(jni, ref);
                }
            }
        };

        template <typename policy>
        struct deleter_val
        {
            JNIEnv *jni;

            void operator()(const jvalue &ref) const
            {
                if (ref.l)
                {
                    policy{}(jni, ref.l);
                }
            }
        };

        struct deleter_string
        {
            JNIEnv* env;
            jstring str;

            void operator()(const char* ptr) const {
                if (ptr) {
                    env->ReleaseStringUTFChars(str, ptr);
                }
            }
        };
    };
}

namespace znb_kit
{
    template <typename T, typename Policy, template <typename, typename> class Deleter = internal::policy::deleter>
    class reference
    {
        std::unique_ptr<T, internal::policy::deleter<T, Deleter<T, Policy>>> ptr;

    public:

        reference() = default;

        explicit reference(T ref, JNIEnv *jni): ptr(ref, Deleter<T, Policy>(jni))
        {
        }

        reference(const reference &other, JNIEnv *jni)
        {
            if (other.ptr && jni)
            {
                T new_ref = nullptr;

                if constexpr (std::is_same_v<Policy, internal::policy::local_policy>)
                {
                    new_ref = static_cast<T>(internal::tracker_manager::add_local_ref(jni, other.ptr.get()));
                }
                else
                {
                    new_ref = static_cast<T>(internal::tracker_manager::add_global_ref(jni, other.ptr.get()));
                }

                ptr.reset(new_ref, Deleter<T, Policy>(jni));
            }
        }

        T get() const
        {
            return ptr.get();
        }

        explicit operator bool() const
        {
            return ptr.get() != nullptr;
        }

        T operator*() const
        {
            return ptr.get();
        }

        T release()
        {
            return ptr.release();
        }

        reference(reference&&) noexcept = default;
        reference& operator=(reference&&) noexcept = default;

        reference(const reference &) = delete;
        reference& operator=(const reference &) = delete;

        reference& operator=(const reference &other, JNIEnv *jni)
        {
            if (this != &other)
            {
                reference temp(other, jni);
                std::swap(ptr, temp.ptr);
            }

            return *this;
        }

        reference& assign(JNIEnv *jni, const reference &other)
        {
            if (this != &other)
            {
                reference temp(other, jni);
                std::swap(ptr, temp.ptr);
            }
            return *this;
        }
    };
}
