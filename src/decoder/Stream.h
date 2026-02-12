#pragma once

#include "../Common.h"

//simple stream class to hold packet and frame data
class Stream {
public:
    Stream();

    AVPacket* getPacket() const { return _packet.get(); }
    AVFrame*  getFrame()  const { return _frame.get();  }
private:
    std::unique_ptr<AVPacket, CustomDeleter> _packet{nullptr};
    std::unique_ptr<AVFrame, CustomDeleter> _frame{nullptr};
    
};
