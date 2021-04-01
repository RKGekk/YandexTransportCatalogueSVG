#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <vector>

#include "domain.h"
#include "json.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

class InputReaderText : public InputReader {
public:
	using name_container = std::vector<std::string>;
	using splitter_i = Splitter<name_container>;
	using strong_splitter = std::unique_ptr<splitter_i>;

	InputReaderText(strong_splitter stopNameSplitter, strong_splitter stopNameSplitterStraight, strong_splitter stopNameSplitterCircle, strong_splitter basicСommaSplitter, strong_splitter distancesSplitter);

	std::vector<std::unique_ptr<UserInputData>> getUserInput(std::istream& in) override;

private:
	std::string getStopName(std::istream& in);
	Coordinates getCoordinates(std::istream& in);
	Coordinates getCoordinates(const std::string& lat, const std::string& lng);

	strong_splitter _coordinatesSplitter;
	strong_splitter _distancesSplitter;
	strong_splitter _stopNameSplitterStraight;
	strong_splitter _stopNameSplitterCircle;
	strong_splitter _basicСommaSplitter;
};

class InputReaderFactory {
public:
	static std::unique_ptr<InputReader> Create(StreamType rt);
	template<typename Derived>
	static std::unique_ptr<Derived> Create();
};

class StatReaderText : public StatReader {
public:
	std::vector<std::unique_ptr<UserStatData>> getUserStat(std::istream& in) override;
private:
	std::string getStopName(std::istream& in);
	static int m_ct;
};

class StatReaderFactory {
public:
	static std::unique_ptr<StatReader> Create(StreamType rt);
	template<typename Derived>
	static std::unique_ptr<Derived> Create();
};

class IOReaderText : public IOReader {
public:
	IOReaderText(InputReaderText::strong_splitter stopNameSplitter, InputReaderText::strong_splitter stopNameSplitterStraight, InputReaderText::strong_splitter stopNameSplitterCircle, InputReaderText::strong_splitter basicСommaSplitter, InputReaderText::strong_splitter distancesSplitter);
	std::vector<std::unique_ptr<UserInputData>> getUserInput(std::istream& in) override;
	std::vector<std::unique_ptr<UserStatData>> getUserStat(std::istream& in) override;
private:
	InputReaderText m_inputReaderText;
	StatReaderText m_statReaderText;
};

class InputReaderJson : public InputReader {
public:
	InputReaderJson();
	std::vector<std::unique_ptr<UserInputData>> getUserInput(std::istream& in) override;
	std::vector<std::unique_ptr<UserInputData>> getUserInput(const json::Document& doc);
private:
	Coordinates getCoordinates(const std::string& lat, const std::string& lng);
};

class StatReaderJson : public StatReader {
public:
	StatReaderJson();
	std::vector<std::unique_ptr<UserStatData>> getUserStat(std::istream& in) override;
	std::vector<std::unique_ptr<UserStatData>> getUserStat(const json::Document& doc);
	RenderSettings getRenderSettings(const json::Document& doc);
private:
	svg::Color getColor(const json::Node& node);
};

class IOReaderJson : public IOReader {
public:
	IOReaderJson();
	std::vector<std::unique_ptr<UserInputData>> getUserInput(std::istream& in) override;
	std::vector<std::unique_ptr<UserInputData>> getUserInput(const json::Document& doc);
	std::vector<std::unique_ptr<UserStatData>> getUserStat(std::istream& in) override;
	std::vector<std::unique_ptr<UserStatData>> getUserStat(const json::Document& doc);
	RenderSettings getRenderSettings(const json::Document& doc);
private:
	InputReaderJson m_inputReaderJson;
	StatReaderJson m_statReaderJson;
};

class IOReaderFactory {
public:
	static std::unique_ptr<IOReader> Create(StreamType rt);
	template<typename Derived>
	static std::unique_ptr<Derived> Create() {
		if constexpr (std::is_same_v<Derived, IOReaderText>) {
			return std::unique_ptr<IOReaderText>(
				new IOReaderText(
					std::make_unique<RegExExtractor<InputReaderText::name_container>>("[0-9]+([.][0-9]*)*"s),
					std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>("-"s),
					std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>(">"s),
					std::make_unique<StlSplitter<InputReaderText::name_container>>(","s),
					std::make_unique<StlSplitter<InputReaderText::name_container>>(" to "s)
				)
			);
		}
		if constexpr (std::is_same_v<Derived, IOReaderJson>) {
			return std::unique_ptr<IOReaderJson>(new IOReaderJson());
		}
		return std::unique_ptr<Derived>();
	}
};

