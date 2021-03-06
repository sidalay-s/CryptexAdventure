#include "prop.hpp"

// Constructor for inanimate props
Prop::Prop(const Texture2D& Texture, 
           const Vector2 Pos, 
           const PropType Type, 
           const GameTexture& GameTextures, 
           const GameAudio& Audio,
           const float Scale, 
           const bool Moveable, 
           const bool Interactable,
           const bool Invisible)
    : Object{Texture}, Type{Type}, GameTextures{GameTextures}, Audio{Audio}, WorldPos{Pos}, Scale{Scale}, Interactable{Interactable}, 
      Moveable{Moveable}, Invisible{Invisible}
{
    if (Type == PropType::BOULDER ||
        Type == PropType::BUSH ||
        Type == PropType::STUMP ||
        Type == PropType::TREE ||
        Type == PropType::FENCE ||
        Type == PropType::rHOUSELEFT ||
        Type == PropType::rHOUSERIGHT ||
        Type == PropType::bHOUSELEFT ||
        Type == PropType::bHOUSERIGHT ||
        Type == PropType::DOOR ||
        Type == PropType::GRASS ||
        Type == PropType::TOPWALL ||
        Type == PropType::LEFTSIDEWALL ||
        Type == PropType::RIGHTSIDEWALL ||
        Type == PropType::BOTTOMWALL ||
        Type == PropType::HOLE ||
        Type == PropType::ROCK ||
        Type == PropType::DUNGEONLEFT ||
        Type == PropType::DUNGEONRIGHT ||
        Type == PropType::DUNGEON ||
        Type == PropType::ALTAR)
    {
        Collidable = true;
    }
}

// Constructor for animated altar pieces
Prop::Prop(const Sprite& Object, 
           const Vector2 Pos, 
           const PropType Type, 
           const GameTexture& GameTextures, 
           const GameAudio& Audio,
           const std::string& ItemName, 
           const bool Spawned, 
           const bool Interactable)
    : Object{Object}, Type{Type}, GameTextures{GameTextures}, Audio{Audio}, WorldPos{Pos}, Interactable{Interactable}, Spawned{Spawned}, ItemName{ItemName} 
{
    Collidable = true;
}

// Constructor for animated props
Prop::Prop(const Sprite& Object, 
           const Vector2 Pos, 
           const PropType Type, 
           const GameTexture& GameTextures, 
           const GameAudio& Audio,
           const Texture2D& Item, 
           const float Scale, 
           const bool Moveable, 
           const bool Interactable,
           const Progress TriggerAct, 
           const PropType TriggerNPC,  
           const bool Spawn,
           const std::string& ItemName, 
           const float ItemScale)
    : Object{Object}, Type{Type}, GameTextures{GameTextures}, Audio{Audio}, WorldPos{Pos}, Scale{Scale}, Interactable{Interactable}, Moveable{Moveable}, 
      Spawned{Spawn}, TriggerAct{TriggerAct}, TriggerNPC{TriggerNPC}, Item{Item}, ItemName{ItemName}, ItemScale{ItemScale} 
{
    if (Type == PropType::GRASS ||
        Type == PropType::TREASURE ||
        Type == PropType::NPC_DIANA ||
        Type == PropType::NPC_JADE ||
        Type == PropType::NPC_SON ||
        Type == PropType::NPC_RUMBY ||
        Type == PropType::ALTAR ||
        Type == PropType::BIGTREASURE)
    {
        Collidable = true;
    }

    if (Type == PropType::TREASURE || Type == PropType::BIGTREASURE) {
        if (ItemScale == 1.f) {
            ItemPos.x = 24.f;
        }
        else if (ItemScale == 2.f) {
            ItemPos.x = 16.f;
        }
        else if (ItemScale == 3.f) {
            ItemPos.x = 8.f;
        }
        else if (ItemScale == 4.f) {
            ItemPos.x = 0.f;
        }
        else if (ItemScale == 5.f) {
            ItemPos.x = -8.f;
        }
        else if (ItemScale == 6.f) {
            ItemPos.x = -16.f;
        }
        else  {
            ItemPos.x = 0.f;
        }
    }
}

void Prop::Tick(const float DeltaTime)
{
    if (Visible) {
        if ((Type == PropType::NPC_DIANA || 
             Type == PropType::NPC_JADE || 
             Type == PropType::NPC_SON || 
             Type == PropType::NPC_RUMBY) && !Talking) 
        {
            NpcTick(DeltaTime);
        }

        if (Type == PropType::ANIMATEDALTAR) {
            AltarTick(DeltaTime);
        }
        
        if (Active) {
            switch (Type)
            {
                case PropType::GRASS:
                    Object.Tick(DeltaTime);
                    break;
                case PropType::TREASURE:
                    TreasureTick(DeltaTime);
                    break;
                case PropType::BIGTREASURE:
                    if (FinalChestKey) {
                        TreasureTick(DeltaTime);
                    }
                    break;
                case PropType::DOOR:
                    Opened = true;
                    break;
                case PropType::NPC_DIANA:
                case PropType::NPC_JADE:
                case PropType::NPC_SON:
                case PropType::NPC_RUMBY:
                    TalkToNpc();
                    break;
                case PropType::ANIMATEDALTAR:
                    InsertAltarPiece();
                    break;
                case PropType::STUMP:
                    Reading = true;
                    break;
                default:
                    break;
            }
        }
        else if (!Active && Opening) {
            OpenChest(DeltaTime);
        }
    }
    UpdateNpcInactive();
    UpdateNewInfo();
    CheckFinalChest();
}

void Prop::Draw(const Vector2 CharacterWorldPos)
{
    Vector2 ScreenPos {Vector2Subtract(WorldPos, CharacterWorldPos)};

    CheckVisibility(CharacterWorldPos);

    if (Visible && !Invisible) {
        if (Type == PropType::BIGTREASURE) {
            if (FinalChestSpawned) {
                DrawTexturePro(Object.Texture, Object.GetSourceRec(), Object.GetPosRec(ScreenPos, Scale), Vector2{}, 0.f, WHITE);
            }
        }
        else {
            DrawTexturePro(Object.Texture, Object.GetSourceRec(), Object.GetPosRec(ScreenPos, Scale), Vector2{}, 0.f, WHITE);
        }

        // Draw the animated altar piece
        if (Type == PropType::ANIMATEDALTAR) {
            for (auto& Piece:AltarPieces) {
                if (std::get<2>(Piece) == true && std::get<0>(Piece) == ItemName) {
                    DrawTextureEx(Object.Texture, WorldPos, 0.f, Scale, WHITE);
                }
            }
        }

        if  (NewInfo && (Type == PropType::NPC_DIANA || Type == PropType::NPC_JADE || Type == PropType::NPC_SON || Type == PropType::NPC_RUMBY)) {
            DrawTextureEx(GameTextures.Interact, Vector2{Object.GetPosRec(ScreenPos, Scale).x + static_cast<float>(Object.Texture.width)/1.8f, Object.GetPosRec(ScreenPos, Scale).y - static_cast<float>(Object.Texture.height)/2.f}, 0.f, 2.f, WHITE);
        }
    }
    
    // Draw Treasure Box Item
    if (Opening) {
        DrawTextureEx(Item, Vector2Add(ScreenPos, ItemPos), 0.f, ItemScale, WHITE);
        ItemPos = Vector2Add(ItemPos, Vector2{0.f,-0.1f});
    }

    // Treasure Speech Box
    if (ReceiveItem) {
        DrawTextureEx(GameTextures.SpeechBox, Vector2{472.f,574.f}, 0.f, 8.f, WHITE);
        DrawPropText();
        TextAudio();
    }

    // Altar Pieces Inserted
    if (InsertPiece) {
        DrawTextureEx(GameTextures.SpeechBox, Vector2{472.f,574.f}, 0.f, 8.f, WHITE);
        DrawPropText();
        TextAudio();
    }

    // Foxy Secret Spot
    if (Reading) {
        DrawTextureEx(GameTextures.SpeechBox, Vector2{352.f,518.f}, 0.f, 12.f, WHITE);
        DrawPropText();
        TextAudio();
    }

    // NPC Speech Box
    if (Talking) {
        DrawTextureEx(GameTextures.SpeechName, Vector2{376.f,438.f}, 0.f, 5.f, WHITE);
        DrawTextureEx(GameTextures.SpeechBox, Vector2{352.f,518.f}, 0.f, 12.f, WHITE);

        if (Type == PropType::NPC_DIANA) {
            DrawText("Diana", 399, 490, 30, WHITE);
            DrawSpeech();
            TextAudio();
        }
        else if (Type == PropType::NPC_JADE) {
            DrawText("Jade", 399, 490, 30, WHITE);
            DrawSpeech();
            TextAudio();
        }
        else if (Type == PropType::NPC_SON) {
            DrawText("Louie", 399, 490, 30, WHITE);
            DrawSpeech();
            TextAudio();
        }
        else if (Type == PropType::NPC_RUMBY) {
            DrawText("Rumby", 393, 490, 30, WHITE);
            DrawSpeech();
            TextAudio();
        }
    }
}

void Prop::NpcTick(const float DeltaTime)
{
    Object.Tick(DeltaTime);

    // Update any progression and triggers for NPCs
    for (auto& [ParentNpc,Pair]:QuestlineProgress) {
        if (Type == Pair.second) {
            Act = Pair.first;
            Pair.first = Progress::ACT_O;
            Pair.second = PropType::NPC_O;
        }
    }
}

void Prop::AltarTick(const float DeltaTime)
{
    for (auto& Piece:AltarPieces) {
        if (ItemName == std::get<0>(Piece) && std::get<2>(Piece) == true) {
            Object.Tick(DeltaTime);
        }
    }
}

void Prop::OpenChest(const float DeltaTime)
{
    ReceiveItem = true;
    if (ItemName == "Cryptex") {
        CryptexReceived = true;
    }
    else {
        ++PiecesReceived;
    }
    // controls 'press enter' delay to close dialogue
    RunningTime += DeltaTime;
    if (RunningTime >= Object.UpdateTime * 6.f) {
        Opening = false;
        RunningTime = 0.f;
        TreasureAudio();
    }
}

void Prop::TreasureTick(const float DeltaTime)
{
    if (!Opened) {
        Object.Tick(DeltaTime);
    }

    RunningTime += DeltaTime;

    // controls duration of opening animation
    if (RunningTime > Object.UpdateTime * 4.f) {
        Opened = true;
        Opening = true;
        RunningTime = 0.f; 
    }

    for (auto& Piece:AltarPieces) {
        if (std::get<0>(Piece) == ItemName) {
            std::get<1>(Piece) = true;
        }
    }

    if (Type == PropType::BIGTREASURE) {
        BraceletReceived = true;
        FinalAct = true;
        MuteNewInfo = false;
    }

    if (TriggerAct != Progress::ACT_O) {
        QuestlineProgress.at(PropType::TREASURE).first = TriggerAct;
        QuestlineProgress.at(PropType::TREASURE).second = TriggerNPC;
    }
}

void Prop::CheckVisibility(const Vector2 CharacterWorldPos)
{
    if (Spawned && WithinScreen(CharacterWorldPos)) {
        Visible = true;
    }
    else {
        Visible = false;
    }
}

void Prop::InsertAltarPiece()
{
    AltarAudio();

    if (PiecesAdded == 0) {
        InsertPiece = true;
    }

    if (PiecesAdded >= 6) {
        FinalChestSpawned = true;
    }

    if (FirstPieceInserted) {
        for (auto& Piece:AltarPieces) {
            if (std::get<1>(Piece) == true) {
                std::get<2>(Piece) = true;
                InsertPiece = true;
            }
            if (std::get<2>(Piece) == true && std::get<3>(Piece) == false) {
                std::get<3>(Piece) = true;
                PiecesAdded++;
            }
        }

        Opened = true;
    }
    else {
        if (IsKeyPressed(KEY_ENTER)) {
            FirstPieceInserted = true;
        }
    }
}

void Prop::TalkToNpc()
{
    Talking = true;
    NewInfo = false;
    UpdateNpcActive();
}

void Prop::UpdateNpcInactive()
{
    if (!Visible) {
        switch (Type)
        {
            case PropType::NPC_DIANA:
            {
                if (BraceletReceived) {
                    Act = Progress::ACT_VIII;
                    WorldPos.x = 500.f;
                    WorldPos.y = 3653.f;
                }

                if (PiecesAdded >= 1 && PiecesAdded <= 5 && ReadyToProgress && !CryptexReceived) {
                    WorldPos.x = 3163.f;
                    WorldPos.y = 2853.f;
                    ReadyToProgress = false;
                }
                else if (PiecesAdded >= 1 && PiecesAdded <= 5 && !ReadyToProgress) {
                    Act = Progress::ACT_IV;
                }

                if (PiecesAdded == 6 && (Act == Progress::ACT_IV || Act == Progress::ACT_V)) {
                    WorldPos.x = 645.f;
                    WorldPos.y = 1777.f;
                    Act = Progress::ACT_VI;
                }
                break;
            }
            case PropType::NPC_JADE:
            {   
                if (BraceletReceived) {
                    Act = Progress::ACT_VIII;
                    WorldPos.x = 1060.f;
                    WorldPos.y = 3409.f;
                    MuteNewInfo = false;
                }
                else if (Act == Progress::ACT_I && SonSaved) {
                    WorldPos.x = 1549.f;
                    WorldPos.y = 2945.f;
                    ReadyToProgress = false;
                }
                else if (PiecesReceived >= 1) {
                    Act = Progress::ACT_IV;
                }
                break;
            }
            case PropType::NPC_SON:
            {
                if (BraceletReceived) {
                    Act = Progress::ACT_VIII;
                    WorldPos.x = 1160.f;
                    WorldPos.y = 3409.f;
                }
                else if (PiecesAdded >= 1 && PiecesAdded <= 4) {
                    Act = Progress::ACT_V;
                    ReadyToProgress = false;
                }
                else if (PiecesAdded > 4) {
                    Act = Progress::ACT_VI;
                }
                else if (Act == Progress::ACT_II && ReadyToProgress) {
                    WorldPos.x = 1283.f;
                    WorldPos.y = 2859.f;
                    Act = Progress::ACT_III;
                    ReadyToProgress = false;
                }
                break;
            }
            case PropType::NPC_RUMBY:
            {
                if (BraceletReceived) {
                    Act = Progress::ACT_VIII;
                    WorldPos.x = 769.f;
                    WorldPos.y = 3665.f;
                }
                break;
            }
            default:
                break;
        }
    }
}

void Prop::UpdateNpcActive()
{
    switch (Act) 
    {
        case Progress::ACT_I:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    if (PiecesReceived >= 1) {
                        Act = Progress::ACT_III;
                    }
                    else {
                        QuestlineProgress.at(PropType::NPC_DIANA).first = Progress::ACT_II; 
                        QuestlineProgress.at(PropType::NPC_DIANA).second = PropType::NPC_DIANA;
                    }
                    break;
                }
                case PropType::NPC_JADE:
                {
                    QuestlineProgress.at(PropType::NPC_JADE).first = Progress::ACT_II;
                    QuestlineProgress.at(PropType::NPC_JADE).second = PropType::NPC_SON;
                    ReadyToProgress = true;
                    break;
                }
                case PropType::NPC_SON:
                {
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    QuestlineProgress.at(PropType::NPC_RUMBY).first = Progress::ACT_II;
                    QuestlineProgress.at(PropType::NPC_RUMBY).second = PropType::NPC_RUMBY;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case Progress::ACT_II:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    QuestlineProgress.at(PropType::NPC_DIANA).first = Progress::ACT_I; 
                    QuestlineProgress.at(PropType::NPC_DIANA).second = PropType::NPC_DIANA;
                    break;
                }
                case PropType::NPC_JADE:
                {
                    QuestlineProgress.at(PropType::NPC_JADE).first = Progress::ACT_III;
                    QuestlineProgress.at(PropType::NPC_JADE).second = PropType::NPC_JADE;
                    ReadyToProgress = true;
                    break;
                }
                case PropType::NPC_SON:
                {
                    QuestlineProgress.at(PropType::NPC_SON).first = Progress::ACT_II;
                    QuestlineProgress.at(PropType::NPC_SON).second = PropType::NPC_JADE;
                    SonSaved = true;
                    ReadyToProgress = true;
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    QuestlineProgress.at(PropType::NPC_RUMBY).first = Progress::ACT_III;
                    QuestlineProgress.at(PropType::NPC_RUMBY).second = PropType::NPC_RUMBY;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case Progress::ACT_III:
        {
            switch (Type)
            {
                case PropType::NPC_JADE:
                {
                    if (PiecesReceived >= 1) {
                        Act = Progress::ACT_IV;
                    }
                    else {
                        Act = Progress::ACT_II;
                    }
                    break;
                }
                case PropType::NPC_SON:
                {
                    if (PiecesReceived >= 1) {
                        Act = Progress::ACT_IV;
                    }
                    ReadyToProgress = true;
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    QuestlineProgress.at(PropType::NPC_RUMBY).first = Progress::ACT_IV;
                    QuestlineProgress.at(PropType::NPC_RUMBY).second = PropType::NPC_RUMBY;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case Progress::ACT_IV:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    ReadyToProgress = true;
                    break;
                }
                case PropType::NPC_SON:
                {
                    ReadyToProgress = false;
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    QuestlineProgress.at(PropType::NPC_RUMBY).first = Progress::ACT_I;
                    QuestlineProgress.at(PropType::NPC_RUMBY).second = PropType::NPC_RUMBY;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case Progress::ACT_V:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    ReadyToProgress = true;
                    break;
                }
                default:
                    break;
            }
            break;    
        }
        case Progress::ACT_VI:
        {
            switch (Type)
            {
                default:
                    break;
            }
            break;
        }
        case Progress::ACT_VII:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    QuestlineProgress.at(PropType::NPC_DIANA).first = Progress::ACT_VI; 
                    QuestlineProgress.at(PropType::NPC_DIANA).second = PropType::NPC_DIANA;
                    FinalChestKey = true;
                    break;
                }
                default:
                    break;
            }
            break;    
        }
        case Progress::ACT_VIII:
        {
            switch (Type)
            {
                default:
                    break;
            }
            break;    
        }    
        case Progress::ACT_IX:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    QuestlineProgress.at(PropType::NPC_DIANA).first = Progress::ACT_VIII; 
                    QuestlineProgress.at(PropType::NPC_DIANA).second = PropType::NPC_DIANA;
                    break;
                }
                default:
                    break;
            }
            break;    
        }    
        default:
            break;
    }
}

void Prop::UpdateNewInfo()
{
    switch (Type)
    {
        case PropType::NPC_DIANA:
        {
            if (Act == Progress::ACT_VIII && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }

            if (PiecesReceived >= 1 && Act == Progress::ACT_I) {
                NewInfo = true;
            }

            if (PiecesAdded >= 1 && PiecesAdded <= 5 && Act == Progress::ACT_IV && !ReadyToProgress) {
                NewInfo = true;
            }

            if (CryptexReceived && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }

            if (PiecesAdded == 6 && (Act == Progress::ACT_IV || Act == Progress::ACT_V)) {
                NewInfo = true;
            }

            if (PiecesAdded == 6 && !FinalChestKey) {
                NewInfo = true;
            }

            break;
        }
        case PropType::NPC_JADE:
        {
            if (Act == Progress::ACT_VIII && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }

            if (Act == Progress::ACT_IV && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }
            
            if (Act == Progress::ACT_II && !ReadyToProgress) {
                NewInfo = true;
            }

            if (PiecesReceived >= 1 && (Act == Progress::ACT_II || Act == Progress::ACT_III)) {
                NewInfo = true;
            }

            break;
        }
        case PropType::NPC_SON:
        {
            if (Act == Progress::ACT_VIII && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }

            if (Act == Progress::ACT_VI && !ReadyToProgress) {
                NewInfo = true;
                ReadyToProgress = true;
                MuteNewInfo = false;
            }

            if (Act == Progress::ACT_V && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }  

            if (Act == Progress::ACT_III && !ReadyToProgress) {
                NewInfo = true;
                ReadyToProgress = true;
            }

            if (PiecesReceived >= 1 && PiecesReceived <= 4 && ReadyToProgress) {
                NewInfo = true;
                ReadyToProgress = false;
            }
            break;
        }
        case PropType::NPC_RUMBY:
        {
            if (Act == Progress::ACT_VIII && !MuteNewInfo) {
                NewInfo = true;
                MuteNewInfo = true;
            }
            break;
        }
        default:
            break;
    }
}

void Prop::CheckFinalChest()
{
    if (Type == PropType::BIGTREASURE && FinalChestSpawned) {
        Spawned = true;
    }
}

bool Prop::WithinScreen(const Vector2 CharacterWorldPos)
{
    if (
        (WorldPos.x >= (CharacterWorldPos.x + 615.f) - static_cast<float>((GetScreenWidth()/2 + (Object.Texture.width * Scale)))) && 
        (WorldPos.x <= (CharacterWorldPos.x + 615.f) + static_cast<float>((GetScreenWidth()/2 + (Object.Texture.width * Scale)))) &&
        (WorldPos.y >= (CharacterWorldPos.y + 335.f) - static_cast<float>((GetScreenHeight()/2 + (Object.Texture.height * Scale)))) && 
        (WorldPos.y <= (CharacterWorldPos.y + 335.f) + static_cast<float>((GetScreenHeight()/2 + (Object.Texture.height * Scale))))
       ) {
            return true;
         }
    else {
        return false;
    }
}

Rectangle Prop::GetCollisionRec(const Vector2 CharacterWorldPos)
{
    Vector2 ScreenPos {Vector2Subtract(WorldPos, CharacterWorldPos)};

    switch (Type)
    {
        case PropType::ALTAR:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                Object.Texture.height * Scale
            };
        }
        case PropType::ANIMATEDALTAR:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                (Object.Texture.width * Scale) / Object.MaxFramesX,
                (Object.Texture.height * Scale) / Object.MaxFramesY
            };
        }
        case PropType::TREE:
        {
            return Rectangle
            {
                ScreenPos.x + ((Object.Texture.width*.23f) * Scale),
                ScreenPos.y + ((Object.Texture.height*.75f) * Scale),
                (Object.Texture.width - Object.Texture.width*.35f) * Scale,
                (Object.Texture.height - Object.Texture.height*.75f) * Scale
            };
        }
        case PropType::ROCK:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                Object.Texture.height * Scale
            };
        }
        case PropType::BOULDER:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                (Object.Texture.width - Object.Texture.width*.125f) * Scale,
                (Object.Texture.height - Object.Texture.height*.125f) * Scale   
            };
        }
        case PropType::FLOWER:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                Object.Texture.height * Scale
            };
        }
        case PropType::FENCE:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y + (Object.Texture.height * Scale) * .1f,
                Object.Texture.width * Scale,
                (Object.Texture.height * Scale) * .9f
            };
        }
        case PropType::GRASS:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .05f,
                ScreenPos.y + (Object.Texture.height * Scale) * .4f,
                (Object.Texture.width - Object.Texture.width * .875f) * Scale,
                (Object.Texture.height - Object.Texture.height * .65f) * Scale
            };
        }
        case PropType::rHOUSELEFT:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .08f,
                ScreenPos.y + (Object.Texture.height * Scale) * .6f,
                (Object.Texture.width * Scale) * .43f,
                (Object.Texture.height * Scale) * .3f
            };
        }
        case PropType::rHOUSERIGHT:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .75f,
                ScreenPos.y + (Object.Texture.height * Scale) * .6f,
                (Object.Texture.width * Scale) * .19f,
                (Object.Texture.height * Scale) * .3f
            };
        }
        case PropType::bHOUSELEFT:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .08f,
                ScreenPos.y + (Object.Texture.height * Scale) * .6f,
                (Object.Texture.width * Scale) * .10f,
                (Object.Texture.height * Scale) * .35f
            };
        }
        case PropType::bHOUSERIGHT:
        {
            return Rectangle
            {
                
                ScreenPos.x + (Object.Texture.width * Scale) * .40f,
                ScreenPos.y + (Object.Texture.height * Scale) * .6f,
                (Object.Texture.width * Scale) * .50f,
                (Object.Texture.height * Scale) * .35f
            };
        }
        case PropType::DOOR:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .31f,
                ScreenPos.y + (Object.Texture.height * Scale) * .20f,
                (Object.Texture.width * Scale) - (Object.Texture.width * Scale) * .59f,
                (Object.Texture.height * Scale) * .35f
            };
        }
        case PropType::TOPWALL:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                (Object.Texture.height * Scale) * .3f
            };
        }
        case PropType::LEFTSIDEWALL:
        {
            return Rectangle   
            {
                ScreenPos.x,
                ScreenPos.y,
                (Object.Texture.width * Scale) * .2f,
                Object.Texture.height * Scale
            };
        }
        case PropType::RIGHTSIDEWALL:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .8f,
                ScreenPos.y,
                (Object.Texture.width * Scale) * .2f,
                Object.Texture.height * Scale
            };
        }
        case PropType::BOTTOMWALL:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                Object.Texture.height * Scale
            };
        }
        case PropType::STUMP:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                Object.Texture.height * Scale
            };
        }
        case PropType::HOLE:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .1f,
                ScreenPos.y + (Object.Texture.height * Scale) * .2f,
                (Object.Texture.width * Scale) * .8f,
                (Object.Texture.height * Scale) * .7f
            };
        }
        case PropType::DUNGEON:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .31f,
                ScreenPos.y + (Object.Texture.height * Scale) * .20f,
                (Object.Texture.width * Scale) - (Object.Texture.width * Scale) * .59f,
                (Object.Texture.height * Scale) * .35f
            };
        }
        case PropType::DUNGEONLEFT:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y + (Object.Texture.height * Scale) * .20f,
                (Object.Texture.width * Scale) - (Object.Texture.width * Scale) * .70f,
                (Object.Texture.height * Scale) * .80f
            };
        }
        case PropType::DUNGEONRIGHT:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale) * .72f,
                ScreenPos.y + (Object.Texture.height * Scale) * .20f,
                (Object.Texture.width * Scale) - (Object.Texture.width * Scale) * .70f,
                (Object.Texture.height * Scale) * .80f
            };
        }
        case PropType::TREASURE:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale)/4.f * .10f,
                ScreenPos.y + (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale)/4.f - (Object.Texture.width * Scale)/4.f * .10f,
                Object.Texture.height * Scale - (Object.Texture.height * Scale) * .10f
            };
        }
        case PropType::BIGTREASURE:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale)/4.f * .10f,
                ScreenPos.y + (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale)/4.f - (Object.Texture.width * Scale)/4.f * .10f,
                Object.Texture.height * Scale - (Object.Texture.height * Scale) * .10f
            };
        }
        case PropType::NPC_DIANA:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale)/4.f * .10f,
                ScreenPos.y + (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale)/4.f - (Object.Texture.width * Scale)/4.f * .10f,
                Object.Texture.height * Scale - (Object.Texture.height * Scale) * .10f
            };
        }
        case PropType::NPC_JADE:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale)/4.f * .10f,
                ScreenPos.y + (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale)/4.f - (Object.Texture.width * Scale)/4.f * .10f,
                Object.Texture.height * Scale - (Object.Texture.height * Scale) * .10f
            };
        }
        case PropType::NPC_SON:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale)/4.f * .10f,
                ScreenPos.y + (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale)/4.f - (Object.Texture.width * Scale)/4.f * .10f,
                Object.Texture.height * Scale - (Object.Texture.height * Scale) * .10f
            };
        }
        case PropType::NPC_RUMBY:
        {
            return Rectangle
            {
                ScreenPos.x + (Object.Texture.width * Scale)/4.f * .10f,
                ScreenPos.y + (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale)/4.f - (Object.Texture.width * Scale)/4.f * .10f,
                Object.Texture.height * Scale - (Object.Texture.height * Scale) * .10f
            };
        }
        default:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                Object.Texture.width * Scale,
                Object.Texture.height * Scale
            };
        }
    }
}

Rectangle Prop::GetInteractRec(const Vector2 CharacterWorldPos)
{
    Vector2 ScreenPos {Vector2Subtract(WorldPos, CharacterWorldPos)};

    switch (Type)
    {
        case PropType::STUMP:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale) * .10f,
                ScreenPos.y - (Object.Texture.height * Scale) * .10f,
                Object.Texture.width * Scale + (Object.Texture.width * Scale) * .20f,
                Object.Texture.height * Scale + (Object.Texture.height * Scale) * .20f
            };
        }
        case PropType::TREASURE:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale)/4.f * .45f,
                ScreenPos.y - (Object.Texture.height * Scale) * .45f,
                (Object.Texture.width * Scale)/4.f + (Object.Texture.width * Scale)/4.f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale)
            };
        }
        case PropType::BIGTREASURE:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale)/4.f * .45f,
                ScreenPos.y - (Object.Texture.height * Scale) * .45f,
                (Object.Texture.width * Scale)/4.f + (Object.Texture.width * Scale)/4.f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale)
            };
        }
        case PropType::NPC_DIANA:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale)/4.f * .45f,
                ScreenPos.y - (Object.Texture.height * Scale) * .45f,
                (Object.Texture.width * Scale)/4.f + (Object.Texture.width * Scale)/4.f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale)
            };
        }
        case PropType::NPC_JADE:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale)/4.f * .45f,
                ScreenPos.y - (Object.Texture.height * Scale) * .45f,
                (Object.Texture.width * Scale)/4.f + (Object.Texture.width * Scale)/4.f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale)
            };
        }
        case PropType::NPC_SON:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale)/4.f * .45f,
                ScreenPos.y - (Object.Texture.height * Scale) * .45f,
                (Object.Texture.width * Scale)/4.f + (Object.Texture.width * Scale)/4.f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale)
            };
        }
        case PropType::NPC_RUMBY:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale)/4.f * .45f,
                ScreenPos.y - (Object.Texture.height * Scale) * .45f,
                (Object.Texture.width * Scale)/4.f + (Object.Texture.width * Scale)/4.f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale)
            };
        }
        case PropType::ALTAR:
        {
            return Rectangle
            {
                ScreenPos.x - (Object.Texture.width * Scale) * .10f,
                ScreenPos.y - (Object.Texture.height * Scale) * .10f,
                (Object.Texture.width * Scale) + (Object.Texture.width * Scale) * .20f,
                (Object.Texture.height * Scale) + (Object.Texture.height * Scale) * .20f
            };
        }
        case PropType::ANIMATEDALTAR:
        {
            return Rectangle
            {
                ScreenPos.x - ((Object.Texture.width * Scale) / Object.MaxFramesX) * .10f,
                ScreenPos.y - (Object.Texture.height * Scale) * .10f,
                ((Object.Texture.width * Scale) / Object.MaxFramesX) + ((Object.Texture.width * Scale) / Object.MaxFramesX) * .20f,
                ((Object.Texture.height * Scale) / Object.MaxFramesY) + ((Object.Texture.height * Scale) / Object.MaxFramesY) * .20f
            };
        }
        default:
        {
            return Rectangle
            {
                ScreenPos.x,
                ScreenPos.y,
                (Object.Texture.width * Scale),
                Object.Texture.height * Scale
            };
        }
    }
}

bool Prop::CheckMovement(Background& Map, const Vector2 CharWorldPos, const Vector2 Direction, const float Speed, std::vector<std::vector<Prop>>& Props)
{
    bool Colliding{false};
    PrevWorldPos = WorldPos;

    if (Vector2Length(Direction) != 0.f) {
        // set MapPos -= Direction
        WorldPos = Vector2Add(WorldPos, Vector2Scale(Vector2Normalize(Direction), Speed));
    }

    if (WorldPos.x < 0.f ||
        WorldPos.y < 0.f ||
        WorldPos.x + (Object.Texture.width * Scale) > Map.GetForestMapSize().x * Map.GetScale() ||
        WorldPos.y + (Object.Texture.height * Scale) > Map.GetForestMapSize().y * Map.GetScale()) 
    {
        OutOfBounds = true;
        UndoMovement();
    }
    else {
        OutOfBounds = false;
    }

    for (auto& Proptype:Props) {
        for (auto& Prop:Proptype) {
            if (Prop.HasCollision()) {
                if (CheckCollisionRecs(GetCollisionRec(CharWorldPos), Prop.GetCollisionRec(CharWorldPos))) {
                    if (!Prop.IsMoveable()) {
                        UndoMovement();
                        Colliding = true;
                    }
                }
            }
        }
    }
    return Colliding;
}

void Prop::DrawPropText()
{
    if (Type == PropType::TREASURE) {
        DrawText("", 510, 550, 20, WHITE);
        DrawText("", 510, 575, 20, WHITE);
        DrawText(TextFormat("Received: %s!", ItemName.c_str()), 490, 625, 20, WHITE);
        DrawText("", 510, 625, 20, WHITE);
        DrawText("", 510, 650, 20, WHITE);
        DrawText("                                               (ENTER to Continue)", 390, 675, 16, WHITE);

        if (IsKeyPressed(KEY_ENTER)) {
            ReceiveItem = false;
        }
    }
    else if (Type == PropType::BIGTREASURE) {
        DrawText(TextFormat("Received: %s!", ItemName.c_str()), 510, 600, 20, WHITE);
        DrawText("This might look good on Diana...", 510, 625, 20, WHITE);
        DrawText("Lets bring it back to her!", 510, 650, 20, WHITE);
        DrawText("                                               (ENTER to Continue)", 390, 675, 16, WHITE);

        if (IsKeyPressed(KEY_ENTER)) {
            ReceiveItem = false;
        }
    }
    else if (Type == PropType::ANIMATEDALTAR) {
        if (PiecesAdded == 0 || !FirstPieceInserted) {
            DrawText("A mysterious altar... You feel", 490, 600, 20, WHITE);
            DrawText("a strange power resonating", 490, 625, 20, WHITE);
            DrawText("from the engravings...", 490, 650, 20, WHITE);
            DrawText("                                               (ENTER to Continue)", 390, 675, 16, WHITE);
        }
        else if (PiecesAdded > 0 && PiecesAdded < 6) {
            DrawText("", 510, 550, 20, WHITE);
            DrawText("", 510, 575, 20, WHITE);
            DrawText("Altar piece inserted!", 510, 625, 20, WHITE);
            DrawText("", 510, 625, 20, WHITE);
            DrawText("", 510, 650, 20, WHITE);
            DrawText("                                               (ENTER to Continue)", 390, 675, 16, WHITE);
        }
        else {
            DrawText("All pieces have been collected!", 490, 600, 20, WHITE);
            DrawText("You hear the elder tree in the", 490, 625, 20, WHITE);
            DrawText("middle of the forest shake...", 490, 650, 20, WHITE);
            DrawText("", 510, 675, 20, WHITE);
            DrawText("                                               (ENTER to Continue)", 390, 675, 16, WHITE);
        }

        if (IsKeyPressed(KEY_ENTER)) {
            InsertPiece = false;
        }
    }
    else if (Type == PropType::STUMP) {
        DrawText("Foxy's favorite spot in the forest.", 390, 550, 20, WHITE);
        DrawText("Every time he steps foot in this area", 390, 575, 20, WHITE);
        DrawText("he feels rejuvenated and can't wipe", 390, 600, 20, WHITE);
        DrawText("the smile off of his face. He keeps", 390, 625, 20, WHITE);
        DrawText("this spot super secret!", 390, 650, 20, WHITE);
        DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);

        if (IsKeyReleased(KEY_ENTER)) {
            Reading = false;
            Opened = true;
            PageOpen = false;
            TextAudio();
        }
    }
}

void Prop::DrawSpeech()
{
    switch(Act)
    {
        case Progress::ACT_I:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("Hello there, little Foxy! You look a little lost.", 390, 550, 20, WHITE);
                    DrawText("Have you ran into my neighbor, Jade? I know she", 390, 575, 20, WHITE);
                    DrawText("can be noisy, but she means no harm...", 390, 600, 20, WHITE);
                    DrawText("", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_JADE:
                {
                    DrawText("Why HELLO, Love! Have you seen my little one?", 390, 550, 20, WHITE);
                    DrawText("Could have sworn he was right here...", 390, 575, 20, WHITE);
                    DrawText("I really hope he didn't wander into the forest", 390, 600, 20, WHITE);
                    DrawText("AGAIN!!", 390, 625, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("Hey there, Foxy... *sniffle*", 390, 550, 20, WHITE);
                    DrawText("Have you seen my mom? We were walking", 390, 575, 20, WHITE);
                    DrawText("together and we got lost... *sniffle*", 390, 600, 20, WHITE);
                    DrawText("She always does this...", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    DrawText("Hi, friend! My name is Rumby. I am the forest", 390, 550, 20, WHITE);
                    DrawText("travel guide. I can show you the basics of", 390, 575, 20, WHITE);
                    DrawText("surviving the great big forest!", 390, 600, 20, WHITE);
                    DrawText("Though a wild Foxy like you probably knows", 390, 625, 20, WHITE);
                    DrawText("more than I would, haha!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyReleased(KEY_ENTER)) {
                if (Type == PropType::NPC_DIANA || Type == PropType::NPC_RUMBY) {
                    Act = Progress::ACT_II;
                }
                else {
                    Opened = true;
                    Talking = false;
                }
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_II:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("By the way, do you live in the flower forest", 390, 550, 20, WHITE);
                    DrawText("WEST of here? There's a strange stone", 390, 575, 20, WHITE);
                    DrawText("monument NORTH of that location...", 390, 600, 20, WHITE);
                    DrawText("People have been wondering what it is", 390, 625, 20, WHITE);
                    DrawText("but no one really knows...", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_JADE:
                {
                    DrawText("You found my boy! Thank you! Always wandering", 390, 550, 20, WHITE);
                    DrawText("off and getting in trouble... This time the FOREST!", 390, 575, 20, WHITE);
                    DrawText("I've been trying to get the treasure behind the,", 390, 600, 20, WHITE);
                    DrawText("boulder but I'm not strong enough!!!!!!!! Do you", 390, 625, 20, WHITE);
                    DrawText("think you can help me one more time, Foxy?!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("...Mom told you to find me?...", 390, 550, 20, WHITE);
                    DrawText("She said that I keep wandering off???", 390, 575, 20, WHITE);
                    DrawText("SHES the one who left ME here!!", 390, 600, 20, WHITE);
                    DrawText("Could you please lead me back home...?", 390, 625, 20, WHITE);
                    DrawText("I'll catch up with you!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    DrawText("Firstly, you can use [SPACE] or [LEFT CLICK]", 390, 550, 20, WHITE);
                    DrawText("to attack and fend off monsters!", 390, 575, 20, WHITE);
                    DrawText("Now, after fighting you might be low on health.", 390, 600, 20, WHITE);
                    DrawText("Hold [SHIFT] while moving to run and find cover", 390, 625, 20, WHITE);
                    DrawText("so you can press [L] to rest!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (Type == PropType::NPC_RUMBY) {
                    Act = Progress::ACT_III;
                }
                else {
                    Opened = true;
                    Talking = false;
                }
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_III:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("Wow Foxy! Looks like you got some kind of", 390, 550, 20, WHITE);
                    DrawText("magical stone... there is a strange altar", 390, 575, 20, WHITE);
                    DrawText("in the forest with mystical engravings..", 390, 600, 20, WHITE);
                    DrawText("Maybe you can check it out?", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_JADE:
                {
                    DrawText("There seem to be treasures hidden", 390, 550, 20, WHITE);
                    DrawText("all throughout the forest! But I", 390, 575, 20, WHITE);
                    DrawText("heard you need to hunt some monsters", 390, 600, 20, WHITE);
                    DrawText("to find them... Be careful out there!!", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("Hi, Foxy! Thanks again for your help! ", 390, 550, 20, WHITE);
                    DrawText("Mom is still blaming me about getting lost..", 390, 575, 20, WHITE);
                    DrawText("While I was lost, I think I saw a treasure", 390, 600, 20, WHITE);
                    DrawText("in the NORTH WEST region of the forest...", 390, 625, 20, WHITE);
                    DrawText("Might be worth checking out!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    DrawText("Thirdly, the forest is gigantic! And", 390, 550, 20, WHITE);
                    DrawText("it is easy to get lost. You're a smart", 390, 575, 20, WHITE);
                    DrawText("Foxy, so here is a map! Press [M] to use it!", 390, 600, 20, WHITE);
                    DrawText("And if you ever need to take a break, [P] will", 390, 625, 20, WHITE);
                    DrawText("bring up the lovely pause menu!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (Type == PropType::NPC_RUMBY) {
                    Act = Progress::ACT_IV;
                }
                else {
                    Opened = true;
                    Talking = false;
                }
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_IV:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("The stone fit in the engraving!", 390, 550, 20, WHITE);
                    DrawText("It seems to be spelling some kind of word..?", 390, 575, 20, WHITE);
                    DrawText("Ah! I have a book on ancient engravings!", 390, 600, 20, WHITE);
                    DrawText("Keep finding more stones and lets try to", 390, 625, 20, WHITE);
                    DrawText("decipher it, Foxy!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_JADE:
                {
                    DrawText("Hey, you found the treasure!!!!!!!", 390, 550, 20, WHITE);
                    DrawText("Wait... It was just a dusty old rock???", 390, 575, 20, WHITE);
                    DrawText("We have plenty of those lying around", 390, 600, 20, WHITE);
                    DrawText("everywhere!!! You can keep it if you like!", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("Wow! That stone looks fancy...", 390, 550, 20, WHITE);
                    DrawText("Is that the treasure we were looking", 390, 575, 20, WHITE);
                    DrawText("for?? I found an altar with matching", 390, 600, 20, WHITE);
                    DrawText("engravings NORTH of here when I got", 390, 625, 20, WHITE);
                    DrawText("lost...maybe check  it out?", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    DrawText("Lastly, if you need to call it quits,", 390, 550, 20, WHITE);
                    DrawText("bring up the quit menu by pressing [PERIOD]", 390, 575, 20, WHITE);
                    DrawText("or [ESCAPE]. Thats about it! Be safe", 390, 600, 20, WHITE);
                    DrawText("and have fun my dear Foxy! Oh and please take", 390, 625, 20, WHITE);
                    DrawText("care of the evil mushrooms...", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                Opened = true;
                Talking = false;
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_V:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("Is that my Cryptex? I put that away long ago...", 390, 550, 20, WHITE);
                    DrawText("It was a gift but I could not figure out how", 390, 575, 20, WHITE);
                    DrawText("to open it... I wonder what secrets it holds inside?", 390, 600, 20, WHITE);
                    DrawText("Lets find the code together, Mr. Foxy!", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE);
                    break;
                }
                case PropType::NPC_JADE:
                {
                    DrawText("", 390, 550, 20, WHITE);
                    DrawText("", 390, 575, 20, WHITE);
                    DrawText("", 390, 600, 20, WHITE);
                    DrawText("", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("There has been a lot of monsters roaming", 390, 550, 20, WHITE);
                    DrawText("about lately... I heard explorer Rumby mentioning", 390, 575, 20, WHITE);
                    DrawText("that the monsters need to be wiped out..Maybe", 390, 600, 20, WHITE);
                    DrawText("something will happen if you clear the camps?", 390, 625, 20, WHITE);
                    DrawText("Maybe there will be more treasures!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    DrawText("", 390, 550, 20, WHITE);
                    DrawText("", 390, 575, 20, WHITE);
                    DrawText("", 390, 600, 20, WHITE);
                    DrawText("", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                Opened = true;
                Talking = false;
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_VI:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("My sweet Foxy, I think I have deciphered", 390, 550, 20, WHITE);
                    DrawText("the engravings on the altar! They seem to", 390, 575, 20, WHITE);
                    DrawText("spell out... L...I...L...A...C.... LILAC!", 390, 600, 20, WHITE);
                    DrawText("My favorite flower! Maybe we should", 390, 625, 20, WHITE);
                    DrawText("try it on our cryptex!!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("You know while I was lost in the woods", 390, 550, 20, WHITE);
                    DrawText("I noticed an area where some trees were all", 390, 575, 20, WHITE);
                    DrawText("BROWN!! I noticed a secret path near the brown", 390, 600, 20, WHITE);
                    DrawText("trees but there were so many spiders I was", 390, 625, 20, WHITE);
                    DrawText("too scared to go in! Maybe you can check it out!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (Type == PropType::NPC_DIANA) {
                    Act = Progress::ACT_VII;
                }
                else {
                    Opened = true;
                    Talking = false;
                }
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_VII:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("It opened!!", 390, 550, 20, WHITE);
                    DrawText("...There was a key inside?", 390, 575, 20, WHITE);
                    DrawText("I wonder what it could be for....?", 390, 600, 20, WHITE);
                    DrawText("", 390, 625, 20, WHITE);
                    DrawText("                   **Key Received**", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("", 390, 550, 20, WHITE);
                    DrawText("", 390, 575, 20, WHITE);
                    DrawText("", 390, 600, 20, WHITE);
                    DrawText("", 390, 625, 20, WHITE);
                    DrawText("", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                Opened = true;
                Talking = false;
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_VIII:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("The key unlocked a treasure chest?", 390, 550, 20, WHITE);
                    DrawText("There was a silver bracelet inside?", 390, 575, 20, WHITE);
                    DrawText("You want... to give it to me?", 390, 600, 20, WHITE);
                    DrawText("Mr. Foxy, you are the sweetest!", 390, 625, 20, WHITE);
                    DrawText("Thank you so muchhhhh", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_JADE:
                {
                    DrawText("Thank you again for finding my boy!", 390, 550, 20, WHITE);
                    DrawText("If he gets lost again the next time I send him", 390, 575, 20, WHITE);
                    DrawText("to look for... err... I mean the next time he", 390, 600, 20, WHITE);
                    DrawText("goes looking for treasure... I can count on", 390, 625, 20, WHITE);
                    DrawText("you to save him, right?!!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_SON:
                {
                    DrawText("Hey, Mr. Foxy! Great job finding all the", 390, 550, 20, WHITE);
                    DrawText("treasures! That bracelet feels magical...", 390, 575, 20, WHITE);
                    DrawText("I'm sure Diana is gonna love it!", 390, 600, 20, WHITE);
                    DrawText("And thank you again for saving me.", 390, 625, 20, WHITE);
                    DrawText("I don't know where I would be without you!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                case PropType::NPC_RUMBY:
                {
                    DrawText("I've heard of your great adventures, Mr. Foxy.", 390, 550, 20, WHITE);
                    DrawText("Well done mastering the forest and besting all", 390, 575, 20, WHITE);
                    DrawText("the wild monsters! You are a wonderful Foxy", 390, 600, 20, WHITE);
                    DrawText("and I wish you the best in all your upcoming", 390, 625, 20, WHITE);
                    DrawText("ventures. Take care, little one!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (Type == PropType::NPC_DIANA) {
                    Act = Progress::ACT_IX;
                }
                else {
                    Opened = true;
                    Talking = false;
                }
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        case Progress::ACT_IX:
        {
            switch (Type)
            {
                case PropType::NPC_DIANA:
                {
                    DrawText("Thank you for bringing me on this adventure", 390, 550, 20, WHITE);
                    DrawText("with you Mr. Foxy! I can't wait to have more", 390, 575, 20, WHITE);
                    DrawText("journeys with you. Come by the house", 390, 600, 20, WHITE);
                    DrawText("sometime, okay? Now run along Mr. Foxy, your", 390, 625, 20, WHITE);
                    DrawText("family must be getting worried about you!", 390, 650, 20, WHITE);
                    DrawText("                                                         (ENTER to Continue)", 390, 675, 16, WHITE); 
                    break;
                }
                default:
                    break;
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                Opened = true;
                Talking = false;
                PageOpen = false;
                TextAudio();
            }
            break;
        }
        default:
            Opened = true;
            Talking = false;
            break;
    }
}

// ------------------------- Audio ---------------------------
void Prop::TreasureAudio()
{
    SetSoundVolume(Audio.TreasureOpen, 0.4f);
    PlaySound(Audio.TreasureOpen);
}

void Prop::AltarAudio()
{
    SetSoundVolume(Audio.AltarInsert, 0.4f);
    PlaySound(Audio.AltarInsert);
}

void Prop::TextAudio()
{
    if (!PageOpen) {
        PlaySoundMulti(Audio.NpcTalk);
        PageOpen = true;
    }
}

// ---------------------------------------------------------------------

Props::Props(const std::vector<std::vector<Prop>>& Under, const std::vector<std::vector<Prop>>& Over)
    : Under{Under}, Over{Over}
{

}