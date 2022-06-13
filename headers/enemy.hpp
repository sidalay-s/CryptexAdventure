#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <random>
#include <unordered_map>
#include "prop.hpp"
#include "window.hpp"

enum class EnemyType 
{
    BEAR, BEHOLDER, CREATURE, GHOST, 
    IMP, MUSHROOM, NECROMANCER, SHADOW, 
    SPIDER, TOAD, NORMAL, BOSS, NPC,
    FOX, CROW, SQUIRREL
};

enum class Monster 
{
    IDLE, WALK, ATTACK, HURT, DEATH, PROJECTILE
}; 

enum class NPC 
{
    IDLE, IDLETWO, WALK, MISC, SLEEP
};

struct Enemies;

class Enemy
{
public:
    // Enemy constructor
    Enemy(const Sprite& Idle,
          const Sprite& Walk,
          const Sprite& Attack,
          const Sprite& Hurt,
          const Sprite& Death,
          const Sprite& Projectile,
          EnemyType Race,
          EnemyType Type,
          Vector2 WorldPos,
          Window& Screen,
          Background& World,
          GameTexture& GameTextures,
          int Health = 3,
          float Scale = 3.2f);
    
    // Wildlife NPC constructor
    Enemy(const Sprite& NpcIdle,
          const Sprite& NpcIdleTwo,
          const Sprite& NpcWalk,
          const Sprite& NpcMisc,
          const Sprite& NpcSleep,
          EnemyType Race,
          Vector2 WorldPos,
          Window& Screen,
          Background& World,
          GameTexture& GameTextures,
          float Scale = 3.2f);

    void Tick(float DeltaTime, Props& Props, Vector2 HeroWorldPos, Vector2 HeroScreenPos, std::vector<Enemy>& Enemies, std::vector<Prop>& Trees);
    void Draw(Vector2 HeroWorldPos);
    void SpriteTick(float DeltaTime);
    void UpdateScreenPos(Vector2 HeroWorldPos);
    void CheckDirection();
    void CheckMovement(Props& Props, Vector2 HeroWorldPos, Vector2 HeroScreenPos, std::vector<Enemy>& Enemies, std::vector<Prop>& Trees);
    void NeutralAction();
    void UndoMovement();
    void CheckOutOfBounds();
    void CheckCollision(std::vector<std::vector<Prop>>& Props, Vector2 HeroWorldPos, std::vector<Enemy>& Enemies, std::vector<Prop>& Trees);
    void CheckAttack();
    void UpdateSource();
    void TakeDamage();
    void Damaged(bool Attacked) {IsAttacked = Attacked;}
    void CheckAlive(float DeltaTime);
    void EnemyAI();
    void EnemyAggro(Vector2 HeroScreenPos);
    void DrawHP();
    void CheckBossSummon(Vector2 HeroWorldPos);

    bool IsBlocked() {return Blocked;}
    bool IsAlive() {return Alive;}
    bool IsInvulnerable() {return Invulnerable;}
    bool IsAttacking() {return Attacking;}
    bool IsDying() {return Dying;}
    bool WithinScreen(Vector2 HeroWorldPos);
    int GetHealth() {return Health;}
    int GetMaxHP() {return MaxHP;}
    int GetMonstersKilled() {return MonsterDeaths;}
    int GetTotalMonsters() {return MonsterCount;}
    Vector2 GetWorldPos() {return WorldPos;}
    Vector2 GetPrevWorldPos() {return PrevWorldPos;}
    Vector2 GetEnemyPos() {return ScreenPos;}
    Vector2 UpdateProjectile();
    EnemyType GetType() {return Type;}
    EnemyType GetRace() {return Race;}
    Rectangle GetCollisionRec();
    Rectangle GetAttackRec();
    int GetMonsterCount(EnemyType Type) {return MonsterCounter.at(Type);}
    
private:
    std::vector<Sprite> Sprites{};

    EnemyType Race{};                    // What kind of Enemy
    EnemyType Type{};                    // If the Enemy is NORMAL, BOSS, or NPC type

    Vector2 ScreenPos{};                  // Where the character is on the screen
    Vector2 WorldPos{};                  // Where the character is in the world
    Vector2 PrevWorldPos{};
    Vector2 Movement{};
    Rectangle Source{};
    Rectangle Destination{};
    Window& Screen;
    Background& World;
    GameTexture& GameTextures;
    
    static int MonsterDeaths;
    static int MonsterCount;
    int CurrentSpriteIndex{};
    int ShootingSpriteIndex{};
    int Health{};
    int MaxHP{};
    int LeftOrRight{};
    int ActionState{};
    float Scale{3.2f};
    float Speed{1.4f};
    float Radius{25.f};                 
    float MaxRange{150.f};
    float MinRange{60.f};
    float MinCollisionRange{0.5f};
    float RunningTime{};
    float DamageTime{};
    float StopTime{};
    float ActionTime{};
    float ActionIdleTime{};
    float MoveXRange{};
    float MoveYRange{};
    float AIX{0.6f};
    float AIY{0.6f};
    float Trajectory{1.f};
    bool Alive{true};
    bool Colliding{false};
    bool Attacking{false};
    bool Invulnerable{false};
    bool Chasing{false};
    bool Walking{false};
    bool IsAttacked{false};
    bool Stopped{false};            // Whether or not the enemy should be able to move
    bool Dying{false};
    bool Blocked{false};            // Whether enemy is colliding with an object or not
    bool Hurting{false};
    bool Intro{true};
    bool OOB{false};
    bool Ranged{false};
    bool Summoned{true};
    bool Visible{false};

    // Wildlife NPC
    bool IdleTwo{false};
    bool Sleeping{false};
    bool MiscAction{false};
    
    Direction Face{Direction::DOWN};

    static std::unordered_map<EnemyType, int> MonsterCounter;
};

#endif // ENEMY_HPP

/* 
    TODO: 
        [x] figure out orientation when chasing   
        [x] add min range to enemy aggro         
        [x] implement new undo movement system                    
        [x] bounce back when being attacked      
        [x] add movement ai                      
        [x] implement attack
        [x] add randomization to movement
        [x] fix multiple enemy instances
        [x] prevent enemies from 'overlapping' and walking over each other
        [x] fix OutOfBounds if check parameters
        [x] fix prop collision not being registered correctly sometimes
        [x] add HP bars
        [x] fix death animation not ticking correctly
        [x] add tombstone for death animation
        [x] add and implement enemy types
        [x] add different enemy attack types ranged/melee
        [x] add monster counter by EnemyType
        [x] add 'boss' after defeating all of one enemy type
        [x] fix enemy collision rec
        [x] wildlife npcs
        [x] refactor crow collision and movement ai
*/