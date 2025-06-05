//
// Created by Damian Netter on 01/06/2025.
//

#include "ZNBKit/jni/internal/wrapper.hpp"

#include <vector>

#include "ZNBKit/debug.hpp"

#define IS_STATIC(x) x != nullptr

namespace znb_kit
{
    std::vector<jvalue> transform_parameters(const std::vector<local_value_reference> &parameters)
    {
        std::vector<jvalue> result;

        for (const auto &parameter : parameters)
        {
            result.push_back(*parameter);
        }

        return result;
    }

    jobject wrapper::add_local_ref(JNIEnv *jni, const jobject &obj,
                                   const std::string &file, const int line,
                                   const std::string &method, const bool needed_new_ref)
    {
        VAR_CHECK(jni);

        jobject ref = obj;
        if (needed_new_ref)
        {
            ref = jni->NewLocalRef(obj);
            EXCEPT_CHECK(jni);
        }

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
            EXCEPT_CHECK(jni);
        }
    }

    jobject wrapper::add_global_ref(JNIEnv *jni, const jobject &obj,
                                    const std::string &file, const int line,
                                    const std::string &method, const bool needed_new_ref)
    {
        VAR_CHECK(jni);

        jobject ref = obj;
        if (needed_new_ref)
        {
            ref = jni->NewGlobalRef(obj);
            EXCEPT_CHECK(jni);
        }

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
            EXCEPT_CHECK(jni);
        }
    }

    template <typename T>
    auto wrapper::change_reference_policy(JNIEnv *jni, const jni_reference_policy new_policy, const T &reference)
    {
        VAR_CHECK(jni);
        VAR_CHECK(reference);

        auto raw_ref = *reference;

        using RawType = std::decay_t<decltype(raw_ref)>;

        const bool is_local = internal::tracker_manager::local_refs.contains(raw_ref);
        const bool is_global = internal::global_tracker::global_refs.contains(raw_ref);

        if ((!is_local && !is_global) || (is_local && is_global))
        {
            throw std::runtime_error("Policy is neither local nor global or it is both. Cannot change it.");
        }

        if (new_policy == jni_reference_policy::LOCAL && is_local)
        {
            return make_local(jni, static_cast<RawType>(raw_ref));
        }

        if (new_policy == jni_reference_policy::GLOBAL && is_global)
        {
            return make_global(jni, static_cast<RawType>(raw_ref));
        }

        if (new_policy == jni_reference_policy::LOCAL)
        {
            auto local_ref = add_local_ref(jni, raw_ref, __FILE__, __LINE__, __func__, true);
            remove_global_ref(jni, raw_ref);

            return make_local(jni, static_cast<RawType>(local_ref));
        }

        auto global_ref = add_global_ref(jni, raw_ref, __FILE__, __LINE__, __func__, true);
        remove_local_ref(jni, raw_ref);

        return make_global(jni, static_cast<RawType>(global_ref));
    }

    void wrapper::check_for_corruption()
    {
        internal::tracker_manager::check_for_corruption();
    }

    local_reference<jclass> wrapper::search_for_class(JNIEnv *jni, const std::string &name,
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

    template <typename RefType>
    jmethodID wrapper::get_method(JNIEnv *jni, const RefType &klass, const std::string &method_name,
                                  const std::string &signature, const bool is_static)
    {
        VAR_CHECK(jni);

        const auto klass_object = *klass;
        VAR_CHECK(klass_object);

        VAR_CONTENT_CHECK(method_name);
        VAR_CONTENT_CHECK(signature);

        jmethodID method = nullptr;

        if (is_static)
        {
            method = jni->GetStaticMethodID(klass_object, method_name.c_str(), signature.c_str());
        }
        else
        {
            method = jni->GetMethodID(klass_object, method_name.c_str(), signature.c_str());
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

        return method_id;
    }

    template <typename KlassRefType, typename ObjRefType>
    local_reference<jobject> wrapper::invoke_object_method(JNIEnv *jni, const KlassRefType &klass,
                                                           const ObjRefType &instance,
                                                           const jmethodID &method_id,
                                                           const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jobject result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticObjectMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallObjectMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        const auto ref = add_local_ref(jni, result, __FILE__, __LINE__, __func__, false);

        return make_local(jni, ref);
    }

    template <typename KlassRefType, typename ObjRefType>
    std::pair<local_reference<jobjectArray>, size_t> wrapper::invoke_object_array_method(
        JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
        const jmethodID &method_id, const std::vector<local_value_reference> &parameters)
    {
        auto obj_ref = invoke_object_method(jni, klass, instance, method_id, parameters);
        VAR_CHECK(*obj_ref);
        const auto raw_obj = obj_ref.release();

        const auto array_ref = reinterpret_cast<jobjectArray>(raw_obj);
        const auto array_size = jni->GetArrayLength(array_ref);

        return std::make_pair(make_local(jni, array_ref), array_size);
    }

    template <typename KlassRefType, typename ObjRefType>
    string_reference wrapper::invoke_string_method(JNIEnv *jni, const KlassRefType &klass,
                                                           const ObjRefType &instance, const jmethodID &method_id,
                                                           const std::vector<local_value_reference> &parameters)
    {
        auto obj = invoke_object_method(jni, klass, instance, method_id, parameters);
        VAR_CHECK(*obj);

        const auto raw_obj = obj.release();
        const auto result = reinterpret_cast<jstring>(raw_obj);

        return make_local(jni, result);
    }

    template <typename KlassRefType, typename ObjRefType>
    jbyte wrapper::invoke_byte_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                      const jmethodID &method_id, const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jbyte result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticByteMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallByteMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    template <typename KlassRefType, typename ObjRefType>
    jint wrapper::invoke_int_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                    const jmethodID &method_id,
                                    const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jint result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticIntMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallIntMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    template <typename KlassRefType, typename ObjRefType>
    jlong wrapper::invoke_long_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                      const jmethodID &method_id, const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jlong result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticLongMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallLongMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    template <typename KlassRefType, typename ObjRefType>
    jshort wrapper::invoke_short_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                        const jmethodID &method_id,
                                        const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jshort result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticShortMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallShortMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    template <typename KlassRefType, typename ObjRefType>
    jfloat wrapper::invoke_float_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                        const jmethodID &method_id,
                                        const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jfloat result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticFloatMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallFloatMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    template <typename KlassRefType, typename ObjRefType>
    jdouble wrapper::invoke_double_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                          const jmethodID &method_id,
                                          const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);
        jdouble result;

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            result = jni->CallStaticDoubleMethodA(*klass, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object);

            result = jni->CallDoubleMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);

        return result;
    }

    template <typename KlassRefType, typename ObjRefType>
    void wrapper::invoke_void_method(JNIEnv *jni, const KlassRefType &klass, const ObjRefType &instance,
                                     const jmethodID &method_id,
                                     const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);

        const auto raw_parameters = transform_parameters(parameters).data();
        const auto klass_object = *klass;

        if (IS_STATIC(klass_object))
        {
            jni->CallStaticVoidMethodA(klass_object, method_id, raw_parameters);
        }
        else
        {
            const auto instance_object = *instance;
            VAR_CHECK(instance_object)

            jni->CallVoidMethodA(instance_object, method_id, raw_parameters);
        }

        EXCEPT_CHECK(jni);
    }

    template <typename KlassRefType>
    local_reference<jobject> wrapper::new_object(JNIEnv *jni, const KlassRefType &klass,
                                                 const jmethodID &method_id,
                                                 const std::vector<local_value_reference> &parameters)
    {
        VAR_CHECK(jni);

        const auto klass_object = *klass;
        VAR_CHECK(klass_object);

        VAR_CHECK(method_id);

        const auto obj = jni->NewObjectA(klass_object, method_id, transform_parameters(parameters).data());
        EXCEPT_CHECK(jni);

        return make_local(jni, obj);
    }

    template <typename KlassRefType>
    void wrapper::register_natives(JNIEnv *jni, const std::string &klass_name, const KlassRefType &klass,
                                   const std::vector<jni_native_method> &methods)
    {
        VAR_CHECK(jni);

        const auto klass_object = *klass;
        VAR_CHECK(klass_object);

        VAR_CONTENT_CHECK(klass_name);

        auto jni_methods = std::vector<JNINativeMethod>(methods.size());

        for (const auto &customer : methods)
        {
            const auto method = customer.jni_method;

            VAR_CHECK(method.fnPtr);

            jni_methods.emplace_back(method);
        }

        jni->RegisterNatives(klass_object, jni_methods.data(), static_cast<jint>(methods.size()));

        EXCEPT_CHECK(jni);

        internal::tracker_manager::tracked_native_classes[klass_name] = methods.size();
    }

    template <typename ArrayRefType>
    local_reference<jobject> wrapper::get_object_array_element(JNIEnv *jni,
                                                               const std::pair<ArrayRefType, size_t> &array,
                                                               const int pos)
    {
        VAR_CHECK(jni);

        const auto array_ref = *array.first;
        VAR_CHECK(array_ref);

        if (pos < 0 || pos >= array.second)
        {
            throw std::out_of_range("Index out of bounds for object array");
        }

        const auto element = jni->GetObjectArrayElement(array_ref, pos);
        EXCEPT_CHECK(jni);

        return make_local(jni, element);
    }

    template <typename KlassRefType>
    void wrapper::unregister_natives(JNIEnv *jni, const std::string &klass_name, const KlassRefType &klass)
    {
        VAR_CHECK(jni);

        const auto klass_object = *klass;
        VAR_CHECK(klass_object);

        VAR_CONTENT_CHECK(klass_name);

        jni->UnregisterNatives(klass_object);

        EXCEPT_CHECK(jni);

        internal::tracker_manager::tracked_native_classes.erase(klass_name);
    }

    std::unique_ptr<const char, internal::policy::deleter_string> get_string_utf_chars(
        JNIEnv *jni, const jstring &string)
    {
        const char *chars = jni->GetStringUTFChars(string, nullptr);
        EXCEPT_CHECK(jni)
        return std::unique_ptr<const char, internal::policy::deleter_string>(
            chars, internal::policy::deleter_string{jni, string});
    }

    std::string wrapper::get_string(JNIEnv *jni, const string_reference &string)
    {
        VAR_CHECK(jni);

        const auto string_object = *string;
        VAR_CHECK(string_object);

        std::string str(get_string_utf_chars(jni, string_object).get());

        return str;
    }

    //TODO: Later do it in more generic way

    template auto wrapper::change_reference_policy<local_reference<jobject>>(
        JNIEnv *, jni_reference_policy, const local_reference<jobject> &);
    template auto wrapper::change_reference_policy<local_reference<jclass>>(
        JNIEnv *, jni_reference_policy, const local_reference<jclass> &);
    template auto wrapper::change_reference_policy<local_reference<jstring>>(
        JNIEnv *, jni_reference_policy, const local_reference<jstring> &);
    template auto wrapper::change_reference_policy<local_reference<jobjectArray>>(
        JNIEnv *, jni_reference_policy, const local_reference<jobjectArray> &);

    template auto wrapper::change_reference_policy<global_reference<jobject>>(
        JNIEnv *, jni_reference_policy, const global_reference<jobject> &);
    template auto wrapper::change_reference_policy<global_reference<jclass>>(
        JNIEnv *, jni_reference_policy, const global_reference<jclass> &);
    template auto wrapper::change_reference_policy<global_reference<jstring>>(
        JNIEnv *, jni_reference_policy, const global_reference<jstring> &);
    template auto wrapper::change_reference_policy<global_reference<jobjectArray>>(
        JNIEnv *, jni_reference_policy, const global_reference<jobjectArray> &);
}
