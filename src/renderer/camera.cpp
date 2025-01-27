//
// Created by Gianni on 21/01/2025.
//

#include "camera.hpp"

Camera::Camera(glm::vec3 position, float fovY, float width, float height, float nearZ, float farZ)
    : mView()
    , mBasis()
    , mPosition(position)
    , mTheta()
    , mPhi()
    , mProjection(glm::perspective(fovY, width / height, nearZ, farZ))
    , mFovY(fovY)
    , mAspectRatio(width / height)
    , mNearZ(nearZ)
    , mFarZ(farZ)
    , mPreviousMousePos()
    , mState(Camera::State::FIRST_PERSON)
    , mSpeed(5.f)
    , mPanSensitivity(0.8f)
{
    calculateViewProjection();
}

void Camera::setFov(float fovY)
{
    mFovY = fovY;
    updateProjection();
}

void Camera::setNearPlane(float nearZ)
{
    mNearZ = nearZ;
    updateProjection();
}

void Camera::setFarPlane(float farZ)
{
    mFarZ = farZ;
    updateProjection();
}

void Camera::setState(Camera::State state)
{
    mState = state;
}

void Camera::handleEvent(const Event &event)
{
    //event.type == Event::Resized
    if (const auto e = event.getIf<Event::WindowResize>())
        resize(e->width, e->height);

    if (const auto e = event.getIf<Event::MouseWheel>())
        scroll(e->x, e->y);
}

void Camera::update(float dt)
{
    switch (mState)
    {
        case FIRST_PERSON:
            updateFirstPerson(dt);
            break;
        case VIEW_MODE:
            updateViewMode(dt);
            break;
        case EDITOR_MODE:
            updateEditorMode(dt);
            break;
    }

    mPreviousMousePos = Input::mousePosition();
}

const glm::mat4 &Camera::viewProjection() const
{
    return mViewProjection;
}

const glm::mat4 &Camera::view() const
{
    return mView;
}

const glm::mat4 &Camera::projection() const
{
    return mProjection;
}

const glm::vec3 &Camera::position() const
{
    return mPosition;
}

const glm::vec3 &Camera::front() const
{
    return mBasis[2];
}

void Camera::resize(uint32_t width, uint32_t height)
{
    if (width && height)
    {
        mAspectRatio = static_cast<float>(width) / static_cast<float>(height);
        updateProjection();
    }
}

void Camera::scroll(float x, float y)
{
    switch (mState)
    {
        case FIRST_PERSON:
        {
            mSpeed += y;
            break;
        }
        case VIEW_MODE:
        case EDITOR_MODE:
        {
            mPosition += mBasis[2] * y; // todo: tweak
            calculateViewProjection();
            break;
        }
    }
}

void Camera::updateFirstPerson(float dt)
{
    int z = 0;
    int x = 0;

    if (Input::keyPressed(GLFW_KEY_W))
        --z;
    if (Input::keyPressed(GLFW_KEY_S))
        ++z;
    if (Input::keyPressed(GLFW_KEY_A))
        --x;
    if (Input::keyPressed(GLFW_KEY_D))
        ++x;

    glm::vec2 mouseDt{};
    if (Input::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        mouseDt = Input::mousePosition() - mPreviousMousePos;

    if (z || x || mouseDt.x || mouseDt.y)
    {
        float xOffset = mSpeed * dt * static_cast<float>(x);
        float zOffset = mSpeed * dt * static_cast<float>(z);
        mPosition += mBasis[0] * xOffset + mBasis[2] * zOffset;

        mTheta += mouseDt.x * dt * mPanSensitivity;
        mPhi += -mouseDt.y * dt * mPanSensitivity;

        calculateViewProjection();
    }
}

void Camera::updateViewMode(float dt)
{
    glm::vec2 mouseDt = Input::mousePosition() - mPreviousMousePos;

    if (mouseDt.x || mouseDt.y)
    {
        if (Input::mouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {
            mTheta += mouseDt.x * dt * mPanSensitivity;
            mPhi += -mouseDt.y * dt * mPanSensitivity;
            calculateViewProjection();
        }
        else if (Input::mouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            float xOffset = mSpeed * dt * mouseDt.x;
            float yOffset = mSpeed * dt * mouseDt.y;
            mPosition += mBasis[0] * xOffset + mBasis[1] * -yOffset;
            calculateViewProjection();
        }
    }
}

void Camera::updateEditorMode(float dt)
{
    glm::vec2 mouseDt{};
    if (Input::mouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        mouseDt = Input::mousePosition() - mPreviousMousePos;

    if (mouseDt.x || mouseDt.y)
    {
        mTheta += mouseDt.x * dt * mPanSensitivity;
        mPhi += -mouseDt.y * dt * mPanSensitivity;
        calculateViewProjection();
    }
}

void Camera::updateProjection()
{
    mProjection = glm::perspective(mFovY, mAspectRatio, mNearZ, mFarZ);
    mViewProjection = mProjection * mView;
}

void Camera::calculateBasis()
{
    mBasis[2] = {
        glm::sin(mTheta) * glm::cos(mPhi),
        glm::sin(mPhi),
        glm::cos(mTheta) * glm::cos(mPhi)
    };

    mBasis[2] *= -1.f;
    mBasis[0] = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), mBasis[2]));
    mBasis[1] = glm::normalize(glm::cross(mBasis[0], -mBasis[2]));
}

void Camera::calculateViewProjection()
{
    calculateBasis();

    mView[0].x = mBasis[0].x;
    mView[0].y = mBasis[1].x;
    mView[0].z = mBasis[2].x;
    mView[1].x = mBasis[0].y;
    mView[1].y = mBasis[1].y;
    mView[1].z = mBasis[2].y;
    mView[2].x = mBasis[0].z;
    mView[2].y = mBasis[1].z;
    mView[2].z = mBasis[2].z;
    mView[3].x = -glm::dot(mPosition, mBasis[0]);
    mView[3].y = -glm::dot(mPosition, mBasis[1]);
    mView[3].z = -glm::dot(mPosition, mBasis[2]);
    mView[3].w = 1.f;

    mViewProjection = mProjection * mView;
}
