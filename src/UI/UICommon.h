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
        float roundness = 0.0f;
        int segments = 0;
    };

    enum class UIElementType {
        Container,
        Element,
        Text
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

        const Rect& GetRectangle() const noexcept { return _rect; };
        const Color& GetColor() const noexcept { return _color; };
        
        void SetXPosition(const float x) noexcept { _rect.x = x; };
        float GetXPosition() const noexcept { return _rect.x; };
        void SetYPosition(const float y) noexcept { _rect.y = y; };
        float GetYPosition() const noexcept { return _rect.y; };

        void SetWidth(const float width) noexcept { _rect.width = width; };  
        float GetWidth() const noexcept { return _rect.width; };
        void SetHeight(const float height) noexcept { _rect.height = height; };  
        float GetHeight() const noexcept { return _rect.height; };

        const std::string& GetAsset() const noexcept { return _asset; };

        void SetRoundness(const float r) noexcept { _rect.roundness = r; };
        float GetRoundness() const noexcept { return _rect.roundness; };

        void SetSegments(const int s) noexcept { _rect.segments = s; };
        int GetSegments() const noexcept { return _rect.segments; };

        void SetType(const UIElementType type) noexcept { _type = type; };
        UIElementType GetType() const noexcept { return _type; };

        void SetScale(const float scale) noexcept { _scale = scale; };
        float GetScale() const noexcept { return _scale; };

    protected:
        Rect _rect;
        Color _color;
        float _scale = 1.0f;
        UIElementType _type;
        std::string _asset = "";
    };

    class UIText : public UIElement {
    public:
        UIText(const Color& color) : UIElement(color) {}

        void SetText(const std::string& text) noexcept { _text = text; };
        const std::string& GetText() const noexcept { return _text; };

        void SetFontSize(const int fontSize) noexcept { _fontSize = fontSize; };
        int GetFontSize() const noexcept { return _fontSize; };
    protected:
        std::string _text;
        int _fontSize;
    };

    class UIContainer : public UIElement {
        protected:
            std::vector<std::unique_ptr<UIElement>> _children;
        public:
            void AddChild(std::unique_ptr<UIElement> elem) noexcept
            {
                _children.push_back(std::move(elem));
            }

            const std::vector<std::unique_ptr<UIElement>>& GetChildren() const noexcept 
            {
                return _children;
            }
    };
}
