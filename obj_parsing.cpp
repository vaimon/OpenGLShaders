

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "obj_parsing.h"


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint Program;
// ID атрибута
GLint Attrib_vertex;
// ID VBO вершин
GLuint VBO_position;
// ID VBO цвета
GLuint VBO_normale;
// ID VBO вершин
GLuint VBO_texture;

// ID VAO вершин
GLuint VAO_position;
// ID VAO цвета
GLuint VAO_normale;
// ID VAO вершин
GLuint VAO_texture;

// ID IBO вершин
GLuint IBO_position;
// ID IBO цвета
GLuint IBO_normale;
// ID IBO вершин
GLuint IBO_texture;

// Вершина
struct Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core

    // Координаты вершины. Атрибут, инициализируется через буфер.
    in vec3 vertexPosition;

   // in vec3 vertexNormale;

   // in vec2 vertexTextureCoords;
    
   // out vec2 vTextureCoordinate;

   // out vec3 vColor; 

    void main() {
// Захардкодим углы поворота
        float x_angle = -1;
        float y_angle = 1;
        
        // Поворачиваем вершину
        vec3 position = vertexPosition * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );

     //   vTextureCoordinate = vertexTextureCoords;

        // TODO: надо переделать во всякие освещательные штуки
      //  vColor = vertexNormale;

        // Присваиваем вершину волшебной переменной gl_Position
        gl_Position = vec4(position, 1.0);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core

   // in vec2 vTextureCoordinate;

   // in vec3 vColor;

    // Цвет, который будем отрисовывать
    out vec4 color;

    void main() {
       color = vec4(1, 0.8, 0, 1);
    }
)";

std::vector<GLfloat> vertices_position {};
std::vector<GLfloat> vertices_normale {};
std::vector<GLfloat> vertices_texture{};

std::vector<GLuint>  indices_position {};
std::vector<GLuint> indices_normale{};
std::vector<GLuint> indices_texture{};

std::vector<std::string> split(const std::string& s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
        // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
    }
    return elems;
}

void parseFile(std::string fileName) {
    
    std::ifstream obj(fileName);

    if (!obj.is_open()) {
        throw std::exception("File cannot be opened");
    }

    std::string line;
    while (std::getline(obj, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "v") {
            auto vertex = split(line, ' ');
            for (size_t j = 1; j < vertex.size(); j++)
            {
                vertices_position.push_back(std::stof(vertex[j]));
            }
        }
        else if (type == "vn") {
            auto normale = split(line, ' ');
            for (size_t j = 1; j < normale.size(); j++)
            {
                vertices_normale.push_back(std::stof(normale[j]));
            }
        }
        else if (type == "vt") {
            auto texture = split(line, ' ');
            for (size_t j = 1; j < texture.size(); j++)
            {
                vertices_texture.push_back(std::stof(texture[j]));
            }
        }
        else if (type == "f") {
            auto splitted = split(line, ' ');
            for (size_t i = 1; i < splitted.size(); i++)
            {
                auto triplet = split(splitted[i], '/');
                indices_position.push_back(std::stoi(triplet[0]) - 1);
                indices_texture.push_back(std::stoi(triplet[1]) - 1);
                indices_normale.push_back(std::stoi(triplet[2]) - 1);
            }
        }
        else {
            continue;
        }
    }
}

int obj_parsing_main(std::string objFilename) {
    sf::Window window(sf::VideoMode(700, 700), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

    // Инициализация glew
    glewInit();
    parseFile(objFilename);
    Init();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::Resized) {
                //glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Draw();

        window.display();
    }

    Release();
    return 0;
}


// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
void checkOpenGLerror(int place) {
    GLenum errCode;
    // Коды ошибок можно смотреть тут
    // https://www.khronos.org/opengl/wiki/OpenGL_Error
    if ((errCode = glGetError()) != GL_NO_ERROR)
        std::cout << "OpenGl error in "<< place << "!: " << errCode << std::endl;
}

// Функция печати лога шейдера
void ShaderLog(unsigned int shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    char* infoLog;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        infoLog = new char[infologLen];
        if (infoLog == NULL)
        {
            std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
            exit(1);
        }
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
        std::cout << "InfoLog: " << infoLog << "\n\n\n";
        delete[] infoLog;
    }
}


void InitBuffers()
{
    /*
    std::vector<GLfloat> vertices_position = {
         -0.5, -0.5, +0.5 ,  -0.5, +0.5, +0.5 ,  +0.5, +0.5, +0.5 ,
         +0.5, +0.5, +0.5 ,  +0.5, -0.5, +0.5 ,  -0.5, -0.5, +0.5 ,
         -0.5, -0.5, -0.5 ,  +0.5, +0.5, -0.5 ,  -0.5, +0.5, -0.5 ,
         +0.5, +0.5, -0.5 ,  -0.5, -0.5, -0.5 ,  +0.5, -0.5, -0.5 ,

         -0.5, +0.5, -0.5 ,  -0.5, +0.5, +0.5 ,  +0.5, +0.5, +0.5 ,
         +0.5, +0.5, +0.5 ,  +0.5, +0.5, -0.5 ,  -0.5, +0.5, -0.5 ,
         -0.5, -0.5, -0.5 ,  +0.5, -0.5, +0.5 ,  -0.5, -0.5, +0.5 ,
         +0.5, -0.5, +0.5 ,  -0.5, -0.5, -0.5 ,  +0.5, -0.5, -0.5 ,

         +0.5, -0.5, -0.5 ,  +0.5, -0.5, +0.5 ,  +0.5, +0.5, +0.5 ,
         +0.5, +0.5, +0.5 ,  +0.5, +0.5, -0.5 ,  +0.5, -0.5, -0.5 ,
         -0.5, -0.5, -0.5 ,  -0.5, +0.5, +0.5 ,  -0.5, -0.5, +0.5 ,
         -0.5, +0.5, +0.5 ,  -0.5, -0.5, -0.5 ,  -0.5, +0.5, -0.5 };

    std::vector<GLuint> indices_position = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35};
    */
    glGenBuffers(1, &VBO_position);
    glGenVertexArrays(1, &VAO_position);
    glGenBuffers(1, &IBO_position);
    

    //Привязываем VAO
    glBindVertexArray(VAO_position);

    
    //glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
    glBufferData(GL_ARRAY_BUFFER, vertices_position.size() * sizeof(GLfloat), vertices_position.data(), GL_STATIC_DRAW);

    // Копируем наши индексы в в буфер для OpenGL
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_position);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_position.size() * sizeof(GLuint), indices_position.data(), GL_STATIC_DRAW);

    // 3. Устанавливаем указатели на вершинные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    //Отвязываем VAO
    glBindVertexArray(0);
    checkOpenGLerror(1);
}

void InitShader() {
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    // Передаем исходный код
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Передаем исходный код
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    ShaderLog(fShader);

    // Создаем программу и прикрепляем шейдеры к ней
    Program = glCreateProgram();
    glAttachShader(Program, vShader);
    glAttachShader(Program, fShader);

    // Линкуем шейдерную программу
    glLinkProgram(Program);
    // Проверяем статус сборки
    int link_ok;
    glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
    if (!link_ok)
    {
        std::cout << "error attach shaders \n";
        return;
    }

    // Вытягиваем ID атрибута из собранной программы
    Attrib_vertex = glGetAttribLocation(Program, "vertexPosition");
    if (Attrib_vertex == -1)
    {
        std::cout << "could not bind attrib " << "vertexPosition" << std::endl;
        return;
    }

    checkOpenGLerror(2);
}

void Init() {
    InitShader();
    InitBuffers();
    // Включаем проверку глубины
    glEnable(GL_DEPTH_TEST);
}


void Draw() {
    //std::vector<GLuint> indices_position = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 };
    // Устанавливаем шейдерную программу текущей
    glUseProgram(Program);
    // Привязываем вао
    glBindVertexArray(VAO_position);
    // Передаем данные на видеокарту(рисуем)
    glDrawElements(GL_TRIANGLES, indices_position.size(), GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
    // Отключаем шейдерную программу
    glUseProgram(0);
    checkOpenGLerror(3);
}


// Освобождение шейдеров
void ReleaseShader() {
    // Передавая ноль, мы отключаем шейдрную программу
    glUseProgram(0);
    // Удаляем шейдерную программу
    glDeleteProgram(Program);
}

// Освобождение буфера
void ReleaseVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO_position);
    glDeleteVertexArrays(1, &VAO_position);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}
