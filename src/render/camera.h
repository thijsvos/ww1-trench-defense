#ifndef TD_CAMERA_H
#define TD_CAMERA_H

#include "../math/math_types.h"
#include "../math/mat4.h"

typedef struct Camera {
    Vec3 position;     /* world position the camera looks at (target point) */
    float distance;    /* distance from target */
    float pitch;       /* tilt angle in radians (30 deg for isometric) */
    float yaw;         /* rotation around Y in radians (45 deg for isometric) */
    float zoom;        /* orthographic zoom factor */
    float zoom_min, zoom_max;
    Mat4 view;
    Mat4 projection;
    Mat4 view_projection;
    Vec3 right;        /* camera right vector in world space (for billboards) */
    Vec3 up_dir;       /* camera up vector in world space (for billboards) */
    int viewport_width, viewport_height;
    /* Screen shake */
    float shake_intensity; /* current magnitude, decays over time */
    float shake_timer;     /* remaining shake time in seconds */
} Camera;

void camera_init(Camera *cam, int viewport_w, int viewport_h);
void camera_update(Camera *cam);
void camera_pan(Camera *cam, float dx, float dz);
void camera_set_zoom(Camera *cam, float zoom);
void camera_shake(Camera *cam, float intensity, float duration);
Vec3 camera_screen_to_world(Camera *cam, float screen_x, float screen_y);

#endif /* TD_CAMERA_H */
