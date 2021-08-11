#pragma once
#ifndef __TEXT_RENDERING_H__
#define __TEXT_RENDERING_H__
#endif

#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "Shader.h"
#include "Renderer.h"
#include "Camera.h"
#include "Constants.h"
#include "TextRendering.h"

TextRendering::TextRendering(int SCR_W, int SCR_H)
{
    this->SCR_W = SCR_W;
    this->SCR_H = SCR_H;

    // Initialize FreeType library
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "Error: Unable to initlaize FreeType library" << std::endl;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "./fonts/OpenSans-Regular.ttf", 0, &face))
        std::cout << "Error: Unable to load font" << std::endl;

    // Set global size and properties
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set into map
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout <<"Error: Unable to load character" << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Set texture properties
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Add character entry to map
        Character character = 
        {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    // Free reoursces
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextRendering::RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Bind vertex array and shader, and set uniform states
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_W), 0.0f, static_cast<GLfloat>(SCR_H));

    shader.bind();
    shader.setUniform4Mat("projection", projection);
    shader.setUniform1i("text", 2);
    shader.setUniform3Vec("ourColor", glm::vec3(color.x, color.y, color.z));
    
    glActiveTexture(GL_TEXTURE2);
    glBindVertexArray(VAO);

    // Draw each character
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        // Load character specific properties
        Character ch = Characters[*c];

        // Calculate x and y position of each character (incrementally)
        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

        // Calculate width and height of each character
        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = 
        {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };

        // Render glyph texture over quad and bind buffer
        glBindTexture(GL_TEXTURE_2D, ch.id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        // Render
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // update advance offset
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    // Free resources
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}