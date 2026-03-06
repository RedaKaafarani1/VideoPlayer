#include "Seek.h"
#include "UI/UICommon.h"

namespace UI {

    void Seek::AdjustSizePosition(const int width, const int height) noexcept {
        _rect.width = width; 
        _rect.height = height;
        //x and y will be in parent Timeline
    }

}
