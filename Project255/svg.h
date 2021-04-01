#pragma once

/*
 * Место для вашей svg-библиотеки
 */

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {

    using namespace std::literals;

    struct Rgb {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;

        Rgb(uint8_t red, uint8_t green, uint8_t blue);
        Rgb() = default;
    };

    struct Rgba : Rgb {
        double opacity = 1.0;

        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);
        Rgba() = default;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    void PrintColor(std::ostream& out, std::monostate);

    void PrintColor(std::ostream& out, const std::string& color);

    void PrintColor(std::ostream& out, const svg::Rgb& color);

    void PrintColor(std::ostream& out, const svg::Rgba& color);

    std::ostream& operator<<(std::ostream& out, const Color& color);

    inline const Color NoneColor = std::monostate{};

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y) : x(x), y(y) {}
        double x = 0;
        double y = 0;
    };

    inline std::ostream& operator<<(std::ostream& out, const StrokeLineCap& other) {
        switch (other) {
        case StrokeLineCap::BUTT: out << "butt"; break;
        case StrokeLineCap::ROUND: out << "round"; break;
        case StrokeLineCap::SQUARE: out << "square"; break;
        }
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out,
        const StrokeLineJoin& other) {
        switch (other) {
        case StrokeLineJoin::ARCS: out << "arcs"; break;
        case StrokeLineJoin::BEVEL: out << "bevel"; break;
        case StrokeLineJoin::MITER: out << "miter"; break;
        case StrokeLineJoin::MITER_CLIP: out << "miter-clip"; break;
        case StrokeLineJoin::ROUND: out << "round"; break;
        }
        return out;
    }

    template <typename Owner>
    class PathProps {
    public:
        virtual ~PathProps() = default;
        Owner& SetFillColor(Color color) { fill_color_ = std::move(color); return AsOwner(); }
        Owner& SetStrokeColor(Color color) { stroke_color_ = std::move(color); return AsOwner(); }
        Owner& SetStrokeWidth(double width) { stroke_width_ = std::move(width); return AsOwner(); }
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) { stroke_linecap_ = std::move(line_cap); return AsOwner(); }
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) { stroke_linejoin_ = std::move(line_join); return AsOwner(); }
    protected:
        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) { out << " fill=\""sv << *fill_color_ << "\" "sv; }
            if (stroke_color_) { out << " stroke=\""sv << *stroke_color_ << "\" "sv; }
            if (stroke_width_) { out << " stroke-width=\""sv << *stroke_width_ << "\" "sv; }
            if (stroke_linecap_) { out << " stroke-linecap=\""sv << *stroke_linecap_ << "\" "sv; }
            if (stroke_linejoin_) { out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\" "sv; }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out);
        RenderContext(std::ostream& out, int indent_step, int indent = 0);

        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::move(std::make_unique<Obj>(std::move(obj))));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point m_center;
        double m_radius = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> m_points;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point m_position;
        Point m_offset;
        uint32_t m_font_size = 11;
        std::string m_font_family;
        std::string m_font_weight;
        std::string m_content;

        std::string PrepareData() const;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Star : public Drawable {
    public:
        Star(Point center, double outer_radius, double inner_radius, int num_rays);
        virtual void Draw(ObjectContainer& container) const override;
    private:
        Polyline CreateStar() const;

        Point m_center;
        double m_outer_radius;
        double m_inner_radius;
        int m_num_rays;
    };

    class Triangle : public Drawable {
    public:
        Triangle(Point p1, Point p2, Point p3);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        Point m_p1;
        Point m_p2;
        Point m_p3;
    };

    class Snowman : public Drawable {
    public:
        Snowman(Point center, double r);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        Point m_center;
        double m_r;
    };

    class Document : public ObjectContainer {
    public:

        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document

    private:
        std::vector<std::unique_ptr<Object>> m_elements;
    };

    template <typename DrawableIterator>
    void DrawPicture(DrawableIterator begin, DrawableIterator end, ObjectContainer& target) {
        for (auto it = begin; it != end; ++it) {
            (*it)->Draw(target);
        }
    }

    template <typename Container>
    void DrawPicture(const Container& container, ObjectContainer& target) {
        using namespace std;
        DrawPicture(begin(container), end(container), target);
    }

}  // namespace svg