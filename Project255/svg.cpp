#include "svg.h"

/*
 * Место для вашей svg-библиотеки
 */

#define _USE_MATH_DEFINES

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846   // pi
#endif // !M_PI

namespace svg {

	using namespace std::literals;

	Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}

	Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : Rgb(red, green, blue), opacity(opacity) {}

	void PrintColor(std::ostream& out, std::monostate) {
		out << "none"s;
	}

	void PrintColor(std::ostream& out, const std::string& color) {
		out << color;
	}

	void PrintColor(std::ostream& out, const Rgb& color) {
		out << "rgb("
			<< std::to_string(color.red) << ','
			<< std::to_string(color.green) << ','
			<< std::to_string(color.blue) << ')';
	}

	void PrintColor(std::ostream& out, const Rgba& color) {
		out << "rgba("
			<< std::to_string(color.red) << ','
			<< std::to_string(color.green) << ','
			<< std::to_string(color.blue) << ','
			<< color.opacity << ')';
	}

	std::ostream& operator<<(std::ostream& out, const Color& color) {
		std::visit(
			[&out](auto& value) {
				PrintColor(out, value);
			},
			color
		);
		return out;
	}

	RenderContext::RenderContext(std::ostream& out) : out(out) {}
	RenderContext::RenderContext(std::ostream& out, int indent_step, int indent) : out(out), indent_step(indent_step), indent(indent) {}

	RenderContext RenderContext::Indented() const {
		return{ out, indent_step, indent + indent_step };
	}

	void RenderContext::RenderIndent() const {
		for (int i = 0; i < indent; ++i) {
			out.put(' ');
		}
	}

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();
		// Делегируем вывод тега своим подклассам
		RenderObject(context);
		context.out << std::endl;
	}

	Circle& Circle::SetCenter(Point center) {
		m_center = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		m_radius = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle cx=\""sv << m_center.x << "\" cy=\""sv << m_center.y << "\" "sv;
		out << "r=\""sv << m_radius << "\" "sv;
		RenderAttrs(out);
		out << "/>"sv;
	}

	Polyline& Polyline::AddPoint(Point point) {
		m_points.push_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const {
		if (m_points.size() == 0) { return; }
		auto& out = context.out;
		out << "<polyline points=\""sv;
		out << m_points.front().x << ","sv << m_points.front().y;
		for (auto it = m_points.cbegin() + 1; it != m_points.cend(); ++it) {
			out << " "sv << (*it).x << ","sv << (*it).y;
		}
		out << "\" "sv;
		RenderAttrs(out);
		out << "/>"sv;
	}

	Text& Text::SetPosition(Point pos) {
		m_position = pos;
		return *this;
	}

	Text& Text::SetOffset(Point offset) {
		m_offset = offset;
		return *this;
	}

	Text& Text::SetFontSize(uint32_t size) {
		m_font_size = size;
		return *this;
	}

	Text& Text::SetFontFamily(std::string font_family) {
		m_font_family = std::move(font_family);
		return *this;
	}

	Text& Text::SetFontWeight(std::string font_weight) {
		m_font_weight = std::move(font_weight);
		return *this;
	}

	Text& Text::SetData(std::string data) {
		m_content = std::move(data);
		return *this;
	}

	void Text::RenderObject(const RenderContext& context) const {
		if (m_content.size() == 0) { return; }
		auto& out = context.out;
		out << "<text "sv;
		RenderAttrs(out);
		out << " x=\""sv << m_position.x << "\" "sv;
		out << " y=\""sv << m_position.y << "\" "sv;
		out << " dx=\""sv << m_offset.x << "\" "sv;
		out << " dy=\""sv << m_offset.y << "\" "sv;
		out << " font-size=\""sv << m_font_size << "\" "sv;
		if (m_font_family.size() > 0) { out << " font-family=\""sv << m_font_family << "\" "sv; }
		if (m_font_weight.size() > 0) { out << " font-weight=\""sv << m_font_weight << "\" "sv; }
		out << ">"sv << PrepareData();
		out << "</text>"sv;
	}

	std::string Text::PrepareData() const {
		std::string tmp = m_content;
		while (size_t index = tmp.find_first_of('&') != std::string::npos) {
			tmp = tmp.replace(index, 1, "&amp;"s);
		}
		while (size_t index = tmp.find_first_of('"') != std::string::npos) {
			tmp = tmp.replace(index, 1, "&quot;"s);
		}
		while (size_t index = tmp.find_first_of('\'') != std::string::npos) {
			tmp = tmp.replace(index, 1, "&apos;"s);
		}
		while (size_t index = tmp.find_first_of('`') != std::string::npos) {
			tmp = tmp.replace(index, 1, "&apos;"s);
		}
		while (size_t index = tmp.find_first_of('<') != std::string::npos) {
			tmp = tmp.replace(index, 1, "&lt;"s);
		}
		while (size_t index = tmp.find_first_of('>') != std::string::npos) {
			tmp = tmp.replace(index, 1, "&gt;"s);
		}
		return tmp;
	}

	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		m_elements.push_back(std::move(obj));
	}

	void Document::Render(std::ostream& out) const {
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

		RenderContext ctx(out, 2, 2);
		for (auto const& i : m_elements) {
			i->Render(ctx);
		}

		out << "</svg>"sv;
	}

	Star::Star(Point center, double outer_radius, double inner_radius, int num_rays) : m_center(center), m_outer_radius(outer_radius), m_inner_radius(inner_radius), m_num_rays(num_rays) {}

	void Star::Draw(ObjectContainer& container) const {
		container.Add(CreateStar().SetFillColor("red"s).SetStrokeColor("black"s));
	}

	Polyline Star::CreateStar() const {
		Polyline polyline;
		for (int i = 0; i <= m_num_rays; ++i) {
			double angle = 2 * M_PI * (i % m_num_rays) / m_num_rays;
			polyline.AddPoint({ m_center.x + m_outer_radius * sin(angle), m_center.y - m_outer_radius * cos(angle) });
			if (i == m_num_rays) {
				break;
			}
			angle += M_PI / m_num_rays;
			polyline.AddPoint({ m_center.x + m_inner_radius * sin(angle), m_center.y - m_inner_radius * cos(angle) });
		}
		return polyline;
	}

	Triangle::Triangle(Point p1, Point p2, Point p3) : m_p1(p1), m_p2(p2), m_p3(p3) {}

	void Triangle::Draw(svg::ObjectContainer& container) const {
		container.Add(Polyline().AddPoint(m_p1).AddPoint(m_p2).AddPoint(m_p3).AddPoint(m_p1));
	}

	Snowman::Snowman(Point center, double r) : m_center(center), m_r(r) {}

	void Snowman::Draw(svg::ObjectContainer& container) const {
		container.Add(Circle().SetCenter(Point{ m_center.x, m_center.y + m_r * 5 }).SetRadius(m_r * 2.0).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
		container.Add(Circle().SetCenter(Point{ m_center.x, m_center.y + m_r * 2 }).SetRadius(m_r * 1.5).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
		container.Add(Circle().SetCenter(m_center).SetRadius(m_r).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
	}

}  // namespace svg