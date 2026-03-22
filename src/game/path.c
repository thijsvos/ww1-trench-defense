#include "path.h"
#include "../math/vec.h"
#include <string.h>

void path_init(PathSet *ps)
{
    memset(ps, 0, sizeof(*ps));
}

void path_add_waypoint(Path *path, int tile_x, int tile_y)
{
    if (path->waypoint_count >= PATH_MAX_WAYPOINTS) {
        return;
    }
    path->waypoints[path->waypoint_count] = vec2(
        (float)tile_x + 0.5f,
        (float)tile_y + 0.5f
    );
    path->waypoint_count++;
}

float path_total_length(Path *path)
{
    if (path->waypoint_count < 2) {
        return 0.0f;
    }

    float total = 0.0f;
    for (int i = 1; i < path->waypoint_count; i++) {
        total += vec2_distance(path->waypoints[i - 1], path->waypoints[i]);
    }
    return total;
}

Vec2 path_get_position(Path *path, float progress)
{
    if (path->waypoint_count == 0) {
        return vec2(0.0f, 0.0f);
    }
    if (path->waypoint_count == 1 || progress <= 0.0f) {
        return path->waypoints[0];
    }
    if (progress >= 1.0f) {
        return path->waypoints[path->waypoint_count - 1];
    }

    float total = path_total_length(path);
    if (total <= 0.0f) {
        return path->waypoints[0];
    }

    float target_dist = progress * total;
    float accumulated = 0.0f;

    for (int i = 1; i < path->waypoint_count; i++) {
        Vec2 seg_start = path->waypoints[i - 1];
        Vec2 seg_end   = path->waypoints[i];
        float seg_len  = vec2_distance(seg_start, seg_end);

        if (accumulated + seg_len >= target_dist) {
            float remaining = target_dist - accumulated;
            float t = (seg_len > 0.0f) ? remaining / seg_len : 0.0f;
            return vec2_add(seg_start, vec2_scale(vec2_sub(seg_end, seg_start), t));
        }

        accumulated += seg_len;
    }

    /* Fallback: return last waypoint */
    return path->waypoints[path->waypoint_count - 1];
}
