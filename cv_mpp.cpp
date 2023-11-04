/******
 * 
 *  
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <mpp.h>
#include <rk_mpi.h>
#include <mpp_enc_cfg.h>
#include <mpp_env.h>
#include <mpp_common.h>
#include <mpp_frame.h>

typedef int BOOL;

#define TRUE 1
#define FALSE 0

#define TARGET_W 1920
#define TARGET_H 1080

MppBufferGroup mppBufGrp = nullptr;


size_t convertToMppFrame(const cv::Mat &cvFrame, MppFrame &mppFrame, BOOL isLastFrame = FALSE) {
    // Convert BGR to RGB

    cv::Mat rgbFrame;
    cv::cvtColor(cvFrame, rgbFrame, cv::COLOR_BGR2RGB);

    // Create an MppFrame object
    mppFrame = nullptr;
    mpp_frame_init(&mppFrame);
    
    // Get frame dimensions
    int width = rgbFrame.cols;
    int height = rgbFrame.rows;
    
    MppFrameFormat fmt = MPP_FMT_RGB888;

    // Set frame dimensions and format
    mpp_frame_set_width(mppFrame, width);
    mpp_frame_set_height(mppFrame, height);
    mpp_frame_set_hor_stride(mppFrame, TARGET_W);
    mpp_frame_set_ver_stride(mppFrame, TARGET_H);
    mpp_frame_set_fmt(mppFrame, fmt);  // Assuming RGB format
    mpp_frame_set_eos(mppFrame, isLastFrame);
    
    
    // Allocate an MppBuffer to hold the frame data
    MppBuffer mppBuffer;
    size_t frame_size = MPP_ALIGN(TARGET_W, 64) * MPP_ALIGN(TARGET_H, 64);
    size_t header_size = 0;
    if (MPP_FRAME_FMT_IS_FBC(fmt)) {
        if ((fmt & MPP_FRAME_FBC_MASK) == MPP_FRAME_FBC_AFBC_V1)
            header_size = MPP_ALIGN(MPP_ALIGN(TARGET_W, 16) * MPP_ALIGN(TARGET_H, 16) / 16, SZ_4K);
        else
            header_size = MPP_ALIGN(TARGET_W, 16) * MPP_ALIGN(TARGET_H, 16) / 16;
    } else {
        header_size = 0;
    }

    MPP_RET ret = mpp_buffer_get(mppBufGrp, &mppBuffer, frame_size + header_size);
    if (ret != MPP_OK)
    {
        printf("Failed to mpp_buffer_get\n");
        printf("Error: %d\n", ret);
        return 4;
    }
    
    // Copy data from rgbFrame to mppBuffer
    memcpy(mpp_buffer_get_ptr(mppBuffer), rgbFrame.data, bufferSize);
    
    // Attach the MppBuffer to the MppFrame
    mpp_frame_set_buffer(mppFrame, mppBuffer);
    
    return bufferSize;
}



int main(int argc, char *argv[]) {
    
    printf("Start Application ...\n");

    MPP_RET mpp_ret = MPP_OK;

    char output_filename[] = "output.mp4";

    // Remove the output file if it already exists
    if (access(output_filename, F_OK) == 0) {
        remove(output_filename);
    }

    // Open the output file
    FILE *fp = fopen(output_filename, "ab+");

    if (fp == NULL) {
        printf("Failed to open output file: %s\n", output_filename);
        return 3;
    }

    printf("Creating MPP API Context ...\n");
    // Initialize Rockchip MPP
    MppCtx ctx;
    MppApi *mpi;
    mpp_ret = mpp_create(&ctx, &mpi);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_create\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    // Initialize MPP for encoding
    printf("Initialize MPP Encoder ...\n");
    mpp_ret = mpp_init(ctx, MPP_CTX_ENC, MPP_VIDEO_CodingAVC);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_init\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    printf("Initialize MppBufferGroup ...\n");
    mpp_ret = mpp_buffer_group_get_internal(&mppBufGrp, MPP_BUFFER_TYPE_DMA_HEAP);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_buffer_group_get_internal\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_buffer_group_limit_config(mppBufGrp, 0, 20);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_buffer_group_limit_config\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }
    

    printf("Set MPP Encoder Configuration ...\n");
    // Set encoding parameters (assuming an API similar to FFmpeg)
    // This is hypothetical and may not match the exact MPP API
    // Initialize the encoder configuration structure
    MppEncCfg mppEncCfg;
    mpp_ret = mpp_enc_cfg_init(&mppEncCfg);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_init\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "prep:hor_stride", TARGET_W);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [prep:hor_stride]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "prep:ver_stride", TARGET_H);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [prep:ver_stride]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "prep:format", MPP_FMT_RGB888);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [prep:format]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:rc_mode", MPP_ENC_RC_MODE_CBR);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:rc_mode]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:quality", MPP_ENC_RC_QUALITY_BEST);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:quality]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    /* drop frame or not when bitrate overflow */
    mpp_ret = mpp_enc_cfg_set_u32(cfg, "rc:drop_mode", MPP_ENC_RC_DROP_FRM_DISABLED);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_u32 [rc:drop_mode]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_u32(cfg, "rc:drop_thd", 20);        /* 20% of max bps */
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_u32 [rc:drop_thd]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_u32(cfg, "rc:drop_gap", 1);         /* Do not continuous drop frame */
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_u32 [rc:drop_gap]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:bps_target", 4 * 1024 * 1024);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:bps_target]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:bps_max", 4 * 1024 * 1024);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:bps_max]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:bps_min", 4 * 1024 * 1024);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:bps_min]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:fps_in_num", FPS);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:fps_in_num]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:fps_in_denorm", 1);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:fps_in_denorm]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:fps_out_num", FPS);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:fps_out_num]\n");
        printf("Error: %d\n", mpp_ret);
    }

    mpp_ret = mpp_enc_cfg_set_s32(mppEncCfg, "rc:fps_out_denorm", 1);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [rc:fps_out_denorm]\n");
        printf("Error: %d\n", mpp_ret);
    }

    mpp_ret = mpp_enc_cfg_set_s32(cfg, "codec:type", MPP_VIDEO_CodingAVC);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_set_s32 [codec:type]\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    RK_U32 constraint_set;

    /*
     * H.264 profile_idc parameter
     * 66  - Baseline profile
     * 77  - Main profile
     * 100 - High profile
     */
    mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
    /*
     * H.264 level_idc parameter
     * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
     * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
     * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
     * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
     * 50 / 51 / 52         - 4K@30fps
     */
    mpp_enc_cfg_set_s32(cfg, "h264:level", 40);
    mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
    mpp_enc_cfg_set_s32(cfg, "h264:cabac_idc", 0);
    mpp_enc_cfg_set_s32(cfg, "h264:trans8x8", 1);

    mpp_env_get_u32("constraint_set", &constraint_set, 0);
    if (constraint_set & 0x3f0000)
        mpp_enc_cfg_set_s32(cfg, "h264:constraint_set", constraint_set);
    

    // Set the encoder configuration
    mpp_ret = mpi->control(ctx, MPP_ENC_SET_CFG, mppEncCfg);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to set MPP encoder configuration\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }
    
    int frame_idx = 0;
    printf("Start Video loop ...\n");
    // if press Ctrl+C, exit loop
    while (true) {
        frame_idx += 1;
        
        // create an empty frame with size 1920x1080x3
        cv::Mat frame = cv::Mat::zeros(TARGET_H, TARGET_W, CV_8UC3);
        
        // write yellow text frame_idx in the center of frame
        char text[256] = {0};
        snprintf(text, sizeof(text), "frame_idx: %d", frame_idx);
        cv::putText(frame, text, cv::Point(TARGET_W/2-100, TARGET_H/2), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 255), 2);
        
        // Assume MPP requires a specific buffer format
        size_t in_bufsize = frame.total() * frame.elemSize() * 2;
        printf("frame.total() = %ld, frame.elemSize() = %ld\n", frame.total(), frame.elemSize());
        printf("converting cv_frame to mpp_frame ... %ld \n", in_bufsize);
        MppFrame mpp_frame = convertToMppFrame(frame);
        printf("[Converting Done]\n");

        // Encode the frame using MPP
        printf("Encoding frame ... \n");
        MppPacket packet = NULL;
        printf("encode_put_frame ... \n");
        mpp_ret = mpi->encode_put_frame(ctx, mpp_frame);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to encode_put_frame\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }

        printf("Deinitialize MPP Frame ...\n");
        mpp_ret = mpp_frame_deinit(&mpp_frame);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to mpp_frame_deinit\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }

        printf("encode_get_packet ... \n");
        mpp_ret = mpi->encode_get_packet(ctx, &packet);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to encode_get_packet\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }
        printf("[Encoding Done]\n");

        if (packet) {
            // write to file
            //unsigned char* bufdata = (unsigned char*)mpp_packet_get_data(packet);
            void *bufdata = mpp_packet_get_pos(packet);  // Get the data pointer from the packet.
            size_t buflen = mpp_packet_get_length(packet);  // Get the data size from the packet.

            // Write the packet to the output file
            fwrite(bufdata, 1, buflen, fp);
            printf("Wrote %zu bytes\n", buflen);

            // Release the packet
            mpp_ret = mpp_packet_deinit(&packet);
            if (mpp_ret != MPP_OK)
            {
                printf("Failed to mpp_packet_deinit\n");
                printf("Error: %d\n", mpp_ret);
                return 4;
            }
        }
        
        // frame_idx > 20, break
        if (frame_idx > 100) {
            break;
        }
    }


MPP_TEST_OUT:

    printf("Video loop exited ...\n");
    mpp_buffer_group_put(mppBufGrp);

    // Finalize MPP Encoder
    printf("Finalize MPP Encoder ...\n");
    mpp_ret = mpp_enc_cfg_deinit(mppEncCfg);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_enc_cfg_deinit\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    // Finalize MPP and release resources
    printf("Finalize MPP API Context ...\n");
    mpp_ret = mpi->reset(ctx);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpi->reset\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    mpp_ret = mpp_destroy(ctx);
    if (mpp_ret != MPP_OK)
    {
        printf("Failed to mpp_destroy\n");
        printf("Error: %d\n", mpp_ret);
        return 4;
    }

    // Close the output file
    if (fp) fclose(fp);

    printf("Application exited ...\n");

    return 0;
}
