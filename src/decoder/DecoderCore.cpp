#include "DecoderCore.h"
#include "Common.h"
#include <libavcodec/avcodec.h>

void DecoderCore::openStream(std::string filename)
{
    gLogger.info("Opening video stream with file name {}", filename);
    AVFormatContext* tmp = nullptr;
    if (avformat_open_input(&tmp, filename.c_str(), nullptr, nullptr) < 0)
    {
        gLogger.error("Could not open video file {}", filename);
        throw std::runtime_error("Could not open video file");
    }
    avformat_find_stream_info(tmp, nullptr);
    FormatContextPtr.reset(tmp);

    setCodecParameters();
}

void DecoderCore::setCodecParameters()
{
    auto fmt = FormatContextPtr.get();

    for (unsigned int streamIdx = 0; streamIdx  < fmt->nb_streams; streamIdx++)
    {
        codecs.emplace_back(Codec(*FormatContextPtr.get(), streamIdx));
    }
}

std::optional<std::reference_wrapper<const Codec>>
DecoderCore::getCodecByType(CodecType codecType) const
{
    for (const auto& codec : codecs)
        if (codec.getCodecType() == codecType)
            return codec;

    return std::nullopt;
}

int DecoderCore::readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const 
{
    int ret = 0;

    while (true)
    {
        ret = av_read_frame(FormatContextPtr.get(), &packet);

        if (ret < 0) break;
        if (packet.stream_index == static_cast<int>(codecIdx)) break;

        av_packet_unref(&packet);
    }

    if (ret == AVERROR_EOF)
    {
        avcodec_send_packet(&codecCtx, nullptr);
        return -1;
    } 
    else if (ret < 0)
    {
        gLogger.error("Error reading frame data");
        return -1;
    } 
    else 
    {
        ret = avcodec_send_packet(&codecCtx, &packet);
        av_packet_unref(&packet);
        if (ret < 0)
        {
            gLogger.error("Error {} sending packet for codec {}", ret, static_cast<int>(codecCtx.codec_id));
            return -1;
        }
    }
    return ret;
}

AVFrame* DecoderCore::decodeNextFrame(const Codec& codec) 
{
    //prevent further calls to this function if we are done decoding
    if (doneDecoding)
        return nullptr;
    
    AVPacket* packet = stream.getPacket();
    AVFrame* frame = stream.getFrame();
    AVCodecContext* codecCtx = codec.getCodecContext();  

    unsigned int codecIdx = codec.getCodecIndex();

    int err;

    while (true) {
        err = readFrame(*packet, codecIdx, *codecCtx);
        if (err < 0)
            return nullptr; //we already logged, return nullptr;

        err = avcodec_receive_frame(codecCtx, frame);
        if (err == AVERROR(EAGAIN)) {
            // need more packets
            continue;
        } else if (err == AVERROR_EOF) {
            //no more output frames
            doneDecoding = true;
            return nullptr;
        } else if (err < 0) {
            gLogger.error("Error receiving frame for codec {}", static_cast<int>(codecCtx->codec_id));
            return nullptr;
        }
        // valid frame
        return frame;
    }
}
