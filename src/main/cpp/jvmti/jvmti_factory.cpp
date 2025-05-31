//
// Created by Damian Netter on 30/05/2025.
//

#include "ZNBKit/jvmti/jvmti_factory.hpp"

/*
 * For now include all JNI types here. It probably isn't good idea to include all of them until they're all implemented but who cares.
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
#define GET_METHOD_SIGNATURE_OBJECT(TYPE) \
    template std::unique_ptr<method_signature<TYPE>> jvmti_factory::get_method_signature<TYPE>(JNIEnv *, jvmtiEnv *, const klass_signature &, const jobject &);

    template std::vector<std::unique_ptr<method_signature<void>>> jvmti_factory::look_for_method_signatures<void>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jobject>>> jvmti_factory::look_for_method_signatures<jobject>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jstring>>> jvmti_factory::look_for_method_signatures<jstring>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jint>>> jvmti_factory::look_for_method_signatures<jint>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jlong>>> jvmti_factory::look_for_method_signatures<jlong>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jboolean>>> jvmti_factory::look_for_method_signatures<
        jboolean>(JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jfloat>>> jvmti_factory::look_for_method_signatures<jfloat>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jdouble>>> jvmti_factory::look_for_method_signatures<jdouble>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jbyte>>> jvmti_factory::look_for_method_signatures<jbyte>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jchar>>> jvmti_factory::look_for_method_signatures<jchar>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);
    template std::vector<std::unique_ptr<method_signature<jshort>>> jvmti_factory::look_for_method_signatures<jshort>(
        JNIEnv *, jvmtiEnv *, const klass_signature &);

    template std::unique_ptr<method_signature<void>> jvmti_factory::get_method_signature<void>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jobject>> jvmti_factory::get_method_signature<jobject>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jstring>> jvmti_factory::get_method_signature<jstring>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jint>> jvmti_factory::get_method_signature<jint>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jlong>> jvmti_factory::get_method_signature<jlong>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jboolean>> jvmti_factory::get_method_signature<jboolean>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jfloat>> jvmti_factory::get_method_signature<jfloat>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jdouble>> jvmti_factory::get_method_signature<jdouble>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jbyte>> jvmti_factory::get_method_signature<jbyte>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jchar>> jvmti_factory::get_method_signature<jchar>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);
    template std::unique_ptr<method_signature<jshort>> jvmti_factory::get_method_signature<jshort>(
        JNIEnv *, jvmtiEnv *, const klass_signature &, std::string, std::vector<std::string>);


    template std::vector<JNINativeMethod> jvmti_factory::map_methods<void>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<void>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jobject>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jobject>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jstring>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jstring>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jint>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jint>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jlong>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jlong>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jboolean>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jboolean>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jfloat>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jfloat>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jdouble>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jdouble>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jbyte>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jbyte>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jchar>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jchar>>> &);
    template std::vector<JNINativeMethod> jvmti_factory::map_methods<jshort>(
        const std::unordered_multimap<std::string, reference> &,
        const std::vector<std::unique_ptr<method_signature<jshort>>> &);
}
