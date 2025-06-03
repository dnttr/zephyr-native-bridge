//
// Created by Damian Netter on 03/06/2025.
//

#include "ZNBKit/jni/internal/internal.hpp"

#include "ZNBKit/debug.hpp"

namespace internal
{
    std::mutex global_tracker::mutex;

    std::unordered_set<jobject> global_tracker::global_refs;
    std::unordered_map<jobject, ref_info> global_tracker::global_ref_sources;

    std::unordered_map<std::string, size_t> tracker_manager::tracked_native_classes;

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
        std::lock_guard lock(mutex);
        debug_print_cerr("Dumping " + std::to_string(global_refs.size()) + " global references:");

        for (const auto& ref : global_refs)
        {
            if (auto it = global_ref_sources.find(ref); it != global_ref_sources.end())
            {
                const auto& info = it->second;
                debug_print_cerr("Global ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                                " created at " + info.file + ":" + std::to_string(info.line) +
                                " in " + info.method);
            }
            else
            {
                debug_print_cerr("Global ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                                " (source unknown)");
            }
        }
    }

    void tracker_manager::check_for_corruption()
    {
        const bool is_global_empty = global_tracker::count() == 0;
        const bool is_local_empty = local_refs.empty();
        const bool are_natives_empty = tracked_native_classes.empty();

        if (!is_global_empty || !is_local_empty)
        {
            debug_print_cerr("Warning: References are not empty.");

            debug_print_cerr("Global references count: " + std::to_string(global_tracker::count()));
            debug_print_cerr("Local references count: " + std::to_string(local_refs.size()));

            if (!is_global_empty)
            {
                debug_print_cerr("Global references are not empty, potential memory leak detected.");
                global_tracker::dump_refs();
            }

            if (!is_local_empty)
            {
                debug_print_cerr("Local references are not empty, potential memory leak detected.");
                dump_local_refs();
            }

            return;
        }

        if (!are_natives_empty)
        {
            debug_print_cerr("Warning: Tracked native classes are not empty. ");

            for (const auto&[name, natives] : tracked_native_classes)
            {
                debug_print_cerr("Class " + name + " has " + std::to_string(natives) + " references which were not deleted.");
            }

            return;
        }

        debug_print_cerr("No references left. All good i think, unless not using wrapper, then well, you are on your own. :>");
    }

    void tracker_manager::dump_local_refs()
    {
        debug_print_cerr("Dumping " + std::to_string(local_refs.size()) + " local references:");
        for (const auto& ref : local_refs)
        {
            if (auto it = local_ref_sources.find(ref); it != local_ref_sources.end())
            {
                const auto&[file, line, method, details] = it->second;
                debug_print_cerr("Local ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                               " created at " + file + ":" + std::to_string(line) +
                               " in " + method +
                               (details.empty() ? "" : " (" + details + ")"));
            }
            else
            {
                debug_print_cerr("Local ref " + std::to_string(reinterpret_cast<uintptr_t>(ref)) +
                               " (source unknown)");
            }
        }
    }

    jobject tracker_manager::add_local_ref(JNIEnv *jni, const jobject &obj,
                                  const std::string &file, const int line,
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

    void tracker_manager::remove_local_ref(JNIEnv *jni, const jobject &obj)
    {
        VAR_CHECK(jni);

        if (obj)
        {
            local_refs.erase(obj);
            local_ref_sources.erase(obj);
            jni->DeleteLocalRef(obj);
        }
    }

    jobject tracker_manager::add_global_ref(JNIEnv *jni, const jobject &obj,
                                   const std::string &file, const int line,
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

    void tracker_manager::remove_global_ref(JNIEnv *jni, const jobject &obj)
    {
        VAR_CHECK(jni);

        if (obj)
        {
            global_tracker::remove(obj);
            jni->DeleteGlobalRef(obj);
        }
    }
}
