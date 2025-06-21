//
// Created by Damian Netter on 01/06/2025.
//

#include "ZNBKit/internal/wrapper.hpp"

#include <vector>
#include <cassert>

#include "ZNBKit/debug.hpp"

namespace znb_kit
{
    std::mutex global_tracker::mutex;
    std::mutex wrapper::tracked_native_classes_mutex;

    std::unordered_set<jobject> global_tracker::global_refs;
    std::unordered_map<jobject, ref_info> global_tracker::global_ref_sources;

    std::unordered_map<std::string, size_t> wrapper::tracked_native_classes;

    void global_tracker::add(const jobject &ref, const std::string &file, const int line, const std::string &method)
    {
        std::lock_guard lock(mutex);

        if (ref != nullptr)
        {
            global_refs.insert(ref);
            global_ref_sources[ref] = {file, line, method};
        }
    }

    void global_tracker::remove(const jobject &ref)
    {
        std::lock_guard lock(mutex);
        global_refs.erase(ref);
        global_ref_sources.erase(ref);
    }

    size_t global_tracker::count()
    {
        std::lock_guard lock(mutex);
        return global_refs.size();
    }

    void global_tracker::dump_refs()
    {
        using ref_data = std::pair<jobject, ref_info>;
        std::vector<ref_data> refs_to_dump;
        size_t total_refs;

        {
            std::lock_guard lock(mutex);

            total_refs = global_refs.size();

            if (total_refs == 0)
            {
                return;
            }

            refs_to_dump.reserve(total_refs);

            for (const auto &ref : global_refs)
            {
                if (auto it = global_ref_sources.find(ref); it != global_ref_sources.end())
                {
                    refs_to_dump.emplace_back(ref, it->second);
                }
                else
                {
                    refs_to_dump.emplace_back(ref, ref_info{});
                }
            }
        }

        debug_print_cerr(std::format("[WRAPPER] Dumping {} global references:", total_refs));

        for (const auto &[ref, info] : refs_to_dump)
        {
            if (!info.file.empty())
            {
                const auto &[file, line, method, details] = info;

                debug_print_cerr(std::format("[WRAPPER] Global ref {} created at {}:{} in {}{}", reinterpret_cast<uintptr_t>(ref), file, line, method, details.empty() ? "" : std::format(" ({}) ", details)));
            }
            else
            {
                debug_print_cerr(std::format("[WRAPPER] Global ref {} (source unknown)", reinterpret_cast<uintptr_t>(ref)));
            }
        }
    }

    void wrapper::check_for_corruption()
    {
        const bool is_global_empty = global_tracker::count() == 0;
        const bool is_local_empty = local_refs.empty();

        bool are_natives_empty;

        std::vector<std::pair<std::string, size_t>> tracked_native_classes_copy;
        {
            std::lock_guard lock(tracked_native_classes_mutex);
            are_natives_empty = tracked_native_classes.empty();

            if (!are_natives_empty)
            {
                tracked_native_classes_copy.assign(tracked_native_classes.begin(), tracked_native_classes.end());
            }
        }

        if (!is_global_empty || !is_local_empty)
        {
            debug_print_cerr("[WRAPPER] Warning: References are not empty.");

            debug_print_cerr(std::format("[WRAPPER] Global references count: {}", std::to_string(global_tracker::count())));
            debug_print_cerr(std::format("[WRAPPER] Local references count: {}", local_refs.size()));

            if (!is_global_empty)
            {
                debug_print_cerr("[WRAPPER] Global references are not empty, potential memory leak detected.");
                global_tracker::dump_refs();
            }

            if (!is_local_empty)
            {
                debug_print_cerr("[WRAPPER] Local references are not empty, potential memory leak detected.");
                dump_local_refs();
            }

            return;
        }

        if (!are_natives_empty)
        {
            debug_print_cerr("[WRAPPER] Warning: Tracked native classes are not empty.");

            for (const auto &[name, natives] : tracked_native_classes_copy)
            {

                debug_print_cerr(std::format("[WRAPPER] Class {} has {} references which were not deleted.", name, natives));
            }

            return;
        }

        debug_print_cerr("[WRAPPER] No references left. All good i think, unless not using wrapper, then well, you are on your own. :>");
    }

    void wrapper::dump_local_refs()
    {
        auto str = std::format("[WRAPPER] Dumping {} local references:", local_refs.size());
        debug_print_cerr(str);

        for (const auto &ref : local_refs)
        {
            if (auto it = local_ref_sources.find(ref); it != local_ref_sources.end())
            {
                const auto &[file, line, method, details] = it->second;

                str = std::format("[WRAPPER] Local ref {} created at {}:{} in {}{}",
                    reinterpret_cast<uintptr_t>(ref), file, line, method, details.empty() ? "" : std::format(" ({}) ", details));
            }
            else
            {
                str = std::format("[WRAPPER] Local ref {} (source unknown)", reinterpret_cast<uintptr_t>(ref));
            }

            debug_print_cerr(str);
        }
    }

    jobject wrapper::add_local_ref(JNIEnv *jni, const jobject &obj,
                                   const std::string &file, int line,
                                   const std::string &method)
    {
        VAR_CHECK(jni);

        const auto ref = jni->NewLocalRef(obj);

        if (ref)
        {
            local_refs.insert(ref);
            local_ref_sources[ref] = {file, line, method};
        }

        return ref;
    }

    void wrapper::remove_local_ref(JNIEnv *jni, const jobject &obj)
    {
        VAR_CHECK(jni);

        if (obj)
        {
            local_refs.erase(obj);
            local_ref_sources.erase(obj);
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
            global_tracker::add(ref, file, line, method);
        }

        return ref;
    }

    void wrapper::cleanup_all_refs(JNIEnv *jni)
    {
        VAR_CHECK(jni);

        std::vector<jobject> refs_to_delete;
        {
            std::lock_guard lock(global_tracker::mutex);

            refs_to_delete.assign(global_tracker::global_refs.begin(), global_tracker::global_refs.end());

            global_tracker::global_refs.clear();
            global_tracker::global_ref_sources.clear();
        }

        const size_t cleanup_count = refs_to_delete.size();

        if (cleanup_count == 0)
        {
            debug_print("[WRAPPER] No global references to clean up during JVM shutdown.");
            return;
        }

        for (const jobject &ref : refs_to_delete)
        {
            try
            {
                jni->DeleteGlobalRef(ref);
            }
            catch (...)
            {
                debug_print_cerr("Exception while deleting global ref during cleanup");
            }
        }

        debug_print(std::format("[WRAPPER] Cleaned up {} global references during JVM shutdown", std::to_string(cleanup_count)));
    }

    void wrapper::remove_global_ref(JNIEnv *jni, const jobject &obj)
    {
        VAR_CHECK(jni);

        if (obj)
        {
            global_tracker::remove(obj);
            jni->DeleteGlobalRef(obj);
        }
    }

    jclass wrapper::search_for_class(JNIEnv *jni, const std::string &name,
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

        local_refs.insert(klass);

        const auto call_site = std::format("Called from {}:{} in {}", get_path(caller_file), caller_line, caller_function);

        local_ref_sources[klass] = {
            __FILE__,
            __LINE__,
            __func__,
            "Class: " + name + " | " + call_site
        };

        return klass;
    }

    jmethodID wrapper::get_method(JNIEnv *jni, const jclass &klass, const std::string &method_name,
                                  const std::string &signature, const bool is_static)
    {
        VAR_CHECK(jni);
        VAR_CHECK(klass);

        VAR_CONTENT_CHECK(method_name);
        VAR_CONTENT_CHECK(signature);

        jmethodID method = nullptr;

        if (is_static)
        {
            method = jni->GetStaticMethodID(klass, method_name.c_str(), signature.c_str());
        }
        else
        {
            method = jni->GetMethodID(klass, method_name.c_str(), signature.c_str());
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
        remove_local_ref(jni, klass);

        return method_id;
    }

    jobject wrapper::invoke_object_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                          const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jobject result;

        if (klass != nullptr)
        {
            result = jni->CallStaticObjectMethodA(klass, method_id, parameters.data());
        }
        else
        {
            VAR_CHECK(instance);

            result = jni->CallObjectMethodA(instance, method_id, parameters.data());
        }

        EXCEPT_CHECK(jni);

        return add_local_ref(jni, result, __FILE__, __LINE__, __func__);
    }

    jbyte wrapper::invoke_byte_method(JNIEnv *jni, const jclass &klass, const jobject &instance,
                                      const jmethodID &method_id, const std::vector<jvalue> &parameters)
    {
        VAR_CHECK(jni);

        jbyte result;

        if (klass != nullptr)
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

        if (klass != nullptr)
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

        if (klass != nullptr)
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

        if (klass != nullptr)
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

        if (klass != nullptr)
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

        if (klass != nullptr)
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

        if (klass != nullptr)
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

    void wrapper::unregister_natives(JNIEnv *jni, const std::string &klass_name)
    {
        VAR_CHECK(jni);

        VAR_CONTENT_CHECK(klass_name);

        const auto klass = search_for_class(jni, klass_name, __FILE__, __LINE__, __func__);
        jni->UnregisterNatives(klass);

        EXCEPT_CHECK(jni);

        remove_local_ref(jni, klass);

        std::lock_guard lock(tracked_native_classes_mutex);
        tracked_native_classes.erase(klass_name);
    }

    void wrapper::register_natives(JNIEnv *jni, const std::string &klass_name, const jclass &klass,
                                   const std::vector<jni_native_method> &methods_vec)
    {
        VAR_CHECK(jni);
        VAR_CHECK(klass);
        VAR_CONTENT_CHECK(klass_name);

        if (methods_vec.empty())
        {
            debug_print_cerr("[WRAPPER] No methods to register for class " + klass_name);
            return;
        }

        std::vector<JNINativeMethod> jni_methods_for_jni_call;
        jni_methods_for_jni_call.reserve(methods_vec.size());

        for (const auto &method_descriptor : methods_vec)
        {
            if (method_descriptor.name_buffer.empty() || method_descriptor.name_buffer.front() == '\0' ||
                method_descriptor.signature_buffer.empty() || method_descriptor.signature_buffer.front() == '\0' ||
                !method_descriptor.fn_ptr)
            {
                debug_print_cerr(
                    "[WRAPPER] Skipping registration of invalid method for class '" + klass_name +
                    "': Name empty or invalid, Sig empty or invalid, or Func ptr null.");

                continue;
            }

            jni_methods_for_jni_call.push_back({
                const_cast<char *>(method_descriptor.name_buffer.data()),
                const_cast<char *>(method_descriptor.signature_buffer.data()),
                method_descriptor.fn_ptr
            });
        }

        std::string message;

        if (jni_methods_for_jni_call.empty())
        {
            message = std::format("[WRAPPER] No valid methods to register for class '{}' after filtering. Original count: {}",
                                       klass_name, std::to_string(methods_vec.size()));
            debug_print_cerr(message);

            return;
        }


        message = std::format("[WRAPPER] Registering {} native methods for class '{}'",
            std::to_string(jni_methods_for_jni_call.size()), klass_name);
        debug_print(message);

        const jint register_result = jni->RegisterNatives(klass, jni_methods_for_jni_call.data(),
                                                    static_cast<jint>(jni_methods_for_jni_call.size()));

        if (register_result != 0)
        {
            message = std::format(
                "[WRAPPER] RegisterNatives failed for class '{}' with error code: {}", klass_name, register_result);

            debug_print_cerr(message);
        }
        else
        {
            message = std::format("[WRAPPER] Successfully registered {} native methods for class '{}'",
                std::to_string(jni_methods_for_jni_call.size()), klass_name);

            debug_print(message);
        }

        EXCEPT_CHECK(jni);

        std::lock_guard lock(tracked_native_classes_mutex);
        tracked_native_classes[klass_name] = jni_methods_for_jni_call.size();
    }
}
