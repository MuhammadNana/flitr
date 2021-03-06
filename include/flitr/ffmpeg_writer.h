/* Framework for Live Image Transformation (FLITr)
 * Copyright (c) 2010 CSIR
 * 
 * This file is part of FLITr.
 *
 * FLITr is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * FLITr is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with FLITr. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef FFMPEG_WRITER_H
#define FFMPEG_WRITER_H 1

#ifndef FLITR_USE_SWSCALE
#define FLITR_USE_SWSCALE 1
#endif

#include <flitr/ffmpeg_utils.h>
#include <flitr/image.h>
#include <flitr/log_message.h>
#include <flitr/stats_collector.h>

//#if !defined INT64_C
//# define INT64_C(c) c ## LL
//#endif

extern "C" {
#if defined FLITR_USE_SWSCALE
# include <libavformat/avformat.h>
# include <libswscale/swscale.h>
# include <libavutil/mathematics.h>
# include <libavcodec/avcodec.h>	
#ifdef WIN32
#define av_pix_fmt_descriptors av_pix_fmt_descriptors_foo
#endif

#include <libavutil/pixdesc.h>
#include <libavutil/opt.h>

#ifdef WIN32
#undef av_pix_fmt_descriptors
    extern __declspec(dllimport) const AVPixFmtDescriptor av_pix_fmt_descriptors[];
#endif
#else
# include <libavformat/avformat.h>
# include <libavutil/mathematics.h>
# include <libavcodec/avcodec.h>
#ifdef WIN32
#define av_pix_fmt_descriptors av_pix_fmt_descriptors_foo
#endif

#include <libavutil/pixdesc.h>

#ifdef WIN32
#undef av_pix_fmt_descriptors
    extern __declspec(dllimport) const AVPixFmtDescriptor av_pix_fmt_descriptors[];
#endif
#endif
}

#undef PixelFormat

#include <iostream>
#include <sstream>
#include <map>

namespace flitr {

/// Class thrown when video errors occur.
struct FFmpegWriterException {
    FFmpegWriterException() {}
};

enum VideoContainer {
    FLITR_ANY_CONTAINER = 0,
    FLITR_AVI_CONTAINER,
    FLITR_MKV_CONTAINER,  //Will cause the ffmpeg consumers to use .mkv extension.
    FLITR_RTSP_CONTAINER
};

enum VideoCodec {//NB: Still need to test all the codecs and only include the ones that work.
    FLITR_NONE_CODEC = AV_CODEC_ID_NONE,
    FLITR_RAWVIDEO_CODEC = AV_CODEC_ID_RAWVIDEO,
    FLITR_MPEG1VIDEO_CODEC = AV_CODEC_ID_MPEG1VIDEO,
    FLITR_MPEG2VIDEO_CODEC = AV_CODEC_ID_MPEG2VIDEO,
    FLITR_MSMPEG4V3_CODEC = AV_CODEC_ID_MSMPEG4V3,
    FLITR_MJPEG_CODEC = AV_CODEC_ID_MJPEG,
    FLITR_MJPEGB_CODEC = AV_CODEC_ID_MJPEGB,
    FLITR_LJPEG_CODEC = AV_CODEC_ID_LJPEG,
    FLITR_JPEGLS_CODEC = AV_CODEC_ID_JPEGLS,
    FLITR_FFV1_CODEC = AV_CODEC_ID_FFV1,
    FLITR_MPEG4_CODEC = AV_CODEC_ID_MPEG4,
    FLITR_HUFFYUV_CODEC = AV_CODEC_ID_HUFFYUV,
    FLITR_THEORA_CODEC = AV_CODEC_ID_THEORA,
    FLITR_DIRAC_CODEC = AV_CODEC_ID_DIRAC,
    FLITR_FFVHUFF_CODEC = AV_CODEC_ID_FFVHUFF
};

/** \brief The FFmpegWriter class.
 *
 * Write data to a file or RTSP stream.
 *
 * After the writer is constructed, the writeVideoFrame() function must be called manually
 * to write the data to the output.
 *
 * \section RTSP Streaming.
 * The FFmpeg Writer supports writing video out over the network
 * as a RTSP client. For this the \a filename must be an RTSP URL in the
 * following format: <tt>rtsp://hostname[:port]/path</tt>.
 * To stream to localhost over port 8554 with path 'mystream' the following
 * \a filename is expected:
 * \code
 * rtsp://127.0.0.1:8554/mystream.sdp
 * \endcode
 *
 * When this is done this writer will be have as an RTSP client. This requires
 * that an RTSP server must be running when the writer is created, otherwise
 * creation will fail with an "Connection Refused" message generated by FFmpeg.
 * The <tt>ffplay</tt> application can be used as a test FFmpeg server using the
 * following command given the above \a filename:
 * \code
 * $ ffplay -rtsp_flags listen -i rtsp://127.0.0.1:8554/mystream.sdp
 * \endcode
 *
 * When streaming as an RTSP client and the connection to the server is lost, a
 * new instance of this class must be created to re-establish streaming to the
 * server. Currently the is no functionality in place to recover from a lost
 * connection.
 *
 * The following example can be used to create a RTSP Writer:
 * \code
 * flitr::FFmpegWriter* writer = new flitr::FFmpegWriter("rtsp://127.0.0.1:8554/mystream.sdp"
 *                                                       , inputFormat // From producer
 *                                                       , 20 // Frame rate
 *                                                       , flitr::FLITR_RTSP_CONTAINER);
 * \endcode
 */
class FLITR_EXPORT FFmpegWriter {
public:
    /**
     * Default constructor for the FFmpegWriter.
     *
     * This constructor allows the user to construct the writer and then set
     * header dictionary options that should be used for the stream using the
     * setHeaderDictionaryOptions() function and set the Codec Context
     * options using the setCodecContextPrivateOptions() function before
     * opening the file for writing openVideo() function.
     */
    FFmpegWriter() NOEXCEPT;
    /**
     * Construct the FFmpegWriter and open the file for writing.
     *
     * If the file can not be opened a FFmpegWriterException() is thrown.
     *
     * If custom header dictionary and codec context options should be set
     * for the file prior to opening, it is recommended to use the default
     * constructor and then the openFile() function. It will also call the
     * getCodecContextPrivateOptions() to set up the Codec Context before
     * writing the file.
     *
     * \param[in] filename Name of the file or URL of the RTSP stream server.
     * \param[in] image_format Input image format of the data that will be written
     *          using writeVideoFrame() on this writer.
     * \param[in] frame_rate Frame rate that the data must be written with.
     * \param[in] container Video Container format that must be used. This must
     *          be set to flitr::FLITR_RTSP_CONTAINER if RTSP streaming is required.
     * \param[in] codec The codec that must be used. For RTSP it is recommended to use
     *          flitr::FLITR_NONE_CODEC so that the codec will be chosen by the application.
     * \param[in] bit_rate Bit rate that must be used for writing the data.
     */
    FFmpegWriter(std::string filename, const ImageFormat& image_format,
                 const uint32_t frame_rate=FLITR_DEFAULT_VIDEO_FRAME_RATE,
                 VideoContainer container=FLITR_ANY_CONTAINER,
                 VideoCodec codec=FLITR_RAWVIDEO_CODEC,
                 int32_t bit_rate=-1);

    ~FFmpegWriter();

    /**
     * Open the file for writing.
     *
     * This function will call getHeaderDictionaryOptions() to get additional
     * FFmpeg dictionary options to pass to the open function.
     *
     * \param[in] filename Name of the file or URL of the RTSP stream server.
     * \param[in] image_format Input image format of the data that will be written
     *          using writeVideoFrame() on this writer.
     * \param[in] frame_rate Frame rate that the data must be written with.
     * \param[in] container Video Container format that must be used. This must
     *          be set to flitr::FLITR_RTSP_CONTAINER if RTSP streaming is required.
     * \param[in] codec The codec that must be used. For RTSP it is recommended to use
     *          flitr::FLITR_NONE_CODEC so that the codec will be chosen by the application.
     * \param[in] bit_rate Bit rate that must be used for writing the data.
     * \param[in] scale_factor The input image size will be scaled (via multiplication)
     *          to get the size of the output stream size.
     */
    bool openFile(std::string filename, const ImageFormat& image_format,
                  const uint32_t frame_rate=FLITR_DEFAULT_VIDEO_FRAME_RATE,
                  VideoContainer container=FLITR_ANY_CONTAINER,
                  VideoCodec codec=FLITR_RAWVIDEO_CODEC,
                  int32_t bit_rate=-1, double scale_factor = 1);

    /**
     * Write the video frame to the FFmpeg output stream.
     *
     * The \a in_buf must be set up according to the \a image_format set during
     * construction of this object.
     *
     * \return True if the frame was successfully written. When doing RTSP
     * streaming this function will fail if the connection to the server was lost.
     */
    bool writeVideoFrame(uint8_t *in_buf);

    /** 
     * Returns the number of frames written in the file.
     * 
     * \return Number of frames.
     */
    uint64_t getNumImages() const { return WrittenFrameCount_; }

#if LIBAVFORMAT_VERSION_INT >= ((53<<16) + (21<<8) + 0)
    /**
     * Set the dictionary options that must be used when calling avformat_write_header.
     *
     * This can be used to change the options as needed for the desired output stream.
     * The different dictionary options are listed on the FFmpeg
     * website at http://www.ffmpeg.org/ffmpeg-protocols.html.
     *
     * It is recommended to first get the previous options and then add new options
     * to the map since this function will replace the options. For example:
     * \code
     * reader = std::make_shared<flitr::FFmpegReader>();
     * options["rtsp_transport"     ] = "tcp";
     * options["stimeout"           ] = "10000000";
     * options["allowed_media_types"] = "video";
     * options["max_delay"          ] = "0";
     * \endcode
     *
     * This function must be called before a call to openFile(). If the non default
     * constructor is used, any call to this function will be too late.
     *
     * \param dictionary_options Map of key value pairs as strings.
     */
    void setHeaderDictionaryOptions(std::map<std::string, std::string> dictionary_options) { DictionaryOptions_ = dictionary_options; }

    /**
     * Get the dictionary options set for the reader.
     *
     * This function returns the options that will used when the file is is opened and the header is written.
     * \sa setHeaderDictionaryOptions()
     */
    std::map<std::string, std::string> getHeaderDictionaryOptions() const { return DictionaryOptions_; }
#endif

    /**
     * Set the Codec Context private options that should be used.
     *
     * The options set here are applied to the AVCodecContext::priv_data after the AVCodecContext
     * was set up for the writer. This is done before the call to avcodec_open2(). Some options
     * can be found on this page: https://libav.org/avconv.html#libx264
     *
     * It is recommended to first get the previous options and then add new options
     * to the map since this function will replace the options. For example:
     * \code
     *  writer = std::make_shared<flitr::FFmpegWriter>();
     *  std::map<std::string, std::string> contextOptions = writer->getCodecContextPrivateOptions();
     *  contextOptions["profile" ] = "high444";
     *  contextOptions["preset"  ] = "ultrafast";
     *  contextOptions["tune"    ] = "zerolatency";
     *  contextOptions["x264opts"] = "slice-max-size=1400";
     *  writer->setCodecContextPrivateOptions(contextOptions);
     * \endcode
     * The above options are an example to set H264 options on the libx264 encoder
     * from the following link: https://libav.org/avconv.html#libx264
     *
     * This function must be called before a call to openFile(). If the the non default constructor
     * is used, any call to this function will be too late.
     *
     * \param options dictionary_options Map of key value pairs as strings.
     */
    void setCodecContextPrivateOptions(std::map<std::string, std::string> options) { CodecContextPrivateOptions_ = options; }

    /**
     * Get the options set for the writer.
     *
     * This function returns the options that will used when the file is opened.
     * \sa setCodecContextPrivateOptions()
     */
    std::map<std::string, std::string> getCodecContextPrivateOptions() const { return CodecContextPrivateOptions_; }

private:

    VideoContainer Container_;
    VideoCodec Codec_;

    AVFormatContext *FormatContext_;
    AVStream *VideoStream_;
    AVCodec *AVCodec_;
    AVCodecContext *AVCodecContext_;

    /// Open video file and prepare codec.
    bool openVideoFile();

    /// Close video file.
    bool closeVideoFile();

    /// Holds the input frame.
    AVFrame *InputFrame_;
    AVPixelFormat InputFrameFormat_;

    /// Holds the frame converted to the format required by the codec.
    AVFrame* SaveFrame_;
    AVPixelFormat SaveFrameFormat_;

    /// Format of the output video
    AVOutputFormat *OutputFormat_;

#if defined FLITR_USE_SWSCALE
    struct SwsContext *ConvertToSaveCtx_;
#endif

    ImageFormat ImageFormat_;
    std::string SaveFileName_;
    AVRational FrameRate_;
    /// Holds the number of frames written to disk.
    uint64_t WrittenFrameCount_;

    std::shared_ptr<StatsCollector> WriteFrameStats_;

    uint8_t* VideoEncodeBuffer_;
    uint32_t VideoEncodeBufferSize_;

    uint32_t SaveFrameWidth_;
    uint32_t SaveFrameHeight_;

#if LIBAVFORMAT_VERSION_INT >= ((53<<16) + (21<<8) + 0)
    std::map<std::string, std::string> DictionaryOptions_;
#endif
    std::map<std::string, std::string> CodecContextPrivateOptions_;
};

}

#endif //FFMPEG_WRITER_H
