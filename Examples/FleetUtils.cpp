// SPDX-FileCopyrightText: 2021-2026 Magic Lane International B.V. <info@magiclane.com>
// SPDX-License-Identifier: Apache-2.0
//
// Contact Magic Lane at <info@magiclane.com> for SDK licensing options.

#include "FleetUtils.h"


// JsonValue implementation
JsonValue::JsonValue() : value(nullptr) {}

JsonValue::JsonValue(JsonVariant v) : value(std::move(v)) {}

bool JsonValue::isObject() const 
{ 
    return std::holds_alternative<JsonObject>(value); 
}

bool JsonValue::isArray() const 
{ 
    return std::holds_alternative<JsonArray>(value); 
}

bool JsonValue::isString() const 
{ 
    return std::holds_alternative<std::string>(value); 
}

bool JsonValue::isNumber() const
{ 
    return std::holds_alternative<double>(value); 
}

bool JsonValue::isBool() const
{ 
    return std::holds_alternative<bool>(value); 
}

const JsonObject& JsonValue::asObject() const 
{ 
    return std::get<JsonObject>(value); 
}

const JsonArray& JsonValue::asArray() const 
{ 
    return std::get<JsonArray>(value); 
}

const std::string& JsonValue::asString() const
{ 
    return std::get<std::string>(value); 
}

double JsonValue::asNumber() const
{ 
    return std::get<double>(value); 
}

bool JsonValue::asBool() const 
{ 
    return std::get<bool>(value); 
}

const JsonValue& JsonValue::operator[](const std::string& key) const
{
    return asObject().at(key);
}

const JsonValue& JsonValue::operator[](size_t index) const
{
    return asArray().at(index);
}

// JsonParser implementation
JsonParser::JsonParser(const std::string& input) : s(input) {}

JsonValue JsonParser::parse() 
{
    skipSpaces();
    JsonValue v = parseValue();
    skipSpaces();
    return v;
}

void JsonParser::skipSpaces()
{
    while (i < s.size() && std::isspace(s[i])) ++i;
}

bool JsonParser::match(char c)
{
    skipSpaces();
    if (i < s.size() && s[i] == c) 
    {
        ++i;
        return true;
    }
    return false;
}

JsonValue JsonParser::parseValue()
{
    skipSpaces();
    if (i >= s.size()) throw std::runtime_error("Unexpected end of input");

    char c = s[i];
    if (c == '{') return parseObject();
    if (c == '[') return parseArray();
    if (c == '"') return parseString();
    if (std::isdigit(c) || c == '-') return parseNumber();
    if (s.compare(i, 4, "true") == 0) { i += 4; return JsonValue(true); }
    if (s.compare(i, 5, "false") == 0) { i += 5; return JsonValue(false); }
    if (s.compare(i, 4, "null") == 0) { i += 4; return JsonValue(nullptr); }

    throw std::runtime_error(std::string("Unexpected character: ") + c);
}

JsonValue JsonParser::parseObject() 
{
    match('{');
    JsonObject obj;
    skipSpaces();

    if (match('}')) return JsonValue(obj);

    while (true) 
    {
        skipSpaces();
        std::string key = parseString().asString();
        skipSpaces();
        if (!match(':')) throw std::runtime_error("Expected ':' after key");
        JsonValue val = parseValue();
        obj[key] = val;
        skipSpaces();

        if (match('}')) break;
        if (!match(',')) throw std::runtime_error("Expected ',' or '}' in object");
    }

    return JsonValue(obj);
}

JsonValue JsonParser::parseArray() 
{
    match('[');
    JsonArray arr;
    skipSpaces();

    if (match(']')) return JsonValue(arr);

    while (true) 
    {
        arr.push_back(parseValue());
        skipSpaces();
        if (match(']')) break;
        if (!match(',')) throw std::runtime_error("Expected ',' or ']' in array");
    }

    return JsonValue(arr);
}

JsonValue JsonParser::parseString()
{
    if (!match('"')) throw std::runtime_error("Expected string opening quote");
    std::string result;
    while (i < s.size() && s[i] != '"') 
    {
        if (s[i] == '\\') {
            ++i;
            if (i >= s.size()) throw std::runtime_error("Invalid escape sequence");
            char esc = s[i];
            if (esc == '"') result.push_back('"');
            else if (esc == '\\') result.push_back('\\');
            else if (esc == '/') result.push_back('/');
            else if (esc == 'n') result.push_back('\n');
            else if (esc == 't') result.push_back('\t');
            else throw std::runtime_error("Unknown escape sequence");
        }
        else {
            result.push_back(s[i]);
        }
        ++i;
    }
    if (!match('"')) throw std::runtime_error("Expected closing quote");
    return JsonValue(result);
}

JsonValue JsonParser::parseNumber()
{
    size_t start = i;
    if (s[i] == '-') ++i;
    while (i < s.size() && std::isdigit(s[i])) ++i;
    if (i < s.size() && s[i] == '.') {
        ++i;
        while (i < s.size() && std::isdigit(s[i])) ++i;
    }
    double val = std::stod(s.substr(start, i - start));
    return JsonValue(val);
}

std::vector<gem::Coordinates> Utils::decodePolyline(const std::string& encoded)
{
    constexpr double kPolylinePrecision = 1E5;
    constexpr double kInvPolylinePrecision = 1.0 / kPolylinePrecision;

    std::vector<gem::Coordinates> poly;
    size_t index = 0;
    const size_t len = encoded.length();
    int lat = 0, lng = 0;

    while (index < len) {
        // Decode latitude delta
        int b, shift = 0, result = 0;
        do {
            b = encoded[index++] - 63;
            result |= (b & 0x1f) << shift;
            shift += 5;
        } while (b >= 0x20);
        int dlat = ((result & 1) ? ~(result >> 1) : (result >> 1));
        lat += dlat;

        // Decode longitude delta
        shift = 0;
        result = 0;
        do {
            b = encoded[index++] - 63;
            result |= (b & 0x1f) << shift;
            shift += 5;
        } while (b >= 0x20);
        int dlng = ((result & 1) ? ~(result >> 1) : (result >> 1));
        lng += dlng;

        // Create coordinate with improved precision handling
        poly.emplace_back(
            static_cast<float>(static_cast<double>(lat) * kInvPolylinePrecision),
            static_cast<float>(static_cast<double>(lng) * kInvPolylinePrecision)
        );
    }

    return poly;
}
