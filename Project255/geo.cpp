#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

double ComputeDistance(Coordinates from, Coordinates to) {
	using namespace std;
	static constexpr double dr = 3.14159265358979323846 / 180.0;
	static constexpr double equatorial_radius = 6378.1370;
	static constexpr double distance_center_to_pole = 6356.7523;
	static constexpr double R = ((2.0 * equatorial_radius + distance_center_to_pole) / 3.0) * 1000.0; //WGS84
	double f1 = from.lat * dr;
	double sf1 = sin(f1);
	double cf1 = cos(f1);
	double f2 = to.lat * dr;
	double sf2 = sin(f2);
	double cf2 = cos(f2);
	double dl = abs(from.lng - to.lng) * dr;
	double sdl = sin(dl);
	double cdl = cos(dl);
	return (atan((sqrt((cf2 * sdl) * (cf2 * sdl) + (cf1 * sf2 - sf1 * cf2 * cdl) * (cf1 * sf2 - sf1 * cf2 * cdl))) / (sf1 * sf2 + cf1 * cf2 * cdl))) * R;
}