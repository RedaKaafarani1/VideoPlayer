#include "Stream.h"

Stream::Stream()
{
    AVPacket* packet = av_packet_alloc();
    if (!packet)
        std::runtime_error("Could not allocate packet");
    _packet.reset(packet);
    

    AVFrame* frame = av_frame_alloc();
    if (!frame)
        std::runtime_error("Could not allocate frame");
    _frame.reset(frame);
}
