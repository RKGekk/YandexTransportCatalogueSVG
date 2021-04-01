#include "json.h"

#include <sstream>

using namespace std::literals;

namespace json {

    Node LoadNumber(std::istream& input) {
        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input] {
            parsed_num += static_cast<char>(input.get());
            if (!input) {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char] {
            if (!std::isdigit(input.peek())) {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek())) {
                read_char();
            }
        };

        if (input.peek() == '-') {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0') {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.') {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E') {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-') {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try {
            if (is_int) {
                // Сначала пробуем преобразовать строку в int
                try {
                    return Node(std::stoi(parsed_num));
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении
                    // код ниже попробует преобразовать строку в double
                }
            }
            return Node(std::stod(parsed_num));
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    namespace {
        Node LoadNode(std::istream& input);
        Node LoadInt(std::istream& input) {
            return LoadNumber(input);
        }
        Node LoadString(std::istream& input) {
            char ch;
            std::string line;
            bool no_error = false;
            while (input.get(ch)) {
                if (ch == '"') {
                    no_error = true;
                    break;
                }
                if (ch == '\\') {
                    input.get(ch);
                    if (ch == 'r') {
                        ch = '\r';
                    }
                    if (ch == 'n') {
                        ch = '\n';
                    }
                    if (ch == '"') {
                        ch = '\"';
                    }
                    if (ch == 't') {
                        ch = '\t';
                    }
                    if (ch == '\\') {
                        ch = '\\';
                    }
                }

                line += ch;
            }

            if (!no_error) {
                throw json::ParsingError("Error parsing string");
            }

            return Node(move(line));
        }

        Node LoadBool(std::istream& input) {
            char ch;
            std::string line;
            int cnt = 0;
            while (cnt != 5) {
                ++cnt;
                input.get(ch);
                line += ch;
                if (line == "true") {
                    break;
                }
            }

            if (line != "true" && line != "false") {
                throw json::ParsingError("");
            }
            if (line == "true") {
                return Node(true);
            }

            return Node(false);
        }

        Node LoadArray(std::istream& input) {
            Array result;
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            return Node(move(result));
        }

        Node LoadDict(std::istream& input) {
            Dict result;
            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }
                std::string key = LoadString(input).AsString();
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }
            return Node(std::move(result));
        }

        Node LoadNull(std::istream& input) {
            char ch;
            std::string line;
            int cnt = 0;
            while (cnt != 4) {
                ++cnt;
                input.get(ch);
                line += ch;
                ch = ' ';
            }
            if (line != "null") {
                throw json::ParsingError("");
            }
            return Node();
        }

        Node LoadNode(std::istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                if (input >> c) {
                    input.putback(c);
                    return LoadArray(input);
                }
                throw json::ParsingError("Wrong array");
            }
            else if (c == '{') {
                if (input >> c) {
                    input.putback(c);
                    return LoadDict(input);
                }
                throw json::ParsingError("Wrong map");
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == ']' || c == '}') {
                throw json::ParsingError("");
            }
            else {
                input.putback(c);
                return LoadInt(input);
            }
        }
    }  // namespace

    Node::Node() : value_() {}

    bool Node::IsNull() const noexcept {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsInt() const noexcept {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const noexcept {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }

    bool Node::IsPureDouble() const noexcept {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsString() const noexcept {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsBool() const noexcept {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsArray() const noexcept {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const noexcept {
        return std::holds_alternative<Dict>(value_);
    }

    bool Node::operator==(const Node& node) const noexcept {
        return value_ == node.value_;
    }

    bool Node::operator!=(const Node& node) const noexcept {
        return value_ != node.value_;
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value_);
        }
        throw std::logic_error("Failed return array"s);
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(value_);
        }
        throw std::logic_error("Failed return map"s);
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        }
        throw std::logic_error("Failed return bool"s);
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        throw std::logic_error("Failed return int"s);
    }

    std::nullptr_t Node::AsNull() const {
        if (IsNull()) {
            return std::get<std::nullptr_t>(value_);
        }
        throw std::logic_error("Failed return null"s);
    }

    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value_);
        }
        if (IsDouble()) {
            return std::to_string(std::get<double>(value_));
        }
        if (IsInt()) {
            return std::to_string(std::get<int>(value_));
        }
        throw std::logic_error("Failed return string"s);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        throw std::logic_error("Failed return double"s);
    }

    Document::Document(Node root) : root_(std::move(root)) {}

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool Document::operator==(const Document& doc) const noexcept {
        return root_ == doc.root_;
    }

    bool Document::operator!=(const Document& doc) const noexcept {
        return root_ != doc.root_;
    }

    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        NodePrinter{ output }.PrintNode(doc.GetRoot());
    }

    void NodePrinter::operator()(std::nullptr_t) {
        output << "null"sv;
    }

    void NodePrinter::operator()(const std::string_view str) {
        std::string result = "\""s;
        for (const char letter : str) {
            switch (letter) {
            case '\\':
                result += "\\\\"s;
                break;
            case '\"':
                result += "\\\""s;
                break;
            case '\n':
                result += "\\n"s;
                break;
            case '\r':
                result += "\\r"s;
                break;
            case '\t':
                result += "\\t"s;
                break;
            default:
                result += letter;
            }
        }
        output << std::move(result) + "\""s;
    }

    void NodePrinter::operator()(double value) {
        output << value;
    }

    void NodePrinter::operator()(int value) {
        output << value;
    }

    void NodePrinter::operator()(bool value) {
        if (value) { output << "true"s; }
        else { output << "false"s; }
    }

    void NodePrinter::operator()(const Array& arr) {
        output << "["sv;
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            if (it != arr.begin()) {
                output << ", "sv;
            }
            PrintNode(*it);
        }
        output << "]"sv;
    }

    void NodePrinter::operator()(const Dict& dict) {
        output << "{"sv;
        for (auto it = dict.begin(); it != dict.end(); ++it) {
            const auto& [str, value] = *it;
            if (it != dict.begin()) {
                output << ", "sv;
            }
            (*this)(str);
            output << ": "sv;
            PrintNode(value);
        }
        output << "}"sv;
    }
    void NodePrinter::PrintNode(const Node& node) {
        if (node.IsArray()) { (*this)(node.AsArray()); }
        else if (node.IsBool()) { (*this)(node.AsBool()); }
        else if (node.IsPureDouble()) { (*this)(node.AsDouble()); }
        else if (node.IsInt()) { (*this)(node.AsInt()); }
        else if (node.IsNull()) { (*this)(node.AsNull()); }
        else if (node.IsString()) { (*this)(node.AsString()); }
        else if (node.IsMap()) { (*this)(node.AsMap()); }
    }

    Document LoadJSON(const std::string& s) {
        std::istringstream strm(s);
        return json::Load(strm);
    }

    // Раскомментируйте эти функции по мере того, как реализуете недостающий функционал
    std::string Print(const Node& node) {
        std::ostringstream out;
        Print(Document{ node }, out);
        return out.str();
    }
}  // namespace json