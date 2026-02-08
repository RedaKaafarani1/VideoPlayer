#include "decoder/DecoderCore.h"
#include "logger/Logger.h"

int main()
{
    //App application{};

    //application.GetAppRender().BeginRender();
    
    DecoderCore decoder;
    decoder.openStream("resources/example_video_720p.mp4");
    gLogger.log("Starting application");

    auto* fmt = decoder.getFormatContext();
    std::cout << "Format: " << fmt->iformat->long_name << ", duration: " << fmt->duration << '\n';

    const std::vector<Codec>& codecs = decoder.getCodecs();

    for (const auto& codec : codecs) {
        const auto& cParams = codec.getCodecParams(); 
        if (cParams->codec_type == AVMEDIA_TYPE_VIDEO)
            std::cout << "Video Codec: resolution " << cParams->width
                      << " x " << cParams->height << '\n';
        else if (cParams->codec_type == AVMEDIA_TYPE_AUDIO)
            std::cout << "Audio Codec: " << cParams->ch_layout.nb_channels
                      << " channels, sample rate " << cParams->sample_rate
                      << '\n';
        std::cout << "Codec "    << codec.getCodec()->long_name
                  << " ID "      << codec.getCodec()->id
                  << " bitrate " << cParams->bit_rate
                  << '\n';
    }
    
    const Stream& stream = decoder.getStream();
    //maybe use a map instead of vector for easier access?
    const Codec& videoCodec = *std::find_if(codecs.begin(), codecs.end(), [](const Codec& codec){ return codec.getCodecType() == CodecType::VideoCodec; });
    const unsigned int videoCodecIdx = videoCodec.getCodecIndex();

    int err = videoCodec.openCodec();
    if (err < 0)
        std::runtime_error("Error opening codec!");

while (stream.readFrame(decoder.getFormatContext(), videoCodecIdx) >= 0)
{
        AVFrame* frame = nullptr;
        if ((frame = stream.decodeFrame(videoCodec.getCodecContext())) != nullptr)
        {
            std::cout << "Frame " << av_get_picture_type_char(frame->pict_type)<< " ("
                      << videoCodec.getCodecContext()->frame_num << ") "
                      << "pts " << frame->pts << " dts " << frame->pkt_dts
                      << " key_frame " << (frame->flags & AV_FRAME_FLAG_KEY)
                      << '\n';
        }

        av_packet_unref(stream.getPacket());
}


    return 0;
}
