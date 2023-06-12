#include "camera.h"

#include <iostream>
#include <math.h>

Camera::Camera ()
{
    // Set delta time to some initial value, the value will later be updated
    // every frame by the visualization handler.
    this->delta_time = 0.1f;
    // The scene center will be at first at the global coordinate system origin.
    // When a scene is loaded by the scene handler, the application will set the 
    // scene center of the camera based on the simulation space of the current scene.
    this->scene_center = glm::vec3(0.0f, 0.0f, 0.0f);
    // Initial position of the camera.
    this->yaw = CAMERA_INITIAL_YAW_DEG;
    this->pitch = CAMERA_INITIAL_PITCH_DEG;
    this->zoom_level = CAMERA_INITIAL_ZOOM;
    // We only allow the rotation to be executed, when the left mouse button is clicked.
    // So this value has to be set by the application whenever the mouse button is clicked
    // and has to be unset afterwards.
    this->rotation_enabled = false;
    // We need to store the last x and y position of the cursor.
    // Also we need to know if this is the first cursor event, so we can handle the first
    // event where no data from before is known.
    this->first_mouse_event = true;
}

void Camera::zoom (float zoom_offset)
{
    this->zoom_level = this->zoom_level - zoom_offset * this->delta_time * CAMERA_ZOOM_SENSITIVITY;
    if (this->zoom_level < CAMERA_MIN_ZOOM) {
        this->zoom_level = CAMERA_MIN_ZOOM;
    }
    else if (this->zoom_level > CAMERA_MAX_ZOOM) {
        this->zoom_level = CAMERA_MAX_ZOOM;
    }
}

void Camera::rotate (float new_cursor_position_x, float new_cursor_position_y)
{
    if (this->first_mouse_event == true) {
        // In the very first event we can not do anything, just save the positions.
        this->old_cursor_position_x = new_cursor_position_x;
        this->old_cursor_position_y = new_cursor_position_y;
        this->first_mouse_event = false;
        return;
    }
    if (this->rotation_enabled == false) {
        // Only save the mouse positions but then return.
        this->old_cursor_position_x = new_cursor_position_x;
        this->old_cursor_position_y = new_cursor_position_y;
        return;
    }
    // Calculate the offsets.
    float x_offset = new_cursor_position_x - this->old_cursor_position_x;
    float y_offset = new_cursor_position_y - this->old_cursor_position_y;
    // Apply the offsets but also keep the sensitivity in mind.
    // In order to make sure the yaw value does not gets too big or too low, do a modulo operation.
    this->yaw = fmod(this->yaw + x_offset * CAMERA_ROTATION_SENSITIVITY, 360.0f);
    this->pitch = this->pitch + y_offset * CAMERA_ROTATION_SENSITIVITY;
    // The pitch values are restricted to a specified range. Make sure, the value is within this range.
    if (this->pitch < CAMERA_MIN_PITCH_DEG) {
        this->pitch = CAMERA_MIN_PITCH_DEG;
    }
    else if (this->pitch > CAMERA_MAX_PITCH_DEG) {
        this->pitch = CAMERA_MAX_PITCH_DEG;
    }
    // Save the now old mouse positions.
    this->old_cursor_position_x = new_cursor_position_x;
    this->old_cursor_position_y = new_cursor_position_y;
}

glm::vec3 Camera::get_camera_position ()
{
    // Calculate the normalized camera position vector, multiply it by the
    // zoom level and move it according to the scene scenter.
    return glm::vec3 (
        cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch)),
        sin(glm::radians(this->pitch)),
        sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch))
    ) * this->zoom_level + this->scene_center;
}

glm::mat4 Camera::get_view_matrix () 
{
    // Calculate the position of the camera based on yaw and pitch.
    glm::vec3 camera_position = this->get_camera_position();
    // Calculate the view vector. Therefore normalize and invert the direction.
    glm::vec3 view_vector = glm::normalize(-camera_position);
    // We also need the view up vector, calculate it using the cross product.
    glm::vec3 world_up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 view_right = glm::normalize(glm::cross(view_vector, world_up));
    glm::vec3 view_up = glm::normalize(glm::cross(view_right, view_vector));  
    return glm::lookAt(
        camera_position, 
        camera_position + view_vector, 
        view_up
    );
}