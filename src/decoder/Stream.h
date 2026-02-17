#pragma once

#include "../Common.h"
#include <memory>

//simple stream class to hold packet and frame data
class Stream {
    using FramePtr = std::unique_ptr<AVFrame, CustomDeleter>;
public:
    Stream();

    AVPacket* getPacket() const { return _packet.get(); }
    AVFrame*  getFrame()  const { return _frame.get();  }
    std::vector<FramePtr>& getDecodedFrames () { return _decodedFrames; }
    const std::vector<FramePtr>& getDecodedFrames () const { return _decodedFrames; }
private:
    std::unique_ptr<AVPacket, CustomDeleter> _packet{nullptr};
    FramePtr _frame{nullptr};
    std::vector<FramePtr> _decodedFrames;
};
