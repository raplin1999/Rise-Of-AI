#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;


    modelMatrix = glm::mat4(1.0f);
}


void Entity::AI(Entity* player) {
    switch (AItype){
        case WALKER:
            AIWalker(player);
            break;
        case STALKER:
            AIStalker(player);
            break;
        case LURKER:
            AILurker(player);
    }
}



void Entity::AILurker(Entity* player) {
    switch (AIState) {
    case LURK:
        if (glm::distance(position, player->position) < 3.0f) {
            animIndices = new int[8]{ 37,36,35,34,33,32, 31,30 };
            animFrames = 8;
            AIState = ATTACKING;
        }
        break;
    case ATTACKING:
        animIndices = animAttack;
        animFrames = 9;
        attack = true;
        break;
    }
}
bool left = true;
void Entity::AIWalker(Entity* player) {
    switch (AIState) {
    case WALKING:

        if (position.x <= -1.25) {
            left = false;
        }
        else if (position.x>=0) {
            left = true;
        }


        if (left) {
            movement = glm::vec3(-1, 0, 0);
        }
        else {
            movement = glm::vec3(1, 0, 0);
        }


        if (glm::distance(position.x, player->position.x) < 1.0f  && position.y==player->position.y) {
            AIState = ATTACKING;
            attack = true; 
            movement = glm::vec3(0);
        }
        break;
    case ATTACKING:
        animIndices = animAttack;
        animFrames = 9;
        break;
    }
}

void Entity::AIStalker(Entity *player) {
    switch (AIState) {
        case WALKING:
            if (player->position.x < position.x) {
                movement = glm::vec3(-1, 0, 0);
                animIndices = new int[8]{ 10,11,12,13,14,15, 16,17 };
                animFrames = 8;
            }
            else {
                movement = glm::vec3(1, 0, 0);
                animIndices = new int[8]{ 10,11,12,13,14,15, 16,17 };
                animFrames = 8;
            }
            
            break;
        case LURK:
            if (glm::distance(position, player->position) < 3.0f) {
                animIndices = new int[8]{ 37,36,35,34,33,32, 31,30 };
                animFrames = 8;
                AIState = WALKING;
            }
            break;
    }
}

void Entity::Update(float deltaTime, Entity* player,  Entity *platforms, int platformsCount, Entity *enemies, int enemyCount)
{
    if (isActive == false) {
        return;
    }
    
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entityType==ENEMY) {
        AI(player);
    }

    if (jump) {
        jump = false;
        velocity.y += jumpPower;
    }
    
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;


    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(platforms, platformsCount);// Fix if needed
    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(platforms, platformsCount);// Fix if needed

    if (entityType == PLAYER) {
        CheckCollisionsY(enemies, enemyCount);// Fix if needed
        CheckCollisionsX(enemies, enemyCount);// Fix if needed
        /*for (int i = 0; i++; i < enemyCount) {
            if (((enemies[i].position.x - position.x) < 10.0f) && attack) {
                enemies[i].isActive = false;
            }
        }*/
    }
    if (entityType==ENEMY) {
        CheckCollisionsY(player, 1);// Fix if needed
        CheckCollisionsX(player, 1);// Fix if needed
    }

    if (animIndices != NULL) {
        if (glm::length(movement) != 0 || attack == true) {
            if (attack == true) {
                animIndices = animAttack;
                attack = false;
            }

            animTime += deltaTime;

            if (animTime >= 0.25f)
            {
                animTime = 0.0f;
                animIndex++;
                if (animIndex >= animFrames)
                {
                    animIndex = 0;
                }
            }

        }
        else {
            animIndex = 0;
        }
    }



    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);

    if (entityType == PLAYER || entityType == ENEMY) {
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.0f));
    }
}

bool Entity::CheckCollision(Entity* other) {

    if (other == this) return false;

    if (isActive == false|| other->isActive==false) {
        return false;
    }


    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) {
        return true;
    }
    return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{


    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                collidedTop = true;
                velocity.y = 0;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
                object->collidedTop = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{


    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
                object->collidedLeft = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
                object->collidedRight = true;
            }
        }
    }
}



void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;

    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program) {

    if (isActive == false) {
        return;
    }


    program->SetModelMatrix(modelMatrix);

    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}