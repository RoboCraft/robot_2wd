//
// eye via OpenCV
//
// http://robocraft.ru
//

//#define USE_KEYBOARD_INPUT 1

#include <stdio.h>
#include <stdlib.h>

#include "robopet.h"

#include "lib/times.h"
#include "lib/console.h"

#include "lib/roboipc.h"

#if defined(LINUX)
# include <unistd.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
#endif

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <algorithm>

#define RECT_COLORS_SIZE 10

// получение пикселя изображения (по типу картинки и координатам)
#define CV_PIXEL(type,img,x,y) (((type*)((img)->imageData+(y)*(img)->widthStep))+(x)*(img)->nChannels)

// Various color types
//	    0			1	  2		 3		4		 5		  6		7		8		9			10
enum {cBLACK=0, cWHITE, cGREY, cRED, cORANGE, cYELLOW, cGREEN, cAQUA, cBLUE, cPURPLE, NUM_COLOR_TYPES};
const char* sCTypes[NUM_COLOR_TYPES] = {"Black", "White","Grey","Red","Orange","Yellow","Green","Aqua","Blue","Purple"};
uchar cCTHue[NUM_COLOR_TYPES] =    {0,       0,      0,     0,     20,      30,      60,    85,   120,    138  };
uchar cCTSat[NUM_COLOR_TYPES] =    {0,       0,      0,    255,   255,     255,     255,   255,   255,    255  };
uchar cCTVal[NUM_COLOR_TYPES] =    {0,      255,    120,   255,   255,     255,     255,   255,   255,    255  };

typedef unsigned int uint;

// число пикселей данного цвета на изображении
uint colorCount[NUM_COLOR_TYPES] = {0,		0,		0,		0,		0,		0,		0,		0,		0,		0 };

// Determine what type of color the HSV pixel is. Returns the colorType between 0 and NUM_COLOR_TYPES.
int getPixelColorType(int H, int S, int V)
{
    int color = cBLACK;

#if 1
    if (V < 75)
        color = cBLACK;
    else if (V > 190 && S < 27)
        color = cWHITE;
    else if (S < 53 && V < 185)
        color = cGREY;
    else
#endif
    {
        if (H < 7)
            color = cRED;
        else if (H < 25)
            color = cORANGE;
        else if (H < 34)
            color = cYELLOW;
        else if (H < 73)
            color = cGREEN;
        else if (H < 102)
            color = cAQUA;
        else if (H < 140)
            color = cBLUE;
        else if (H < 170)
            color = cPURPLE;
        else	// full circle
            color = cRED;	// back to Red
    }
    return color;
}

// сортировка цветов по количеству
bool colors_sort(std::pair< int, uint > a, std::pair< int, uint > b)
{
    return (a.second > b.second);
}

int calc_hsv_colors(IplImage *frame)
{
    if(!frame)
        return -1;

    IplImage* image=0, *hsv=0, *dst=0, *dst2=0, *color_indexes=0, *dst3=0;

    image = cvCloneImage(frame);
    hsv = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 3 );
    cvCvtColor( image, hsv, CV_BGR2HSV );

    // for store results
    dst = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 3 );
    dst2 = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 3 );
    color_indexes = cvCreateImage( cvGetSize(image), IPL_DEPTH_8U, 1 ); // store color indexes

    // для хранения RGB-х цветов
    CvScalar rgb_colors[NUM_COLOR_TYPES];

    int i=0, j=0, x=0, y=0;

    // reset colors
    memset(colorCount, 0, sizeof(colorCount));
    for(i=0; i<NUM_COLOR_TYPES; i++) {
        rgb_colors[i] = cvScalarAll(0);
    }

    for (y=0; y<hsv->height; y++) {
        for (x=0; x<hsv->width; x++) {

            // get HSV pixel
            uchar H = CV_PIXEL(uchar, hsv, x, y)[0];	// Hue
            uchar S = CV_PIXEL(uchar, hsv, x, y)[1];	// Saturation
            uchar V = CV_PIXEL(uchar, hsv, x, y)[2];	// Value (Brightness)

            // define pixel color type
            int ctype = getPixelColorType(H, S, V);

            // set values
            CV_PIXEL(uchar, dst, x, y)[0] = cCTHue[ctype];	// Hue
            CV_PIXEL(uchar, dst, x, y)[1] = cCTSat[ctype];	// Saturation
            CV_PIXEL(uchar, dst, x, y)[2] = cCTVal[ctype];	// Value

            // collect RGB
            rgb_colors[ctype].val[0] += CV_PIXEL(uchar, image, x, y)[0]; // B
            rgb_colors[ctype].val[1] += CV_PIXEL(uchar, image, x, y)[1]; // G
            rgb_colors[ctype].val[2] += CV_PIXEL(uchar, image, x, y)[2]; // R

            // сохраняем к какому типу относится цвет
            CV_PIXEL(uchar, color_indexes, x, y)[0] = ctype;

            // подсчитываем :)
            colorCount[ctype]++;
        }
    }

    // усреднение RGB-составляющих
    for(i=0; i<NUM_COLOR_TYPES; i++) {
        rgb_colors[i].val[0] /= colorCount[i];
        rgb_colors[i].val[1] /= colorCount[i];
        rgb_colors[i].val[2] /= colorCount[i];
    }

    // теперь загоним массив в вектор и отсортируем :)
    std::vector< std::pair< int, uint > > colors;
    colors.reserve(NUM_COLOR_TYPES);

    for(i=0; i<NUM_COLOR_TYPES; i++){
        std::pair< int, uint > color;
        color.first = i;
        color.second = colorCount[i];
        colors.push_back( color );
    }
    // сортируем
    std::sort( colors.begin(), colors.end(), colors_sort );

    // для отладки - выводим коды, названия цветов и их количество
    for(i=0; i<colors.size(); i++){
        printf("[i] color %d (%s) - %d\n", colors[i].first, sCTypes[colors[i].first], colors[i].second );
    }

    // выдаём код первых цветов
    printf("[i] color code: \n");
    for(i=0; i<NUM_COLOR_TYPES; i++)
        printf("%02d ", colors[i].first);
    printf("\n");
    printf("[i] color names: \n");
    for(i=0; i<NUM_COLOR_TYPES; i++)
        printf("%s ", sCTypes[colors[i].first]);
    printf("\n");

#if 0
    cvSaveImage("image.bmp", image);
#endif

    cvReleaseImage(&image);
    cvReleaseImage(&hsv);
    cvReleaseImage(&dst);
    cvReleaseImage(&dst2);
    cvReleaseImage(&color_indexes);
    cvReleaseImage(&dst3);

    return 0;
}

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


int main(int argc, char* argv[])
{
    printf("[i] Start...\n");

    register_signal_handlers();

    roboipc::CommunicatorServer communicator;
    roboipc::CommunicatorClient client;

    if( communicator.init(EYE_SOCKET_NAME) ) {
        fprintf(stderr, "[!] Error: cant create communication: %s!\n", EYE_SOCKET_NAME);
        return -1;
    }

    int res = 0;

#if defined(USE_KEYBOARD_INPUT) && defined(LINUX)
    // Use termios to turn off line buffering
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    setbuf(stdin, NULL);
#endif

    CmdEyeData cmd_eye;

    memset(&cmd_eye, 0, sizeof(cmd_eye));

    CvCapture* capture = cvCreateCameraCapture( 200 );
    if(!capture) {
        fprintf(stderr, "[!] Error: cant create camera capture!\n");
    }

    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, EYE_FRAME_WIDTH);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, EYE_FRAME_HEIGHT);

    double width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    double height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    printf("[i] Capture: %.0f x %.0f\n", width, height );

    IplImage* frame = 0;

    printf("[i] Capture images...\n");

    // frame counter
    int counter = 0;

    double fps = 0;
    double begin = 0;
    double end = 0;

    while( !terminated ) {

#if defined(USE_KEYBOARD_INPUT)
        int key = console::waitKey(30);
        if(key != 0 ) printf( "[i] Key: %c (%d)\n", key ,key );
        if(key == 27) { //ESC
            break;
        }
#endif //#if defined(USE_KEYBOARD_INPUT)

        begin = (double)cvGetTickCount();

        frame = cvQueryFrame( capture );

        calc_hsv_colors(frame);

        end = (double)cvGetTickCount();

        // calculate current FPS
        counter++;
        fps = (end-begin) / (cvGetTickFrequency()*1000.0);

        // will print out Inf until sec is greater than 0
        printf("[i] FPS = %.2f\n", fps);

#if 1
        SOCKET sockfd = SOCKET_ERROR;
        if( (sockfd = communicator.connected(30)) != SOCKET_ERROR ) {
            printf("[i] Client connected...\n");
            client.close();
            client.sockfd = sockfd;
        }

        if(client.sockfd != SOCKET_ERROR) {
            strncpy(cmd_eye.sig, "eyedat", CMD_SIG_SIZE);



            res = client.write(&cmd_eye, sizeof(cmd_eye));
            printf( "[i] Send Eye data (%d)...\n", res);
        }

#endif
    }  // while( !terminated ) {

    cvReleaseCapture( &capture );

    printf("[i] End.\n");

    return 0;
}
