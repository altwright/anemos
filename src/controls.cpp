#include "controls.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include "timing.h"

CameraControls cam_createControls()
{
    CameraControls cam = {
        .position = {3.0f, 3.0f, 3.0f},
        .right = {1.0f, 0.0f, 0.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .forward = {0.0f, 0.0f, 1.0f}};

    vec3 focus = GLM_VEC3_ZERO_INIT;
    vec3 up = {0.0f, 0.0f, 1.0f};
    glm_quat_forp(cam.position, focus, up, cam.globalOri);
    glm_quat_inv(cam.globalOri, cam.globalOri);

    cam.rad_s = glm_rad(90.0f);

    return cam;
}

void cam_setInputHandler(CameraControls *cam, InputHandler *handler)
{
    handler->ctx = cam;
    handler->w = &cam_handleKeyW;
    handler->a = &cam_handleKeyA;
    handler->s = &cam_handleKeyS;
    handler->d = &cam_handleKeyD;
    handler->scroll = &cam_handleMouseScroll;
}

Matrix4 cam_genViewMatrix(CameraControls *cam)
{
    Matrix4 view = {GLM_MAT4_IDENTITY_INIT};

    vec3 negPosition = {};
    glm_vec3_negate_to(cam->position, negPosition);
    mat4 translation = GLM_MAT4_IDENTITY_INIT;
    glm_translate_make(translation, negPosition);
    mat4 rotation = GLM_MAT4_IDENTITY_INIT;
    glm_quat_mat4(cam->globalOri, rotation);
    glm_mat4_mul_sse2(rotation, translation, view.matrix);

    return view;
}

Matrix4 cam_genProjectionMatrix(CameraControls *cam, VkExtent2D renderArea)
{
    Matrix4 proj = {};
    glm_perspective(glm_rad(45.0f), renderArea.width / (float)renderArea.height, 0.1f, 10.0f, proj.matrix);
    proj.matrix[1][1] *= -1;//Originally designed for OpenGL, so must be inverted
    return proj;
}

static s64 orbitCamLaterally(CameraControls *cam, s64 startTimeNs, float rad_s)
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    s64 current_ns = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiff_ns = current_ns - startTimeNs;

    float angle = timeDiff_ns * rad_s / SEC_TO_NS(1);

    versor invGlobalOri = {};
    glm_quat_inv(cam->globalOri, invGlobalOri);
    vec3 relUp = {};
    glm_quat_rotatev(invGlobalOri, cam->up, relUp);
    glm_vec3_rotate(cam->position, angle, relUp);
    //printf("{%.2f, %.2f, %.2f}\n", relUp[0], relUp[1], relUp[2]);

    //Replaces contents of first param
    glm_quatv(invGlobalOri, -1*angle, cam->up);
    glm_quat_mul_sse2(invGlobalOri, cam->globalOri, cam->globalOri);

    return current_ns;
}

static s64 orbitCamLongitudinally(CameraControls *cam, s64 start_ns, float rad_s)
{
    timespec currentTime = {};
    checkGetTime(clock_gettime(TIMING_CLOCK, &currentTime));
    s64 current_ns = SEC_TO_NS(currentTime.tv_sec) + currentTime.tv_nsec;
    s64 timeDiff_ns = current_ns - start_ns;

    float angle = timeDiff_ns * rad_s / SEC_TO_NS(1);

    versor invGlobalOri = {};
    glm_quat_inv(cam->globalOri, invGlobalOri);
    vec3 relRight = {};
    glm_quat_rotatev(invGlobalOri, cam->right, relRight);
    glm_vec3_rotate(cam->position, angle, relRight);
    //printf("{%.2f, %.2f, %.2f}\n", relRight[0], relRight[1], relRight[2]);

    //Replaces contents of first param
    glm_quatv(invGlobalOri, -1*angle, cam->right);
    glm_quat_mul_sse2(invGlobalOri, cam->globalOri, cam->globalOri);

    return current_ns;
}

void cam_processInput(CameraControls *cam)
{
    if (cam->wPressed){
        if (!cam->wPressedStart_ns)
            cam->wPressedStart_ns = getCurrentTime_ns();
        else
            cam->wPressedStart_ns = orbitCamLongitudinally(cam, cam->wPressedStart_ns, -1*cam->rad_s);
    }
    else
        cam->wPressedStart_ns = 0;

    if (cam->aPressed){
        if (!cam->aPressedStart_ns)
            cam->aPressedStart_ns = getCurrentTime_ns();
        else
            cam->aPressedStart_ns = orbitCamLaterally(cam, cam->aPressedStart_ns, -1*cam->rad_s);
    }
    else 
        cam->aPressedStart_ns = 0;

    if (cam->sPressed){
        if (!cam->sPressedStart_ns)
            cam->sPressedStart_ns = getCurrentTime_ns();
        else
            cam->sPressedStart_ns = orbitCamLongitudinally(cam, cam->sPressedStart_ns, cam->rad_s);
    }
    else
        cam->sPressedStart_ns = 0;

    if (cam->dPressed){
        if (!cam->dPressedStart_ns)
            cam->dPressedStart_ns = getCurrentTime_ns();
        else
            cam->dPressedStart_ns = orbitCamLaterally(cam, cam->dPressedStart_ns, cam->rad_s);
    }
    else 
        cam->dPressedStart_ns = 0;
}

void cam_handleKeyW(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->wPressed = true;
    }
    else{
        cam->wPressed = false;
    }
}

void cam_handleKeyS(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->sPressed = true;
    }
    else{
        cam->sPressed = false;
    }
}

void cam_handleKeyD(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->dPressed = true;
    }
    else{
        cam->dPressed = false;
    }
}

void cam_handleKeyA(void *ctx, int action, int mods)
{
    CameraControls *cam = (CameraControls*)ctx;

    if (action != GLFW_RELEASE){
        cam->aPressed = true;
    }
    else{
        cam->aPressed = false;
    }
}  

void cam_handleMouseScroll(void *ctx, double offset)
{
    printf("%f\n", offset);
}