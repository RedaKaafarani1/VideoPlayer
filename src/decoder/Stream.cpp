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

int Stream::readFrame(AVFormatContext* fmtCtx, const unsigned int codecIdx) const 
{
    int ret = 0;
    do {
            ret = av_read_frame(fmtCtx, _packet.get());
    } while (_packet->stream_index != static_cast<int>(codecIdx));
    
    return ret;
}

AVFrame* Stream::decodeFrame(AVCodecContext* codecCtx) const
{
    int err = avcodec_send_packet(codecCtx, _packet.get());
    if (err < 0)
        std::runtime_error("Error sending packet!");

    while (true) {
        err = avcodec_receive_frame(codecCtx, _frame.get());
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
            // need more packets
            return nullptr;
        } else if (err < 0) {
            std::cerr<<err<<'\n';
            throw std::runtime_error("Error receiving frame!");
        }
        // valid frame
        return _frame.get();
    }
}
