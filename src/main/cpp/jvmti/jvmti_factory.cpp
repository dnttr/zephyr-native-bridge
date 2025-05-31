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

    JNI_TYPES(INSTANTIATE_GET_METHOD_SIGNATURE_OBJECT)
    JNI_TYPES(INSTANTIATE_GET_METHOD_SIGNATURE_PARAMETERS)
    JNI_TYPES(INSTANTIATE_LOOK_FOR_METHOD_SIGNATURES)
    JNI_TYPES(INSTANTIATE_MAP_METHODS)
}
