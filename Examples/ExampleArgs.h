// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

// Shared command-line parsing helpers for examples.
#pragma once

#include "Environment.h"

#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>
#include <filesystem>

namespace ExampleArgs
{
    namespace detail
    {
        inline bool starts_with(const char* s, const char* prefix)
        {
            if (!s || !prefix) return false;
            const size_t n = std::strlen(prefix);
            return std::strncmp(s, prefix, n) == 0;
        }

        inline bool iequals(const char* a, const char* b)
        {
            if (!a || !b) return false;
            while (*a && *b)
            {
                const unsigned char ca = static_cast<unsigned char>(*a++);
                const unsigned char cb = static_cast<unsigned char>(*b++);

                const unsigned char la = (ca >= 'A' && ca <= 'Z') ? static_cast<unsigned char>(ca - 'A' + 'a') : ca;
                const unsigned char lb = (cb >= 'A' && cb <= 'Z') ? static_cast<unsigned char>(cb - 'A' + 'a') : cb;

                if (la != lb) return false;
            }
            return *a == '\0' && *b == '\0';
        }
    }

    // Parses: --ui_backend=lvgl|imgui
    // Returns the chosen framework; if arg is missing/invalid, returns defaultFramework.
    inline Environment::WindowFrameworks ParseUiBackend(
        int argc,
        char** argv,
        Environment::WindowFrameworks defaultFramework = Environment::WindowFrameworks::ImGUI)
    {
        const char* kPrefix = "--ui_backend=";

        for (int i = 1; i < argc; ++i)
        {
            const char* arg = argv[i];
            if (!detail::starts_with(arg, kPrefix))
                continue;

            const char* value = arg + std::strlen(kPrefix);
            if (!value || *value == '\0')
                return defaultFramework;

            if (detail::iequals(value, "lvgl"))
                return Environment::WindowFrameworks::LVGL;

            if (detail::iequals(value, "imgui"))
                return Environment::WindowFrameworks::ImGUI;

            // Unknown value -> keep default (don’t fail examples)
            return defaultFramework;
        }

        return defaultFramework;
    }

    // Parses: --size=WxH (e.g. --size=480x800)
    // Returns the parsed size; if arg is missing/invalid, returns defaultSize.
    inline gem::Size ParseSize(int argc, char** argv, gem::Size defaultSize = gem::Size(0, 0))
    {
        const char* kPrefix = "--size=";

        for (int i = 1; i < argc; ++i)
        {
            const char* arg = argv[i];
            if (!detail::starts_with(arg, kPrefix))
                continue;

            const char* value = arg + std::strlen(kPrefix);
            if (!value || *value == '\0')
                return defaultSize;

            std::string s(value);
            auto xPos = s.find('x');
            if (xPos == std::string::npos)
                return defaultSize;

            int w = std::atoi(s.substr(0, xPos).c_str());
            int h = std::atoi(s.substr(xPos + 1).c_str());
            if (w > 0 && h > 0)
                return gem::Size(w, h);

            return defaultSize;
        }

        return defaultSize;
    }

#if defined(__unix__) && !defined(__EMSCRIPTEN__)
    // Detect the primary display resolution from DRM sysfs.
    // Iterates /sys/class/drm/ looking for a connected connector with a valid mode.
    // Returns gem::Size(0,0) if detection fails.
    inline gem::Size DetectDisplaySize()
    {
        const std::filesystem::path drmPath("/sys/class/drm");
        if (!std::filesystem::exists(drmPath))
            return gem::Size(0, 0);

        for (const auto& entry : std::filesystem::directory_iterator(drmPath))
        {
            auto modesPath = entry.path() / "modes";
            if (!std::filesystem::exists(modesPath))
                continue;

            std::ifstream modesFile(modesPath);
            if (!modesFile.is_open())
                continue;

            std::string line;
            if (!std::getline(modesFile, line) || line.empty())
                continue;

            auto xPos = line.find('x');
            if (xPos == std::string::npos)
                continue;

            int w = std::atoi(line.substr(0, xPos).c_str());
            int h = std::atoi(line.substr(xPos + 1).c_str());
            if (w > 0 && h > 0)
                return gem::Size(w, h);
        }

        return gem::Size(0, 0);
    }
#else
    inline gem::Size DetectDisplaySize()
    {
        return gem::Size(0, 0);
    }
#endif

    // Parses: --rotation=0|90|180|270
    // Returns the chosen rotation angle; if arg is missing/invalid, returns defaultRotation.
    inline int ParseRotation(int argc, char** argv, int defaultRotation = 0)
    {
        const char* kPrefix = "--rotation=";

        for (int i = 1; i < argc; ++i)
        {
            const char* arg = argv[i];
            if (!detail::starts_with(arg, kPrefix))
                continue;

            const char* value = arg + std::strlen(kPrefix);
            if (!value || *value == '\0')
                return defaultRotation;

            int angle = std::atoi(value);
            if (angle == 0 || angle == 90 || angle == 180 || angle == 270)
                return angle;

            return defaultRotation;
        }

        return defaultRotation;
    }
}
