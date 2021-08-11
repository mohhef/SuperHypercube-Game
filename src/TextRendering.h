#pragma once
#ifndef __TEXT_RENDERING_H__
#define __TEXT_RENDERING_H__
#endif

#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ft2build.h"
#include FT_FREETYPE_H

class TextRendering
{
private:
    struct Character 
    {
        unsigned int    id;         // texture id
        glm::ivec2      size;       // Size of glyph
        glm::ivec2      bearing;    // Offset from baseline to left/top of glyph
        unsigned int    advance;    // Offset to advance to next glyph
    };
  
    std::map<char, Character> Characters; // Map for character texture and offsets
    unsigned int VAO, VBO; // Vertex array and buffer for text rendering
    int SCR_W, SCR_H; 

public:
    TextRendering(int SCR_W, int SCR_H);
    void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
};