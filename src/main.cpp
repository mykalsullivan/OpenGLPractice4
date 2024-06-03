//
// Created by msullivan on 5/31/24.
//

#include <iostream>
#include <complex>
#include <cstring>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Window dimensions
const unsigned int WIDTH = 640, HEIGHT = 480;

// IDs for VAO, VBO, and shader objects
unsigned int vao, vbo, shader;

// Uniform variables
unsigned int uniformModel;
bool direction = true;
float triOffset = 0.0f, triMaxOffset = 1.0f, triTranslationIncrement = 0.015f;

// Angle stuff
float currentAngle = 0.0f;

// Vertex shader
static const char* vertexShader =
        "#version 330\n"
        "\n"
        "layout (location = 0) in vec3 pos;\n"                  /* Note: 'pos' is a built-in variable in OpenGL */
        "uniform mat4 model;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_Position = model * vec4(pos.x, pos.y, pos.z, 1.0);\n"    /* Note: 'gl_Position' is also built-in */
        "}\n";

// Fragment shader
static const char* fragmentShader =
        "#version 330\n"
        "\n"
        "out vec4 color;\n"                                     /* Note: color is built-in */
        "\n"
        "void main()\n"
        "{\n"
        "   color = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n";

void createTriangle()
{
    // Create vertex coordinates
    float vertices[] = {
            -1.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f
    };

    // Generate and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate, bind, and buffer VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /* index: Which vertex in buffer
     * size: Number of elements in buffer
     * type: Data type
     * normalized: If 0.0-0.1, then it is already normalized; if 0-255, then it is NOT normalized (I think)
     * stride: How many elements to skip
     * pointer: Where to start
     */
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void addShader(unsigned int program, const char* source, GLenum type)
{
    unsigned int newShader = glCreateShader(type);
    const char* theCode[1];
    theCode[0] = source;

    int codeLength[1];
    codeLength[0] = (int) strlen(source);

    /* shader: Shader ID
     * count: # of shaders being created (I think)
     * string: Source code
     * length: Length of source code
     */
    glShaderSource(newShader, 1, theCode, codeLength);
    glCompileShader(newShader);

    int result = 0;
    char errorLog[1024] = {};

    glGetShaderiv(newShader, GL_COMPILE_STATUS, &result);

    if (!result)
    {
        glGetShaderInfoLog(newShader, sizeof(errorLog), nullptr, errorLog);
        std::cout << "Error compiling the " << type << " shader: " << errorLog << '\n';
        return;
    }

    // Attach new shader to the program
    glAttachShader(program, newShader);
}

void compileShaders()
{
    shader = glCreateProgram();

    if (!shader)
    {
        std::cout << "Error creating shader program\n";
        /* Note: Program could crash if shader fails to compile... so be more thorough here in the future */
        return;
    }

    addShader(shader, vertexShader, GL_VERTEX_SHADER);
    addShader(shader, fragmentShader, GL_FRAGMENT_SHADER);

    int result = 0;
    char errorLog[1024] = {};

    // Creates executable on GPU using shaders
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &result);

    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(errorLog), nullptr, errorLog);
        std::cout << "Error linking program: " << errorLog << '\n';
        return;
    }

    // Ensures that the shader was created correctly
    glValidateProgram(shader);
    glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);

    if (!result)
    {
        glGetProgramInfoLog(shader, sizeof(errorLog), nullptr, errorLog);
        std::cout << "Error validating program: " << errorLog << '\n';
        return;
    }

    uniformModel = glGetUniformLocation(shader, "model");
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cout << "GLFW failed to initialize\n";

        // Unload GLFW memory
        glfwTerminate();
        return 1;
    }

    // Setup GLFW window properties
    // OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Core profile = no backwards compatibility
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

    // Create main window
    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Transform Test", nullptr, nullptr);

    // Return if the main GLFW window cannot be created
    if (mainWindow == nullptr)
    {
        std::cout << "Could not create main GLFW window\n";
        glfwTerminate();
        return 1;
    }

    // Get buffer size information
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    // Set context for GLEW to use
    glfwMakeContextCurrent(mainWindow);

    // Allow modern extension features
    glewExperimental = true;

    // Setup GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Could not initialize GLEW\n";
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    // Setup viewport size
    glViewport(0, 0, bufferWidth, bufferHeight);

    // Create triangle and compile shaders
    createTriangle();
    compileShaders();

    // Main loop
    while (!glfwWindowShouldClose(mainWindow))
    {
        // Set window size
        glfwGetWindowSize(mainWindow, &bufferWidth, &bufferHeight);
        glViewport(0, 0, bufferWidth, bufferHeight);

        // Get/handle user input
        glfwPollEvents();

        {
            static float i = 0;

//            // Rotates between "hard" RGB values
//            auto r = (float) std::sin(i+(2*M_PI/3));
//            auto g = (float) std::sin(i);
//            auto b = (float) std::sin(i-(2*M_PI/3));

            // Rotates between RGB values, but with a LOT of color blending
            auto r = (float) std::abs(std::sin(i+(2*M_PI/3)));
            auto g = (float) std::abs(std::sin(i));
            auto b = (float) std::abs(std::sin(i-(2*M_PI/3)));

            glm::mat4 model(1.0f);
            if (direction)
            {
                triOffset += triTranslationIncrement;
            }
            else
            {
                triOffset -= triTranslationIncrement;
            }

            if (std::abs(triOffset) >= triMaxOffset)
            {
                direction = !direction;
            }

            if (currentAngle >= 360)
            {
                currentAngle -= 360;
            }

            // Clear window
            glClearColor(r, g, b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(shader);

            model = glm::rotate(model, currentAngle * (float) M_PI/180, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(triOffset, triOffset, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 1.0f));

            glUniform1f((int) uniformModel, triOffset);
            glUniformMatrix4fv((int) uniformModel, 1, false, glm::value_ptr(model));

            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            glUseProgram(0);

            std::this_thread::sleep_for(std::chrono::microseconds(16667));
            i += 0.005f;
            currentAngle += 1.5f;
        }

        // Swap display buffers
        glfwSwapBuffers(mainWindow);
    }

    // Delete main window
    glfwDestroyWindow(mainWindow);

    return 0;
}