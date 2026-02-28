#pragma once

#include "FFMpegDeleter.h"
#include <memory>

//forward declare, they are included in Stream.cpp
struct AVPacket;
struct AVFrame;

//simple stream class to hold packet and frame data
//don't even know if that's really necessary
class Stream {
    using FramePtr = std::unique_ptr<AVFrame, CustomDeleter>;
public:
    Stream();

    AVPacket* GetPacket() const { return _packet.get(); }
    AVFrame*  GetFrame()  const { return _frame.get();  }
    void ResetInternalState() {
        _packet.reset();
        _frame.reset();
    }
private:
    std::unique_ptr<AVPacket, CustomDeleter> _packet{nullptr};
    FramePtr _frame{nullptr};
};
