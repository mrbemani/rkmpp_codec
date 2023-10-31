/******
 * 
 *  
 */


#include <stdio.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <mpp.h>
#include <rk_mpi.h>
#include <mpp_enc_cfg.h>


#define TARGET_W 1920
#define TARGET_H 1080


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

    printf("Show Configuration ...\n");
    mpp_enc_cfg_show();

    printf("Set MPP Encoder Configuration ...\n");
    // Set encoding parameters (assuming an API similar to FFmpeg)
    // This is hypothetical and may not match the exact MPP API
    // Initialize the encoder configuration structure
    MppEncCfgSet cfg_set;
    memset(&cfg_set, 0, sizeof(cfg_set));


    // Set the resolution
    cfg_set.prep.width = TARGET_W;
    cfg_set.prep.height = TARGET_H;

    cfg_set.rc.rc_mode = MPP_ENC_RC_MODE_CBR;
    cfg_set.rc.quality = MPP_ENC_RC_QUALITY_BEST;

    cfg_set.rc.bps_target = 4 * 1024 * 1024;
    cfg_set.rc.bps_max = 4 * 1024 * 1024;
    cfg_set.rc.bps_min = 4 * 1024 * 1024;
    
    cfg_set.rc.fps_in_num = 25;
    cfg_set.rc.fps_in_denorm = 1;
    cfg_set.rc.fps_out_num = 25;
    cfg_set.rc.fps_out_denorm = 1;

    mpp_ret = mpi->control(ctx, MPP_ENC_SET_CFG, &cfg_set);
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
        MppBuffer input_buf = NULL;
        frame_idx += 1;
        
        // create an empty frame with size 1920x1080x3
        cv::Mat frame = cv::Mat::zeros(TARGET_H, TARGET_W, CV_8UC3);
        
        // write yellow text frame_idx in the center of frame
        char text[256] = {0};
        snprintf(text, sizeof(text), "frame_idx: %d", frame_idx);
        cv::putText(frame, text, cv::Point(TARGET_W/2-100, TARGET_H/2), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 255), 2);
        
        // Assume MPP requires a specific buffer format
        size_t in_bufsize = frame.total() * frame.elemSize();
        printf("converting cv_frame to mpp_frame ... %ld \n", in_bufsize);
        mpp_buffer_get(NULL, &input_buf, in_bufsize);
        memcpy(mpp_buffer_get_ptr(input_buf), frame.data, in_bufsize);

        MppFrame mpp_frame = NULL;
        mpp_ret = mpp_frame_init(&mpp_frame);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to mpp_frame_init\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }

        mpp_frame_set_buffer(mpp_frame, input_buf);
        printf("[Done]\n");

        // Encode the frame using MPP
        printf("Encoding frame ... \n");
        MppPacket packet = NULL;
        mpp_ret = mpi->encode_put_frame(ctx, mpp_frame);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to encode_put_frame\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }
        
        mpp_ret = mpi->encode_get_packet(ctx, &packet);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to encode_get_packet\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }
        printf("[Done]\n");

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

        mpp_ret = mpp_frame_deinit(&mpp_frame);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to mpp_frame_deinit\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }

        // Release the input buffer
        mpp_ret = mpp_buffer_put(input_buf);
        if (mpp_ret != MPP_OK)
        {
            printf("Failed to mpp_buffer_put\n");
            printf("Error: %d\n", mpp_ret);
            return 4;
        }

        // if environment variable STOP_MPP_ENC is 1, exit loop
        if (getenv("STOP_MPP_ENC") && strcmp(getenv("STOP_MPP_ENC"), "1") == 0) {
            break;
        }
        
        // frame_idx > 20, break
        if (frame_idx > 100) {
            break;
        }
    }

    printf("Video loop exited ...\n");

    // Finalize MPP and release resources
    printf("Finalize MPP API Context ...\n");
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
