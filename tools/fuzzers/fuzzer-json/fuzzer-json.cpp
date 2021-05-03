/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <hermes/hermes.h>
#include <jsi/jsi.h>
#include "hermes/Parser/JSONParser.h"
#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"
#include "json.pb.h"
#include <sstream>
#include <string>

using namespace hermes;
using namespace hermes::parser;

// adapted from https://github.com/google/oss-fuzz/blob/master/projects/jsoncpp/json_proto_converter.cc
class JsonProtoConverter {
public:
    std::string convert(const json_proto::JsonObject& json_object) {
        appendObject(json_object);
        return data_.str();
    }
private:
    std::stringstream data_;

    static void escapeName(std::string& name){
        for (char& c:name){
            if ((c >= 0 && c < 32) || c == 127)
                c = '0';
        }
    }

    void appendArray(const json_proto::ArrayValue& array_value) {
        data_ << '[';
        bool need_comma = false;
        for (const auto& value : array_value.value()) {
            if (need_comma) data_ << ',';
            else need_comma = true;
            appendValue(value);
        }
        data_ << ']';
    }

    void appendNumber(const json_proto::NumberValue& number_value) {
        if (number_value.has_float_value()) {
            data_ << number_value.float_value().value();
        } else if (number_value.has_exponent_value()) {
            const auto& value = number_value.exponent_value();
            data_ << value.base();
            data_ << (value.use_uppercase() ? 'E' : 'e');
            data_ << value.exponent();
        } else if (number_value.has_exponent_frac_value()) {
            const auto& value = number_value.exponent_value();
            data_ << value.base();
            data_ << (value.use_uppercase() ? 'E' : 'e');
            data_ << value.exponent();
        } else {
            data_ << number_value.integer_value().value();
        }
    }

    void appendObject(const json_proto::JsonObject& json_object) {
        auto name = json_object.name();
        escapeName(name);
        data_ << '{' << '"' << name << '"' << ':';
        appendValue(json_object.value());
        data_ << '}';
    }

    void appendValue(const json_proto::JsonValue& json_value) {
        if (json_value.has_object_value()) {
            appendObject(json_value.object_value());
        } else if (json_value.has_array_value()) {
            appendArray(json_value.array_value());
        } else if (json_value.has_number_value()) {
            appendNumber(json_value.number_value());
        } else if (json_value.has_string_value()) {
            data_ << '"' << json_value.string_value().value() << '"';
        } else if (json_value.has_boolean_value()) {
            data_ << (json_value.boolean_value().value() ? "true" : "false");
        } else {
            data_ << "null";
        }
    }
};


DEFINE_PROTO_FUZZER(const json_proto::JsonParseAPI &json_proto) {
    JSLexer::Allocator alloc;
    JSONFactory factory(alloc);
    SourceErrorManager sm;
    JsonProtoConverter converter;
    std::string data_str = converter.convert(json_proto.object_value());
    JSONParser parser(factory, data_str, sm);
    auto parsedValue = parser.parse();
    if (parsedValue.hasValue()){
        parsedValue.getValue();
    }
}