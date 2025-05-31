//
// Created by Damian Netter on 30/05/2025.
//

#include "ZNBKit/jvmti/jvmti_factory.hpp"

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
    template std::unique_ptr<method_signature<TYPE>> jvmti_factory::get_method_signature<TYPE>(JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);

#define INSTANTIATE_LOOK_FOR_METHOD_SIGNATURES(TYPE) \
    template std::vector<std::unique_ptr<method_signature<TYPE>>> jvmti_factory::look_for_method_signatures<TYPE>(JNIEnv *, jvmtiEnv *, const klass_signature &);

#define INSTANTIATE_MAP_METHODS(TYPE) \
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<TYPE>(const std::unordered_multimap<std::string, reference> &, const std::vector<std::unique_ptr<method_signature<TYPE>>> &);

    template <typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature, const jobject &method) // Parameter changed to const&
    {
        const auto method_id = jni->FromReflectedMethod(method);
        if (!method_id) {
            debug_print("factory::get_method_signature() FromReflectedMethod failed.");
            return nullptr;
        }

        char *raw_name_ptr = nullptr;
        char *raw_signature_ptr = nullptr;
        
        jvmtiError error = jvmti->GetMethodName(method_id, &raw_name_ptr, &raw_signature_ptr, nullptr);

        jvmti_ptr<char> name_ptr(raw_name_ptr, {jvmti});
        jvmti_ptr<char> signature_ptr(raw_signature_ptr, {jvmti});

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

        std::string name = name_ptr.get();
        std::string signature = signature_ptr.get();

        jint modifiers = 0;

        error = jvmti->GetMethodModifiers(method_id, &modifiers);
        if (error != JVMTI_ERROR_NONE) {
            debug_print("factory::get_method_signature() JVMTI GetMethodModifiers error");
            return nullptr;
        }

        bool is_static = (modifiers & ACC_STATIC) != 0;
        auto params = get_parameters(jni, method);
        auto ptr = std::make_shared<znb_kit::klass_signature>(klass_signature);
        
        if constexpr (std::is_same_v<T, jobject>) {
            return std::make_unique<object_method>(jni, ptr, name, signature, params, is_static);
        } else if constexpr (std::is_same_v<T, jshort>) {
            return std::make_unique<short_method>(jni, ptr, name, signature, params, is_static);
        } else if constexpr (std::is_same_v<T, jbyte>) {
            return std::make_unique<byte_method>(jni, ptr, name, signature, params, is_static);
        } else if constexpr (std::is_same_v<T, jint>) {
            return std::make_unique<int_method>(jni, ptr, name, signature, params, is_static);
        } else if constexpr (std::is_same_v<T, jlong>) {
            return std::make_unique<long_method>(jni, ptr, name, signature, params, is_static);
        } else if constexpr (std::is_same_v<T, void>) {
            return std::make_unique<void_method>(jni, ptr, name, signature, params, is_static);
        }

        debug_print("factory::get_method_signature() unhandled type for T in template for method: " + name);
        return nullptr;
    }

    template <typename T>
    std::unique_ptr<method_signature<T>> jvmti_factory::get_method_signature(JNIEnv *jni,
        jvmtiEnv *jvmti,
        const klass_signature &klass_signature,
        std::string method_name,
        const std::vector<std::string> target_params)
    {
        for (std::vector<jobject> methods_arr = get_methods(jni, klass_signature.get_owner()); jobject &method_obj : methods_arr)
        {
            auto method_desc = jvmti_factory::get_method_signature<T>(jni, jvmti, klass_signature, method_obj);
            if (method_desc && method_name == method_desc->name)
            {
                if (method_desc->parameters.has_value() &&
                    compare_parameters(target_params, method_desc->parameters.value())) {
                    for(const auto cleanup_obj : methods_arr) {
                        if (cleanup_obj != method_obj) jni->DeleteLocalRef(cleanup_obj);
                    }
                    return method_desc;
                }
            }
            jni->DeleteLocalRef(method_obj);
        }
        return nullptr;
    }

    template <typename T>
    std::vector<std::unique_ptr<method_signature<T>>> jvmti_factory::look_for_method_signatures(JNIEnv *jni, jvmtiEnv *jvmti, const klass_signature &klass_signature_param)
    {
        const auto method_objects = get_methods(jni, klass_signature_param.get_owner());
        std::vector<std::unique_ptr<method_signature<T>>> descriptors;
        descriptors.reserve(method_objects.size());

        for (auto &method_obj : method_objects)
        {
            if (auto method_desc = jvmti_factory::get_method_signature<T>(jni, jvmti, klass_signature_param, method_obj)) {
                descriptors.push_back(std::move(method_desc));
            }
            jni->DeleteLocalRef(method_obj);
        }
        return descriptors;
    }

    template <typename T>
    std::vector<JNINativeMethod> jvmti_factory::map_methods(const std::unordered_multimap<std::string, reference> &map,
        const std::vector<std::unique_ptr<method_signature<T>>> &methods)
    {
        std::vector<JNINativeMethod> result_methods;
        result_methods.reserve(methods.size());

        for (const auto& method_ptr : methods) {
            if (!method_ptr) continue;
            const auto& method = *method_ptr;

            auto range = map.equal_range(method.name);
            for (auto it = range.first; it != range.second; ++it) {
                if (method.parameters.has_value() && znb_kit::compare_parameters(it->second.parameters, method.parameters.value())) {
                    char* name_dup = strdup(method.name.c_str());
                    char* sig_dup = strdup(method.signature.c_str());
                    if (!name_dup || !sig_dup) {
                        free(name_dup);
                        free(sig_dup);
                        continue;
                    }
                    result_methods.push_back(JNINativeMethod{
                        name_dup,
                        sig_dup,
                        it->second.has_func() ? it->second.func_ptr : nullptr
                    });
                    break;
                }
            }
        }
        return result_methods;
    }

    JNI_TYPES(INSTANTIATE_GET_METHOD_SIGNATURE_OBJECT)
    JNI_TYPES(INSTANTIATE_GET_METHOD_SIGNATURE_PARAMETERS)
    JNI_TYPES(INSTANTIATE_LOOK_FOR_METHOD_SIGNATURES)
    JNI_TYPES(INSTANTIATE_MAP_METHODS)
}
