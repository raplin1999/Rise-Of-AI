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

enum EntityType {PLAYER, PLATFORM, ENEMY, BULLET};

enum AIType {WALKER, LURKER, STALKER};
enum AISTATE {IDLE, WALKING, ATTACKING, LURK};


class Entity {
public:
    EntityType entityType;
    AIType AItype;
    AISTATE AIState;

    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;
    float speed;

    bool attack = false;
    bool jump = false;
    float jumpPower = 0;


    float width = 1;
    float height = 1;

    GLuint textureID;

    glm::mat4 modelMatrix;

    int* animRight = NULL;
    int* animLeft = NULL;
    int* animUp = NULL;
    int* animDown = NULL;
    int* animIdle = NULL;
    int* animAttack = NULL;


    int* animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;

    bool isActive = true;

    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;

    Entity();


    bool CheckCollision(Entity* other);
    void Update(float deltaTime, Entity* player, Entity* platforms, int platformsCount, Entity* enemies, int enemyCount);
    void Render(ShaderProgram* program);
    void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
    void CheckCollisionsY(Entity* objects, int objectCount);
    void CheckCollisionsX(Entity* objects, int objectCount);

    void AI(Entity* player);
    void AILurker(Entity* player);
    void AIWalker(Entity* player);
    void AIStalker(Entity* player);
};