// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <variant>
#include <cctype>
#include <stdexcept>

#include <API/GEM_Coordinates.h>  // Now you can include this
class JsonValue;

using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;
using JsonVariant = std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject>;

class JsonValue 
{
public:
    JsonValue();
    JsonValue(JsonVariant v);

    bool isObject() const;
    bool isArray() const;
    bool isString() const;
    bool isNumber() const;
    bool isBool() const;

    const JsonObject& asObject() const;
    const JsonArray& asArray() const;
    const std::string& asString() const;
    double asNumber() const;
    bool asBool() const;

    const JsonValue& operator[](const std::string& key) const;
    const JsonValue& operator[](size_t index) const;

    JsonVariant value;
};

class JsonParser 
{
public:
    explicit JsonParser(const std::string& input);

    JsonValue parse();

private:
    void skipSpaces();
    bool match(char c);
    JsonValue parseValue();
    JsonValue parseObject();
    JsonValue parseArray();
    JsonValue parseString();
    JsonValue parseNumber();

private:
    const std::string& s;
    size_t i = 0;
};

class Utils
{
public:
    static std::vector<gem::Coordinates> decodePolyline(const std::string& encoded);
};


