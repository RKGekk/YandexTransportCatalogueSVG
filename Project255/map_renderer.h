#pragma once

#include <vector>
#include <utility>
#include <memory>

#include "svg.h"
#include "domain.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

class RoutePicture : public svg::Drawable {
public:
    RoutePicture(const RenderSettings& settings, RoutesInfo info);
    void Draw(svg::ObjectContainer& container) const override;
private:
    std::vector<svg::Polyline> CreateRouteLineStrip() const;
        
    const RenderSettings m_render_settings;
    RoutesInfo m_routes_info;
};

class RoutePictureRef : public svg::Drawable {
public:
    RoutePictureRef(const RenderSettings& settings, LocalBusFullRef info);
    void Draw(svg::ObjectContainer& container) const override;
private:
    void DrawRouteLineStrip(svg::ObjectContainer& container) const;
    void DrawRouteNames(svg::ObjectContainer& container) const;

    const RenderSettings m_render_settings;
    LocalBusFullRef m_routes_info;
};