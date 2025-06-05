//
// Created by Damian Netter on 30/05/2025.
//

#include "ZNBKit/jvmti/jvmti_factory.hpp"

#include "ZNBKit/jni/signatures/method/byte_method.hpp"
#include "ZNBKit/jni/signatures/method/int_method.hpp"
#include "ZNBKit/jni/signatures/method/long_method.hpp"
#include "ZNBKit/jni/signatures/method/object_method.hpp"
#include "ZNBKit/jni/signatures/method/short_method.hpp"
#include "ZNBKit/jni/signatures/method/void_method.hpp"

#include "ZNBKit/debug.hpp"
#include "ZNBKit/jni/utils/util.hpp"

/*
 * For now include all JNI types here. It probably isn't good idea to include all of them until they're all implemented but who cares.
 * I hope this works btw, no idea if it will compile at all.
 */

#define JNI_TYPES(TYPE) \
    TYPE(void) \
    TYPE(jobject) \
    TYPE(jstring) \
    TYPE(jint) \
    TYPE(jlong) \
    TYPE(jboolean) \
    TYPE(jfloat) \
    TYPE(jdouble) \
    TYPE(jbyte) \
    TYPE(jchar) \
    TYPE(jshort)

namespace
{
    template<typename JVMTI_ALLOC_TYPE>
    struct jvmti_deleter
    {
        jvmtiEnv *jvmti_env;

        void operator()(JVMTI_ALLOC_TYPE *ptr) const
        {
            if (jvmti_env && ptr)
            {
                jvmti_env->Deallocate(reinterpret_cast<unsigned char *>(ptr));
            }
        }
    };

    template<typename JVMTI_ALLOC_TYPE>
    using jvmti_ptr = std::unique_ptr<JVMTI_ALLOC_TYPE, jvmti_deleter<JVMTI_ALLOC_TYPE>>;
}
namespace znb_kit
{
#define INSTANTIATE_GET_METHOD_SIGNATURE_OBJECT(TYPE) \
    template std::unique_ptr<method_signature<TYPE>> jvmti_factory::get_method_signature<TYPE>(JNIEnv *, jvmtiEnv *, const klass_signature &, const jobject &);

#define INSTANTIATE_GET_METHOD_SIGNATURE_PARAMETERS(TYPE) \
    template std::unique_ptr<method_signature<TYPE>> jvmti_factory::get_method_signature<TYPE>(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &owner_ks, const std::string& method_name, const std::vector<std::string>& target_params);

#define INSTANTIATE_LOOK_FOR_METHOD_SIGNATURES(TYPE) \
    template std::vector<std::unique_ptr<method_signature<TYPE>>> jvmti_factory::look_for_method_signatures<TYPE>(JNIEnv *, jvmtiEnv *, const klass_signature &);

#define INSTANTIATE_MAP_METHODS(TYPE) \
    template std::vector<jni_native_method> jvmti_factory::map_methods<TYPE>(const std::unordered_multimap<std::string, jni_bridge_reference> &, const std::vector<std::unique_ptr<method_signature<TYPE>>> &);

#define INSTANTIATE_METHOD_SPEC(JNI_TYPE, SUFFIX) \
    template<> \
    std::unique_ptr<method_signature<JNI_TYPE>> jvmti_factory::create_method_instance<JNI_TYPE>( \
        JNIEnv *jni, \
        const klass_signature &owner_ks, \
        const std::string &name, \
        const std::string &signature, \
        const std::optional<std::vector<std::string>> &params, \
        bool is_static) \
    { \
        auto new_ks = std::make_shared<klass_signature>(jni, owner_ks.get_owner()); \
        return std::make_unique<SUFFIX##_method>(jni, new_ks, name, signature, params, is_static); \
    }

#define MAPPINGS(APPLY) \
    APPLY(void, void) \
    APPLY(jobject, object) \
    APPLY(jbyte, byte) \
    APPLY(jshort, short) \
    APPLY(jint, int) \
    APPLY(jlong, long)

    MAPPINGS(INSTANTIATE_METHOD_SPEC)

    /*
     * Do I really need this piece of code? Who cares, ill just leave it here for now. 31/05/2025
     */
    template<typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::create_method_instance(JNIEnv *, const klass_signature &, const std::string &name, const std::string &, const std::optional<std::vector<std::string>> &, bool)
    {
        debug_print("factory::create_method_instance() unhandled type for T in template for method: " + name);
        return nullptr;
    }

    template <typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &owner_ks, const jobject &method)
    {
        const auto method_id = jni->FromReflectedMethod(method);
        if (!method_id) {
            debug_print("factory::get_method_signature() FromReflectedMethod failed.");
            return nullptr;
        }

        char *raw_name_ptr = nullptr;
        char *raw_signature_ptr = nullptr;
        
        jvmtiError error = jvmti->GetMethodName(method_id, &raw_name_ptr, &raw_signature_ptr, nullptr);

        const jvmti_ptr<char> name_ptr(raw_name_ptr, {jvmti});
        const jvmti_ptr<char> signature_ptr(raw_signature_ptr, {jvmti});

        if (error != JVMTI_ERROR_NONE)
        {
            char *raw_error_buffer = nullptr;
            jvmti->GetErrorName(error, &raw_error_buffer);
            jvmti_ptr<char> error_buffer(raw_error_buffer, {jvmti});
            
            debug_print("factory::get_method_signature() JVMTI GetMethodName error: " + (error_buffer ? std::string(error_buffer.get()) : "Unknown error"));

            return nullptr;
        }

        if (name_ptr == nullptr || signature_ptr == nullptr)
        {
            return nullptr;
        }

        const std::string name = name_ptr.get();
        const std::string signature = signature_ptr.get();

        jint modifiers = 0;

        error = jvmti->GetMethodModifiers(method_id, &modifiers);
        if (error != JVMTI_ERROR_NONE) {
            debug_print("factory::get_method_signature() JVMTI GetMethodModifiers error");
            return nullptr;
        }

        const bool is_static = (modifiers & ACC_STATIC) != 0;
        auto params = get_parameters(jni, method);
        auto ptr = std::make_shared<klass_signature>(owner_ks);

        return create_method_instance<T>(jni, owner_ks, name, signature, params, is_static);
    }

    template <typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(JNIEnv *jni,
        jvmtiEnv *jvmti,
        const klass_signature &owner_ks,
        const std::string& method_name,
        const std::vector<std::string>& target_params)
    {
        std::vector<jobject> methods = get_methods(jni, owner_ks.get_owner());
        std::unique_ptr<method_signature<T>> match = nullptr;

        for (jobject &method_obj : methods)
        {
            if (auto probable_match = jvmti_factory::get_method_signature<T>(jni, jvmti, owner_ks, method_obj))
            {
                if (method_name != probable_match->name)
                {
                    continue;
                }
                if (!probable_match->parameters.has_value()) {
                    continue;
                }

                if (compare_parameters(target_params, probable_match->parameters.value())) {
                    match = std::move(probable_match);
                    break;
                }
            }
        }

        for (const auto object : methods)
        {
            if (object)
            {
                jni->DeleteLocalRef(object);
            }
        }

        methods.clear();

        return match;
    }

    template <typename T>
    std::vector<std::unique_ptr<method_signature<T>>> jvmti_factory::look_for_method_signatures(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &owner_ks)
    {
        const auto method_objects = get_methods(jni, owner_ks.get_owner());
        std::vector<std::unique_ptr<method_signature<T>>> descriptors;
        descriptors.reserve(method_objects.size());

        for (auto &method_obj : method_objects)
        {
            if (auto method_desc = jvmti_factory::get_method_signature<T>(jni, jvmti, owner_ks, method_obj)) {
                descriptors.push_back(std::move(method_desc));
            }

            jni->DeleteLocalRef(method_obj);
        }
        return descriptors;
    }

    template <typename T>
    std::vector<jni_native_method> jvmti_factory::map_methods(const std::unordered_multimap<std::string, jni_bridge_reference> &map,
                                                                         const std::vector<std::unique_ptr<method_signature<T>>> &methods)
    {
        std::vector<jni_native_method> result_methods;
        result_methods.reserve(methods.size());

        for (const auto& method_ptr : methods) {
            if (!method_ptr)
            {
                continue;
            }

            const auto& method = *method_ptr;

            auto range = map.equal_range(method.name);
            for (auto it = range.first; it != range.second; ++it) {
                if (!method.parameters.has_value())
                {
                    continue;
                }

                if (!znb_kit::compare_parameters(it->second.parameters, method.parameters.value()))
                {
                    continue;
                }

                result_methods.emplace_back(
                    method.name,
                    method.signature,
                    it->second.has_func() ? it->second.func_ptr : nullptr
                );
                break;
            }
        }
        return result_methods;
    }

    JNI_TYPES(INSTANTIATE_GET_METHOD_SIGNATURE_OBJECT)
    JNI_TYPES(INSTANTIATE_GET_METHOD_SIGNATURE_PARAMETERS)
    JNI_TYPES(INSTANTIATE_LOOK_FOR_METHOD_SIGNATURES)
    JNI_TYPES(INSTANTIATE_MAP_METHODS)
}
