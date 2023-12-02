#include "first_app.h"

#include "simple_render_system.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace gala {

    // Note: also would need to add RigidBody2dComponent to game object


    class GravityPhysicsSystem {
    public:
        GravityPhysicsSystem(float strength) : strengthGravity{ strength } {}

        const float strengthGravity;

        // dt stands for delta time, and specifies the amount of time to advance the simulation
        // substeps is how many intervals to divide the forward time step in. More substeps result in a
        // more stable simulation, but takes longer to compute
        void update(std::vector<GalaGameObject>& objs, float dt, unsigned int substeps = 1) {
            const float stepDelta = dt / substeps;
            for (int i = 0; i < substeps; i++) {
                stepSimulation(objs, stepDelta);
            }
        }

        glm::vec2 computeForce(GalaGameObject& fromObj, GalaGameObject& toObj) const {
            auto offset = fromObj.transform2d.translation - toObj.transform2d.translation;
            float distanceSquared = glm::dot(offset, offset);

            // clown town - just going to return 0 if objects are too close together...
            if (glm::abs(distanceSquared) < 1e-10f) {
                return { .0f, .0f };
            }

            float force =
                strengthGravity * toObj.rigidBody2d.mass * fromObj.rigidBody2d.mass / distanceSquared;
            return force * offset / glm::sqrt(distanceSquared);
        }

    private:
        void stepSimulation(std::vector<GalaGameObject>& physicsObjs, float dt) {
            // Loops through all pairs of objects and applies attractive force between them
            for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) {
                auto& objA = *iterA;
                for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) {
                    if (iterA == iterB) continue;
                    auto& objB = *iterB;

                    auto force = computeForce(objA, objB);
                    objA.rigidBody2d.velocity += dt * -force / objA.rigidBody2d.mass;
                    objB.rigidBody2d.velocity += dt * force / objB.rigidBody2d.mass;
                }
            }

            // update each objects position based on its final velocity
            for (auto& obj : physicsObjs) {
                obj.transform2d.translation += dt * obj.rigidBody2d.velocity;
            }
        }
    };

    class Vec2FieldSystem {
    public:
        void update(
            const GravityPhysicsSystem& physicsSystem,
            std::vector<GalaGameObject>& physicsObjs,
            std::vector<GalaGameObject>& vectorField) {
            // For each field line we caluclate the net graviation force for that point in space
            for (auto& vf : vectorField) {
                glm::vec2 direction{};
                for (auto& obj : physicsObjs) {
                    direction += physicsSystem.computeForce(obj, vf);
                }

                // This scales the length of the field line based on the log of the length
                // values were chosen just through trial and error based on what i liked the look
                // of and then the field line is rotated to point in the direction of the field
                vf.transform2d.scale.x =
                    0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
                vf.transform2d.rotation = atan2(direction.y, direction.x);
            }
        }
    };

    std::unique_ptr<GalaModel> createSquareModel(GalaDevice& device, glm::vec2 offset) {
        std::vector<GalaModel::Vertex> vertices = {
            {{-0.5f, -0.5f}},
            {{0.5f, 0.5f}},
            {{-0.5f, 0.5f}},
            {{-0.5f, -0.5f}},
            {{0.5f, -0.5f}},
            {{0.5f, 0.5f}},  //
        };
        for (auto& v : vertices) {
            v.position += offset;
        }
        return std::make_unique<GalaModel>(device, vertices);
    }

    std::unique_ptr<GalaModel> createCircleModel(GalaDevice& device, unsigned int numSides) {
        std::vector<GalaModel::Vertex> uniqueVertices{};
        for (int i = 0; i < numSides; i++) {
            float angle = i * glm::two_pi<float>() / numSides;
            uniqueVertices.push_back({ {glm::cos(angle), glm::sin(angle)} });
        }
        uniqueVertices.push_back({});  // adds center vertex at 0, 0

        std::vector<GalaModel::Vertex> vertices{};
        for (int i = 0; i < numSides; i++) {
            vertices.push_back(uniqueVertices[i]);
            vertices.push_back(uniqueVertices[(i + 1) % numSides]);
            vertices.push_back(uniqueVertices[numSides]);
        }
        return std::make_unique<GalaModel>(device, vertices);
    }

    FirstApp::FirstApp() { loadGameObjects(); }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        // create some models
        std::shared_ptr<GalaModel> squareModel = createSquareModel(
            galaDevice,
            { .5f, .0f });  // offset model by .5 so rotation occurs at edge rather than center of square
        std::shared_ptr<GalaModel> circleModel = createCircleModel(galaDevice, 64);

        // create physics objects
        std::vector<GalaGameObject> physicsObjects{};
        auto red = GalaGameObject::createGameObject();
        red.transform2d.scale = glm::vec2{ .05f };
        red.transform2d.translation = { .5f, .5f };
        red.color = { 1.f, 0.f, 0.f };
        red.rigidBody2d.velocity = { -.5f, .0f };
        red.model = circleModel;
        physicsObjects.push_back(std::move(red));
        auto blue = GalaGameObject::createGameObject();
        blue.transform2d.scale = glm::vec2{ .05f };
        blue.transform2d.translation = { -.45f, -.25f };
        blue.color = { 0.f, 0.f, 1.f };
        blue.rigidBody2d.velocity = { .5f, .0f };
        blue.model = circleModel;
        physicsObjects.push_back(std::move(blue));

        // create vector field
        std::vector<GalaGameObject> vectorField{};
        int gridCount = 40;
        for (int i = 0; i < gridCount; i++) {
            for (int j = 0; j < gridCount; j++) {
                auto vf = GalaGameObject::createGameObject();
                vf.transform2d.scale = glm::vec2(0.005f);
                vf.transform2d.translation = {
                    -1.0f + (i + 0.5f) * 2.0f / gridCount,
                    -1.0f + (j + 0.5f) * 2.0f / gridCount };
                vf.color = glm::vec3(1.0f);
                vf.model = squareModel;
                vectorField.push_back(std::move(vf));
            }
        }

        GravityPhysicsSystem gravitySystem{ 0.81f };
        Vec2FieldSystem vecFieldSystem{};

        SimpleRenderSystem simpleRenderSystem{ galaDevice, galaRenderer.getSwapChainRenderPass() };

        while (!galaWindow.shouldClose()) {
            glfwPollEvents();

            if (auto commandBuffer = galaRenderer.beginFrame()) {
                // update systems
                gravitySystem.update(physicsObjects, 1.f / 60, 5);
                vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

                // render system
                galaRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects);
                simpleRenderSystem.renderGameObjects(commandBuffer, vectorField);
                galaRenderer.endSwapChainRenderPass(commandBuffer);
                galaRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(galaDevice.device());
    }

    void FirstApp::loadGameObjects() {
        std::vector<GalaModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} };
        auto galaModel = std::make_shared<GalaModel>(galaDevice, vertices);

        auto triangle = GalaGameObject::createGameObject();
        triangle.model = galaModel;
        triangle.color = { .1f, .8f, .1f };
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = { 2.f, .5f };
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }

}  // namespace gala