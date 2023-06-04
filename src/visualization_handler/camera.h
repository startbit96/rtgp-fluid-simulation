#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define CAMERA_INITIAL_YAW_DEG          45.0f
#define CAMERA_INITIAL_PITCH_DEG        20.0f
#define CAMERA_MAX_PITCH_DEG            80.0f
#define CAMERA_MIN_PITCH_DEG            -80.0f
#define CAMERA_ROTATION_SENSITIVITY     1.0f
#define CAMERA_INITIAL_ZOOM             8.0f
#define CAMERA_MIN_ZOOM                 2.0f
#define CAMERA_MAX_ZOOM                 15.0f
#define CAMERA_ZOOM_SENSITIVITY         2.0f


// The camera class. In this project we do not want the user
// to move freely in the scene but just give enough freedom 
// to rotate the scene itself and zoom in / out.
// Therefore we impelement an arc ball camera.
class Camera 
{
    private:
        float yaw;
        float pitch;
        float zoom_level;
        bool first_mouse_event;
        float old_cursor_position_x;
        float old_cursor_position_y;

    public:
        float delta_time;
        glm::vec3 scene_center;
        // The rotation will be enabled / disabled by clicking the left mouse button.
        bool rotation_enabled;

        Camera ();

        void zoom (float zoom_offset);
        void rotate (float new_cursor_position_x, float new_cursor_position_y);
        glm::mat4 get_view_matrix ();
};