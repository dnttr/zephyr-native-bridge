//
// Created by Damian Netter on 01/06/2025.
//

#include "ZNBKit/jni/internal/wrapper.hpp"

#include <vector>

#include "ZNBKit/debug.hpp"


namespace znb_kit {

    jobject wrapper::add_local_ref(JNIEnv *jni, const jobject &obj,
                                  const std::string &file, int line,
                                  const std::string &method)
    {
        VAR_CHECK(jni);

        const auto ref = jni->NewLocalRef(obj);

        if (ref)
        {
            internal::tracker_manager::local_refs.insert(ref);
            internal::tracker_manager::local_ref_sources[ref] = {file, line, method};
        }

        return ref;
    }

    void wrapper::remove_local_ref(JNIEnv *jni, const jobject &obj)
    {
        VAR_CHECK(jni);

        if (obj)
        {
            internal::tracker_manager::local_refs.erase(obj);
            internal::tracker_manager::local_ref_sources.erase(obj);
            jni->DeleteLocalRef(obj);
        }
    }

    jobject wrapper::add_global_ref(JNIEnv *jni, const jobject &obj,
                                   const std::string &file, int line,
                                   const std::string &method)
    {
        VAR_CHECK(jni);

        const auto ref = jni->NewGlobalRef(obj);

        if (ref)
        {
            internal::global_tracker::add(ref, file, line, method);
        }

        return ref;
    }

    void wrapper::remove_global_ref(JNIEnv *jni, const jobject &obj)
    {
        VAR_CHECK(jni);

        if (obj)
        {
            internal::global_tracker::remove(obj);
            jni->DeleteGlobalRef(obj);
        }
    }

    void wrapper::check_for_corruption()
    {
        internal::tracker_manager::check_for_corruption();
    }

    jni_local_ref<jclass> wrapper::search_for_class(JNIEnv *jni, const std::string &name,
                                     const std::string &caller_file,
                                     const int caller_line,
                                     const std::string &caller_function)
    {
        VAR_CHECK(jni);
        VAR_CONTENT_CHECK(name);

        const auto klass = jni->FindClass(name.c_str());

        EXCEPT_CHECK(jni);

        if (klass == nullptr)
        {
            throw std::runtime_error("Cannot find class '" + name + "'");
        }

        internal::tracker_manager::local_refs.insert(klass);

        const std::string call_site = "Called from " + get_path(caller_file) + ":" +
                             std::to_string(caller_line) + " in " + caller_function;

        internal::tracker_manager::local_ref_sources[klass] = {
            __FILE__,
            __LINE__,
            __func__,
            "Class: " + name + " | " + call_site
        };

        return make_local(jni, klass);
    }

    jmethodID wrapper::get_method(JNIEnv *jni, const jni_local_ref<jclass> &klass, const std::string &method_name,
                                  const std::string &signature, const bool is_static)
    {
        VAR_CHECK(jni);
        VAR_CHECK(klass);

        VAR_CONTENT_CHECK(method_name);
        VAR_CONTENT_CHECK(signature);

        jmethodID method = nullptr;

        if (is_static)
        {
            method = jni->GetStaticMethodID(*klass, method_name.c_str(), signature.c_str());
        }
        else
        {
            method = jni->GetMethodID(*klass, method_name.c_str(), signature.c_str());
        }

        EXCEPT_CHECK(jni);

        if (method == nullptr)
        {
            throw std::runtime_error(
                "Method not found: " + method_name + " with signature: " + signature + " and static val: " +
                std::to_string(is_static));
        }

        return method;
    }

    jmethodID wrapper::get_method(JNIEnv *jni, const std::string &name, const std::string &method,
                                  const std::string &signature, const bool is_static)
    {
        const auto klass = search_for_class(jni, name);
        const auto method_id = get_method(jni, klass, method, signature, is_static);
//        remove_local_ref(jni, klass); no longer needed?

        return method_id;
    }

    jni_local_ref<jobject> wrapper::invoke_object_method(JNIEnv *jni, const jni_local_ref<jclass> &klass, const jni_local_ref<jobject> &instance,
                                         const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jobject result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticObjectMethodA(*klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallObjectMethodA(*instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        const auto ref = add_local_ref(jni, result, __FILE__, __LINE__, __func__);

        return make_local(jni, ref);
    }

    jbyte wrapper::invoke_byte_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jbyte result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticByteMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallByteMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    jint wrapper::invoke_int_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                    const jmethodID &method_id,
                                    const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jint result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticIntMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallIntMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    jlong wrapper::invoke_long_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jlong result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticLongMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallLongMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    jshort wrapper::invoke_short_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jshort result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticShortMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallShortMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    jfloat wrapper::invoke_float_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                        const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jfloat result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticFloatMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallFloatMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    jdouble wrapper::invoke_double_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jdouble result;
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            result = jni->CallStaticDoubleMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallDoubleMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    void wrapper::invoke_void_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                     const jmethodID &method_id,
                                     const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);
        const bool is_static = klass != nullptr;

        if (is_static)
        {
            jni->CallStaticVoidMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance)

            jni->CallVoidMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);
    }

    void wrapper::register_natives(JNIEnv *jni, const std::string &klass_name, const jni_global_ref<jclass> &klass, const std::vector<jni_native_method> &methods)
    {
        VAR_CHECK(jni);
        VAR_CHECK(klass);

        VAR_CONTENT_CHECK(klass_name);

        auto jni_methods = std::vector<JNINativeMethod>(methods.size());

        for (const auto& customer : methods)
        {
            const auto method = customer.jni_method;

            VAR_CHECK(method.fnPtr);

            jni_methods.emplace_back(method);
        }

        jni->RegisterNatives(*klass, jni_methods.data(), static_cast<jint>(methods.size()));

        EXCEPT_CHECK(jni);

        internal::tracker_manager::tracked_native_classes[klass_name] = methods.size();
    }

    void wrapper::unregister_natives(JNIEnv *jni, const std::string &klass_name, const jni_global_ref<jclass> &klass)
    {
        VAR_CHECK(jni);
        VAR_CHECK(klass);

        VAR_CONTENT_CHECK(klass_name);

        jni->UnregisterNatives(*klass);

        EXCEPT_CHECK(jni);

        internal::tracker_manager::tracked_native_classes.erase(klass_name);
    }

    std::string wrapper::get_string(JNIEnv *env, const jstring &string)
    {
        VAR_CHECK(env);
        VAR_CHECK(string);

        const char *key = env->GetStringUTFChars(string, nullptr);
        EXCEPT_CHECK(env);

        std::string str(key);

        env->ReleaseStringUTFChars(string, key);
        EXCEPT_CHECK(env);

        return str;
    }
}
