

#include <opencv4/opencv2/opencv.hpp>
#include <rockchip/mpp_rc_api.h>
#include <rockchip/mpp_buffer.h>
#include <rockchip/mpp_frame.h>
#include <rockchip/mpp_packet.h>

// ... Your setup code ...

#define TS_MAX_PATH_LEN 256


MppFrame CvFrameToMppFrame(const cv::Mat& cvFrame)
{
    // Check if the color conversion is needed (e.g., from BGR to YUV)
    cv::Mat yuvFrame;
    cv::cvtColor(cvFrame, yuvFrame, cv::COLOR_BGR2YUV_I420);

    // Allocate a buffer for the MPP frame
    MppBuffer buffer;
    mpp_buffer_get(NULL, &buffer, yuvFrame.total() * yuvFrame.elemSize());

    // Copy the data from the OpenCV frame to the MPP buffer
    memcpy(mpp_buffer_get_ptr(buffer), yuvFrame.data, mpp_buffer_get_size(buffer));

    // Create and initialize an MPP frame
    MppFrame mpp_frame;
    mpp_frame_init(&mpp_frame);
    mpp_frame_set_width(mpp_frame, cvFrame.cols);
    mpp_frame_set_height(mpp_frame, cvFrame.rows);
    mpp_frame_set_buffer(mpp_frame, buffer);

    return mpp_frame;
}


void ts_encode_frame(MppApi *api, MppCtx ctx, MppEncCfg cfg, const cv::Mat& cvFrame, unsigned char output_filename[TS_MAX_PATH_LEN]) 
{
    // Convert the OpenCV frame to an MPP frame
    if (frame.empty()) {
        break;  // Break the loop if we've read all frames
    }

    // Convert the OpenCV frame to a format that MPP can understand, if necessary
    // This may involve converting the color space, reformatting the data, etc.
    // Assume CvFrameToMppFrame is a function you've defined to handle this conversion
    MppFrame mpp_frame = CvFrameToMppFrame(frame);

    // Set up the MPP encoding context, if not already set up
    // This would typically be done outside the loop, before entering it

    // Encode the frame
    MppPacket packet;
    mpp_encode(api, mpp_frame, &packet);

    // Write the encoded data to your output file
    // Assume writePacketToFile is a function you've defined to handle this
    writePacketToFile(packet, "output_video.mp4");

    // Free any resources associated with this frame
    mpp_frame_deinit(&mpp_frame);
    mpp_packet_deinit(&packet);
}



void destroyContext(MppApi *api, MppCtx ctx) {
    // Destroy the MPP context
    mpp_destroy(ctx);
    mpp_destroy(api);
}



int main()
{
    MppApi *api = mpp_create();
    MppCtx ctx;
    mpp_create(&ctx, &api);

    MppEncCfg cfg;
    mpp_enc_cfg_init(&cfg);
    // Configure the encoder for H264, 1920x1080, 25fps
    mpp_enc_cfg_set_s32(cfg, "width", 1920);
    mpp_enc_cfg_set_s32(cfg, "height", 1080);
    mpp_enc_cfg_set_s32(cfg, "fps", 25);

    mpp_enc_cfg_set_s32(cfg, "rc_mode", MPP_ENC_RC_MODE_CBR);
    mpp_enc_cfg_set_s32(cfg, "bps", 5000000);  // Set bitrate, e.g., 5Mbps


    
    // Initialize the MPP encoder
    mpp_enc_init(ctx, cfg);

    
}