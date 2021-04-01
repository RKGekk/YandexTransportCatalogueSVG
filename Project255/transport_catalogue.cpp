#include "transport_catalogue.h"

#include <algorithm>

void TransportCatalogue::addRoute(BusID bus_num, Route route) {
	std::for_each(route.stops.cbegin(), route.stops.cend(), [&](const std::string& name) { _route_stops[name].buses.insert(bus_num); });
	_buses[bus_num] = std::move(route);
}

void TransportCatalogue::addRoute(BusID bus_num, std::vector<RouteStopName> stops) {
	std::for_each(stops.cbegin(), stops.cend(), [&](const std::string& name) { _route_stops[name].buses.insert(bus_num); });
	bool isCircle = stops[0] == stops[stops.size() - 1];
	if (isCircle) {
		_buses[bus_num] = Route{ {stops.begin(), stops.end() - 1}, isCircle };
	}
	else {
		_buses[bus_num] = Route{ std::move(stops), isCircle };
	}
}

void TransportCatalogue::addRoute(BusID bus_num, std::vector<RouteStopName> stops, bool isCircle) {
	std::for_each(stops.cbegin(), stops.cend(), [&](const std::string& name) { _route_stops[name].buses.insert(bus_num); });
	if (isCircle) {
		if (stops[0] == stops[stops.size() - 1]) {
			_buses[bus_num] = Route{ {stops.begin(), stops.end() - 1}, isCircle };
		}
		else {
			_buses[bus_num] = Route{ std::move(stops), isCircle };
		}
	}
	else {
		_buses[bus_num] = Route{ std::move(stops), isCircle };
	}
}

void TransportCatalogue::addRouteStop(const std::string& stop_name, Coordinates coords, Distances distances) {
	if (_route_stops.count(stop_name)) {
		LocalBuses& lb = _route_stops.at(stop_name);
		lb.location = coords;
		lb.distances = distances;
	}
	else {
		_route_stops[stop_name] = { coords, {}, distances };
	}
}

void TransportCatalogue::setDistances(const std::string& stop_name, Distances distances) {
	if (_route_stops.count(stop_name)) {
		_route_stops.at(stop_name).distances = distances;
	}
	else {
		_route_stops[stop_name] = { {0.0, 0.0}, {}, distances };
	}
}

void TransportCatalogue::setDistance(const std::string& stop_name_from, const std::string& stop_name_to, dist distance) {
	if (_route_stops.count(stop_name_from)) {
		_route_stops.at(stop_name_from).distances.insert({ stop_name_to, distance });
	}
	else {
		Distances d;
		d.insert({ stop_name_to, distance });
		_route_stops[stop_name_from] = { {0.0, 0.0}, {}, std::move(d) };
	}
}

dist TransportCatalogue::getFromDistance(const std::string& stop_name_from, const std::string& stop_name_to) const {
	if (_route_stops.count(stop_name_from) && _route_stops.at(stop_name_from).distances.count(stop_name_to)) {
		return _route_stops.at(stop_name_from).distances.at(stop_name_to);
	}
	return 0;
}

double TransportCatalogue::getLength(const std::string& stop_name_from, const std::string& stop_name_to) const {
	if (_route_stops.count(stop_name_from) && _route_stops.count(stop_name_to)) {
		return ComputeDistance(_route_stops.at(stop_name_from).location, _route_stops.at(stop_name_to).location);
	}
	return 0.0;
}

double TransportCatalogue::getFromDistanceOrLength(const std::string& stop_name_from, const std::string& stop_name_to) const {
	if (_route_stops.count(stop_name_from)) {
		if (_route_stops.at(stop_name_from).distances.count(stop_name_to)) {
			return _route_stops.at(stop_name_from).distances.at(stop_name_to);
		}
		else {
			if (_route_stops.at(stop_name_to).distances.count(stop_name_from)) {
				return _route_stops.at(stop_name_to).distances.at(stop_name_from);
			}
			if (_route_stops.count(stop_name_to)) {
				return ComputeDistance(_route_stops.at(stop_name_from).location, _route_stops.at(stop_name_to).location);
			}
		}
	}
	return 0.0;
}

std::vector<Trace> TransportCatalogue::findTracesByStopName(const std::string& name) const {
	std::vector<Trace> result;
	if (_route_stops.count(name)) {
		std::for_each(
			_route_stops.at(name).buses.cbegin(),
			_route_stops.at(name).buses.cend(),
			[&](BusID bid) {
			Trace res;
			res.bus_num = bid;
			res.route = &_buses.at(bid);
			result.push_back(std::move(res));
		}
		);
	}

	return result;
}

std::vector<BusID> TransportCatalogue::getAllBusesIds() const {
	std::vector<BusID> res;
	res.reserve(_buses.size());
	for (auto const& [key, _] : _buses) { res.push_back(key); }
	return res;
}

RoutesInfo TransportCatalogue::getAllRoutesInfo() const {
	RoutesInfo res;
	res.reserve(_buses.size());
	for (auto const& [bid, route] : _buses) {
		std::vector<LocalBusFull> all_lbf;
		for (auto const& sn : route.stops) {
			const LocalBuses& lb = _route_stops.at(sn);
			all_lbf.push_back({ lb.location, sn, lb.buses });
		}
		res.insert({ bid, { all_lbf, route.isRouteCircle } });
	}

	return RoutesInfo();
}

LocalBusFullRef TransportCatalogue::getAllRoutesInfoRef() const {
	return {&_buses, &_route_stops};
}

bool TransportCatalogue::isStopNameExists(const std::string& name) const {
	if (_route_stops.count(name)) {
		return true;
	}
	else {
		return false;
	}
}

const LocalBuses& TransportCatalogue::findLocalBusesByStopName(const std::string& name) const {
	return _route_stops.at(name);
}


bool TransportCatalogue::isBusIDExists(const BusID& bid) const {
	if (_buses.count(bid)) {
		return true;
	}
	else {
		return false;
	}
}

const Route& TransportCatalogue::findRouteByBusID(const BusID& bid) const {
	return _buses.at(bid);
}

double TransportCatalogue::routeLength(const BusID& bid) const {
	return routeMeasure<double>(
		bid,
		[=](const std::string& stop_name_from, const std::string& stop_name_to) {
		return this->getLength(stop_name_from, stop_name_to);
	},
		false
		);
}

double TransportCatalogue::routeDistance(const BusID& bid) const {
	return routeMeasure<double>(
		bid,
		[=](const std::string& stop_name_from, const std::string& stop_name_to) {
		return this->getFromDistanceOrLength(stop_name_from, stop_name_to);
	},
		true
		);
}