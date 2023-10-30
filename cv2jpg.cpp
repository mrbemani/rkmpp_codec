/******
 * 
 *  
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>

#define TARGET_W 1920
#define TARGET_H 1080


int main(int argc, char *argv[]) {
    
    printf("Start Application ...\n");

    // check if the user has provided the RTSP URL and output filename
    if (argc < 3) 
    {
        printf ("cv2jpg <rtsp_url> <output_dir>\n");
        return 1;
    }

    char output_filename[256] = {0};
    snprintf(output_filename, sizeof(output_filename), "%s", argv[2]);

    // check output filename. It should be alphanumeric with extension .mp4
    if (strlen(output_filename) < 3) {
        printf("output filename should at least have 3 characters in length: %s\n", output_filename);
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
    if (strpbrk(output_filename, "!@#$%^&*()+=-[]{}\\|;:'\",<>?`~") != NULL) {
        printf("Invalid output filename: %s\n", output_filename);
        return 2;
    }

    // if output_dir exists, remove tree
    if (access(argv[2], F_OK) == 0) {
        printf("output_dir exists, remove it!\n");
        char cmd[256] = {0};
        snprintf(cmd, sizeof(cmd), "rm -rf %s/*", argv[2]);
        system(cmd);
    }

    // if output_dir not exists
    if (access(argv[2], F_OK) != 0) {
        printf("output_dir not exists, create it!\n");
        mkdir(argv[2], 0777);
    }

    printf("Start CV VideoCapture ...\n");

    // Open the RTSP feed using OpenCV
    cv::VideoCapture cap(argv[1]);
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the RTSP feed!" << std::endl;
        return -1;
    }

    printf("Creating MPP API Context ...\n");
    // Initialize Rockchip MPP
    
    cv::Mat frame;
    unsigned long frame_idx = 1;
    printf("Start Video loop ...\n");
    // if press Ctrl+C, exit loop
    while (true) {
        printf("cap >> frame\n");
        cap >> frame;
        printf("if (frame.empty())\n");
        if (frame.empty()) {
            printf("frame is empty!!! \n");
            break;
        }

        printf("frame read: %dx%d\n", frame.cols, frame.rows);
        // resize frame to target size
        cv::resize(frame, frame, cv::Size(TARGET_W, TARGET_H));

        // write to output_dir/frame_number.jpg
        char filename[256] = {0};
        snprintf(filename, sizeof(filename), "%s/%d.jpg", argv[2], frame_idx);
        printf("imwrite: %s\n", filename);
        cv::imwrite(filename, frame);

        frame_idx += 1;
        // if environment variable STOP_MPP_ENC is 1, exit loop
        if (getenv("STOP_MPP_ENC") && strcmp(getenv("STOP_MPP_ENC"), "1") == 0) {
            break;
        }
    }

    printf("Video loop exited ...\n");
    printf("Application exited ...\n");

    return 0;
}
