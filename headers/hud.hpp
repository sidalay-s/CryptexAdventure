#ifndef HUD_HPP
#define HUD_HPP

#include "character.hpp"

class HUD 
{
public:
    explicit HUD(const GameTexture& GameTextures);
    
    void Tick();
    void Draw(float Health, Emotion State);
    
private:
    const GameTexture& GameTextures;
    Texture2D Fox{};

    float Scale{2.f};
};

#endif // HUD_HPP