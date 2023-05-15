#pragma once
enum class imageFormat
{
    // RGB
    IMAGE_RGB8=0,					/**< uchar3 RGB8    (`'rgb8'`) */
    IMAGE_RGBA8,					/**< uchar4 RGBA8   (`'rgba8'`) */
    IMAGE_RGB32F,					/**< float3 RGB32F  (`'rgb32f'`) */
    IMAGE_RGBA32F,					/**< float4 RGBA32F (`'rgba32f'`) */

    // BGR
    IMAGE_BGR8,					/**< uchar3 BGR8    (`'bgr8'`) */
    IMAGE_BGRA8,					/**< uchar4 BGRA8   (`'bgra8'`) */
    IMAGE_BGR32F,					/**< float3 BGR32F  (`'bgr32f'`) */
    IMAGE_BGRA32F,					/**< float4 BGRA32F (`'bgra32f'`) */

    // YUV
    IMAGE_YUYV,					/**< YUV YUYV 4:2:2 packed (`'yuyv'`) */
    IMAGE_YUY2=IMAGE_YUYV,			/**< Duplicate of YUYV     (`'yuy2'`) */
    IMAGE_YVYU,					/**< YUV YVYU 4:2:2 packed (`'yvyu'`) */
    IMAGE_UYVY,					/**< YUV UYVY 4:2:2 packed (`'uyvy'`) */
    IMAGE_I420,					/**< YUV I420 4:2:0 planar (`'i420'`) */
    IMAGE_YV12,					/**< YUV YV12 4:2:0 planar (`'yv12'`) */
    IMAGE_NV12,					/**< YUV NV12 4:2:0 planar (`'nv12'`) */

    // Bayer
    IMAGE_BAYER_BGGR,				/**< 8-bit Bayer BGGR (`'bayer-bggr'`) */
    IMAGE_BAYER_GBRG,				/**< 8-bit Bayer GBRG (`'bayer-gbrg'`) */
    IMAGE_BAYER_GRBG,				/**< 8-bit Bayer GRBG (`'bayer-grbg'`) */
    IMAGE_BAYER_RGGB,				/**< 8-bit Bayer RGGB (`'bayer-rggb'`) */

    // grayscale
    IMAGE_GRAY8,					/**< uint8 grayscale  (`'gray8'`)   */
    IMAGE_GRAY32F,					/**< float grayscale  (`'gray32f'`) */

    // extras
    IMAGE_COUNT,					/**< The number of image formats */
    IMAGE_UNKNOWN=999,				/**< Unknown/undefined format */
    IMAGE_DEFAULT=IMAGE_RGBA32F		/**< Default format (IMAGE_RGBA32F) */
};
enum class audioFormat
{
    // Signed integer formats
    AUDIO_S16LE = 0,    /**< Signed 16-bit little-endian */
    AUDIO_S16BE,        /**< Signed 16-bit big-endian */
    AUDIO_S24LE,        /**< Signed 24-bit little-endian */
    AUDIO_S24BE,        /**< Signed 24-bit big-endian */
    AUDIO_S32LE,        /**< Signed 32-bit little-endian */
    AUDIO_S32BE,        /**< Signed 32-bit big-endian */

    // Unsigned integer formats
    AUDIO_U8,           /**< Unsigned 8-bit */

    // Floating-point formats
    AUDIO_F32LE,        /**< 32-bit float little-endian */
    AUDIO_F32BE,        /**< 32-bit float big-endian */
    AUDIO_F64LE,        /**< 64-bit float little-endian */
    AUDIO_F64BE,        /**< 64-bit float big-endian */

    // Special formats
    AUDIO_UNKNOWN=999,      /**< Unknown/undefined format */
    AUDIO_DEFAULT = AUDIO_S16LE   /**< Default format (AUDIO_S16LE) */
};