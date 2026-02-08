#pragma once

#include "../Common.h"

class Stream {
public:
    Stream();
    int readFrame(AVFormatContext* fmtCtx, const unsigned int codecIdx) const;
    AVFrame* decodeFrame(AVCodecContext* codecCtx) const;


    AVPacket* getPacket() const { return _packet.get(); }
    AVFrame*  getFrame()  const { return _frame.get();  }
private:
    std::unique_ptr<AVPacket, CustomDeleter> _packet{nullptr};
    std::unique_ptr<AVFrame, CustomDeleter> _frame{nullptr};
    
};
