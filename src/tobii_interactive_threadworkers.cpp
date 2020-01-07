#include "tobii_interactive.h"
#include <assert.h>
#include <tuple>
#include "mainwindow.h"
#include "action_integration.h"

using namespace TobiiInteractive;

static tobii_device_t* device;

void TobiiInteractive::connectivityWorker::doWork(void* data1, void* data2) {
    device = reinterpret_cast<tobii_device_t*>(data1);
    auto enabled = reinterpret_cast<bool*>(data2);
    while(*enabled)
    {
        // Do a timed blocking wait for new gaze data, will time out after some hundred milliseconds

        // I don't know why they have two different functions for Windows and Linux.....
        // See: tobii.h:131
#ifdef __WIN32
        auto error = tobii_wait_for_callbacks( nullptr, 1, &device ); //The nullptr is for &tobii_engine
#else
        auto error = tobii_wait_for_callbacks( 1, &device ); //Linux API Provides no such tobii_engine implementation. AFAIK.
#endif
        if( error == TOBII_ERROR_TIMED_OUT ) continue;
        if( error == TOBII_ERROR_CONNECTION_FAILED )
        {
            // Block here while attempting reconnect, if it fails, exit the thread
            error = reconnect( device );
            if( error != TOBII_ERROR_NO_ERROR )
            {
                    std::cerr << "Connection was lost and reconnection failed." << std::endl;
                    return;
            }
            continue;
        }
        else if( error != TOBII_ERROR_NO_ERROR )
        {
            std::cerr << "tobii_wait_for_callbacks failed: " << tobii_error_message( error ) << "." << std::endl;
            return;
        }
        // Calling this function will execute the subscription callback functions
        error = tobii_device_process_callbacks( device );
        if( error == TOBII_ERROR_CONNECTION_FAILED )
        {
            // Block here while attempting reconnect, if it fails, exit the thread
            error = reconnect( device );
            if( error != TOBII_ERROR_NO_ERROR )
            {
                std::cerr << "Connection was lost and reconnection failed." << std::endl;
                return;
            }
            continue;
        }
        else if( error != TOBII_ERROR_NO_ERROR )
        {
            std::cerr << "tobii_device_process_callbacks failed: " << tobii_error_message( error ) << "." << std::endl;
            emit ResultReady(nullptr);
        }
    }
}

void TobiiInteractive::gazeWorker::doWork(void* data1, void* data2){

    //&HeadEnabled, &GazeEnabled, This (used in slot emitter)
    tuple<bool*, bool*, QThreadWorker*> pointers (reinterpret_cast<bool*>(data1), reinterpret_cast<bool*>(data2), this);

    //subscribe the gaze point stream
    auto err = tobii_gaze_point_subscribe( device,
        []( tobii_gaze_point_t const* gaze_point, void* user_data ) // user_data is tuple<MainWindow*, bool*, QThreadWorker*>
        {
            auto data_collection = *reinterpret_cast<tuple<bool*, bool*, QThreadWorker*>*>(user_data);
            auto enabled = get<1>(data_collection);

            if(*enabled && gaze_point->validity == TOBII_VALIDITY_VALID)
            {
                ActionIntegration::OnGaze(gaze_point->position_xy[0], gaze_point->position_xy[1]);
            }
        }, &pointers);

    if( err != TOBII_ERROR_NO_ERROR )
    {
        std::cerr << "Failed to subscribe to gaze stream." << tobii_error_message(err) << std::endl;
    }

    //subscribe the origin data for detect the blink callback
    err = tobii_gaze_origin_subscribe( device,
        []( tobii_gaze_origin_t const* gaze_point, void* user_data ) // user_data is tuple<MainWindow*, bool*, QThreadWorker*>
        {
            auto data_collection = *reinterpret_cast<tuple<bool*, bool*, QThreadWorker*>*>(user_data);
            //auto mwindow = get<0>(data_collection);
            auto enabled = get<1>(data_collection);

            if(*enabled && (gaze_point->left_validity || gaze_point->right_validity)){
                ActionIntegration::OnClick(gaze_point->left_validity, gaze_point->right_validity, gaze_point->timestamp_us);
            }
        }, &pointers);

    if( err != TOBII_ERROR_NO_ERROR )
    {
       std::cerr << "Failed to subscribe to origin gaze stream." << tobii_error_message(err) << std::endl;
    }

    //subscribe the head pose change callback
    err = tobii_head_pose_subscribe( device,
        []( tobii_head_pose_t const* head_pose, void* user_data ) // user_data is tuple<MainWindow*, bool*, QThreadWorker*>
        {
            auto data_collection = *reinterpret_cast<tuple<bool*, bool*, QThreadWorker*>*>(user_data);
            auto head_enabled = get<0>(data_collection);
            auto enabled = get<1>(data_collection);

            if(*enabled && *head_enabled && head_pose->rotation_validity_xyz[0]){
                ActionIntegration::HeadRot(head_pose->rotation_xyz[0], head_pose->rotation_xyz[1], head_pose->timestamp_us);
            }
        }, &pointers);

    if( err != TOBII_ERROR_NO_ERROR )
    {
       std::cerr << "Failed to subscribe to head pose stream." << tobii_error_message(err) << std::endl;
    }
}

void TobiiInteractive::HandleConnectivityCallback(void* data){
    assert(data == nullptr);
}

void TobiiInteractive::HandleGazeCallback(void* data){
    auto data_collection = reinterpret_cast<tuple<MainWindow*, float, float>*>(data);
    auto x = get<1>(*data_collection);
    auto y = get<2>(*data_collection);
    ActionIntegration::OnGaze(x, y);
}
