// g++ prism.cpp -lSDL2 -lGL -ldl
// linux only

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#undef GL_GLEXT_PROTOTYPES
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

static const char *vertcode = R"(
#version 330 core
layout (location = 0) in vec3 apos;
layout (location = 1) in vec2 in_texcoord;
out vec2 texcoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main()
{
    gl_Position = proj * view * model * vec4(apos, 1.0);
    texcoord = in_texcoord;
}
)";

static const char *fragcode = R"(
#version 330 core
out vec4 fragcolor;
in vec2 texcoord;
uniform sampler2D ourtex;
void main()
{
	fragcolor = texture(ourtex, texcoord);
}
)";

float vertices[] = {
     0.0f,  0.5f, 0.5f,  0.5f, 0.0f, // top of prism
     0.5f, -0.5f, 0.0f,  1.0f, 1.0f, // right, facing front
     0.5f, -0.5f, 1.0f,  1.0f, 1.0f, // right, behind
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, // left, facing front
    -0.5f, -0.5f, 1.0f,  0.0f, 1.0f, // left, behind
};

int indices[] = {
    0, 1, 3,
    0, 1, 2,
    0, 3, 4,
    0, 2, 4,
};

unsigned compile()
{
    auto compile_shader = [](const char *code, GLenum type) {
        unsigned shid = glCreateShader(type);
        glShaderSource(shid, 1, &code, NULL);
        glCompileShader(shid);
        return shid;
    };
    unsigned vs = compile_shader(vertcode, GL_VERTEX_SHADER);
    unsigned fs = compile_shader(fragcode, GL_FRAGMENT_SHADER);
    unsigned id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return id;
}

void setmat(unsigned id, const char *name, glm::mat4 mat)
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, glm::value_ptr(mat));
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_Window *wnd = SDL_CreateWindow("Cat prism",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  640, 480,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext ctx = SDL_GL_CreateContext(wnd);
    SDL_GL_SetSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    unsigned id = compile();
    unsigned vao, vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int width, height, channels;
    unsigned texid;
    unsigned char *imgdata = stbi_load("cat.jpg", &width, &height, &channels, 0);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imgdata);

    glm::mat4 model{1.0f};
    glm::mat4 view{1.0f};
    glm::mat4 proj{1.0f};
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glUseProgram(id);
    setmat(id, "model", model);
    setmat(id, "view",  view);
    setmat(id, "proj",  proj);

    for (bool running = true; running; ) {
        for (SDL_Event ev; SDL_PollEvent(&ev) != 0; )
            if (ev.type == SDL_QUIT)
                running = false;
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.5f));
        model = glm::rotate(model, glm::radians(5.0f), glm::vec3(0.0f, 0.5f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, -0.5f));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(id);
        setmat(id, "model", model);
        glBindTexture(GL_TEXTURE_2D, texid);
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        SDL_GL_SwapWindow(wnd);
    }

    stbi_image_free(imgdata);
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(wnd);
    SDL_Quit();
}
