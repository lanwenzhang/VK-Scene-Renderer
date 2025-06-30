#pragma once
#include "../../common.h"
#include "../../core/type.h"
namespace LZ::Renderer {


	class Camera {
	public:

		Camera();
		~Camera();

		void setMouseControl(bool enable) { mEnableMouseControl = enable; }
		void setSpeed(float speed){ mSpeed = speed;}
		void setPosition(const glm::vec3& pos);
		void setPerpective(float angle, float ratio, float near, float far);
		void setSentitivity(float s);

		void lookAt(glm::vec3 pos, glm::vec3 front, glm::vec3 up);
		void update();
		void updatePitch(float deltaAngle);

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectMatrix();
		glm::mat4 getViewProjectionMatrix();


		void move(LZ::Core::CAMERA_MOVE mode);

		void pitch(float yOffset);
		void yaw(float xOffset);
	
		void onMouseMove(double xpos, double ypos);


	public:
		float       mPitchAngle;
		bool        mEnableMouseControl = false;

	private:
		glm::vec3	mPosition;
		glm::vec3	mFront;
		glm::vec3	mUp;
		float		mSpeed;

		float		mPitch;
		float		mYaw;
		float		mSensitivity;

		float		mXPos;
		float       mYPos;
		bool		mFirstMove;

		glm::mat4	mViewMatrix;
		glm::mat4	mProjectionMatrix;
	};
}


