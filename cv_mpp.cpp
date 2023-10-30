/******
 * 
 *  
 */


#include <stdio.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <mpp.h>
#include <rk_mpi.h>


#define TARGET_W 1920
#define TARGET_H 1080


int main(int argc, char *argv[]) {
    
    // check if the user has provided the RTSP URL and output filename
    if (argc < 3) 
    {
        printf ("cv_mpp <rtsp_url> <output_file.mp4>\n");
        return 1;
    }

    char output_filename[256] = {0};
    snprintf(output_filename, sizeof(output_filename), "%s", argv[2]);

    // check output filename. It should be alphanumeric with extension .mp4
    if (strlen(output_filename) < 5) {
        printf("Invalid output filename: %s\n", output_filename);
        return 2;
    }

    if (strcmp(output_filename + strlen(output_filename) - 4, ".mp4") != 0) {
        printf("Invalid output filename: %s\n", output_filename);
        return 2;
    }

    // check if output filename is alphanumeric with or without underscores
    for (int i = 0; i < strlen(output_filename) - 4; i++) {
        if (!isalnum(output_filename[i]) && output_filename[i] != '_') {
            printf("Invalid output filename: %s\n", output_filename);
            return 2;
        }
    }

    // filename should not contain any special characters
    if (strpbrk(output_filename, "!@#$%^&*()+=-[]{}\\|;:'\",.<>/?`~") != NULL) {
        printf("Invalid output filename: %s\n", output_filename);
        return 2;
    }

    // Remove the output file if it already exists
    if (access(output_filename, F_OK) == 0) {
        remove(output_filename);
    }

    // Open the output file
    FILE *fp = fopen(argv[2], "wb");

    if (fp == NULL) {
        printf("Failed to open output file: %s\n", output_filename);
        return 3;
    }

    // Open the RTSP feed using OpenCV
    cv::VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the RTSP feed!" << std::endl;
        return -1;
    }

    // Initialize Rockchip MPP
    MppCtx ctx;
    MppApi *mpi;
    mpp_create(&ctx, &mpi);

    // Set encoding parameters (assuming an API similar to FFmpeg)
    // This is hypothetical and may not match the exact MPP API
    MppEncCfg cfg;
    mpp_enc_cfg_init(&cfg);
    mpp_enc_cfg_set_s32(cfg, "width", TARGET_W);
    mpp_enc_cfg_set_s32(cfg, "height", TARGET_H);
    mpp_enc_cfg_set_s32(cfg, "rc_mode", MPP_ENC_RC_MODE_VBR);
    mpp_enc_cfg_set_s32(cfg, "format", MPP_FMT_YUV420SP);
    mpp_enc_cfg_set_s32(cfg, "codec", MPP_VIDEO_CodingHEVC);
    mpi->control(ctx, MPP_ENC_SET_CFG, cfg);

    cv::Mat frame;
    MppBuffer input_buf;

    // if press Ctrl+C, exit loop
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // resize frame to target size
        cv::resize(frame, frame, cv::Size(TARGET_W, TARGET_H));

        // Assume MPP requires a specific buffer format
        size_t in_bufsize = frame.total() * frame.elemSize();
        mpp_buffer_get(NULL, &input_buf, in_bufsize);
        memcpy(mpp_buffer_get_ptr(input_buf), frame.data, in_bufsize);

        // Encode the frame using MPP
        printf("Encoding frame ... \n");
        MppPacket packet = NULL;
        mpi->encode_put_frame(ctx, input_buf);
        mpi->encode_get_packet(ctx, &packet);
        printf("[Done]\n");

        if (packet) {
            // write to file
            unsigned char* bufdata = (unsigned char*)mpp_packet_get_data(packet);
            size_t buflen = mpp_packet_get_length(packet);

            fwrite(bufdata, sizeof(byte), buflen, fp);
            printf("Wrote %zu bytes\n", buflen);

            // Release the packet
            mpp_packet_deinit(&packet);
        }

        // Release the input buffer
        mpp_buffer_put(input_buf);

        // if environment variable STOP_MPP_ENC is 1, exit loop
        if (getenv("STOP_MPP_ENC") && strcmp(getenv("STOP_MPP_ENC"), "1") == 0) {
            break;
        }
    }

    // Finalize MPP and release resources
    mpp_destroy(ctx);

    // Close the output file
    if (fp) fclose(fp);

    return 0;
}
