#pragma once

#include <memory>
#include <vector>

namespace UI {
    constexpr int TIMELINE_SIZE = 26;

    // This is used to not mix UI with render, i.e. to not import raylib
    struct Color {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };

    // This is used to not mix UI with render, i.e. to not import raylib
    struct Rect {
        float x;
        float y;
        float width;
        float height;
    };

    enum class UIElementShape {
        Rectangle,
        Circle,
        Triangle
    };

    class UIElement {
    public:
        UIElement() = default;
        UIElement (const UIElementShape& shape, const Color& color) : 
        _shape(shape), _color(color) {}

        virtual ~UIElement() = default;
        virtual void AdjustSizePosition(const int width, const int height) noexcept = 0; 

        const Rect& GetRectangle() const { return _rect; };
        const UIElementShape& GetShape() const { return _shape; };
        const Color& GetColor() const { return _color; };
        void SetXPosition(const int x) { _rect.x = x; };
        int GetXPosition() const { return _rect.x; };
        void SetYPosition(const int y) { _rect.y = y; };
        int GetYPosition() const { return _rect.y; };
        float GetWidth() const { return _rect.width; };
        float GetHeight() const { return _rect.height; };

        virtual const std::vector<UIElement*> GetChildren() const {
            std::vector<UIElement*> empty;
            return empty;
        }

    protected:
        Rect _rect;
        UIElementShape _shape;
        Color _color;
    };

    class UIContainer : public UIElement {
        protected:
            std::vector<std::unique_ptr<UIElement>> _children;
        public:
            void AddChild(std::unique_ptr<UIElement> elem)
            {
                _children.push_back(std::move(elem));
            }

            const std::vector<UIElement*> GetChildren() const override {
                std::vector<UIElement*> ret;
                ret.clear();
                for (auto& c : _children) ret.push_back(c.get());
                return ret;
            }
    };

}
