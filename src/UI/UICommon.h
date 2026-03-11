#pragma once

#include <memory>
#include <vector>

namespace UI {
    constexpr int TIMELINE_SIZE = 30;
    constexpr int CONTROLS_WIDTH = 180;

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

    class UIElement {
    public:
        UIElement() = default;
        UIElement (const Color& color) : 
        _color(color) {}

        UIElement (const Color& color, const std::string& asset) : 
        _color(color), _asset(asset) {}

        virtual ~UIElement() = default;
        virtual void AdjustSizePosition(const int width, const int height) noexcept = 0; 

        const Rect& GetRectangle() const { return _rect; };
        const Color& GetColor() const { return _color; };
        void SetXPosition(const int x) { _rect.x = x; };
        float GetXPosition() const { return _rect.x; };
        void SetYPosition(const int y) { _rect.y = y; };
        float GetYPosition() const { return _rect.y; };
        float GetWidth() const { return _rect.width; };
        float GetHeight() const { return _rect.height; };
        const std::string& GetAsset() const { return _asset; };

        virtual const std::vector<UIElement*> GetChildren() const {
            std::vector<UIElement*> empty;
            return empty;
        }

    protected:
        Rect _rect;
        Color _color;
        std::string _asset = "";
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
                for (auto& c : _children) ret.push_back(c.get());
                return ret;
            }
    };

}
