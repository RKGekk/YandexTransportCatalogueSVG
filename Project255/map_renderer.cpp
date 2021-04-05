#include "map_renderer.h"

#include <limits>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

RoutePicture::RoutePicture(const RenderSettings& settings, RoutesInfo info) : m_render_settings(settings), m_routes_info(std::move(info)) {}

void RoutePicture::Draw(svg::ObjectContainer& container) const {

}

std::vector<svg::Polyline> RoutePicture::CreateRouteLineStrip() const {
    std::vector<svg::Polyline> res;
    size_t color_ct = 0;
    size_t colors = m_render_settings.color_palette.size();
    std::unordered_map<BusID, std::vector<svg::Point>> points;
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double max_y = std::numeric_limits<double>::min();
    for (auto const& [bid, route_info] : m_routes_info) {
        std::vector<svg::Point> lp;
        lp.reserve(route_info.stops.size());
        for (const LocalBusFull& lbf : route_info.stops) {
            double x = lbf.location.lat;
            double y = lbf.location.lng;
            if (x < min_x) { min_x = x; }
            if (y < min_y) { min_y = y; }
            if (x > max_x) { max_x = x; }
            if (y > max_y) { max_y = y; }
            lp.push_back({ x, y });
        }
        points.insert({ bid, std::move(lp) });
    }
    double length_x = max_x - min_x;
    double length_y = max_y - min_y;
    double x_resolution = m_render_settings.height - m_render_settings.padding * 2.0;
    double y_resolution = m_render_settings.width - m_render_settings.padding * 2.0;
    for (auto const& [_, p_vector] : points) {
        if (p_vector.size() == 0) { continue; }
        svg::Polyline polyline;
        polyline
            .SetStrokeColor(m_render_settings.color_palette[color_ct++ % colors])
            .SetFillColor(svg::NoneColor)
            .SetStrokeWidth(m_render_settings.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        for (const svg::Point& p : p_vector) {
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            polyline.AddPoint({x, y});
        }
        res.push_back(polyline);
    }

    return res;
}

RoutePictureRef::RoutePictureRef(const RenderSettings& settings, LocalBusFullRef info) : m_render_settings(settings), m_routes_info(info) {}

void RoutePictureRef::Draw(svg::ObjectContainer& container) const {

    size_t color_ct = 0;
    size_t colors = m_render_settings.color_palette.size();
    std::unordered_map<BusID, std::vector<std::pair<svg::Point, std::string>>> points;
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double max_y = std::numeric_limits<double>::min();
    for (auto const& [bid, route_info] : *m_routes_info.all_buses) {
        if (route_info.stops.size() == 0) { continue; }
        std::vector<std::pair<svg::Point, std::string>> lp;
        size_t sz = route_info.stops.size();
        lp.reserve(sz);
        for (auto it = route_info.stops.cbegin(); it != route_info.stops.cend(); ++it) {
            const RouteStopName& stop_name = *it;
            const LocalBuses& lb = (*m_routes_info.route_stops).at(stop_name);
            double x = lb.location.lat;
            double y = lb.location.lng;
            if (x < min_x) { min_x = x; }
            if (y < min_y) { min_y = y; }
            if (x > max_x) { max_x = x; }
            if (y > max_y) { max_y = y; }
            lp.push_back({ { x, y }, stop_name });
        }
        if (route_info.isRouteCircle) {
            const RouteStopName& stop_name = route_info.stops[0];
            const LocalBuses& lb = (*m_routes_info.route_stops).at(stop_name);
            double x = lb.location.lat;
            double y = lb.location.lng;
            lp.push_back({ { x, y }, ""s });
        }
        points.insert({ bid, std::move(lp) });
    }
    double length_x = max_x - min_x;
    double length_y = max_y - min_y;
    double x_resolution = m_render_settings.height - m_render_settings.padding * 2.0;
    double y_resolution = m_render_settings.width - m_render_settings.padding * 2.0;
    for (auto const& [bid, p_vector] : points) {
        if (p_vector.size() == 0) { continue; }
        const svg::Color& current_color = m_render_settings.color_palette[color_ct++ % colors];
        svg::Polyline polyline;
        polyline
            .SetStrokeColor(current_color)
            .SetFillColor(svg::NoneColor)
            .SetStrokeWidth(m_render_settings.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (const auto& [p, s] : p_vector) {
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            polyline.AddPoint({ x, y });
        }
        container.Add(polyline);

        for (const auto& [p, s] : p_vector) {
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            svg::Circle circle;
            circle
                .SetCenter({ x, y })
                .SetRadius(m_render_settings.stop_radius)
                .SetFillColor("white");
            container.Add(circle);
            svg::Text stopNameUnder;
            stopNameUnder
                .SetPosition({ x, y })
                .SetOffset(m_render_settings.stop_label_offset)
                .SetFontSize(m_render_settings.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(s)
                .SetFillColor(m_render_settings.underlayer_color)
                .SetStrokeColor(m_render_settings.underlayer_color)
                .SetStrokeWidth(m_render_settings.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            container.Add(stopNameUnder);
            svg::Text stopName;
            stopName
                .SetPosition({x, y})
                .SetOffset(m_render_settings.stop_label_offset)
                .SetFontSize(m_render_settings.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(s)
                .SetFillColor("black"s);
            container.Add(stopName);
        }

        {
            const svg::Point& p = p_vector[0].first;
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            svg::Text textBusUnder;
            textBusUnder
                .SetPosition({ x, y })
                .SetOffset(m_render_settings.bus_label_offset)
                .SetFontSize(m_render_settings.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold")
                .SetData(bid)
                .SetFillColor(m_render_settings.underlayer_color)
                .SetStrokeColor(m_render_settings.underlayer_color)
                .SetStrokeWidth(m_render_settings.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            container.Add(textBusUnder);
            svg::Text textBus;
            textBus
                .SetPosition({ x, y })
                .SetOffset(m_render_settings.bus_label_offset)
                .SetFontSize(m_render_settings.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold")
                .SetData(bid)
                .SetFillColor(current_color);
            container.Add(textBus);
        }
        if (p_vector.back().second.size() != 0) {
            const svg::Point& p = p_vector.back().first;
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            svg::Text textBusUnder;
            textBusUnder
                .SetPosition({ x, y })
                .SetOffset(m_render_settings.bus_label_offset)
                .SetFontSize(m_render_settings.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold")
                .SetData(bid)
                .SetFillColor(m_render_settings.underlayer_color)
                .SetStrokeColor(m_render_settings.underlayer_color)
                .SetStrokeWidth(m_render_settings.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            container.Add(textBusUnder);
            svg::Text textBus;
            textBus
                .SetPosition({ x, y })
                .SetOffset(m_render_settings.bus_label_offset)
                .SetFontSize(m_render_settings.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold")
                .SetData(bid)
                .SetFillColor(current_color);
            container.Add(textBus);
        }
    }
}

void RoutePictureRef::DrawRouteLineStrip(svg::ObjectContainer& container) const {
    size_t color_ct = 0;
    size_t colors = m_render_settings.color_palette.size();
    std::unordered_map<BusID, std::vector<svg::Point>> points;
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double max_y = std::numeric_limits<double>::min();
    for (auto const& [bid, route_info] : *m_routes_info.all_buses) {
        if (route_info.stops.size() == 0) { continue; }
        std::vector<svg::Point> lp;
        size_t sz = route_info.stops.size();
        lp.reserve(sz);
        for (auto it = route_info.stops.cbegin(); it != route_info.stops.cend(); ++it) {
            const RouteStopName& stop_name = *it;
            const LocalBuses& lb = (*m_routes_info.route_stops).at(stop_name);
            double x = lb.location.lat;
            double y = lb.location.lng;
            if (x < min_x) { min_x = x; }
            if (y < min_y) { min_y = y; }
            if (x > max_x) { max_x = x; }
            if (y > max_y) { max_y = y; }
            lp.push_back({ x, y });
        }
        if (route_info.isRouteCircle) {
            const RouteStopName& stop_name = route_info.stops[0];
            const LocalBuses& lb = (*m_routes_info.route_stops).at(stop_name);
            double x = lb.location.lat;
            double y = lb.location.lng;
            lp.push_back({ x, y });
        }
        points.insert({ bid, std::move(lp) });
    }
    double length_x = max_x - min_x;
    double length_y = max_y - min_y;
    double x_resolution = m_render_settings.height - m_render_settings.padding * 2.0;
    double y_resolution = m_render_settings.width - m_render_settings.padding * 2.0;
    for (auto const& [_, p_vector] : points) {
        if (p_vector.size() == 0) { continue; }
        svg::Polyline polyline;
        polyline
            .SetStrokeColor(m_render_settings.color_palette[color_ct++ % colors])
            .SetFillColor(svg::NoneColor)
            .SetStrokeWidth(m_render_settings.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        for (const svg::Point& p : p_vector) {
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            polyline.AddPoint({ x, y });
        }
        container.Add(polyline);
    }
}

void RoutePictureRef::DrawRouteNames(svg::ObjectContainer& container) const {
    size_t color_ct = 0;
    size_t colors = m_render_settings.color_palette.size();
    std::unordered_map<BusID, std::vector<std::pair<svg::Point, std::string>>> points;
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::min();
    double max_y = std::numeric_limits<double>::min();
    for (auto const& [bid, route_info] : *m_routes_info.all_buses) {
        if (route_info.stops.size() == 0) { continue; }
        std::vector<std::pair<svg::Point, std::string>> lp;
        size_t sz = route_info.stops.size();
        lp.reserve(sz);
        for (auto it = route_info.stops.cbegin(); it != route_info.stops.cend(); ++it) {
            const RouteStopName& stop_name = *it;
            const LocalBuses& lb = (*m_routes_info.route_stops).at(stop_name);
            double x = lb.location.lat;
            double y = lb.location.lng;
            if (x < min_x) { min_x = x; }
            if (y < min_y) { min_y = y; }
            if (x > max_x) { max_x = x; }
            if (y > max_y) { max_y = y; }
            lp.push_back({ { x, y }, stop_name });
        }
        points.insert({ bid, std::move(lp) });
    }
    double length_x = max_x - min_x;
    double length_y = max_y - min_y;
    double x_resolution = m_render_settings.height - m_render_settings.padding * 2.0;
    double y_resolution = m_render_settings.width - m_render_settings.padding * 2.0;
    for (auto const& [_, p_vector] : points) {
        if (p_vector.size() == 0) { continue; }
        for (const auto& [p, s] : p_vector) {
            double x = ((p.x - min_x) / (length_x)) * x_resolution + m_render_settings.padding;
            double y = ((p.y - min_y) / (length_y)) * y_resolution + m_render_settings.padding;
            svg::Text text;
            text
                .SetPosition({x, y})
                .SetOffset(m_render_settings.bus_label_offset)
                .SetFontSize(m_render_settings.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold")
                .SetData(s)
                .SetFillColor(m_render_settings.underlayer_color)
                .SetStrokeColor(m_render_settings.underlayer_color)
                .SetStrokeWidth(m_render_settings.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            container.Add(text);
        }
    }
}
