#include "gl/platform/init.hpp"
#include "gl/platform/window.hpp"
#include "gl/raii/shader.hpp"
#include "gl/raii/buffer.hpp"
#include "gl/raii/texture2d.hpp"
#include "gl/scene/debugCamera.hpp"
#include "gl/util/stopwatch.hpp"
#include "gl/raii/sampler.hpp"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

int main(void) {

    gl::init(true);

    {
        gl::window w;
        w.create(1920, 1080, "labGL");
        glViewport(0, 0, 1920, 1080);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        constexpr GLfloat vertices[] = {
            //  x      y      z      u     v
            // +X
            0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
            0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
            // -X
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
            // +Y
            -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
            0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
            0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
            // -Y
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, -0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, 0.0f, 1.0f,
            // +Z
            -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
            0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
            0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
            // -Z
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
            0.5f,  0.5f, -0.5f, 0.0f, 1.0f
        };
        gl::buffer vertexBuffer;
        vertexBuffer.create(sizeof(vertices), vertices);

        constexpr GLuint indices[] = {
            0, 1, 2, 0, 2, 3,       // +X
            4, 5, 6, 4, 6, 7,       // -X
            8, 9, 10, 8, 10, 11,    // +Y
            12, 13, 14, 12, 14, 15, // -Y
            16, 17, 18, 16, 18, 19, // +Z
            20, 21, 22, 20, 22, 23  // -Z
        };
        constexpr GLsizei indexCount = sizeof(indices) / sizeof(indices[0]);
        gl::buffer indexBuffer;
        indexBuffer.create(sizeof(indices), indices);

        gl::texture2d texture;
        texture.create("../assets/textures/red_brick.png", GL_RGB8, true);
        texture.generateMipmaps();

        gl::sampler sampler;

        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -1.2f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f));
        gl::buffer modelMatrixBuffer;
        modelMatrixBuffer.create(sizeof(glm::mat4), &modelMatrix);

        gl::debugCamera cam;

        gl::shader simpleShader;
        simpleShader.loadFromFile("../assets/shaders/shader.vert", "../assets/shaders/shader.frag");

        gl::stopwatch time;
        double deltaTime = 0.0;
        bool mouseCaptured = false;
        bool running = true;
        while (running) {
            deltaTime = time.get();
            time.reset();

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;

                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT) mouseCaptured = w.captureMouse(true);
                if (event.type == SDL_EVENT_MOUSE_BUTTON_UP   && event.button.button == SDL_BUTTON_RIGHT) { mouseCaptured = false; w.captureMouse(false); }
                if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) { mouseCaptured = false; w.captureMouse(false); }

                if (event.type == SDL_EVENT_MOUSE_MOTION && mouseCaptured) cam.updateDirection(event.motion.xrel, -event.motion.yrel);
            }

            if (mouseCaptured) cam.updateOrigin(deltaTime);
            cam.apply(0);

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            vertexBuffer.bind(gl::bufferTarget::SHADER_STORAGE_BUFFER, 0);
            indexBuffer.bind( gl::bufferTarget::SHADER_STORAGE_BUFFER, 1);

            modelMatrixBuffer.bind(gl::bufferTarget::UNIFORM_BUFFER, 1);

            texture.bind(0);
            sampler.bind(0);

            simpleShader.use();

            glDrawArrays(GL_TRIANGLES, 0, indexCount);
            
            w.swapBuffers();
        }
    }

    gl::quit();
    return 0;

}
