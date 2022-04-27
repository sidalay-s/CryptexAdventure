#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include "headers/prop.hpp"
#include "headers/sprite.hpp"
#include "headers/enemy.hpp"
#include "headers/window.hpp"
#include "headers/background.hpp"
#include "headers/basecharacter.hpp"

enum class Emotion 
{
    DEFAULT, ANGRY, HAPPY, NERVOUS, SAD, SLEEPING, HURT, DEAD
};

class Character : public BaseCharacter
{
private:
    Sprite Idle{};
    Sprite Walk{};
    Sprite Run{};
    Sprite Attack{};
    Sprite Hurt{};
    Sprite Death{};
    Sprite Push{};
    Sprite Sleep{};
    Sprite ItemGrab{};
    Sprite* CurrentSprite{&Idle};  
    Texture2D Interact{LoadTexture("sprites/props/Interact.png")};
    std::vector<Sprite*> Sprites {};

    Window* Screen{};
    Background* World{};
    Vector2 Offset{615,335};             // Player offset vs Enemy/Prop WorldPos
    Vector2 CharacterPos{};              // Where the character is on the screen
    Vector2 WorldPos{362.f,2594.f};      // Where the character is in the world
    Vector2 PrevWorldPos{};
    Rectangle Source{};
    Rectangle Destination{};
    
    float Health{11.f};
    float Scale{1.5f};
    float Speed{1.0f};
    float RunningTime{};
    float DamageTime{};
    float AttackTime{};
    bool Colliding{false};
    bool Locked{false};
    bool Walking{false};
    bool Running{false};
    bool Attacking{false};
    bool Sleeping{false};
    bool Interacting{false};
    bool Interactable{false};
    bool Hurting{false};
    Emotion State{Emotion::DEFAULT};
    Direction Face{Direction::DOWN};

public:
    Character(Sprite Idle, 
              Sprite Walk, 
              Sprite Run, 
              Sprite Attack, 
              Sprite Hurt, 
              Sprite Death, 
              Sprite Push, 
              Sprite Sleep, 
              Sprite ItemGrab, 
              Window* Screen, 
              Background* World);
    ~Character();
    
    void Tick(float DeltaTime, Props& Props, std::vector<Enemy>& Enemies);
    void Draw();
    void SpriteTick(float DeltaTime);
    void UpdateCharacterPos();
    void CheckDirection();
    void CheckMovement(Props& Props, std::vector<Enemy>& Enemies);
    void UndoMovement();
    void CheckCollision(std::vector<std::vector<Prop>>* Props, Vector2 Direction, std::vector<Enemy>& Enemies);
    void WalkOrRun();
    void CheckAttack();
    void UpdateSource();
    void CheckSleep();
    void CheckEmotion();
    void IsAlive();
    void SetSleep() {Sleeping = !Sleeping;}
    void DrawIndicator();
    void TakingDamage();

    float GetHealth() {return Health;}
    float GetSpeed() {return Speed;}
    Vector2 GetWorldPos() {return WorldPos;}
    Vector2 GetPrevWorldPos() {return PrevWorldPos;}
    Vector2 GetCharPos() {return CharacterPos;}
    Emotion GetEmotion() {return State;}
    Rectangle GetCollisionRec();
    Rectangle GetAttackRec();

    // Debug function
    void AddHealth(float HP);
};

#endif // CHARACTER_HPP

/*
    TODO:
        [x] add attack cooldown
        [x] add damage knockback
        [x] add half-hearts to hp
        [x] fix hurt portrait not showing up
        [ ] redo attacking and combos to feel less clunky
*/