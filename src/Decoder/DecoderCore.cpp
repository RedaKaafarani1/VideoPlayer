#include "DecoderCore.h"
#include "../Logger/GLogger.h"
#include <libavutil/avutil.h>
#include <libavutil/mathematics.h>
#include <libavutil/rational.h>

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
    stream = Stream{};

    // open video codec
    const auto& codec = codecsMap[CodecType::VideoCodec]; 
    int ret = codec.OpenCodec(); 
    if (ret < 0)
    {
        gLogger.error("Could not open codec of type CodecType::VideoCodec");
        _state.store(DecoderState::DecoderFailed, std::memory_order_release);
        return -1;
    }
    computeFrameDuration(codec);
    doneDecoding = false;
    return 0;
}

void DecoderCore::setCodecParameters()
{
    auto fmt = FormatContextPtr.get();

    std::vector<Codec> codecs;
    if (!codecsMap.empty())
        std::runtime_error("map not empty");
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
                std::scoped_lock lock(_queueMutex);
                doneDecoding = true;
            }
            _queueCondition.notify_all();
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
                std::scoped_lock lock(_queueMutex);
                doneDecoding = true;
            }
            _queueCondition.notify_all();
            gLogger.info("Done decoding");
            return nullptr;
        } else if (err < 0) {
            gLogger.error("Error receiving frame for codec {}", static_cast<int>(codecCtx->codec_id));
            return nullptr;
        }

        gLogger.debug("Frame {} ({}) pts {} dts {} key_frame {}", av_get_picture_type_char(frame->pict_type), codecCtx->frame_num, frame->pts, frame->pkt_dts, (frame->flags & AV_FRAME_FLAG_KEY));
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
    if (rgbFrame->pts == AV_NOPTS_VALUE)
        rgbFrame->pts = source->pkt_dts;
    if (rgbFrame->pts == AV_NOPTS_VALUE)
        rgbFrame->pts = source->best_effort_timestamp;;
    // use our internal frame duration calculation to estimate pts
    if (rgbFrame->pts == AV_NOPTS_VALUE)
        rgbFrame->pts = _lastPTS + _frameDuration;
    //store last valid pts for next pts calculation when needed
    _lastPTS = rgbFrame->pts;

    _swsCtx = sws_getCachedContext(
            _swsCtx,
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
    
    sws_scale(_swsCtx, source->data, source->linesize, 0, source->height, rgbFrame->data, rgbFrame->linesize);

    return rgbFrame;
}

void DecoderCore::runDecoderLoop(std::stop_token stopToken)
{
    AVFrame* frame = nullptr;
    while (!stopToken.stop_requested())
    {
        DecoderCommand command;
        {
            std::scoped_lock lock(_commandMutex);
            if (!_commandQueue.empty())
            {
                command = _commandQueue.front();
                _commandQueue.pop();
            }
        }

        switch (command.type)
        {
            case DecoderCommandType::None: break;
            // we only stop when quitting the app, the destructor will handle cleanup
            case DecoderCommandType::Stop: return;
            case DecoderCommandType::Wait:
                gLogger.info("Decoder entering wait state");
                _state.store(DecoderState::DecoderWaiting, std::memory_order_release);
                {
                    std::unique_lock lock(_commandMutex);
                    _commandCondition.wait(lock, [&](){
                            return !_commandQueue.empty() || 
                                    stopToken.stop_requested();
                    });
                    continue;
                }
                break;
            case DecoderCommandType::Seek:
                handleSeek(command.seekPTS);
                break;
            case DecoderCommandType::DecodeVideo:
                handleNewVideoFile(command.videoFilename);
                _state.store(DecoderState::DecoderReady, std::memory_order_release);
                break;
        }
       
        //set codec here, since handleNewVideoFile invalidates current
        //codecs map
        auto& codec = codecsMap.at(CodecType::VideoCodec);

        frame = decodeNextFrame(codec);
        if (!frame)
        {
            //doneDecoding should already be set if that's the case
            continue;
        }
        AVFrame* rgbFrame = convertFrameToRGB(frame);
        if (rgbFrame)
        {
            {
                std::scoped_lock lock(_queueMutex);
                //set this here when the mutex is locked, we will be using it in the app thread
                // _firstFramePTS is not atomic since we synchronize on _queueMutex
                if (_firstFramePTS == AV_NOPTS_VALUE)
                {
                    _firstFramePTS = frame->pts;
                    if (_firstFramePTS == AV_NOPTS_VALUE)
                        _firstFramePTS = frame->pkt_dts;
                    if (_firstFramePTS == AV_NOPTS_VALUE)
                        _firstFramePTS = frame->best_effort_timestamp; // fall back in case pts not available
                }
                _decodedRGBFrames.emplace(
                        std::unique_ptr<AVFrame, CustomDeleter>(rgbFrame)
                ); 
            }
            _queueCondition.notify_one();
        }
    }
    // notify all conditions
    _queueCondition.notify_all();
    _commandCondition.notify_all();
    return; 
}
    
std::unique_ptr<AVFrame, CustomDeleter> DecoderCore::getFrame()
{
    if (_state.load(std::memory_order_acquire) == DecoderState::DecoderLoading)
        return nullptr;
    std::unique_lock lock(_queueMutex);
    //wait for a frame, done decoding or stop request
    _queueCondition.wait(lock, [&](){return !_decodedRGBFrames.empty() || doneDecoding; });
    if (!_decodedRGBFrames.empty())
    {
        auto frame = std::move(_decodedRGBFrames.front());
        _decodedRGBFrames.pop();
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

double DecoderCore::getVideoDurationSeconds() const
{
    int64_t duration = FormatContextPtr.get()->duration;
    return (duration / static_cast<double>(AV_TIME_BASE));
}

void DecoderCore::handleNewVideoFile(const std::string& filename)
{
    doneDecoding = true;
    gLogger.info("Received new video file {}, resetting", filename);  
    // For both lines below, the custom deleter will handle
    // cleaning ffmpeg related stuff
    FormatContextPtr.reset();
    codecsMap.clear();
    _firstFramePTS = AV_NOPTS_VALUE;
    //empty current decoded frames
    {
        std::scoped_lock lock(_queueMutex);
        while (!_decodedRGBFrames.empty())
            _decodedRGBFrames.pop();
    }
    _queueCondition.notify_all();
    int ret = openStream(filename);
    if (ret != 0)
        gLogger.error("Failed to open file {} for decoding", filename);
    _commandCondition.notify_one();
}

void DecoderCore::pushCommand(const DecoderCommand& decoderCommand)
{
    {
        std::scoped_lock lock(_commandMutex);
        _commandQueue.push(decoderCommand);
    }
    _commandCondition.notify_one();
}

void DecoderCore::handleSeek(const int seekPTS) noexcept
{
    {
        std::scoped_lock lock(_queueMutex);
        while (!_decodedRGBFrames.empty())
            _decodedRGBFrames.pop();
    }

    auto& codec = codecsMap.at(CodecType::VideoCodec);

    gLogger.info("Seeking to position {}", seekPTS);
    av_seek_frame(FormatContextPtr.get(), codec.GetCodecIndex(), seekPTS, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codec.GetCodecContext());
    doneDecoding = false;
    _queueCondition.notify_all();
    _lastPTS = 0;
    _state.store(DecoderState::DecoderDoneSeeking, std::memory_order_release);
}

void DecoderCore::computeFrameDuration(const Codec& codec) noexcept
{
    const auto& stream = FormatContextPtr.get()->streams[codec.GetCodecIndex()];
    AVRational timeBase = stream->time_base;
    AVRational frameRate = stream->avg_frame_rate;

    _frameDuration = av_rescale_q(1, av_inv_q(frameRate), timeBase);
}
