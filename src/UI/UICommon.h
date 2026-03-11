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

        const Rect& GetRectangle() const { return _rect; };
        const Color& GetColor() const { return _color; };
        
        void SetXPosition(const float x) { _rect.x = x; };
        float GetXPosition() const { return _rect.x; };
        void SetYPosition(const float y) { _rect.y = y; };
        float GetYPosition() const { return _rect.y; };

        void SetWidth(const float width) { _rect.width = width; };  
        float GetWidth() const { return _rect.width; };
        void SetHeight(const float height) { _rect.height = height; };  
        float GetHeight() const { return _rect.height; };

        const std::string& GetAsset() const { return _asset; };

        void SetRoundness(const float r) { _rect.roundness = r; };
        float GetRoundness() const { return _rect.roundness; };

        void SetSegments(const int s) { _rect.segments = s; };
        int GetSegments() const { return _rect.segments; };

        void SetType(const UIElementType type) { _type = type; };
        UIElementType GetType() const { return _type; };
        
        virtual const std::vector<std::unique_ptr<UIElement>>& GetChildren() const {
            //static so the memory of empty lives for the entirety of the program
            static std::vector<std::unique_ptr<UIElement>> empty;
            return empty;
        }

    protected:
        Rect _rect;
        Color _color;
        UIElementType _type;
        std::string _asset = "";
    };

    class UIText : public UIElement {
    public:
        UIText(const Color& color) : UIElement(color) {}

        void SetText(const std::string& text) { _text = text; };
        const std::string& GetText() const { return _text; };

        void SetFontSize(const int fontSize) { _fontSize = fontSize; };
        int GetFontSize() const { return _fontSize; };
    protected:
        std::string _text;
        int _fontSize;
    };

    class UIContainer : public UIElement {
        protected:
            std::vector<std::unique_ptr<UIElement>> _children;
        public:
            void AddChild(std::unique_ptr<UIElement> elem)
            {
                _children.push_back(std::move(elem));
            }

            const std::vector<std::unique_ptr<UIElement>>& GetChildren() const override {
                return _children;
            }
    };
}
