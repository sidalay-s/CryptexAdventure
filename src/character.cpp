#include "headers/character.hpp"

Character::Character(Sprite Idle, 
                     Sprite Walk, 
                     Sprite Run, 
                     Sprite Attack, 
                     Sprite Hurt, 
                     Sprite Death, 
                     Sprite Push, 
                     Sprite Sleep, 
                     Sprite ItemGrab,
                     Texture2D Interact, 
                     Window* Screen, 
                     Background* World)
    : Idle{Idle},
      Walk{Walk},
      Run{Run},
      Attack{Attack},
      Hurt{Hurt},
      Death{Death},
      Push{Push},
      Sleep{Sleep},
      ItemGrab{ItemGrab},
      Interact{Interact},
      Screen{Screen},
      World{World}
{
    WorldPos = Vector2Subtract(WorldPos, Offset);
}

void Character::Tick(float DeltaTime, Props& Props, std::vector<Enemy>& Enemies, std::vector<Prop>& Trees)
{
    UpdateCharacterPos();

    SpriteTick(DeltaTime);

    CheckDirection();

    WalkOrRun();

    CheckAttack();

    UpdateSource();

    CheckMovement(Props, Enemies, Trees);

    CheckEmotion();

    CheckSleep();

    CheckHealing();

    IsAlive();
}

// Draw character animation
void Character::Draw()
{
    DrawTexturePro(CurrentSprite->Texture, Source, Destination, Vector2{}, 0.f, WHITE);
}

void Character::SpriteTick(float DeltaTime)
{
    Idle.Tick(DeltaTime);
    Walk.Tick(DeltaTime);
    Run.Tick(DeltaTime);
    Attack.Tick(DeltaTime);
    Hurt.Tick(DeltaTime);
    Death.Tick(DeltaTime);
    Push.Tick(DeltaTime);
    Sleep.Tick(DeltaTime);
    ItemGrab.Tick(DeltaTime);
}

// Update Character to middle of screen if screen is resized
void Character::UpdateCharacterPos()
{
    CharacterPos.x = Screen->x/2.f - (Scale * (0.5f * CurrentSprite->Texture.width/CurrentSprite->MaxFramesX));
    CharacterPos.y = Screen->y/2.f - (Scale * (0.5f * CurrentSprite->Texture.height/CurrentSprite->MaxFramesY));
    Destination.x = CharacterPos.x;
    Destination.y = CharacterPos.y;
    Destination.width = Scale * CurrentSprite->Texture.width/CurrentSprite->MaxFramesX;    
    Destination.height = Scale * CurrentSprite->Texture.height/CurrentSprite->MaxFramesY;    
}

// Check which orientation the character is facing
void Character::CheckDirection()
{
    if (!Locked)
    {
        if (IsKeyDown(KEY_W)) Face = Direction::UP;
        if (IsKeyDown(KEY_A)) Face = Direction::LEFT;
        if (IsKeyDown(KEY_S)) Face = Direction::DOWN;
        if (IsKeyDown(KEY_D)) Face = Direction::RIGHT;
    }

        switch (Face)
        {
            case Direction::DOWN: 
                CurrentSprite->FrameY = 0;
                break;
            case Direction::LEFT: 
                CurrentSprite->FrameY = 1;
                break;
            case Direction::RIGHT:
                CurrentSprite->FrameY = 2;
                break;
            case Direction::UP: 
                CurrentSprite->FrameY = 3;
                break;
        }
}

// Check for movement input
void Character::CheckMovement(Props& Props, std::vector<Enemy>& Enemies, std::vector<Prop>& Trees)
{
    PrevWorldPos = WorldPos;
    Vector2 Direction{};

    if (!Locked) {

        if (IsKeyDown(KEY_W)) {
            Direction.y -= Speed;
        }
        if (IsKeyDown(KEY_A)) {
            Direction.x -= Speed;
        }
        if (IsKeyDown(KEY_S)) {
            Direction.y += Speed;
        }
        if (IsKeyDown(KEY_D)) {
            Direction.x += Speed;
        }

        if (Vector2Length(Direction) != 0.f) {
            // set MapPos -= Direction
            WorldPos = Vector2Add(WorldPos, Vector2Scale(Vector2Normalize(Direction), Speed));
        }

        // Undo Movement if walking out-of-bounds
        if (WorldPos.x + CharacterPos.x < 0.f - (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/2.f||
            WorldPos.y + CharacterPos.y < 0.f - (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/2.f||
            WorldPos.x + (Screen->x - CharacterPos.x) > World->GetMapSize().x * World->GetScale() + (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/2.f ||
            WorldPos.y + (Screen->y - CharacterPos.y) > World->GetMapSize().y * World->GetScale() + (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/2.f)
        {
            UndoMovement();
        }

    }

    CheckCollision(Props.Under, Direction, Enemies, Trees);
    CheckCollision(Props.Over, Direction, Enemies, Trees);
}

// Undo movement if walking out-of-bounds or colliding
void Character::UndoMovement()
{
    WorldPos = PrevWorldPos;
}

// Check if colliding with props / npcs / enemies 
void Character::CheckCollision(std::vector<std::vector<Prop>>* Props, Vector2 Direction, std::vector<Enemy>& Enemies, std::vector<Prop>& Trees)
{
    DamageTime += GetFrameTime();
    
    if (Collidable) {

        // Loop through all Props for collision
        for (auto& PropType:*Props) {
            for (auto& Prop:PropType) {
                if (Prop.HasCollision()) {
                    
                    // check physical collision
                    if (CheckCollisionRecs(GetCollisionRec(), Prop.GetCollisionRec(WorldPos))) {   
                        
                        // manage pushable props
                        if (Prop.IsMoveable()) {
                            if (Prop.GetType() == PropType::BOULDER || Prop.GetType() == PropType::STUMP) {
                                Colliding = true;   
                                if(!Prop.IsOutOfBounds()) {
                                    if (Prop.CheckMovement(*World, WorldPos, Direction, Speed, Props)) {
                                        UndoMovement();
                                    }
                                }
                                else {
                                    UndoMovement();
                                }
                            }
                            if (Prop.GetType() == PropType::GRASS) {
                                Prop.SetActive(true);
                            }
                        }

                        // if not pushable, block movement   
                        else {
                            if (Prop.IsSpawned()) {
                                UndoMovement();
                            }
                        }
                    }
                    else {
                        Prop.SetActive(false);
                    }

                    // check interactable collision
                    if (CheckCollisionRecs(GetCollisionRec(), Prop.GetInteractRec(WorldPos))) {
                        // Check for interact collision to display ! over character
                        if (Prop.IsInteractable()) {
                            if (Prop.IsSpawned()) {
                                Interactable = true;
                            }
                        }

                        // Manage interacting with props
                        if (Interactable == true) {
                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE))
                                Interacting = true;
                                    
                            if (Interacting == true) {
                                Prop.SetActive(true);
                                Interactable = false;
                                Locked = true;
                            }

                            if (Prop.IsOpened()) {
                                Interacting = false;
                                Interactable = false;
                                Prop.SetActive(false);
                                Locked = false;

                                // Make NPC's interactable again
                                if (Prop.GetType() == PropType::NPC_A || Prop.GetType() == PropType::NPC_B || Prop.GetType() == PropType::NPC_C || Prop.GetType() == PropType::ANIMATEDALTAR)
                                {
                                    Prop.SetOpened(false);
                                }
                            }
                        }
                    }
                }
                else {
                    Interactable = false;
                    Colliding = false;
                }
            }
        }

        // Loop for tree collision
        for (auto& Tree:Trees) {
            if (Tree.HasCollision()) {
                if (CheckCollisionRecs(GetCollisionRec(), Tree.GetCollisionRec(WorldPos))) {
                    if (Tree.IsSpawned()) {
                        UndoMovement();
                    }
                }
            }
        }
        
        // Loop through all Enemies for collision
        for (auto& Enemy:Enemies) {

            if (Enemy.GetType() != EnemyType::NPC) {
                if (Enemy.IsAlive()) {

                    // Check collision of Player against Enemy
                    if (CheckCollisionRecs(GetCollisionRec(), Enemy.GetCollisionRec())) {
                        if (!Enemy.IsDying()) {
                            TakeDamage();
                        }
                    }

                    // Check collision of Player's attack against Enemy
                    if (!Enemy.IsInvulnerable()) {
                        if (Attacking) {
                            if (CheckCollisionRecs(GetAttackRec(), Enemy.GetCollisionRec())) {
                                Enemy.Damaged(true);
                            }
                        }
                        else {
                            Enemy.Damaged(false);
                        }
                    }

                    // Check if Enemy Attack Collision is hitting Player
                    if (Enemy.IsAttacking()) {
                        if (CheckCollisionRecs(GetCollisionRec(), Enemy.GetAttackRec())) {
                            if (!Enemy.IsDying()) {
                                TakeDamage();
                            }
                        }
                    }

                    // Heal fox if enemy is killed
                    if (Enemy.IsDying()) {
                        if (Enemy.GetMaxHP() >= 8) {
                            if (Enemy.GetHealth() <= 0) {
                                Healing = true;
                                AmountToHeal = 4.f;
                            }
                        }
                        else if (Enemy.GetMaxHP() >= 5) {
                            if (Enemy.GetHealth() <= 0) {
                                Healing = true;
                                AmountToHeal = 2.5f;
                            }
                        }
                        else {
                            if (Enemy.GetHealth() <= 0) {
                                Healing = true;
                                AmountToHeal = 1.5f;
                            }
                        }
                    }
                }
                if (Hurting) {
                    if (DamageTime >= 1.f) {
                        Hurting = false;
                    }
                }
            }
        }
    }
}

// Check if character is moving and change sprites if needed
void Character::WalkOrRun()
{
    if (IsKeyDown(KEY_LEFT_SHIFT))
    {
        Running = true;

        if (Colliding)
            Speed = 0.9f;
        else 
            Speed = 2.5f;
    }
    else 
    {
        Running = false;

        if (Colliding)
            Speed = 0.4f;
        else
            Speed = 1.5f;

    }
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D))
    {
        Walking = true;
        Sleeping = false;
    }
    else
    {
        Walking = false;
    }

    if (Running && Walking) 
    {
        CurrentSprite = &Run;
    }
    else if (Walking) 
    {
        CurrentSprite = &Walk;
    }
    else if (Sleeping)
    {
        CurrentSprite = &Sleep;
    }
    else 
    {
        CurrentSprite = &Idle;
    }
}

// manage attack sprites
void Character::CheckAttack()
{
    AttackTime += GetFrameTime();
    float AttackResetTime{0.7f};

    if (!Locked) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE)) {
            // Attack animation & damage window lasts 0.4 seconds
            if (AttackTime < 0.4f) {
                Sleeping = false;
                Attacking = true;
                CurrentSprite = &Attack;
            }
            // Reset window when reaching AttackResetTime
            else if (AttackTime >= AttackResetTime) {
                Attacking = false;
                AttackTime = 0.0f;
            }
            // 0.3 cooldown before Attack Window opens up again
            else {
                Attacking = false;
            }
        }
        else {
            Attacking = false;
        }        
    }
    else {
        Attacking = false;
    }
}

// manage sleep skill
void Character::CheckSleep()
{
    float DeltaTime{GetFrameTime()};
    float UpdateTime{2.f/1.f};

    if (Sleeping)
    {
        RunningTime += DeltaTime;    

        if (RunningTime >= UpdateTime) {
            if (Health < 11.f) {
                AddHealth(0.5f);
                RunningTime = 0.f;
            }
        }
    }
}

// manage character portraits 
void Character::CheckEmotion()
{
    if (Health < 1.f)
        State = Emotion::DEAD;
    else if (Attacking)
        State = Emotion::ANGRY;
    else if (Hurting)
        State = Emotion::HURT;
    else if (Sleeping)
        State = Emotion::SLEEPING;
    else if (Health >= 10.f)
        State = Emotion::HAPPY;
    else if (Health > 6.f && Health < 10.f)
        State = Emotion::DEFAULT;
    else if (Health > 3.f && Health <= 6.f)
        State = Emotion::NERVOUS;
    else if (Health > 1.f && Health <= 3.f)
        State = Emotion::SAD;
}

// Draw ! when interactable is true
void Character::DrawIndicator() 
{
    if (Interactable) {
        DrawTextureEx(Interact, Vector2Subtract(CharacterPos, Vector2{-58, -20}), 0.f, 2.f, WHITE);
    }
};

// Update which portion of the spritesheet is drawn
void Character::UpdateSource()
{
    Source.x = CurrentSprite->FrameX * CurrentSprite->Texture.width / CurrentSprite->MaxFramesX;
    Source.y = CurrentSprite->FrameY * CurrentSprite->Texture.height / CurrentSprite->MaxFramesY;
    Source.width = CurrentSprite->Texture.width/CurrentSprite->MaxFramesX;
    Source.height = CurrentSprite->Texture.height/CurrentSprite->MaxFramesY;
}

// Return character collision dimensions
Rectangle Character::GetCollisionRec()
{
    return Rectangle 
    {
        CharacterPos.x + CurrentSprite->Texture.width/CurrentSprite->MaxFramesX/2.f,
        CharacterPos.y + CurrentSprite->Texture.height/CurrentSprite->MaxFramesY/2.f,
        ((CurrentSprite->Texture.width/CurrentSprite->MaxFramesX) - (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/1.5f) * Scale,
        ((CurrentSprite->Texture.height/CurrentSprite->MaxFramesY) - (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/1.5f)  * Scale
    };
}

// Return attack collision rectangle
Rectangle Character::GetAttackRec()
{
    switch (Face)
    {
        case Direction::DOWN:
            return Rectangle
            {
                CharacterPos.x + CurrentSprite->Texture.width/CurrentSprite->MaxFramesX/2.f,
                CharacterPos.y + (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY/2.f * 2),
                ((CurrentSprite->Texture.width/CurrentSprite->MaxFramesX) - (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/1.5f) * Scale,
                ((CurrentSprite->Texture.height/CurrentSprite->MaxFramesY) - (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/1.5f)  * Scale
            }; 
        case Direction::LEFT: 
            return Rectangle
            {
                CharacterPos.x,
                CharacterPos.y + (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY/2.f),
                ((CurrentSprite->Texture.width/CurrentSprite->MaxFramesX) - (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/1.5f) * Scale,
                ((CurrentSprite->Texture.height/CurrentSprite->MaxFramesY) - (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/1.5f)  * Scale
            }; 
        case Direction::RIGHT:
            return Rectangle
            {
                CharacterPos.x + (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX/2.f * 2),
                CharacterPos.y + CurrentSprite->Texture.height/CurrentSprite->MaxFramesY/2.f,
                ((CurrentSprite->Texture.width/CurrentSprite->MaxFramesX) - (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/1.5f) * Scale,
                ((CurrentSprite->Texture.height/CurrentSprite->MaxFramesY) - (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/1.5f)  * Scale
            }; 
        case Direction::UP:
            return Rectangle
            {
                CharacterPos.x + CurrentSprite->Texture.width/CurrentSprite->MaxFramesX/2.f,
                CharacterPos.y,
                ((CurrentSprite->Texture.width/CurrentSprite->MaxFramesX) - (CurrentSprite->Texture.width/CurrentSprite->MaxFramesX)/1.5f) * Scale,
                ((CurrentSprite->Texture.height/CurrentSprite->MaxFramesY) - (CurrentSprite->Texture.height/CurrentSprite->MaxFramesY)/1.5f)  * Scale
            };
        default:
            return Rectangle
            {
                
            };
    }

}

// Check if Player is Alive and if not set dead sprite
void Character::IsAlive()
{
    if (Health <= 1.f)
    {
        Locked = true;
        Alive = false;
        CurrentSprite = &Death;
    }
    else
    {
        Locked = false;
        Alive = true;
    }
}

// Manage Player animation when taking damage
void Character::TakeDamage()
{
    float UpdateTime {2.f/1.f};
    float HurtUpdateTime{1.f};
    // float KnockBack{2.f};

    // Cancel sleeping state if attacked
    if (Sleeping) {
        Sleeping = false;
    }

    // How often the hurt animation should play
    if (DamageTime <= HurtUpdateTime) {
        CurrentSprite = &Hurt;
        Hurting = true;
    }

    // How often health should decrease when colliding into enemy
    if (DamageTime >= UpdateTime) {
        if (Health > 0.f) {
            Health -= 0.5f;
        }
        DamageTime = 0.f;
    }


    // Knock player back a few units while hurt
    // if (DamageTime <= HurtUpdateTime) {
    //     switch (Face) 
    //     {
    //         case Direction::UP:
    //             WorldPos.y += KnockBack;
    //             break;
    //         case Direction::DOWN:
    //             WorldPos.y -= KnockBack;
    //             break;
    //         case Direction::LEFT:
    //             WorldPos.x += KnockBack;
    //             break;
    //         case Direction::RIGHT:
    //             WorldPos.x -= KnockBack;
    //             break;
    //     }
    // }
}

// -------------------------------------------------------- //

// Debug Function
void Character::AddHealth(float HP)
{
    Health += HP;
    if (Health > 11) {
        Health = 11;
    }
}

// Gradually heal fox when killing an enemy
void Character::HealOverTime(float HP)
{
    float TimeToHeal{2.f/3.f};
    HealTime += GetFrameTime();
    static float StopHealing{HP};

    if (StopHealing >= HP) {
        StopHealing = 0.f;
        Healing = false;
        return;
    }
    else {
        if (HealTime >= TimeToHeal) {
            AddHealth(0.5f);
            StopHealing += 0.5f;
            HealTime = 0.f;
        }
    }
}

void Character::CheckHealing()
{
    if (Healing) {
        HealOverTime(AmountToHeal);
    }
}