//
// definitions for robopet
//
//
// http://robocraft.ru
//

#ifndef _ROBOPET_H_
#define _ROBOPET_H_

#include "commands.h"

#define DRIVER_2WD_SOCKET_NAME  "/tmp/socket_2wd_driver"
#define SPEECHER_SOCKET_NAME    "/tmp/socket_speecher"
#define EYE_SOCKET_NAME         "/tmp/socket_eye"

// pet
#define SOUND_FOR_DETECTION     "./snd/dog_growl.wav"
#define SOUND_FOR_NEAR          "./snd/dog_bark2.wav"
//Elwin
#define SOUND_FOR_DETECTION2     "./snd/privet.wav"
#define SOUND_FOR_NEAR2          "./snd/jaelwin.wav"
#define SOUND_FOR_INFO           "./snd/rythm-robot.wav"

#define EYE_FRAME_WIDTH     320
#define EYE_FRAME_HEIGHT    240

#endif //#ifndef _ROBOPET_H_
