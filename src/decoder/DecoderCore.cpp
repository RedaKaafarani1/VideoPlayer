#include "DecoderCore.h"
#include "Common.h"

void DecoderCore::openStream(std::string filename)
{
    gLogger.log("Opening video stream with file name {}", filename);
    AVFormatContext* tmp = nullptr;
    if (avformat_open_input(&tmp, filename.c_str(), nullptr, nullptr) < 0)
    {
        gLogger.log("Could not open video file {}", filename);
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
