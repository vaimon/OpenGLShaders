#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include "obj_parsing.h"


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint Program;
// ID атрибута
GLint Attrib_vertex;
// ID Vertex Buffer Object
GLuint VBO;

GLuint VAO;

GLuint IBO;

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
    
    // Выходной параметр с координатами вершины, интерполируется и передётся во фрагментный шейдер 
    out vec3 vPosition;

    void main() {
        // Передаём непреобразованную координату во фрагментный шейдер
        vPosition = vertexPosition;

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

        // Присваиваем вершину волшебной переменной gl_Position
        gl_Position = vec4(position, 1.0);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core

    // Интерполированные координаты вершины, передаются из вершинного шейдера
    in vec3 vPosition;

    // Цвет, который будем отрисовывать
    out vec4 color;

    void main() {
       color = vec4(1, 0.8, 0, 1);
    }
)";


int obj_parsing_main(std::string objFilename) {
    sf::Window window(sf::VideoMode(700, 700), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

    // Инициализация glew
    glewInit();

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

GLfloat vertices[] = {
0.5f, 0.5f, 0.0f, // Верхний правый угол
0.5f, -0.5f, 0.0f, // Нижний правый угол
-0.5f, -0.5f, 0.0f, // Нижний левый угол
-0.5f, 0.5f, 0.0f // Верхний левый угол
};

GLuint indices[] = {
0, 1, 3, // Первый треугольник
1, 2, 3 // Второй треугольник
};


void InitBuffers()
{
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &IBO);
    

    //Привязываем VAO
    glBindVertexArray(VAO); 

    // Копируем массив вершин в буфер для OpenGL
    glEnableVertexAttribArray(Attrib_vertex);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Копируем наши индексы в в буфер для OpenGL
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Устанавливаем указатели на вершинные атрибуты
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
    const char* attr_name = "vertexPosition";
    Attrib_vertex = glGetAttribLocation(Program, attr_name);
    if (Attrib_vertex == -1)
    {
        std::cout << "could not bind attrib " << attr_name << std::endl;
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
    // Устанавливаем шейдерную программу текущей
    glUseProgram(Program);
    // Привязываем вао
    glBindVertexArray(VAO);
    // Передаем данные на видеокарту(рисуем)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,0);
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
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}
