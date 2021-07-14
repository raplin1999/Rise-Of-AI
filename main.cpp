#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "SDL_mixer.h"
#include <vector>

using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"
#include <vector>

#define Platform_Count 19
#define Enemy_count 3


struct GameState {
    Entity* player;
    Entity* platforms;
    Entity* enemies;
    Entity* bullets;
};



GameState state;

GLuint fontTextureID;
SDL_Window* displayWindow;
bool gameIsRunning = true;
bool missionInProgress = true;
bool won = false;
bool lost = false;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

Mix_Music* music;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Textured!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);


#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    music = Mix_LoadMUS("Beauty_Flow.mp3");
    Mix_PlayMusic(music, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);



    // Initialize Game Objects

    // Initialize Player
    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(-4.0f, 1.0f, 0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0, -9.8f, 0);
    state.player->speed = 1.5f;
    state.player->textureID = LoadTexture("Warrior.png");

    state.player->animIdle = new int[6]{ 0, 1, 2, 3, 4 ,5 };
    state.player->animRight = new int[6]{ 6, 7, 8, 9,10, 11 };
    state.player->animAttack = new int[6]{18,19,20,21,22,23};



    state.player->animIndices = state.player->animIdle;
    state.player->animFrames = 6;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 6;
    state.player->animRows = 17;

    state.player->height = 1.5f;
    state.player->width = 0.8f;

    state.player->jumpPower = 5.0f;

    state.platforms = new Entity[Platform_Count];
    GLuint platformTexutreID = LoadTexture("Tile1.png");
    GLuint platformTexutreID2 = LoadTexture("Tile2.png"); 
    GLuint bulletTexureID = LoadTexture("arm_projectile.png");
    fontTextureID = LoadTexture("font1.png");


    float bottomRocks[10] = { -4.5, -3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5 ,4.5 };
    float sideRocks[9] = { -4.25, -3.25, -2.25, 0.75, 1.75, 2.75, -2.25, -1.25, -0.25};
    for (int i = 0; i < 10; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTexutreID;
        state.platforms[i].position = glm::vec3(bottomRocks[i], -3.25f, 0);
    }
    for (int i = 10; i < 16; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTexutreID2;
        state.platforms[i].position = glm::vec3(sideRocks[i-10], -1.25f, 0);
    }
    for (int i = 16; i < 19; i++) {
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].textureID = platformTexutreID2;
        state.platforms[i].position = glm::vec3(sideRocks[i - 10], 1.25f, 0);
    }


    state.platforms[6].textureID = platformTexutreID2;
    state.platforms[7].textureID = platformTexutreID2;


    for (int i = 0; i < Platform_Count; i++) {
        state.platforms[i].Update(0, NULL, NULL, 0, NULL, 0);
    
    }

    state.enemies = new Entity[Enemy_count];
    GLuint enemeyTextureID = LoadTexture("Golem.png");

    state.enemies[0].position = glm::vec3(4,-2, 0);
    state.enemies[0].AItype = STALKER;
    state.enemies[0].AIState = LURK;

    state.enemies[1].position = glm::vec3(3, 2, 0);
    state.enemies[1].AItype = LURKER;
    state.enemies[1].AIState = LURK;


    state.enemies[2].position = glm::vec3(-0.5f, 2, 0);
    state.enemies[2].AItype = WALKER;
    state.enemies[2].AIState = WALKING;



    for (int i = 0; i < Enemy_count; i++) {
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].textureID = enemeyTextureID;
        state.enemies[i].acceleration = glm::vec3(0, -9.8f, 0);
        state.enemies[i].speed = 0.5;

        state.enemies[i].animIdle = new int[4]{ 0, 1, 2, 3 };
        state.enemies[i].animAttack = new int[9]{ 20, 21, 22, 23,24, 25,26,27,28 };
        state.enemies[i].height = 0.8;
        state.enemies[i].width = 0.5;

        state.enemies[i].animIndices = new int[1]{ 37 };
        state.enemies[i].animFrames = 1;
        state.enemies[i].animIndex = 0;
        state.enemies[i].animTime = 0;
        state.enemies[i].animCols = 10;
        state.enemies[i].animRows = 10;
    }

    //state.bullets = new Entity[3]; 
    //for (int i = 0; i < 3; i++) {
    //    state.bullets[i].textureID = bulletTexureID;
    //    state.bullets[i].speed = 1.0f;
    //    state.bullets[i].entityType = BULLET;
    //    state.bullets[i].position = glm::vec3(state.enemies[i].position.x - 0.5f, state.enemies[i].position.y, state.enemies[i].position.z);
    //    state.bullets[i].isActive = false;
    //}
}


void ProcessInput() {
    state.player->movement = glm::vec3(0);
    state.player->animIndices = state.player->animIdle;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                break;

            case SDLK_RIGHT:
                break;

            case SDLK_SPACE:
                state.player->jump = true;
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -30.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 30.0f;
        state.player->animIndices = state.player->animRight;
    }


    if (keys[SDL_SCANCODE_A]) {
        state.player->attack = true;
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}


#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }
    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        if (missionInProgress) {
            state.player->Update(FIXED_TIMESTEP, state.player, state.platforms, Platform_Count, state.enemies, Enemy_count);

            if (state.player->collidedBottom) {
                for (int i = 0; i < Enemy_count; i++) {
                    if (state.enemies[i].collidedTop) {
                        state.enemies[i].isActive = false;
                    }
                }
            }
            if (state.player->collidedLeft ||state.player->collidedRight) {
                for (int i = 0; i < Enemy_count; i++) {
                    if (state.enemies[i].collidedRight || state.enemies[i].collidedLeft) {
                        lost = true;
                    }
                }
            }
        }

        for (int i = 0; i < Enemy_count; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.platforms, Platform_Count, NULL, 0);
           /* if (state.enemies[i].attack == true) {
                state.bullets[i].isActive = true;
                state.bullets[i].movement = glm::vec3(-1, 0, 0);
            }*/
        }

 /*       for (int i = 0; i < 3; i++) {
            state.bullets[i].Update(FIXED_TIMESTEP, state.player, NULL, 0, NULL, 0);
        }*/




        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text,
    float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;
    std::vector<float> vertices;
    std::vector<float> texCoords;
    for (int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
        offset + (-0.5f * size), 0.5f * size,
        offset + (-0.5f * size), -0.5f * size,
        offset + (0.5f * size), 0.5f * size,
        offset + (0.5f * size), -0.5f * size,
        offset + (0.5f * size), 0.5f * size,
        offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), { u, v,u, v + height,u + width, v,u + width, v + height,u + width, v,u, v + height, });
    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (lastTicks < 5) {
        DrawText(&program, fontTextureID, "Jump on the enemy to destory it", 0.5f, -0.25f,
            glm::vec3(-3, 3.3, 0));
    
    }

    won = true;
    for (int i = 0; i < Enemy_count; i++) {
        if (state.enemies[i].isActive) {
            won = false;
        }
    }
    if (won) {
        DrawText(&program, fontTextureID, "You Won", 0.5f, -0.25f,
            glm::vec3(-1.5, 3.3, 0));
        missionInProgress = false;
    }
    if(lost){
        DrawText(&program, fontTextureID, "You Lost", 0.5f, -0.25f,
            glm::vec3(-1.5, 3.3, 0));
        missionInProgress = false;
    }



    for (int i = 0; i < Platform_Count; i++) {

        state.platforms[i].Render(&program); 
    }

    for (int i = 0; i < Enemy_count; i++) {

        state.enemies[i].Render(&program);
    }

    //for (int i = 0; i < 3; i++) {

    //    state.bullets[i].Render(&program);
    //}

    state.player->Render(&program);

    SDL_GL_SwapWindow(displayWindow);
}


void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}