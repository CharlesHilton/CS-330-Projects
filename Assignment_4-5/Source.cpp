/*
    Author: Charles Hilton
            Charles.Hilton@snhu.edu

    Notes: Largely derived from CS-330, Tutorial 4.5.
           Added Up/Down mvoement, and MovementSpeed manipulation. 

*/

#include <vector>
#include <iostream>             // cout, cerr
#include <fstream>
#include <sstream>
#include <cstdlib>              // EXIT_FAILURE
#include <GL/glew.h>            // GLEW library
#include <GLFW/glfw3.h>         // GLFW library
#include <chrono>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h" // Camera class. Modified to add up down movement.

#include "Sphere.h"
//#include "HiltonGL.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Assignment 4-5"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 1920;
    const int WINDOW_HEIGHT = 1080;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint ebo;         // Handle for element array buffer object.
        GLuint nVertices;   // Number of vertices of the mesh
        GLuint nIndicies;
    };

    // Stores the GL data relative to a given mesh, using vertices and indicies (faces)
    struct GLMeshVF // Mesh for object defined with verticies (V) and faces (indicies) (f)
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects. One each for vertices and faces.
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh mouseDockMesh;
    GLMeshVF MouseDockBaseMeshVF;

    //Sphere *aSphere =  new Sphere(0.08, 30, 30);
    
    // Shader program
    GLuint gProgramId;

    bool projectionMode = 0;

    // Array to store when the last time a particular was pressed.
    // Use the index to handle which key was pressed. Example GLFW_KEY_P returns an integer 80.
    std::chrono::time_point<std::chrono::steady_clock> lastKeyPressedTime[];    
    
    //objModel MouseDockObj;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UCreateMesh(GLMeshVF& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
void UDrawMesh(GLMesh& mesh);
void UDrawMesh(GLMeshVF& mesh);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
//void loadObjectModelFile(const char* fileName, objModel& object);
void drawCircle(float cx, float cy, float r, int num_segments);

bool parseOBJ(const char* fileName);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);



int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
 
    UCreateMesh(mouseDockMesh); // Calls the function to create the Vertex Buffer Object
    UCreateMesh(MouseDockBaseMeshVF);

    //loadObjectModelFile("Resources/MouseDock.obj", MouseDockObj);

    //parseOBJ("Resorces/MouseDock.obj");


    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame

        URender();
     

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(mouseDockMesh);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    // Added up/down keybinds.
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
        auto now = std::chrono::steady_clock::now();
        if ((now - lastKeyPressedTime[GLFW_KEY_P]) > std::chrono::milliseconds(200)) {
            projectionMode = !projectionMode;
            lastKeyPressedTime[GLFW_KEY_P] = now;
        }
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);    // Changed ProcessMouseScroll to manipulate MoveSpeed. See camera.h
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Function called to render a frame
void URender()
{
    const int nrows = 10;
    const int ncols = 10;
    const int nlevels = 10;

    const float xsize = 10.0f;
    const float ysize = 10.0f;
    const float zsize = 10.0f;

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, (181.0f / 256.0f), (226.0f / 256.0f), 1.0f);     // Sky Blue Background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection;
    switch (projectionMode)
    {
    case 0:
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        break;
    case 1:
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
        break;
    default:
        break;
    }

    // Set the shader to be used
    glUseProgram(gProgramId);

    glm::mat4 scale;    // Matrix to control scaling.
    glm::mat4 rotation; // Matrix to control the rotation.
    glm::vec3 location = glm::vec3(0.0, 0, 0);  // Vector for object location
    glm::mat4 translation;
    glm::mat4 model;

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));



    // Draw the Mouse Dock Base
    // ------------------------

    // 1. Scales the object
    scale = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));
    // 2. Rotates shape in the x axis
    rotation = glm::rotate(0.0f, glm::vec3(0.0, 0.0f, 1.0f));
    // 3. Place object at the origin
    translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    //// Model matrix: transformations are applied right-to-left order
    model = translation * rotation * scale;

    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    //glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    UDrawMesh(MouseDockBaseMeshVF);


    // Draw the mouse dock connectors
    // ------------------------------
 

    scale = glm::scale(glm::vec3(1.0f, 1.0f, 3.0f));
    rotation = glm::rotate(0.0f, glm::vec3(1.0, 0.0f, 0.0f));

    // Connector 1
    location = glm::vec3(-0.2f, 0.0f, 0.4f);
    translation = glm::translate(location);
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    Sphere aSphere(0.08, 30, 30);
    aSphere.Draw();

    // Connector 2
    location = glm::vec3(0.2f, 0.0f, 0.4f);
    translation = glm::translate(location);
    model = translation * rotation * scale;
    modelLoc = glGetUniformLocation(gProgramId, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    Sphere bSphere(0.08, 30, 30);
    bSphere.Draw();



    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,

         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

// Implements the UCreateMesh function
void UCreateMesh(GLMeshVF& mesh)
{
    // Position and Color data for Mouse Dock
    GLfloat verts[] = {
        // Vertex Positions     // Colors (r,g,b,a)     // Description
         0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f, 1.0f, // 0: Bottom, Back, Right Vertex
         0.4f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f, 1.0f, // 1: Bottom, Front, Right Vertex
        -0.4f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f, 1.0f, // 2: Bottom, Front, Left Vertex
        -0.5f,  0.5f, 0.0f,     1.0f, 1.0f, 0.0f, 1.0f, // 3: Bottom, Back, Left Vertex

         0.3f,  0.4f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f, // 4: Top, Back, Right Vertex
         0.3f, -0.4f, 0.2f,     0.0f, 1.0f, 1.0f, 1.0f, // 5: Top, Front, Right Vertex
        -0.3f, -0.4f, 0.2f,     0.0f, 0.0f, 1.0f, 1.0f, // 6: Top, Front, Left Vertex
        -0.3f,  0.4f, 0.5f,     1.0f, 1.0f, 0.0f, 1.0f, // 7: Top, Back, Left Vertex
    };

    // Index data to share position data
    GLushort indices[] = {
        // Right Face
        0, 1, 5,    // Triangle 1
        0, 5, 4,    // Triangle 2

        // Top Face
        4, 7, 6,    // Triangle 3
        4, 5, 6,

        // Back Face         
        0, 4, 7,
        0, 3, 7,

        // Bottom Face
        0, 1, 3,
        1, 2, 3,

        // Left Face
        2, 3, 7,
        2, 6, 7,

        // Front Face
        1, 5, 6,
        1, 2, 6

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
}

void UDrawMesh(GLMesh& mesh) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(mesh.vao);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, mouseDockMesh.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void UDrawMesh(GLMeshVF& mesh) {
    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(mesh.vao);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, mesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

void UDestroyMesh(GLMeshVF& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

//void UDestroyMesh(objModel& object)
//{
//    glDeleteVertexArrays(1, &object.mesh.vao);
//    glDeleteBuffers(1, &object.mesh.vbo);
//    glDeleteBuffers(1, &object.mesh.ebo);
//}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}




//void loadObjectModelFile(const char* fileName, objModel& object)
//{
//    // Source: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Load_OBJ
//    // Modified to suit.
//
//    //objModel object;
//
//    ifstream file(fileName);
//    if (!file)
//    {
//        cerr << "Cannot open " << fileName << endl; exit(EXIT_FAILURE);
//    }
//
//    string line;
//    while (std::getline(file, line))
//    {
//        cout << "Read line: " << line << endl;
//
//        if (line.substr(0, 2) == "v ")  //  Read in vertex
//        {
//            istringstream s(line.substr(2));
//            glm::vec4 vl;                                  // Vertex location (x, y, z, w).
//            s >> vl.x; s >> vl.y; s >> vl.z; vl.w = 1.0f;
//            object.vertices.push_back(vl);
//            cout << "Read Vertex: " << vl.a << endl;
//        }
//        else if (line.substr(0, 3) == "vn ")    // Read in face normal vectors.
//        {
//            istringstream s(line.substr(3));
//            glm::vec3 nv;                       // Nomral Vector (i (x), j (y), k (z)).
//            //GLfloat iv, jv, kv;               // Vector components (i, j, k)
//            s >> nv.x; s >> nv.y; s >> nv.z;
//            object.normals.push_back(nv);
//            cout << "Read normal vector: " << nv.x << endl;
//        }
//        else if (line.substr(0, 2) == "f ")     // Read in element. Effictively is conects the vertices to form a face/triangle.
//        {
//            istringstream s(line.substr(2));
//            GLushort a, b, c;
//            s >> a; s >> b; s >> c;
//            a--; b--; c--;
//            object.faces.push_back(a); object.faces.push_back(b); object.faces.push_back(c);
//            cout << "Read element: " << a << " " << b << " " << c << endl;
//        }
//    }
//
//    glGenVertexArrays(1, &object.mesh.vao); // Create the Vertex Array Object.
//    glBindVertexArray(object.mesh.vao);
//
//    glGenBuffers(1, &object.mesh.vbo);                  // Create Vertex Buffer object.
//    glBindBuffer(GL_ARRAY_BUFFER, object.mesh.vbo);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(object.vertices), &object.vertices, GL_STATIC_DRAW);
//
//    object.mesh.nIndicies = sizeof(object.faces) / sizeof(object.faces[0]);   // Create the Element Buffer Object
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.mesh.ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(object.faces), &object.faces, GL_STATIC_DRAW);
//
//    const GLuint floatsPerVertex = 4;
//    const GLuint floatsPerColor = 0;
//    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each
//
//    // Create Vertex Attribute Pointers
//    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
//    glEnableVertexAttribArray(0);
//
//    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
//    glEnableVertexAttribArray(1);
//}

void drawCircle(float cx, float cy, float r, int num_segments)
{
    float theta = 3.1415926 * 2 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor 

    float radial_factor = cosf(theta);//calculate the radial factor 

    float x = r;//we start at angle = 0 

    float y = 0;
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    for (int ii = 0; ii < num_segments; ii++)
    {
        glVertex2f(x + cx, y + cy);//output vertex 

        //calculate the tangential vector 
        //remember, the radial vector is (x, y) 
        //to get the tangential vector we flip those coordinates and negate one of them 

        float tx = -y;
        float ty = x;

        //add the tangential vector 

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        //correct using the radial factor 

        x *= radial_factor;
        y *= radial_factor;
    }
    glEnd();
}

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

struct Vertex {
    float x, y, z;
};

struct TextureCoordinate {
    float u, v;
};

struct Normal {
    float x, y, z;
};

struct Face {
    int v1, v2, v3;
    int t1, t2, t3;
    int n1, n2, n3;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<TextureCoordinate> textureCoordinates;
    std::vector<Normal> normals;
    std::vector<Face> faces;
};

bool parseOBJ(const char* filePath) {

    MeshData meshData;

    std::vector<Vertex> vertices;
    std::vector<TextureCoordinate> textureCoordinates;
    std::vector<Normal> normals;
    std::vector<Face> faces;

    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {   // Skip empty lines or comment lines...
            continue;
        }

        std::stringstream lineStream(line);
        std::string type;
        lineStream >> type;

        if (type == "v") {
            float x, y, z;
            lineStream >> x >> y >> z;
            meshData.vertices.push_back({ x, y, z });
        }
        else if (type == "vt") {
            float u, v;
            lineStream >> u >> v;
            textureCoordinates.push_back({ u, v });
        }
        else if (type == "vn") {
            float x, y, z;
            lineStream >> x >> y >> z;
            normals.push_back({ x, y, z });
        }
        else if (type == "f") {
            int v1, v2, v3, t1 = 0, t2 = 0, t3 = 0, n1 = 0, n2 = 0, n3 = 0;
            char discard;
            lineStream >> v1 >> discard >> t1 >> discard >> n1
                >> v2 >> discard >> t2 >> discard >> n2
                >> v3 >> discard >> t3 >> discard >> n3;
            faces.push_back({ v1 - 1, v2 - 1, v3 - 1, t1 - 1, t2 - 1, t3 - 1, n1 - 1, n2 - 1, n3 - 1 });
        }
    }

    // Pass the data to OpenGL here
    return true;
}