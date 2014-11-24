//
// simple face detection  based on:
//
// @file objectDetection2.cpp
// @author A. Huaman ( based in the classic facedetect.cpp in samples/c )
// @brief A simplified version of facedetect.cpp, show how to load a cascade classifier and how to find objects (Face + eyes) in a video stream - Using LBP here
//
//
// http://robocraft.ru
//

#include "opencv2/opencv.hpp"

#include <iostream>
#include <stdio.h>

#include "commands.h"
#include "robopet.h"

#include "orcp2/orcp2.h"
#include "lib/serial.h"
#include "lib/times.h"
#include "lib/console.h"
#include "orcp2/robot_2wd.h"

#include "lib/roboipc.h"

#if defined(LINUX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
#endif

using namespace std;
using namespace cv;

#if defined(LINUX)
volatile sig_atomic_t terminated = 0;
volatile sig_atomic_t is_sigpipe = 0;

void signal_handler(int sig)
{
    if(sig == SIGINT) {
        terminated = 1;
    }
    if(sig == SIGPIPE) {
        is_sigpipe = 1;
    }
}
#else
volatile int terminated = 0;
#endif //#if defined(LINUX)

void register_signal_handlers()
{
#if defined(LINUX)
    // register signal handler for <CTRL>+C in order to clean up
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("[i] Error: register signal handler!\n");
    }

    if (signal(SIGPIPE, signal_handler) == SIG_ERR) {
        printf("[i] Error: register signal handler!\n");
    }
#endif //#if defined(LINUX)
}

/** Function Headers */
int detectAndDisplay( Mat frame );

/** Global variables */
String face_cascade_name = "/usr/share/opencv/lbpcascades/lbpcascade_frontalface.xml";
String eyes_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";
/**
 * @function main
 */

int counter=0;
char filename[512] = {0};

int main( void )
{
    VideoCapture capture;
    Mat frame;

    printf("[i] Start...\n");

    register_signal_handlers();

    //-- 1. Load the cascade
    if( !face_cascade.load( face_cascade_name ) ) {
        printf("[!] Error loading face cascade\n"); return -1;
    };
    if( !eyes_cascade.load( eyes_cascade_name ) ) {
        printf("[!] Error loading eyes cascade\n"); return -1;
    };

    //-- 2. Read the video stream
    capture.open( -1 );
    if ( ! capture.isOpened() ) {
        printf("[!] Error opening video capture\n"); return -1;
    }

    capture.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

    roboipc::CommunicatorClient speecher;

    CmdSpeech cmd_speech;
    CmdAcknowledgment cmd_ack;

    char cmd_buf[256]={0};
    int cmd_buf_size = 0;
    bool is_speech_end = true;
    int res = 0;

    if( speecher.connect(SPEECHER_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", SPEECHER_SOCKET_NAME);
        //return -1;
    }
    memset(&cmd_speech, 0, sizeof(cmd_speech));
    memset(&cmd_ack, 0, sizeof(cmd_ack));
    memset(cmd_buf, 0, sizeof(cmd_buf));


    while ( !terminated && capture.read(frame) ) {
        printf("[i] Capture frame...\n");
        if( frame.empty() ) {
            printf("[!] No captured frame -- Break!\n");
            break;
        }

        //-- 3. Apply the classifier to the frame
        res = detectAndDisplay( frame );
        if( res ) {
            printf("[i] Play woof!\n");

            if(is_speech_end && speecher.sockfd != SOCKET_ERROR) {
                is_speech_end = false;

                strncpy(cmd_speech.sig, "speech", CMD_SIG_SIZE);
                cmd_speech.code = 0;
                strncpy(cmd_speech.name, SOUND_FOR_DETECTION, sizeof(cmd_speech.name));
                if(res == 2) {
                    strncpy(cmd_speech.name, SOUND_FOR_NEAR, sizeof(cmd_speech.name));
                }

                res = speecher.write(&cmd_speech, sizeof(cmd_speech));
                printf("[i] Send speech command (%d)...\n", res);
                printf("[i] Data: %d %s\n", cmd_speech.code, cmd_speech.name);
            }
        }

        if(speecher.available(50) > 0) {
            printf("[i] Speecher action...\n");
            if( (cmd_buf_size = speecher.read(cmd_buf, sizeof(cmd_buf))) > 0 ) {
                printf("[i] Data size: %d\n", cmd_buf_size);
                if(cmd_buf_size > CMD_SIG_SIZE) {
                    if( !strncmp(cmd_buf, "ackmnt", CMD_SIG_SIZE) && cmd_buf_size >= sizeof(cmd_ack) ) {
                        memcpy(&cmd_ack, cmd_buf, sizeof(cmd_ack));
                        printf("[i] Clinet ACK: %d\n", cmd_ack.code);
                        is_speech_end = true;
                    }
                }
            }
            else {
                printf("[i] Connection closed...\n");
                speecher.close();
            }
        }

        //-- bail out if escape was pressed
        int c = waitKey(10);
        if( (char)c == 27 ) {
            break;
        }
    }
    return 0;
}

/**
 * @function detectAndDisplay
 */
int detectAndDisplay( Mat frame )
{
    int res = 0;
    std::vector<Rect> faces;
    Mat frame_gray;

    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0, Size(80, 80) );

    printf("[i] frame %d x %d detect faces: %d\n", frame.cols, frame.rows, faces.size());
    if(faces.size()) {
        res = 1;
    }

    for( size_t i = 0; i < faces.size(); i++ ) {
        Mat faceROI = frame_gray( faces[i] );
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
        eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );
        printf("[i] eyes: %d\n", eyes.size());

        if( eyes.size() == 2) {
            //-- Draw the face
            Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
            ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );

            for( size_t j = 0; j < eyes.size(); j++ ) {
                //-- Draw the eyes
                Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
                int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
                circle( frame, eye_center, radius, Scalar( 255, 0, 255 ), 3, 8, 0 );
            }

            //printf("[i] counter: %d!\n", ++counter);
            //sprintf(filename, "image%d.png", counter);
            //imwrite(filename, frame);

            res = 2;
        }
    }
    //-- Show what you got
    //imshow( window_name, frame );

    return res;
}
