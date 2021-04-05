#include "request_handler.h"

#include "json.h"
#include "json_builder.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

void InputDataProcessor::Process(TransportCatalogue& transport_catalog, std::vector<std::unique_ptr<UserInputData>> userInputData) {
	for (std::unique_ptr<UserInputData>& data : userInputData) {
		if (data->getRequestType() == InputRequestType::RouteStop) {
			RouteStopInputData* stopData = static_cast<RouteStopInputData*>(data.get());
			transport_catalog.addRouteStop(stopData->getStopName(), stopData->getCoordinates(), std::move(stopData->getDistances()));
		}
		if (data->getRequestType() == InputRequestType::Bus) {
			BusInputData* busData = static_cast<BusInputData*>(data.get());
			transport_catalog.addRoute(std::move(busData->getBusID()), std::move(busData->getStopNames()), busData->getIsCircle());
		}
	}
}

StatDataProcessor::StatDataProcessor() : m_evt_mgr(new EventManager("Event Manager 1"s, false)) {}

void StatDataProcessor::Process(const TransportCatalogue& transport_catalog, std::vector<std::unique_ptr<UserStatData>> userStatData, std::ostream& out) {
	m_evt_mgr->VTriggerEvent(std::shared_ptr<IEventData>(new EvtData_Before_Start_Processing(std::cout)));
	size_t sz = userStatData.size();
	bool first = true;
	bool last = sz <= 1;
	auto last_it = std::next(userStatData.cbegin(), sz - 1);
	for (auto it = userStatData.cbegin(); it != userStatData.cend(); ++it) {
		last = it == last_it;
		const std::unique_ptr<UserStatData>& data = *it;
		StatRequestType rt = data->getRequestType();
		if (!_processes.count(rt)) {
			continue;
		}
		m_evt_mgr->VTriggerEvent(std::shared_ptr<IEventData>(new EvtData_Before_User_Data_Processing(std::cout, last, first)));
		for (const auto& [key, value] : _processes.at(data->getRequestType())) {
			value(transport_catalog, data, out);
		}
		m_evt_mgr->VTriggerEvent(std::shared_ptr<IEventData>(new EvtData_After_User_Data_Processing(std::cout, last, first)));
		first = false;
	}
	m_evt_mgr->VTriggerEvent(std::shared_ptr<IEventData>(new EvtData_After_End_Processing(std::cout)));
}

int StatDataProcessor::_ct = 0;

int StatDataProcessor::RegisterProcess(StatRequestType rt, ProcessFn fn) {
	_processes[rt][_ct++] = fn;
	return _ct;
}

void StatDataProcessor::RegisterEventListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) {
	m_evt_mgr->VAddListener(eventDelegate, type);
}

void ProcessBus(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {

	BusStatInputData* stopData = static_cast<BusStatInputData*>(userStatData.get());
	BusID& bid = stopData->getBusID();
	if (transport_catalog.isBusIDExists(bid)) {
		std::ios::fmtflags oldFlag = out.flags();

		out << std::setprecision(6);
		out << "Bus " << bid << ": ";

		const Route& rt = transport_catalog.findRouteByBusID(bid);
		out << (rt.isRouteCircle ? rt.stops.size() + 1 : rt.stops.size() + rt.stops.size() - 1) << " stops on route, ";
		out << rt.stops.size() << " unique stops, ";
		out << transport_catalog.routeLength(bid) << " route length";

		out.flags(oldFlag);
	}
	else {
		out << "Bus " << bid << ": not found";
	}
	out << std::endl;
}

void ProcessBusDistance(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {

	BusStatInputData* stopData = static_cast<BusStatInputData*>(userStatData.get());
	BusID& bid = stopData->getBusID();
	if (transport_catalog.isBusIDExists(bid)) {
		std::ios::fmtflags oldFlag = out.flags();

		out << std::setprecision(6);
		out << "Bus " << bid << ": ";

		const Route& rt = transport_catalog.findRouteByBusID(bid);
		out << (rt.isRouteCircle ? rt.stops.size() + 1 : rt.stops.size() + rt.stops.size() - 1) << " stops on route, ";
		out << rt.stops.size() << " unique stops, ";
		double d = transport_catalog.routeDistance(bid);
		double l = transport_catalog.routeLength(bid);
		out << d << " route length, ";
		out << (d / l) << " curvature";

		out.flags(oldFlag);
	}
	else {
		out << "Bus " << bid << ": not found";
	}
	out << std::endl;
}

void ProcessBusDistanceJson(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {
	json::Dict res;
	res.insert({ "request_id"s, userStatData->getRequestID() });
	BusStatInputData* stopData = static_cast<BusStatInputData*>(userStatData.get());
	BusID& bid = stopData->getBusID();
	if (transport_catalog.isBusIDExists(bid)) {
		const Route& rt = transport_catalog.findRouteByBusID(bid);

		res.insert({ "stop_count"s, (int)(rt.isRouteCircle ? rt.stops.size() + 1 : rt.stops.size() + rt.stops.size() - 1) });
		res.insert({ "unique_stop_count"s, (int)rt.stops.size() });

		double d = transport_catalog.routeDistance(bid);
		double l = transport_catalog.routeLength(bid);
		res.insert({ "route_length"s, d });
		res.insert({ "curvature"s, (d / l) });
	}
	else {
		res.insert({ "error_message"s, "not found"s });
	}
	json::Print(json::Document{ json::Node(res) }, out);
}

void ProcessBusDistanceJson2(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {
	
	json::Builder builder{};
	builder.StartDict()
		.Key("request_id"s).Value(userStatData->getRequestID());

	BusStatInputData* stopData = static_cast<BusStatInputData*>(userStatData.get());
	BusID& bid = stopData->getBusID();
	if (transport_catalog.isBusIDExists(bid)) {
		const Route& rt = transport_catalog.findRouteByBusID(bid);

		double d = transport_catalog.routeDistance(bid);
		double l = transport_catalog.routeLength(bid);
		builder
			.Key("stop_count"s).Value((int)(rt.isRouteCircle ? rt.stops.size() + 1 : rt.stops.size() + rt.stops.size() - 1))
			.Key("unique_stop_count"s).Value((int)rt.stops.size())
			.Key("route_length"s).Value(d)
			.Key("curvature"s).Value((d / l));
	}
	else {
		builder
			.Key("error_message"s).Value("not found"s);
	}
	builder.EndDict();
	json::Print(json::Document{ builder.Build()}, out);
}

void ProcessStopJson(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {
	json::Dict res;
	res.insert({ "request_id"s, userStatData->getRequestID() });
	StopStatInputData* stopData = static_cast<StopStatInputData*>(userStatData.get());
	std::string& stopName = stopData->getStopName();
	if (transport_catalog.isStopNameExists(stopName)) {
		std::vector<Trace> traces = transport_catalog.findTracesByStopName(stopName);
		if (traces.size() == 0) {
			res.insert({ "error_message"s, "not found"s });
		}
		else {
			json::Array arrayRes;
			arrayRes.reserve(traces.size());
			for (Trace trace : traces) {
				arrayRes.push_back(json::Node(trace.bus_num));
			}
			res.insert({ "buses"s, arrayRes });
		}
	}
	else {
		res.insert({ "error_message"s, "not found"s });
	}
	json::Print(json::Document{ json::Node(res) }, out);
}

void ProcessStopJson2(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {
	json::Builder builder{};
	builder.StartDict()
		.Key("request_id"s).Value(userStatData->getRequestID());

	StopStatInputData* stopData = static_cast<StopStatInputData*>(userStatData.get());
	std::string& stopName = stopData->getStopName();
	if (transport_catalog.isStopNameExists(stopName)) {
		std::vector<Trace> traces = transport_catalog.findTracesByStopName(stopName);
		if (traces.size() == 0) {
			builder
				.Key("error_message"s).Value("not found"s);
		}
		else {
			builder
				.Key("buses"s)
				.StartArray();
			for (Trace trace : traces) {
				builder.Key(trace.bus_num);
			}
			builder.EndArray();
		}
	}
	else {
		builder
			.Key("error_message"s).Value("not found"s);
	}
	builder.EndDict();
	json::Print(json::Document{ builder.Build() }, out);
}

void ProcessMapJson(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {
	json::Dict res;
	res.insert({ "request_id"s, userStatData->getRequestID() });

	MapStatInputData* stopData = static_cast<MapStatInputData*>(userStatData.get());
	const RenderSettings& render_settings = stopData->getRenderSettings();

	svg::Document doc;
	RoutePictureRef picture(render_settings, transport_catalog.getAllRoutesInfoRef());
	picture.Draw(doc);

	std::ostringstream myString;
	doc.Render(myString);

	res.insert({ "map"s, myString.str() });
	
	json::Print(json::Document{ json::Node(res) }, out);
}

void ProcessMapJson2(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {
	MapStatInputData* stopData = static_cast<MapStatInputData*>(userStatData.get());
	const RenderSettings& render_settings = stopData->getRenderSettings();

	svg::Document doc;
	RoutePictureRef picture(render_settings, transport_catalog.getAllRoutesInfoRef());
	picture.Draw(doc);

	std::ostringstream myString;
	doc.Render(myString);

	json::Print(
		json::Document{
			json::Builder{}
			.StartDict()
				.Key("request_id"s).Value(userStatData->getRequestID())
				.Key("map"s).Value(myString.str())
			.EndDict()
			.Build()
		},
		out
	);
}

void ProcessStop(const TransportCatalogue& transport_catalog, const std::unique_ptr<UserStatData>& userStatData, std::ostream& out) {

	StopStatInputData* stopData = static_cast<StopStatInputData*>(userStatData.get());
	std::string& stopName = stopData->getStopName();
	if (transport_catalog.isStopNameExists(stopName)) {
		std::vector<Trace> traces = transport_catalog.findTracesByStopName(stopName);
		if (traces.size() == 0) {
			out << "Stop " << stopName << ": not found";
		}
		else {
			std::ios::fmtflags oldFlag = out.flags();

			out << std::setprecision(6);
			out << "Stop " << stopName << ": buses";

			for (Trace trace : traces) {
				out << " "s << trace.bus_num;
			}

			out.flags(oldFlag);
		}
	}
	else {
		out << "Stop " << stopName << ": not found";
	}
	out << std::endl;
}

void StartEventHandlerJson(IEventDataPtr e) {
	std::shared_ptr<EvtData_Before_Start_Processing> pEvt = std::static_pointer_cast<EvtData_Before_Start_Processing>(e);
	pEvt->GetOutput() << "[";
}

void EndEventHandlerJson(IEventDataPtr e) {
	std::shared_ptr<EvtData_After_End_Processing> pEvt = std::static_pointer_cast<EvtData_After_End_Processing>(e);
	pEvt->GetOutput() << "]";
}

void MidEventHandlerJson(IEventDataPtr e) {
	std::shared_ptr<EvtData_Before_User_Data_Processing> pEvt = std::static_pointer_cast<EvtData_Before_User_Data_Processing>(e);
	if (!pEvt->IsFirstEntry()) {
		pEvt->GetOutput() << ",";
	}
}

StatDataProcessor StatDataProcessorFactory::Create(StreamType st) {
	StatDataProcessor res;
	if (st == StreamType::TEXT) {
		res.RegisterProcess(StatRequestType::BusStat, ProcessBusDistance);
		res.RegisterProcess(StatRequestType::StopStat, ProcessStop);
	}
	if (st == StreamType::JSON) {
		res.RegisterProcess(StatRequestType::BusStat, ProcessBusDistanceJson2);
		res.RegisterProcess(StatRequestType::StopStat, ProcessStopJson2);
		res.RegisterProcess(StatRequestType::Map, ProcessMapJson2);
		res.RegisterEventListener({ connect_arg<&StartEventHandlerJson> }, EvtData_Before_Start_Processing::sk_EventType);
		res.RegisterEventListener({ connect_arg<&EndEventHandlerJson> }, EvtData_After_End_Processing::sk_EventType);
		res.RegisterEventListener({ connect_arg<&MidEventHandlerJson> }, EvtData_Before_User_Data_Processing::sk_EventType);
	}
	return res;
}