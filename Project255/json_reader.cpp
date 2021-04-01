#include "json_reader.h"

/*
 * Çäåñü ìîæíî ğàçìåñòèòü êîä íàïîëíåíèÿ òğàíñïîğòíîãî ñïğàâî÷íèêà äàííûìè èç JSON,
 * à òàêæå êîä îáğàáîòêè çàïğîñîâ ê áàçå è ôîğìèğîâàíèå ìàññèâà îòâåòîâ â ôîğìàòå JSON
 */

IOReaderText::IOReaderText(InputReaderText::strong_splitter stopNameSplitter, InputReaderText::strong_splitter stopNameSplitterStraight, InputReaderText::strong_splitter stopNameSplitterCircle, InputReaderText::strong_splitter basicÑommaSplitter, InputReaderText::strong_splitter distancesSplitter) : m_inputReaderText(std::move(stopNameSplitter), std::move(stopNameSplitterStraight), std::move(stopNameSplitterCircle), std::move(basicÑommaSplitter), std::move(distancesSplitter)), m_statReaderText() {}

std::vector<std::unique_ptr<UserInputData>> IOReaderText::getUserInput(std::istream& in) {
	return m_inputReaderText.getUserInput(in);
}

std::vector<std::unique_ptr<UserStatData>> IOReaderText::getUserStat(std::istream& in) {
	return m_statReaderText.getUserStat(in);
}

InputReaderText::InputReaderText(strong_splitter coordinatesSplitter, strong_splitter stopNameSplitterStraight, strong_splitter stopNameSplitterCircle, strong_splitter basicÑommaSplitter, strong_splitter distancesSplitter) : _coordinatesSplitter(std::move(coordinatesSplitter)), _stopNameSplitterStraight(std::move(stopNameSplitterStraight)), _stopNameSplitterCircle(std::move(stopNameSplitterCircle)), _basicÑommaSplitter(std::move(basicÑommaSplitter)), _distancesSplitter(std::move(distancesSplitter)) {}

std::unique_ptr<InputReader> InputReaderFactory::Create(StreamType rt) {
	if (rt == StreamType::TEXT) {
		return std::unique_ptr<InputReader>(
			new InputReaderText(
				std::make_unique<RegExExtractor<InputReaderText::name_container>>("[0-9]+([.][0-9]*)*"s),
				std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>("-"s),
				std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>(">"s),
				std::make_unique<StlSplitter<InputReaderText::name_container>>(","s),
				std::make_unique<StlSplitter<InputReaderText::name_container>>(" to "s)
			)
			);
	}
	if (rt == StreamType::JSON) {
		return std::unique_ptr<InputReader>(new InputReaderJson());
	}
	return std::unique_ptr<InputReader>();
}

template<typename Derived>
std::unique_ptr<Derived> InputReaderFactory::Create() {
	if constexpr (std::is_same_v<Derived, InputReaderText>) {
		return std::unique_ptr<InputReaderText>(
			new InputReaderText(
				std::make_unique<RegExExtractor<InputReaderText::name_container>>("[0-9]+([.][0-9]*)*"s),
				std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>("-"s),
				std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>(">"s),
				std::make_unique<StlSplitter<InputReaderText::name_container>>(","s),
				std::make_unique<StlSplitter<InputReaderText::name_container>>(" to "s)
			)
			);
	}
	if constexpr (std::is_same_v<Derived, InputReaderJson>) {
		return std::unique_ptr<InputReaderJson>(new InputReaderJson());
	}
	return std::unique_ptr<Derived>();
}

std::vector<std::unique_ptr<UserInputData>> InputReaderText::getUserInput(std::istream& in) {

	std::vector<std::unique_ptr<UserInputData>> res;

	int ct = 0;
	in >> ct;
	res.reserve(ct);
	while (ct--) {
		std::string command;
		in >> command;
		if (command == "Stop") {
			std::string stopName = getStopName(in);
			std::vector<std::string> s = _basicÑommaSplitter->Split(in);
			Distances distances;
			if (s.size() == 2) {
				res.push_back(std::make_unique<RouteStopInputData>(std::move(stopName), getCoordinates(s[0], s[1]), std::move(distances)));
			}
			else {
				Coordinates c = getCoordinates(s[0], s[1]);
				for (auto it = s.begin() + 2; it != s.end(); ++it) {
					std::vector<std::string> ds = _distancesSplitter->Split(*it);
					std::string stopName = ds[1];
					ftrim(stopName);
					if constexpr (std::is_integral_v<dist>) { distances.insert({ stopName, std::stoi(ds[0]) }); }
					if constexpr (std::is_floating_point_v<dist>) { distances.insert({ stopName, std::stod(ds[0]) }); }
				}
				res.push_back(std::make_unique<RouteStopInputData>(std::move(stopName), c, std::move(distances)));
			}
		}
		else if (command == "Bus") {
			BusID bid = getStopName(in);
			std::string line;
			std::getline(in, line);
			if (line.find_first_of(_stopNameSplitterStraight->getSeparatorVerb()) != std::string::npos) {
				res.push_back(std::make_unique<BusInputData>(_stopNameSplitterStraight->Split(line), bid, false));
			}
			else {
				res.push_back(std::make_unique<BusInputData>(_stopNameSplitterCircle->Split(line), bid, true));
			}
		}
	}

	return res;
}

std::string InputReaderText::getStopName(std::istream& in) {
	std::string result;
	std::string tmp;
	std::getline(in, tmp, ':');
	std::stringstream sstream;
	sstream << tmp;
	std::string stopPart;
	sstream >> stopPart;
	result += stopPart;
	while (sstream >> stopPart) {
		result += " "s + stopPart;
	}
	return result;
}

Coordinates InputReaderText::getCoordinates(std::istream& in) {
	std::vector<std::string> nums = _coordinatesSplitter->Split(in);
	return Coordinates{ std::stod(nums[0]), std::stod(nums[1]) };
}

Coordinates InputReaderText::getCoordinates(const std::string& lat, const std::string& lng) {
	return Coordinates{ std::stod(lat), std::stod(lng) };
}

InputReaderJson::InputReaderJson() {}

std::vector<std::unique_ptr<UserInputData>> InputReaderJson::getUserInput(std::istream& in) {
	return getUserInput(json::Load(in));
}

Coordinates InputReaderJson::getCoordinates(const std::string& lat, const std::string& lng) {
	return Coordinates{ std::stod(lat), std::stod(lng) };
}

std::vector<std::unique_ptr<UserInputData>> InputReaderJson::getUserInput(const json::Document& doc) {
	std::vector<std::unique_ptr<UserInputData>> res;
	const json::Array& base_requests = doc.GetRoot().AsMap().at("base_requests").AsArray();
	res.reserve(base_requests.size());
	for (auto it = base_requests.cbegin(); it != base_requests.cend(); ++it) {
		const json::Dict& rq = (*it).AsMap();
		const std::string& command = rq.at("type").AsString();
		if (command == "Stop") {
			Distances distances;
			for (const auto& [key, value] : rq.at("road_distances").AsMap()) {
				distances.insert({ key, value.AsInt() });
			}
			res.push_back(
				std::make_unique<RouteStopInputData>(
					rq.at("name").AsString(),
					Coordinates{ rq.at("latitude").AsDouble(), rq.at("longitude").AsDouble() },
					std::move(distances)
					)
			);
		}
		else if (command == "Bus") {
			std::vector<std::string> stop_names;
			const json::Array& names = rq.at("stops").AsArray();
			stop_names.reserve(names.size());
			for (const auto& node : names) {
				stop_names.push_back(node.AsString());
			}
			res.push_back(
				std::make_unique<BusInputData>(
					std::move(stop_names),
					rq.at("name").AsString(),
					rq.at("is_roundtrip").AsBool()
					)
			);
		}
	}

	return res;
}

std::string StatReaderText::getStopName(std::istream& in) {
	std::string result;
	std::string tmp;
	std::getline(in, tmp);
	std::stringstream sstream;
	sstream << tmp;
	std::string stopPart;
	sstream >> stopPart;
	result += stopPart;
	while (sstream >> stopPart) {
		result += " "s + stopPart;
	}
	return result;
}

std::unique_ptr<StatReader> StatReaderFactory::Create(StreamType rt) {
	if (rt == StreamType::TEXT) {
		return std::unique_ptr<StatReader>(new StatReaderText());
	}
	if (rt == StreamType::JSON) {
		return std::unique_ptr<StatReader>(new StatReaderJson());
	}
	return std::unique_ptr<StatReader>();
}

template<typename Derived>
std::unique_ptr<Derived> StatReaderFactory::Create() {
	if constexpr (std::is_same_v<Derived, StatReaderText>) {
		return std::unique_ptr<StatReaderText>(new StatReaderText());
	}
	if constexpr (std::is_same_v<Derived, StatReaderJson>) {
		return std::unique_ptr<StatReaderJson>(new StatReaderJson());
	}
	return std::unique_ptr<Derived>();
}

int StatReaderText::m_ct = 0;

std::vector<std::unique_ptr<UserStatData>> StatReaderText::getUserStat(std::istream& in) {
	std::vector<std::unique_ptr<UserStatData>> res;
	int ct = 0;
	in >> ct;
	res.reserve(ct);
	while (ct--) {
		std::string command;
		in >> command;
		if (command == "Bus") {
			BusID bid;
			in >> bid;
			res.push_back(std::make_unique<BusStatInputData>(m_ct++, std::move(bid)));
		}
		if (command == "Stop") {
			std::string stopName = getStopName(in);
			res.push_back(std::make_unique<StopStatInputData>(m_ct++, std::move(stopName)));
		}
	}
	return res;
}

StatReaderJson::StatReaderJson() {}

std::vector<std::unique_ptr<UserStatData>> StatReaderJson::getUserStat(std::istream& in) {
	return getUserStat(json::Load(in));
}

std::vector<std::unique_ptr<UserStatData>> StatReaderJson::getUserStat(const json::Document& doc) {
	std::vector<std::unique_ptr<UserStatData>> res;
	const json::Array& stat_requests = doc.GetRoot().AsMap().at("stat_requests").AsArray();
	res.reserve(stat_requests.size());
	for (auto it = stat_requests.cbegin(); it != stat_requests.cend(); ++it) {
		const json::Dict& rq = (*it).AsMap();
		const std::string& command = rq.at("type").AsString();
		if (command == "Bus") {
			res.push_back(
				std::make_unique<BusStatInputData>(
					rq.at("id").AsInt(),
					rq.at("name").AsString()
				)
			);
		}
		else if (command == "Stop") {
			res.push_back(
				std::make_unique<StopStatInputData>(
					rq.at("id").AsInt(),
					rq.at("name").AsString()
				)
			);
		}
		else if (command == "Map") {
			res.push_back(
				std::make_unique<MapStatInputData>(
					rq.at("id").AsInt(),
					getRenderSettings(doc)
				)
			);
		}
	}

	return res;
}

RenderSettings StatReaderJson::getRenderSettings(const json::Document& doc) {
	RenderSettings res;
	const json::Dict& render_settings = doc.GetRoot().AsMap().at("render_settings").AsMap();
	res.width = render_settings.at("width").AsDouble();
	res.height = render_settings.at("height").AsDouble();
	res.padding = render_settings.at("padding").AsDouble();
	res.line_width = render_settings.at("line_width").AsDouble();
	res.stop_radius = render_settings.at("stop_radius").AsDouble();
	res.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();
	res.bus_label_offset = { render_settings.at("bus_label_offset").AsArray()[0].AsDouble(), render_settings.at("bus_label_offset").AsArray()[1].AsDouble() };
	res.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
	res.stop_label_offset = { render_settings.at("stop_label_offset").AsArray()[0].AsDouble(), render_settings.at("stop_label_offset").AsArray()[1].AsDouble() };
	res.underlayer_color = getColor(render_settings.at("underlayer_color"));
	res.underlayer_width = render_settings.at("underlayer_width").AsDouble();
	const json::Array& colors = render_settings.at("color_palette").AsArray();
	res.color_palette.reserve(colors.size());
	for (const json::Node& c : colors) {
		res.color_palette.push_back(getColor(c));
	}
	return res;
}

IOReaderJson::IOReaderJson() : m_inputReaderJson(), m_statReaderJson() {}

std::vector<std::unique_ptr<UserInputData>> IOReaderJson::getUserInput(std::istream& in) {
	return m_inputReaderJson.getUserInput(in);
}

std::vector<std::unique_ptr<UserInputData>> IOReaderJson::getUserInput(const json::Document& doc) {
	return m_inputReaderJson.getUserInput(doc);
}

std::vector<std::unique_ptr<UserStatData>> IOReaderJson::getUserStat(std::istream& in) {
	return m_statReaderJson.getUserStat(in);
}

std::vector<std::unique_ptr<UserStatData>> IOReaderJson::getUserStat(const json::Document& doc) {
	return m_statReaderJson.getUserStat(doc);
}

RenderSettings IOReaderJson::getRenderSettings(const json::Document& doc) {
	return m_statReaderJson.getRenderSettings(doc);
}

svg::Color StatReaderJson::getColor(const json::Node& node) {
	if (node.IsString()) {
		return node.AsString();
	}
	if (node.IsArray()) {
		const json::Array& components = node.AsArray();
		if (components.size() == 3) {
			return svg::Rgb(components[0].AsInt(), components[1].AsInt(), components[2].AsInt());
		}
		if (components.size() == 4) {
			return svg::Rgba(components[0].AsInt(), components[1].AsInt(), components[2].AsInt(), components[3].AsDouble());
		}
	}
	return svg::Color();
}

std::unique_ptr<IOReader> IOReaderFactory::Create(StreamType rt) {
	if (rt == StreamType::TEXT) {
		return std::unique_ptr<IOReader>(
			new IOReaderText(
				std::make_unique<RegExExtractor<InputReaderText::name_container>>("[0-9]+([.][0-9]*)*"s),
				std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>("-"s),
				std::make_unique<StlSplitterTrimmer<InputReaderText::name_container>>(">"s),
				std::make_unique<StlSplitter<InputReaderText::name_container>>(","s),
				std::make_unique<StlSplitter<InputReaderText::name_container>>(" to "s)
			)
			);
	}
	if (rt == StreamType::JSON) {
		return std::unique_ptr<IOReader>(new IOReaderJson());
	}
	return std::unique_ptr<IOReader>();
}
