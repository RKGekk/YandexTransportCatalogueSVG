#include "json_builder.h"

#include <utility>

namespace json {

	namespace detail {

		bool IsNodeOfArray(LastUsedMetod e, const std::vector<Node>& stack) {
			return stack.back().IsArray() && (e == LastUsedMetod::VALUE || e == LastUsedMetod::START_ARRAY || e == LastUsedMetod::END_ARRAY || e == LastUsedMetod::END_DICT);
		}

		bool InRoot(LastUsedMetod e, const std::vector<Node>& stack) {
			return e != LastUsedMetod::NONE && stack.empty() && (e == LastUsedMetod::END_ARRAY || e == LastUsedMetod::END_DICT);
		}

		bool InStack(LastUsedMetod e, const std::vector<Node>& stack) {
			return stack.size() == 1u && (e == LastUsedMetod::END_ARRAY || e == LastUsedMetod::END_DICT || e == LastUsedMetod::VALUE);
		}
	}

	Node NodeGetter::operator()(std::nullptr_t) const {
		return Node();
	}

	Node NodeGetter::operator()(std::string&& value) const {
		return Node(std::move(value));
	}

	Node NodeGetter::operator()(bool&& value) const {
		return Node(value);
	}

	Node NodeGetter::operator()(int&& value) const {
		return Node(value);
	}

	Node NodeGetter::operator()(double&& value) const {
		return Node(value);
	}

	Node NodeGetter::operator()(Array&& value) const {
		return Node(std::move(value));
	}

	Node NodeGetter::operator()(Dict&& value) const {
		return Node(std::move(value));
	}

	KeyItemContext Builder::Key(std::string s) {
		using namespace std::literals;
		if (nodes_stack_.empty() || !nodes_stack_.back().IsDict()) {
			throw std::logic_error("Called Key() outside the dictionary"s);
		}
		if (last_method_ == LastUsedMetod::KEY) {
			throw std::logic_error("Called Key() after calling Key()"s);
		}
		key_stack_.push_back(std::move(s));
		last_method_ = LastUsedMetod::KEY;

		return { *this };
	}

	Builder& Builder::Value(Node::Value v) {
		using namespace std::literals;
		if (last_method_ == LastUsedMetod::NONE) {
			nodes_stack_.emplace_back(std::move(MakeNode(std::move(v))));
		}
		else if (nodes_stack_.back().IsDict() && last_method_ == LastUsedMetod::KEY) {
			Dict tmp_dict = nodes_stack_.back().AsDict();
			tmp_dict[key_stack_.back()] = MakeNode(std::move(v));
			key_stack_.pop_back();
			nodes_stack_.pop_back();
			nodes_stack_.emplace_back(std::move(tmp_dict));
		}
		else if (detail::IsNodeOfArray(last_method_, nodes_stack_)) {
			Array tmp_arr = nodes_stack_.back().AsArray();
			tmp_arr.push_back(std::move(MakeNode(std::move(v))));
			nodes_stack_.pop_back();
			nodes_stack_.emplace_back(std::move(tmp_arr));
		}
		else {
			throw std::logic_error("Wrong call Value()"s);
		}
		last_method_ = LastUsedMetod::VALUE;

		return *this;
	}

	DictItemContext Builder::StartDict() {
		using namespace std::literals;
		if (last_method_ == LastUsedMetod::NONE) {
			//do nothing here
		}
		else if (nodes_stack_.back().IsDict() && last_method_ == LastUsedMetod::KEY) {
			//do nothing here
		}
		else if (detail::IsNodeOfArray(last_method_, nodes_stack_)) {
			//do nothing here
		}
		else {
			throw std::logic_error("Wrong call StartDict()"s);
		}
		nodes_stack_.emplace_back(std::move(Dict{}));
		last_method_ = LastUsedMetod::START_DICT;

		return { *this };
	}

	ArrayItemContext Builder::StartArray() {
		using namespace std::literals;
		if (last_method_ == LastUsedMetod::NONE) {
			//do nothing here
		}
		else if (nodes_stack_.back().IsDict() && last_method_ == LastUsedMetod::KEY) {
			//do nothing here
		}
		else if (detail::IsNodeOfArray(last_method_, nodes_stack_)) {
			//do nothing here
		}
		else {
			throw std::logic_error("Wrong call StartArray()"s);
		}
		nodes_stack_.emplace_back(std::move(Array{}));
		last_method_ = LastUsedMetod::START_ARRAY;

		return { *this };
	}

	Builder& Builder::EndDict() {
		using namespace std::literals;
		if (!nodes_stack_.empty() && nodes_stack_.back().IsDict()) {
			Dict tmp_dict = nodes_stack_.back().AsDict();
			nodes_stack_.pop_back();
			if (nodes_stack_.empty()) {
				root_ = MakeNode(std::move(tmp_dict));
			}
			else {
				if (nodes_stack_.back().IsArray()) {
					Array tmp_arr = nodes_stack_.back().AsArray();
					tmp_arr.emplace_back(std::move(tmp_dict));
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(std::move(tmp_arr));
				}
				else {
					Dict tmp_dict_1 = nodes_stack_.back().AsDict();
					tmp_dict_1[key_stack_.back()] = std::move(tmp_dict);
					key_stack_.pop_back();
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(std::move(tmp_dict_1));
				}
			}
		}
		else {
			throw std::logic_error("Wrong call EndDict()"s);
		}
		last_method_ = LastUsedMetod::END_DICT;

		return *this;
	}

	Builder& Builder::EndArray() {
		using namespace std::literals;
		if (!nodes_stack_.empty() && nodes_stack_.back().IsArray()) {
			Array tmp_arr = nodes_stack_.back().AsArray();
			nodes_stack_.pop_back();
			if (nodes_stack_.empty()) {
				root_ = MakeNode(std::move(tmp_arr));
			}
			else {
				if (nodes_stack_.back().IsArray()) {
					Array tmp_arr_1 = nodes_stack_.back().AsArray();
					tmp_arr_1.emplace_back(std::move(tmp_arr));
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(std::move(tmp_arr_1));
				}
				else {
					Dict tmp_dict = nodes_stack_.back().AsDict();
					tmp_dict[key_stack_.back()] = std::move(tmp_arr);
					key_stack_.pop_back();
					nodes_stack_.pop_back();
					nodes_stack_.emplace_back(std::move(tmp_dict));
				}
			}
		}
		else {
			throw std::logic_error("Wrong call EndArray()"s);
		}
		last_method_ = LastUsedMetod::END_ARRAY;

		return *this;
	}

	Node Builder::Build() {
		using namespace std::literals;
		if (detail::InStack(last_method_, nodes_stack_)) {
			return nodes_stack_.back();
		}
		else if (detail::InRoot(last_method_, nodes_stack_)) {
			return root_;
		}
		else {
			throw std::logic_error("Wrong call Build()"s);
		}
	}

	Node Builder::MakeNode(Node::Value&& v) {
		json::Node result;
		std::visit(
			[&result](auto&& v) {
			NodeGetter ng;
			result = ng(std::move(v));
		},
			std::move(v)
			);

		return result;
	}

	bool Builder::LMIsNone() {
		return last_method_ == LastUsedMetod::NONE;
	}

	bool Builder::LMIsStartDict() {
		return last_method_ == LastUsedMetod::START_DICT;
	}

	bool Builder::LMIsKey() {
		return last_method_ == LastUsedMetod::KEY;
	}

	bool Builder::LMIsValue() {
		return last_method_ == LastUsedMetod::VALUE;
	}

	bool Builder::LMIsEndDict() {
		return last_method_ == LastUsedMetod::END_DICT;
	}

	bool Builder::LMIsStartArray() {
		return last_method_ == LastUsedMetod::START_ARRAY;
	}

	bool Builder::LMIsEndArray() {
		return last_method_ == LastUsedMetod::END_ARRAY;
	}

	KeyValueCommand::KeyValueCommand(Builder& b) : builder_(b) {}

	KeyValueContext KeyValueCommand::Value(Node::Value v) {
		builder_.Value(std::move(v));

		return { builder_ };
	}

	ArrayValueCommand::ArrayValueCommand(Builder& b) : builder_(b) {}

	ArrayValueItemContext ArrayValueCommand::Value(Node::Value v) {
		builder_.Value(std::move(v));

		return { builder_ };
	}

	StartArrayCommand::StartArrayCommand(Builder& b) : builder_(b) {}

	ArrayItemContext StartArrayCommand::StartArray() {
		return builder_.StartArray();
	}

	EndArrayCommand::EndArrayCommand(Builder& b)
		: builder_(b) {}

	Builder& EndArrayCommand::EndArray() {
		return builder_.EndArray();
	}

	KeyEndDictCommands::KeyEndDictCommands(Builder& builder) : builder_(builder) {}

	KeyItemContext KeyEndDictCommands::Key(std::string s) {
		return builder_.Key(std::move(s));
	}

	Builder& KeyEndDictCommands::EndDict() {
		return builder_.EndDict();
	}

	StartDictCommand::StartDictCommand(Builder& builder) : builder_(builder) {}

	DictItemContext StartDictCommand::StartDict() {
		return builder_.StartDict();
	}

	KeyItemContext::KeyItemContext(Builder& b) : StartDictCommand(b), StartArrayCommand(b), KeyValueCommand(b) {}

	KeyValueContext::KeyValueContext(Builder& b) : KeyEndDictCommands(b) {}

	DictItemContext::DictItemContext(Builder& b) : KeyEndDictCommands(b) {}

	ArrayValueItemContext::ArrayValueItemContext(Builder& b) : StartDictCommand(b), EndArrayCommand(b), StartArrayCommand(b), ArrayValueCommand(b) {}

	ArrayItemContext::ArrayItemContext(Builder& b) : StartDictCommand(b), EndArrayCommand(b), StartArrayCommand(b), ArrayValueCommand(b) {}
}