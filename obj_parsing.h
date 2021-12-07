#pragma once

int obj_parsing_main(std::string objFilename);

void checkOpenGLerror(int place = 0);

void ShaderLog(unsigned int shader);

void InitBuffers();

void Init();

void Draw();

void ReleaseShader();

void ReleaseVBO();

void Release();
