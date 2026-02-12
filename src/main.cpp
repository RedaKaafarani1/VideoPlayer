#include "decoder/DecoderCore.h"
#include "logger/Logger.h"
#include <libavutil/frame.h>

int main()
{
    //App application{};

    //application.GetAppRender().BeginRender();
    
    DecoderCore decoder;
    decoder.openStream("resources/example_video_720p.mp4");
    gLogger.info("Starting application");

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
    
    auto codec = decoder.getCodecByType(CodecType::VideoCodec);
    if (!codec)
    {
        gLogger.error("Could not get codec");
        return -1;
    }
    codec->get().openCodec();

    AVFrame* frame = nullptr; 
    while ((frame = decoder.decodeNextFrame(codec->get())))
    {     
            gLogger.info("Frame {} ({}) pts {} dts {} key_frame {}", av_get_picture_type_char(frame->pict_type), codec->get().getCodecContext()->frame_num, frame->pts, frame->pkt_dts, (frame->flags & AV_FRAME_FLAG_KEY));
    }

    return 0;
}
