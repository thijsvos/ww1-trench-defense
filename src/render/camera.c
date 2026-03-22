#define _USE_MATH_DEFINES
#include <math.h>

#include "camera.h"
#include "../math/vec.h"

#define PIXELS_PER_UNIT 64.0f
#define DEG_TO_RAD(d) ((d) * (float)M_PI / 180.0f)

void camera_init(Camera *cam, int viewport_w, int viewport_h)
{
    cam->position = vec3(0.0f, 0.0f, 0.0f);
    cam->distance = 20.0f;
    cam->pitch    = DEG_TO_RAD(30.0f);
    cam->yaw      = DEG_TO_RAD(45.0f);
    cam->zoom     = 1.0f;
    cam->zoom_min = 0.5f;
    cam->zoom_max = 3.0f;

    cam->viewport_width  = viewport_w;
    cam->viewport_height = viewport_h;

    cam->view            = mat4_identity();
    cam->projection      = mat4_identity();
    cam->view_projection = mat4_identity();

    camera_update(cam);
}

void camera_update(Camera *cam)
{
    /* 1. Compute eye position from spherical coordinates around target. */
    float sp = sinf(cam->pitch);
    float cp = cosf(cam->pitch);
    float sy = sinf(cam->yaw);
    float cy = cosf(cam->yaw);

    Vec3 offset = vec3(sy * cp, sp, cy * cp);
    Vec3 eye    = vec3_add(cam->position, vec3_scale(offset, cam->distance));

    /* 2. Build view matrix. */
    Vec3 up = vec3(0.0f, 1.0f, 0.0f);
    cam->view = mat4_look_at(eye, cam->position, up);

    /* 2b. Compute billboard basis vectors.
       Right = perpendicular to view direction on the XZ ground plane.
       Up = world Y (cylindrical billboard — sprites stay upright). */
    cam->right = vec3_normalize(vec3(-cy, 0.0f, sy));
    cam->up_dir = vec3(0.0f, 1.0f, 0.0f);

    /* 3. Orthographic projection. */
    float hw = (float)cam->viewport_width  / (2.0f * cam->zoom * PIXELS_PER_UNIT);
    float hh = (float)cam->viewport_height / (2.0f * cam->zoom * PIXELS_PER_UNIT);
    cam->projection = mat4_ortho(-hw, hw, -hh, hh, -100.0f, 100.0f);

    /* 4. Combined view-projection. */
    cam->view_projection = mat4_multiply(cam->projection, cam->view);
}

void camera_pan(Camera *cam, float dx, float dz)
{
    cam->position.x += dx;
    cam->position.z += dz;
}

void camera_set_zoom(Camera *cam, float zoom)
{
    if (zoom < cam->zoom_min) zoom = cam->zoom_min;
    if (zoom > cam->zoom_max) zoom = cam->zoom_max;
    cam->zoom = zoom;
}

Vec3 camera_screen_to_world(Camera *cam, float screen_x, float screen_y)
{
    /* Convert screen coordinates to NDC (-1..1). */
    float ndc_x =  (2.0f * screen_x / (float)cam->viewport_width)  - 1.0f;
    float ndc_y = -(2.0f * screen_y / (float)cam->viewport_height) + 1.0f;

    /* Inverse view-projection. */
    Mat4 inv_vp = mat4_inverse(cam->view_projection);

    /* Unproject two points along the ray (near and far). */
    Vec4 near_ndc = vec4(ndc_x, ndc_y, -1.0f, 1.0f);
    Vec4 far_ndc  = vec4(ndc_x, ndc_y,  1.0f, 1.0f);

    Vec4 near_world = mat4_mul_vec4(inv_vp, near_ndc);
    Vec4 far_world  = mat4_mul_vec4(inv_vp, far_ndc);

    /* Perspective divide. */
    if (near_world.w != 0.0f) {
        near_world.x /= near_world.w;
        near_world.y /= near_world.w;
        near_world.z /= near_world.w;
    }
    if (far_world.w != 0.0f) {
        far_world.x /= far_world.w;
        far_world.y /= far_world.w;
        far_world.z /= far_world.w;
    }

    /* Ray direction. */
    Vec3 ray_origin = vec3(near_world.x, near_world.y, near_world.z);
    Vec3 ray_end    = vec3(far_world.x,  far_world.y,  far_world.z);
    Vec3 ray_dir    = vec3_sub(ray_end, ray_origin);

    /* Intersect with Y = 0 plane: origin.y + t * dir.y = 0 */
    if (fabsf(ray_dir.y) < 1e-6f) {
        /* Ray is parallel to the ground plane; return target as fallback. */
        return cam->position;
    }

    float t = -ray_origin.y / ray_dir.y;
    return vec3(ray_origin.x + t * ray_dir.x,
                0.0f,
                ray_origin.z + t * ray_dir.z);
}
