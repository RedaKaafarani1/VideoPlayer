#include "DecoderCore.h"
#include "Common.h"
#include <libavcodec/avcodec.h>
#include <libavutil/rational.h>
#include <mutex>
#include <stop_token>

int DecoderCore::openStream(const std::string& filename)
{
    gLogger.info("Opening video stream with file name {}", filename);
    AVFormatContext* tmp = nullptr;
    if (avformat_open_input(&tmp, filename.c_str(), nullptr, nullptr) < 0)
    {
        return -1;
    }
    avformat_find_stream_info(tmp, nullptr);
    FormatContextPtr.reset(tmp);

    setCodecParameters();
    return 0;
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
        {
            if (avcodec_receive_frame(codecCtx, frame)==0)
            {
                return frame;
            }

            {
                std::scoped_lock lock(queueMutex);
                doneDecoding = true;
            }
            queueCondition.notify_all();
            gLogger.info("Done decoding");
            return nullptr;
        }
        err = avcodec_receive_frame(codecCtx, frame);
        if (err == AVERROR(EAGAIN)) {
            // need more packets
            continue;
        } else if (err == AVERROR_EOF) {
            //no more output frames
            {
                std::scoped_lock lock(queueMutex);
                doneDecoding = true;
            }
            queueCondition.notify_all();
            gLogger.info("Done decoding");
            return nullptr;
        } else if (err < 0) {
            gLogger.error("Error receiving frame for codec {}", static_cast<int>(codecCtx->codec_id));
            return nullptr;
        }
        return frame;
    }
}

AVFrame* DecoderCore::convertFrameToRGB(const AVFrame* const source)
{
    if (!source)
        return nullptr;

    // assume this is stored in a container that takes ownership of the data
    // and that will free it
    AVFrame* rgbFrame = av_frame_alloc();
    if (!rgbFrame)
    {
        gLogger.error("Could not allocate frame for conversion");
        return nullptr;
    }

    rgbFrame->format = AV_PIX_FMT_RGBA;
    rgbFrame->width = source->width;
    rgbFrame->height = source->height;

    if (av_frame_get_buffer(rgbFrame, 0) < 0)
    {
        gLogger.error("Could not allocate frame buffer data");
        av_frame_free(&rgbFrame);
        return nullptr;
    }

    swsCtx = sws_getCachedContext(
            swsCtx,
            source->width,
            source->height,
            static_cast<AVPixelFormat>(source->format),
            rgbFrame->width,
            rgbFrame->height,
            static_cast<AVPixelFormat>(rgbFrame->format),
            SWS_BILINEAR,
            nullptr,
            nullptr,
            nullptr
            );
    
    sws_scale(swsCtx, source->data, source->linesize, 0, source->height, rgbFrame->data, rgbFrame->linesize);

    return rgbFrame;
}

void DecoderCore::decodeVideoStream(std::stop_token stopToken)
{
    if (!doneDecoding)
    {
        AVFrame* frame = nullptr;
        auto codec = getCodecByType(CodecType::VideoCodec);
        if (!codec)
        {
            gLogger.error("Could not get codec");
            return;
        }
        codec->get().openCodec(); 

        while ((frame = decodeNextFrame(codec->get())) && !stopToken.stop_requested())
        {
            gLogger.info("Frame {} ({}) pts {} dts {} key_frame {}", av_get_picture_type_char(frame->pict_type), codec->get().getCodecContext()->frame_num, frame->pts, frame->pkt_dts, (frame->flags & AV_FRAME_FLAG_KEY));
            AVFrame* rgbFrame = convertFrameToRGB(frame);
            if (rgbFrame)
            {
                {
                    std::scoped_lock lock(queueMutex);
                    decodedRGBFrames.emplace(
                            std::unique_ptr<AVFrame, CustomDeleter>(rgbFrame)
                    ); 
                }
                queueCondition.notify_one();
            }
        }
        queueCondition.notify_all();

        return; 
    }
    // we didn't decode anything
    gLogger.warn("Stream already decoded!");
    return;
}
    
std::unique_ptr<AVFrame, CustomDeleter> DecoderCore::getFrame()
{
    std::unique_lock lock(queueMutex);
    queueCondition.wait(lock, [&](){return !decodedRGBFrames.empty() || doneDecoding; });
    if (!decodedRGBFrames.empty())
    {
        auto frame = std::move(decodedRGBFrames.front());
        decodedRGBFrames.pop();
        return frame;
    }
    return nullptr;
}

double DecoderCore::getVideoTimeBase()
{
    auto codec = getCodecByType(CodecType::VideoCodec);
    if (!codec)
    {
        gLogger.error("Could not get codec of type CodecType::VideoCodec");
        return -1.0;
    }

    return av_q2d(FormatContextPtr.get()->streams[codec->get().getCodecIndex()]->time_base);
}
