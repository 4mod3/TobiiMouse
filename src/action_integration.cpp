#include "action_integration.h"
#include <iostream>
#include <vector>

namespace ActionIntegration
{
static int ScreenHeight;
static int ScreenWidth;

static int LastX;
static int LastY;
static tobii_validity_t LastLeft;
static tobii_validity_t LastRight;
static int64_t ClickStamp;
static bool click_flag;

static bool HeadEnabled = FALSE;
static head_pose_states HeadState = HOLD;
static int64_t HeadStamp;
static bool head_action_flag;
#ifdef __linux__
static Display* _display;
static Window _root_window;
#elif _WIN32
static vector<RECT> Monitors;
#endif
}

#ifdef _WIN32
WINBOOL CALLBACK ActionIntegration::EnumMonitors_CALLBACK(HMONITOR hmonitor, HDC hdc,LPRECT lPRect, LPARAM _param){
    UNUSED(hmonitor);
    UNUSED(hdc);
    UNUSED(_param);
    Monitors.insert(Monitors.end(), *lPRect);
    ScreenWidth = lPRect->right - lPRect->left;
    ScreenHeight = lPRect->bottom - lPRect->top;
    return FALSE; // We currently only support the first one...
}
#endif

void ActionIntegration::init()
{
    NoiseCancellation::init();
    // TODO: Multiple Display supports.
#ifdef __linux__
    _display = XOpenDisplay(None);
    _root_window = XRootWindow(_display, 0);
    XRRScreenResources *screens = XRRGetScreenResources(_display, DefaultRootWindow(_display));
    XRRCrtcInfo *info = XRRGetCrtcInfo(_display, screens, screens->crtcs[0]);
    ScreenWidth = static_cast<int>(info->width);
    ScreenHeight = static_cast<int>(info->height);
    XRRFreeCrtcInfo(info);
    XRRFreeScreenResources(screens);
#elif _WIN32
    EnumDisplayMonitors(nullptr, nullptr, EnumMonitors_CALLBACK, NULL);
#endif
}


void ActionIntegration::MoveMouseTo(int x, int y)
{
#ifdef __linux__
    XTestFakeMotionEvent(_display, 0, x, y, 0);
    XFlush(_display);
#elif _WIN32
    SetCursorPos(x, y);
#endif
}

tuple<int, int> ActionIntegration::ProcessGazePosition(float x, float y)
{
    float width = ScreenWidth;
    float height = ScreenHeight;

    auto realGazeX = width * x;
    auto realGazeY = height * y;

    auto filtered = NoiseCancellation::CancelNoise(realGazeX,realGazeY);
    //auto count = ScreenCount(_display); //Get total screen count.
    float gazeX_f = get<0>(filtered);
    float gazeY_f = get<1>(filtered);


    return tuple<int, int>(static_cast<int>(gazeX_f), static_cast<int>(gazeY_f));
}

void ActionIntegration::HeadActionHandle()
{
    switch (HeadState) {
        case TURN_LEFT:
            keybd_event('A',0,0,0);
            keybd_event('A',0,KEYEVENTF_KEYUP,0);
        break;
        case TURN_RIGHT:
            keybd_event('D',0,0,0);
            keybd_event('D',0,KEYEVENTF_KEYUP,0);
        break;
        case NOD_DOWN:
            keybd_event('S',0,0,0);
            keybd_event('S',0,KEYEVENTF_KEYUP,0);
        break;
        case RISE_UP:
            keybd_event('W',0,0,0);
            keybd_event('W',0,KEYEVENTF_KEYUP,0);
        break;
        default:
        break;
    }
    head_action_flag = FALSE;
}

void ActionIntegration::OnGaze(float x, float y)
{
    auto data = ProcessGazePosition(x,y);
    auto posiX = get<0>(data);
    auto posiY = get<1>(data);

    MoveMouseTo(posiX, posiY);
    LastX = posiX;
    LastY = posiY;
}

void ActionIntegration::OnClick(tobii_validity_t left_valid, tobii_validity_t right_valid, int64_t time_stamp)
{

    if(left_valid ^ right_valid)
    {
        if(LastLeft & LastRight){
            ClickStamp = time_stamp;
            click_flag = TRUE;
        }else if(!(LastLeft ^ left_valid) && (time_stamp - ClickStamp > 200000) && click_flag)
        {
            switch (left_valid) {
                case TOBII_VALIDITY_VALID:
                    HeadEnabled = !HeadEnabled;
                break;
                case TOBII_VALIDITY_INVALID:
                    mouse_event(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP,0,0,0,0);
                break;
            }
            click_flag = FALSE;
        }
    }
    click_flag = (left_valid & right_valid)? FALSE : click_flag;

    //update LastValid
    LastLeft = left_valid;
    LastRight = right_valid;
}

void ActionIntegration::HeadRot(float r_x, float r_y, int64_t time_stamp)
{
    //rotation priority: left > right > down > up
    auto instant_state = r_y > 0.6 ? TURN_LEFT:(
                                          r_y < -0.4 ? TURN_RIGHT:(
                                                           r_x < -0.4 ? NOD_DOWN:(
                                                                            r_x > 0.1 ? RISE_UP:NONE_ACTION)));

    switch (HeadState) {
        case HOLD:
            HeadState = (instant_state == NONE_ACTION) ? NONE_ACTION : HOLD;
        break;
        case NONE_ACTION:
            if(instant_state != NONE_ACTION){
                head_action_flag = TRUE;
                HeadStamp = time_stamp;
                HeadState = instant_state;
            }
        break;
        default:
            auto time_gap = time_stamp - HeadStamp;
            if(time_gap > 1500000)
            {
                HeadState = HOLD;
                head_action_flag = FALSE;
            }else if(instant_state == NONE_ACTION){
                //action
                if((time_gap > 200000) && head_action_flag){
                    HeadActionHandle();
                }
                HeadState = instant_state;
            }
    }
}


