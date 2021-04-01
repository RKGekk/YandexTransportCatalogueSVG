#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <numeric>
#include <vector>
#include <set>
#include <functional>

#include "geo.h"
#include "domain.h"

class TransportCatalogue {
	Buses _buses;
	RouteStops _route_stops;

public:
	void addRoute(BusID bus_num, Route route);
	void addRoute(BusID bus_num, std::vector<RouteStopName> stops);
	void addRoute(BusID bus_num, std::vector<RouteStopName> stops, bool isCircle);

	void addRouteStop(const std::string& stop_name, Coordinates coords, Distances distances);

	void setDistances(const std::string& stop_name, Distances distances);
	void setDistance(const std::string& stop_name_from, const std::string& stop_name_to, dist distance);
	dist getFromDistance(const std::string& stop_name_from, const std::string& stop_name_to) const;
	double getLength(const std::string& stop_name_from, const std::string& stop_name_to) const;
	double getFromDistanceOrLength(const std::string& stop_name_from, const std::string& stop_name_to) const;

	std::vector<Trace> findTracesByStopName(const std::string& name) const;
	std::vector<BusID> getAllBusesIds() const;
	RoutesInfo getAllRoutesInfo() const;
	LocalBusFullRef getAllRoutesInfoRef() const;

	bool isStopNameExists(const std::string& name) const;
	const LocalBuses& findLocalBusesByStopName(const std::string& name) const;

	bool isBusIDExists(const BusID& bid) const;
	const Route& findRouteByBusID(const BusID& bid) const;

	double routeLength(const BusID& bid) const;
	double routeDistance(const BusID& bid) const;

private:
	template<typename D>
	double routeMeasure(const BusID& bid, std::function<D(const std::string& stop_name_from, const std::string& stop_name_to)> dist_from_fn, bool full_measure) const;
};

template<typename D>
inline double TransportCatalogue::routeMeasure(const BusID& bid, std::function<D(const std::string& stop_name_from, const std::string& stop_name_to)> dist_from_fn, bool full_measure) const {
	const Route& rt = this->findRouteByBusID(bid);
	if (rt.stops.size() == 0) {
		return 0.0;
	}
	D res = 0.0;
	const std::string& start = rt.stops[0];
	std::string temp = start;
	if (rt.isRouteCircle) {
		std::for_each(
			rt.stops.cbegin() + 1,
			rt.stops.cend(),
			[&](const std::string& stopName) {
			res += dist_from_fn(temp, stopName);
			temp = stopName;
		}
		);
		return res + dist_from_fn(temp, start);
	}
	else {
		if (full_measure) {
			std::for_each(
				rt.stops.cbegin() + 1,
				rt.stops.cend(),
				[&](const std::string& stopName) {
				res += dist_from_fn(temp, stopName);
				temp = stopName;
			}
			);
			std::for_each(
				rt.stops.crbegin() + 1,
				rt.stops.crend(),
				[&](const std::string& stopName) {
				res += dist_from_fn(temp, stopName);
				temp = stopName;
			}
			);
		}
		else {
			std::for_each(
				rt.stops.cbegin() + 1,
				rt.stops.cend(),
				[&](const std::string& stopName) {
				res += dist_from_fn(temp, stopName);
				temp = stopName;
			}
			);
			res *= 2;
		}
		return res;
	}
}
