#pragma once
#include "base.h"


class Camera {
public:
	Camera()
	{
		m_position = glm::vec3(1.0f);
		m_front = glm::vec3(1.0f);
		m_up = glm::vec3(1.0f);
		m_speed = 0.01f;

		m_pitch = 0.0f;
		m_yaw = -90.0f;
		m_sensitivity = 0.1f;

		m_xpos = 0;
		m_ypos = 0;
		m_firstMove = true;

		m_vMatrix = glm::mat4(1.0f);
	}

	~Camera(){}

	void setMouseControl(bool enable) { m_enableMouseControl = enable; }
	void lookAt(glm::vec3 _pos, glm::vec3 _front, glm::vec3 _up);
	void update();
	void updatePitch(float deltaAngle);

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectMatrix();
	glm::mat4 getViewProjectionMatrix();


	void move(CAMERA_MOVE _mode);
	void setSpeed(float _speed)
	{
		m_speed = _speed;
	}

	void pitch(float _yOffset);
	void yaw(float _xOffset);
	void setSentitivity(float _s);
	void setPosition(const glm::vec3& pos);

	void onMouseMove(double _xpos, double _ypos);

	void setPerpective(float angle, float ratio, float near, float far);


public:
	float       m_pitchAngle;
private:

	glm::vec3	m_position;
	glm::vec3	m_front;
	glm::vec3	m_up;
	float		m_speed;

	float		m_pitch;
	float		m_yaw;
	float		m_sensitivity;

	float		m_xpos;
	float       m_ypos;
	bool		m_firstMove;

	glm::mat4	m_vMatrix;
	glm::mat4	m_pMatrix;

	bool m_enableMouseControl = false;
};

