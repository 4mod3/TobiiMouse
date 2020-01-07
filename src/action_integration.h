#ifndef ACTION_INTEGRATION_H
#define ACTION_INTEGRATION_H

#include <tuple>
#include "tobiimouse.h"
#include "noise_cancellation.h"
#include "tobii/tobii_streams.h"

#ifdef __linux__
#include <X11/X.h>
#include "X11/Xlib.h"
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xrandr.h>
#elif _WIN32
#include <windows.h>
#include <winuser.h>
// windows code goes here
#else
#error "Unknown Operating System..."
#endif

using namespace std;

namespace ActionIntegration
{
void init();
void MoveMouseTo(int x, int y);
tuple<int, int> ProcessGazePosition(float x, float y);
void OnGaze(float x, float y);
void OnClick(tobii_validity_t left_valid, tobii_validity_t right_valid, int64_t time_stamp);
void HeadRot(float r_x, float r_y, int64_t timestamp);
void HeadActionHandle();

typedef enum head_pose_states
{
    HOLD,
    NONE_ACTION,
    TURN_LEFT,
    TURN_RIGHT,
    RISE_UP,
    NOD_DOWN
} head_pose_states;
#ifdef _WIN32
WINBOOL CALLBACK EnumMonitors_CALLBACK(HMONITOR a,HDC b,LPRECT c,LPARAM d);
#endif

};

#endif // MOUSE_INTEGRATION_H
