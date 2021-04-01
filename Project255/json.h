#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <string_view>
#include <stdexcept>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node();
        template <typename T>
        Node(T v) : value_(std::move(v)) {}

        bool IsNull() const noexcept;
        bool IsInt() const noexcept;
        bool IsDouble() const noexcept;
        bool IsPureDouble() const noexcept;
        bool IsString() const noexcept;
        bool IsBool() const noexcept;
        bool IsArray() const noexcept;
        bool IsMap() const noexcept;

        bool operator==(const Node& node) const noexcept;
        bool operator!=(const Node& node) const noexcept;

        std::nullptr_t AsNull() const;
        const std::string& AsString() const;
        double AsDouble() const;
        int AsInt() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        bool AsBool() const;

    private:
        NodeValue value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& doc) const noexcept;
        bool operator!=(const Document& doc) const noexcept;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    struct NodePrinter {
        std::ostream& output;
        void operator()(std::nullptr_t);
        void operator()(const std::string_view str);
        void operator()(double value);
        void operator()(int value);
        void operator()(bool value);
        void operator()(const Array& arr);
        void operator()(const Dict& dict);
        void PrintNode(const Node& node);
    };

    Document LoadJSON(const std::string& s);
    std::string Print(const Node& node);
}  // namespace json