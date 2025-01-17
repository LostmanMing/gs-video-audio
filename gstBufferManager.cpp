#include "gstBufferManager.h"
#include "gstVideoAudioUtils.hpp"
#include "spdlog/spdlog.h"
#include <memory_resource>

std::shared_ptr<std::vector<uint8_t>> gstBufferManager::Dequeue(imageFormat format, uint64_t timeout)
{

   auto pLastTimestamp = mTimestamps.next();

    if( !pLastTimestamp )
    {
        spdlog::warn("gstBufferManager -- failed to retrieve timestamp buffer (default to 0)");
        mLastTimestamp = 0;
    }
    else
    {
        //断言，该容器内部表示时间的一块内存的长度为8字节
        assert( pLastTimestamp->size() == sizeof(uint64_t) );
        //将该内存的内容转换为uint64_t类型
        mLastTimestamp =reinterpret_cast<uint64_t>(pLastTimestamp->data());
    }

    // output raw image if conversion format is unknown
    if ( format == imageFormat::IMAGE_UNKNOWN )
    {
        spdlog::error("the format is unknown");
        return nullptr;
    }
    return mBufferYUV.next();
}



bool gstBufferManager::videoEnqueue(GstBuffer *buffer, GstCaps *caps){

    if(!buffer || !caps ){
        return false;
    }

    uint64_t timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    // map the buffer memory for read access
    GstMapInfo map;

    if( !gst_buffer_map(buffer, &map, GST_MAP_READ) )
    {
        spdlog::error("gstBufferManager -- failed to map gstreamer buffer memory");
        return false;
    }

    const void* gstData = map.data;
    const gsize gstSize = map.maxsize; //map.size;

    if( !gstData )
    {
        spdlog::error("gstBufferManager -- gst_buffer_map had NULL data pointer...");
        return false;
    }

    if( map.maxsize > map.size && mFrameCount == 0 )
    {
        spdlog::warn("gstBufferManager -- map buffer size was less than max size ({} vs {})", map.size, map.maxsize);
    }

    // on the first frame, print out the recieve caps
    if( mFrameCount == 0 ){
        spdlog::info("gstBufferManager recieve caps:  {}", gst_caps_to_string(caps));

    }

    // retrieve caps structure
    GstStructure* gstCapsStruct = gst_caps_get_structure(caps, 0);

    if( !gstCapsStruct )
    {
        spdlog::error("gstBufferManager -- gst_caps had NULL structure...");
        return false;
    }

    // retrieve the width and height of the buffer
    int width  = 0;
    int height = 0;
    if( !gst_structure_get_int(gstCapsStruct, "width", &width) ||
        !gst_structure_get_int(gstCapsStruct, "height", &height) )
    {
        spdlog::error("gstBufferManager -- gst_caps missing width/height...");
        return false;
    }

    if( width < 1 || height < 1 )
        return false;

    mOptions->width = width;
    mOptions->height = height;

    // verify format
    if( mFrameCount == 0 )
    {
        mFormatYUV = gst_parse_video_format(gstCapsStruct);

        if( mFormatYUV == imageFormat::IMAGE_UNKNOWN )
        {
            spdlog::error("gstBufferManager -- stream {} does not have a compatible decoded format", mOptions->resource);
            return false;
        }
        spdlog::info("gstBufferManager -- recieved first frame, codec={} format={} width={} height={} size={}",VideoAudioOption::CodecToStr(mOptions->codec), imageFormatToStr(mFormatYUV), mOptions->width, mOptions->height, gstSize);

    }

    spdlog::debug("gstBufferManager -- recieved {}x{} frame ({} bytes)", width, height, gstSize);


    auto yuvPtr=mBufferYUV.prepareBuffer((long)gstSize);
    memcpy(yuvPtr->data(), gstData, gstSize);


    // handle timestamps in either case (CPU or NVMM path)
    size_t timestamp_size = sizeof(uint64_t);
    auto timestampPtr=mTimestamps.prepareBuffer((long)timestamp_size);


    if (GST_BUFFER_DTS_IS_VALID(buffer) || GST_BUFFER_PTS_IS_VALID(buffer))
    {
        timestamp = GST_BUFFER_DTS_OR_PTS(buffer);
    }
    memcpy(timestampPtr->data(), (void*)&timestamp, timestamp_size);

    mFrameCount++;
    gst_buffer_unmap(buffer, &map);

    return true;
}

bool gstBufferManager::audioEnqueue(GstBuffer *buffer, GstCaps *caps) {
    if(!buffer || !caps ){
        return false;
    }

    uint64_t timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    // map the buffer memory for read access
    GstMapInfo map;

    if( !gst_buffer_map(buffer, &map, GST_MAP_READ) )
    {
        spdlog::error("gstBufferManager -- failed to map gstreamer buffer memory");
        return false;
    }

    const void* gstData = map.data;
    const gsize gstSize = map.maxsize; //map.size;

    if( !gstData )
    {
        spdlog::error("gstBufferManager -- gst_buffer_map had NULL data pointer...");
        return false;
    }

    if( map.maxsize > map.size && mSampleCount == 0 )
    {
        spdlog::warn("gstBufferManager -- audio map buffer size was less than max size ({} vs {})", map.size, map.maxsize);
    }

    // on the first frame, print out the recieve caps
    if( mSampleCount == 0 ){
        spdlog::info("gstBufferManager recieve caps:  {}", gst_caps_to_string(caps));

    }

    // retrieve caps structure
    GstStructure* gstCapsStruct = gst_caps_get_structure(caps, 0);
    const gchar* capsName = gst_structure_get_name(gstCapsStruct);

    if( !gstCapsStruct )
    {
        spdlog::error("gstBufferManager -- gst_caps had NULL structure...");
        return false;
    }

    // retrieve the width and height of the buffer

    int rate = 0;
    int channels = 0;
    if( !gst_structure_get_int(gstCapsStruct, "rate", &rate) ||
        !gst_structure_get_int(gstCapsStruct, "channels", &channels) )
    {
        spdlog::error("gstBufferManager -- gst_caps missing rate/channels...");
        return false;
    }

    if( rate < 1 || channels < 1 )
        return false;

    mOptions->Sampling_rate = rate;
    mOptions->channels = channels;

    // verify format
    if( mSampleCount == 0 )
    {
        mFormatPCM = gst_parse_audio_format(gstCapsStruct);

        if(mFormatPCM == audioFormat::AUDIO_UNKNOWN )
        {
            spdlog::error("gstBufferManager -- stream {} does not have a compatible decoded format", mOptions->resource);
            return false;
        }
        spdlog::info("gstBufferManager -- recieved first sample, codec={} format={} width={} height={} size={}",VideoAudioOption::CodecToStr(mOptions->codec), imageFormatToStr(mFormatYUV), mOptions->width, mOptions->height, gstSize);

    }

    spdlog::debug("gstBufferManager -- recieved {}x{} frame ({} bytes)", rate, channels, gstSize);


    auto audioPtr=mBufferPCM.prepareBuffer((long)gstSize);
    memcpy(audioPtr->data(), gstData, gstSize);


    // handle timestamps in either case (CPU or NVMM path)
    //size_t timestamp_size = sizeof(uint64_t);
    //auto timestampPtr=mTimestamps.prepareBuffer((long)timestamp_size);


    if (GST_BUFFER_DTS_IS_VALID(buffer) || GST_BUFFER_PTS_IS_VALID(buffer))
    {
        timestamp = GST_BUFFER_DTS_OR_PTS(buffer);
    }
    //memcpy(timestampPtr->data(), (void*)&timestamp, timestamp_size);

    mSampleCount++;
    gst_buffer_unmap(buffer, &map);

    return true;
}

gstBufferManager::~gstBufferManager() {


}

gstBufferManager::gstBufferManager(std::unique_ptr<VideoAudioOption>option) {

    mOptions=std::move(option);

}


