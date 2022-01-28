#pragma once

#include "definitions.h"

class TrackballCamera
{
public:
	// Creates an instance of a trackball camera
	// The default options proved to be reasonable
	// * int width - window width
	// * int height - window height
	// * float sens - mouse movement sensitivity
	// * float r - initial radius for camera
	// * float zoomsens - sensitivity for forward movement of the camera
	// * glm::vec3 center - center of the trackball camera
	// * float fov - field of view in degree
	// * float near - near value for projection
	// * float far - far value for projection
	TrackballCamera(int width, int height, float sens = 0.01f, float r = 2.0, float zoomsens = 1.0f, glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f), float fov = 60.0f, float near = 0.001f, float far = 1000.0f);
	~TrackballCamera();

	glm::mat4 getViewMatrix();
	glm::mat4 getViewLMatrix();
	glm::mat4 getProjectionMatrix();

	// Updates the camera view using mouse controls
	// * GLFWwindow* window - window to access mouse position and controls
	// * float dt - delta time between camera updates
	void update(GLFWwindow* window, float dt);

private:
	//camera
	glm::vec3 m_center;
	glm::vec3 m_cameraPos;
	glm::mat4 m_view;
	glm::mat4 m_projection;
	float m_sens;
	float m_zoomsens;
	float m_radius;
	double m_theta;
	double m_phi;
	double m_x;
	double m_y;
	double m_oldX;
	double m_oldY;
	int m_width;
	int m_height;
	bool m_lmb_pressed;

	 //light
	glm::vec4 m_lightPos;
	glm::mat4 m_viewL;
	glm::mat4 m_projectionL;
	float m_sensL;
	float m_zoomsensL;
	float m_radiusL;
	double m_thetaL;
	double m_phiL;
	double m_xL;
	double m_yL;
	double m_oldXL;
	double m_oldYL;


};
