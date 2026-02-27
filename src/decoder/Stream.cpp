#include "Stream.h"
#include "../logger/GLogger.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

Stream::Stream()
{
    AVPacket* packet = av_packet_alloc();
    if (!packet)
    {
        gLogger.error("Could not allocate packet!");
        std::runtime_error("Could not allocate packet");
    }
    _packet.reset(packet);
    

    AVFrame* frame = av_frame_alloc();
    if (!frame)
    {
        gLogger.error("Could not allocate frame!");
        std::runtime_error("Could not allocate frame");
    }
    _frame.reset(frame);
}
