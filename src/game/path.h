#ifndef TD_PATH_H
#define TD_PATH_H

#include "../math/math_types.h"

#define PATH_MAX_WAYPOINTS 64
#define MAX_PATHS 4

typedef struct Path {
    Vec2 waypoints[PATH_MAX_WAYPOINTS];  /* world-space positions (center of tiles) */
    int waypoint_count;
} Path;

typedef struct PathSet {
    Path paths[MAX_PATHS];
    int path_count;
} PathSet;

void path_init(PathSet *ps);
/* Add a waypoint to a path. x,y are tile coordinates - stored as center of tile (x+0.5, y+0.5) */
void path_add_waypoint(Path *path, int tile_x, int tile_y);
/* Get interpolated position along path given progress 0.0-1.0 */
Vec2 path_get_position(Path *path, float progress);
float path_total_length(Path *path);

#endif /* TD_PATH_H */
