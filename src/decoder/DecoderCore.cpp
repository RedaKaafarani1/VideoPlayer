#include "DecoderCore.h"
#include "Common.h"
#include <stdexcept>

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

    // open video codec
    const auto& codec = codecsMap[CodecType::VideoCodec]; 
    int ret = codec.OpenCodec(); 
    if (ret < 0)
    {
        gLogger.error("Could not open codec of type CodecType::VideoCodec");
        _state.store(PlayerState::DecoderFailed, std::memory_order_release);
        return -1;
    }
    return 0;
}

void DecoderCore::setCodecParameters()
{
    auto fmt = FormatContextPtr.get();

    std::vector<Codec> codecs;
    for (unsigned int streamIdx = 0; streamIdx  < fmt->nb_streams; streamIdx++)
    {
        //Codec is fully initialized in the constructor
        codecs.emplace_back(Codec(*FormatContextPtr.get(), streamIdx));
        codecsMap[codecs[streamIdx].GetCodecType()] = std::move(codecs[streamIdx]); 
    }
}

std::optional<std::reference_wrapper<const Codec>>
DecoderCore::getCodecByType(CodecType codecType) const
{
    std::optional<std::reference_wrapper<const Codec>> ret = std::nullopt;
    try {
        ret = codecsMap.at(codecType);
    } catch (const std::out_of_range& e)
    {
        gLogger.error("Codec of type VideoCodec not found: {}", e.what());
        ret = std::nullopt;
    }
    return ret;
}

int DecoderCore::readFrame(AVPacket& packet, const unsigned int codecIdx, AVCodecContext& codecCtx) const 
{
    int ret = 0;

    while (true)
    {
        ret = av_read_frame(FormatContextPtr.get(), &packet);

        // ret < 0 will be handled outside the while block 
        if (ret < 0) break;
        // if we have a packet that belongs to our stream, we break
        // to handle it in the else block outside
        if (packet.stream_index == static_cast<int>(codecIdx)) break;

        // otherwise we unref the packet
        av_packet_unref(&packet);
    }

    if (ret == AVERROR_EOF)
    {
        //flush the decoder
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
        // send the packet so we can receive a frame later
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
    // prevent further calls to this function if we are done decoding
    // just in case
    if (doneDecoding)
        return nullptr;
    
    AVPacket* packet = stream.GetPacket();
    AVFrame* frame = stream.GetFrame();
    AVCodecContext* codecCtx = codec.GetCodecContext();  

    unsigned int codecIdx = codec.GetCodecIndex();
    
    int err;

    while (true) {
        err = readFrame(*packet, codecIdx, *codecCtx);
        if (err < 0)
        {
            // if we are in this block, it doesn't mean we actually have an error so we try to receive buffered frames and return otherwise we finalize decoding and we return nullptr
            if (avcodec_receive_frame(codecCtx, frame)==0)
            {
                return frame;
            }

            //getFrame reads this, acquire the mutex before writing
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

        gLogger.info("Frame {} ({}) pts {} dts {} key_frame {}", av_get_picture_type_char(frame->pict_type), codecCtx->frame_num, frame->pts, frame->pkt_dts, (frame->flags & AV_FRAME_FLAG_KEY));
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

    //copy pts which we will use for timing
    rgbFrame->pts = source->pts;
    rgbFrame->best_effort_timestamp = source->best_effort_timestamp;

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
    const auto& codec = codecsMap.at(CodecType::VideoCodec);
    AVFrame* frame = nullptr;
    while (!stopToken.stop_requested())
    {
        if (seekRequested.load(std::memory_order_acquire))
        {
            {
                std::scoped_lock lock(queueMutex);
                while (!decodedRGBFrames.empty())
                    decodedRGBFrames.pop();
            }

            gLogger.info("Seeking to position {}", seekPTS);
            av_seek_frame(FormatContextPtr.get(), codec.GetCodecIndex(), seekPTS, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(codec.GetCodecContext());
            doneDecoding = false;
            seekRequested.store(false, std::memory_order_release);
            queueCondition.notify_all();
        }

        if (doneDecoding)
        {
            std::unique_lock lock(controlMutex);
            controlCondition.wait(lock, [&](){ 
                return seekRequested.load() || stopToken.stop_requested() || stopRequested.load();
            });
            continue;
        } 

        frame = decodeNextFrame(codec);
        if (!frame)
        {
            //doneDecoding should already be set if that's the case
            continue;
        }
        AVFrame* rgbFrame = convertFrameToRGB(frame);
        //set this here when the mutex is locked, we will be using it in the app thread
        // _firstFramePTS is not atomic since we synchronize on queueMutex
        if (_firstFramePTS == AV_NOPTS_VALUE)
        {
            _firstFramePTS = frame->pts;
            if (_firstFramePTS == AV_NOPTS_VALUE)
                _firstFramePTS = frame->best_effort_timestamp; // fall back in case pts not available
        }
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
    // notify all conditions
    queueCondition.notify_all();
    controlCondition.notify_all();
    return; 
}
    
std::unique_ptr<AVFrame, CustomDeleter> DecoderCore::getFrame()
{
    std::unique_lock lock(queueMutex);
    //wait for a frame, done decoding or stop request
    queueCondition.wait(lock, [&](){return !decodedRGBFrames.empty() || doneDecoding || stopRequested.load(); });
    if (!decodedRGBFrames.empty())
    {
        auto frame = std::move(decodedRGBFrames.front());
        decodedRGBFrames.pop();
        return frame;
    }
    // no more frames to return, send nullptr to signal end
    return nullptr;
}

double DecoderCore::getVideoTimeBase() const
{
    const auto& codec = codecsMap.at(CodecType::VideoCodec); 
    return av_q2d(FormatContextPtr.get()->streams[codec.GetCodecIndex()]->time_base);
}

void DecoderCore::seekFrame(int64_t pts) noexcept
{
    {
        std::scoped_lock lock(controlMutex);
        seekPTS = pts;
        seekRequested.store(true, std::memory_order_release);
    }
    controlCondition.notify_one();
}

/*void DecoderCore::changeVideoFile(const std::string& filename)
{
    {
        std::scoped_lock lock(controlMutex);
        decoderCommand = DecoderCommand::ChangeVideo;
    }
    controlCondition.notify_one();
}*/

void DecoderCore::pushCommand(const DecoderCommand& decoderCommand)
{
    {
        std::scoped_lock lock(commandMutex);
        commandQueue.push(decoderCommand);
    }
    commandCondition.notify_one();
}
