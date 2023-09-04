//----------------------------------------------------------

#include "window.hpp"

//----------------------------------------------------------


void updateInput(GLFWwindow* window, Camera& cam)
{

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    bool panUp = false, panLeft = false, panDown = false, panRight = false;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) panUp = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) panLeft = true;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) panDown = true;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) panRight = true;
    if (panUp || panLeft || panDown || panRight) cam.pan(panUp, panLeft, panDown, panRight);
}

void windowFunction(unsigned int W, unsigned int H, std::vector<Object>& objects, Observatory& observatory)
{
    // -------------------- init glfw
    if(!glfwInit()){
        fmt::println("Error: GLFW init failed");
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, true);
    glfwWindowHint(GLFW_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(W, H, "Minor Planet Center Solver 3", NULL, NULL);

    glfwMakeContextCurrent(window);

    // -------------------- init glew
    glewExperimental = true;
    GLenum err = glewInit();
    if(err != GLEW_OK){
        glfwTerminate();
        fmt::println("Error: GLEW init failed:\n{}", (char*)glewGetErrorString(err));
        return;
    }
    
    // -------------------- opengl options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // dont draw the side of a vertex that cant be seen
    glCullFace(GL_BACK); // the back side cant be seen
    glFrontFace(GL_CCW); // front is where the points of a triangle are connected counterclockwise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // -------------------- camera
    Camera cam(W, H, 100);

    // -------------------- shader
    GLProgram program(g_resourcesPath+"/shaders/vertex.glsl", g_resourcesPath+"/shaders/fragment.glsl");

    // -------------------- glfw window callback functions
    struct CallbackData {
        Camera* mainCamera;
    };
    CallbackData callbackData;
    callbackData.mainCamera = &cam;
    glfwSetWindowUserPointer(window, &callbackData); // had to do this because i need camera in a callback function

    // mouse scrolling
    glfwSetScrollCallback(window, 
        [](GLFWwindow* window, double xoffset, double yoffset) -> void {
            CallbackData* data = static_cast<CallbackData*>(glfwGetWindowUserPointer(window)); // retrieve camera callback data
            if (yoffset > 0)  data->mainCamera->zoom(true);
            else data->mainCamera->zoom(false);
        }
    );

    // window resizing
    glfwSetFramebufferSizeCallback(window, 
        [](GLFWwindow* window, int frameBufferWidth, int frameBufferHeight) -> void {
            glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        }
    );

    // ------------------- set up drawing data
    // objects
    std::vector<float> vertices;
    for(auto obj : objects){
        vertices.emplace_back(obj.getCoords3D().X);
        vertices.emplace_back(obj.getCoords3D().Y);
        vertices.emplace_back(obj.getCoords3D().Z);
    }

    unsigned int VBobjects, VAobjects;
    glGenVertexArrays(1, &VAobjects);
    glGenBuffers(1, &VBobjects);

    glBindVertexArray(VAobjects);

    glBindBuffer(GL_ARRAY_BUFFER, VBobjects);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // horizon line
    vertices.clear();
    CoordinatesSkyLocal c = cam.getLookingPosition();
    cam.updateRotations({0, 0});
    glm::vec3 camRot = cam.getRotationAngles();
    glm::quat quaternion; quaternion = glm::quat(glm::vec3(camRot[0]+std::numbers::pi/2.0, camRot[1], camRot[2]));
    glm::mat4 rot = glm::toMat4(quaternion);
    cam.updateRotations(c);
    const int CIRCLE_POINTS = 16;
    for(int i = 0; i < CIRCLE_POINTS; i++){
        double rad = 2.0*std::numbers::pi * i/CIRCLE_POINTS;
        glm::vec4 circlePoint = rot * glm::vec4(std::cos(rad), std::sin(rad), 0.0, 0.0);
        vertices.emplace_back(circlePoint[0]);
        vertices.emplace_back(circlePoint[1]);
        vertices.emplace_back(circlePoint[2]);
    }

    unsigned int VBground, VAground;
    glGenVertexArrays(1, &VAground);
    glGenBuffers(1, &VBground);

    glBindVertexArray(VAground);

    glBindBuffer(GL_ARRAY_BUFFER, VBground);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // -------------------- window
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glPointSize(3.0f);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        updateInput(window, cam);
        program.setUniformMat4f("MVP", cam.getTransformation());

        // clear
        glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);
        
        // draw objects
        glBindVertexArray(VAobjects);
        glDrawArrays(GL_POINTS, 0, objects.size());

        // draw horizon line
        glBindVertexArray(VAground);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_POINTS);

        // bureaucracy..
        glfwSwapBuffers(window);
        glFlush();
    }

    // -------------------- deinit
    glDeleteBuffers(1, &VBobjects);
    glDeleteVertexArrays(1, &VAobjects);
    glDeleteBuffers(1, &VBground);
    glDeleteVertexArrays(1, &VAground);
    glfwDestroyWindow(window);
    glfwTerminate();
}
