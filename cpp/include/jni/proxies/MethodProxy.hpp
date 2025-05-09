//
// Created by Damian Netter on 09/05/2025.
//

#pragma once

#include <jni.h>
#include <memory>
#include <string>
#include <vector>

#include "jni/proxies/KlassProxy.hpp"

class MethodProxy
{
    JNIEnv *env;
    jmethodID identity;

    std::unique_ptr<KlassProxy> owner;

    std::string name;
    std::string signature;
    std::vector<std::string> parameters;
    bool is_static;

    jmethodID build_identity() const
    {
        const auto identity = is_static ? env->GetStaticMethodID(owner->get_klass(), name.c_str(), signature.c_str())
                                        : env->GetMethodID(owner->get_klass(), name.c_str(), signature.c_str());

        if (identity == nullptr)
        {
            throw std::runtime_error("Unable to construct method identity for '" + name + "' with signature '" + signature + "'");
        }

        return identity;
    }
public:

    MethodProxy(JNIEnv *env, std::unique_ptr<KlassProxy> owner, std::string &name, std::string &signature, std::vector<std::string> parameters, const bool is_static)
    :
    env(env),
    owner(std::move(owner)),
    name(std::move(name)),
    signature(std::move(signature)),
    parameters(std::move(parameters)),
    is_static(is_static)
    {
        identity = build_identity();
    }
};
