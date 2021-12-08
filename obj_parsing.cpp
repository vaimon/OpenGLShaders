

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "obj_parsing.h"


// Переменные с индентификаторами ID
// ID шейдерной программы
GLuint Program;

// ID VBO вершин
GLuint VBO;

// ID VAO вершин
GLuint VAO;

// ID IBO вершин
GLuint IBO;

GLuint Unif_lpos;
GLuint Unif_lamb;
GLuint Unif_ldiff;
GLuint Unif_lspec;
GLuint Unif_latt;
GLuint Unif_viewpos;
GLuint Unif_mamb;
GLuint Unif_mdiff;
GLuint Unif_mspec;
GLuint Unif_mshine;

// Вершина
struct Vertex
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};
struct Vert4
{
    GLfloat a;
    GLfloat b;
    GLfloat c;
    GLfloat d;
    
};

// Исходный код вершинного шейдера
const char* VertexShaderSource = R"(
    #version 330 core
// параметры точечного источника освещения
uniform vec4 lposition;
uniform vec4 lambient;
uniform vec4 ldiffuse;
uniform vec4 lspecular;
uniform vec3 lattenuation;
uniform vec3 viewPosition;

    // Координаты вершины. Атрибут, инициализируется через буфер.
    in vec3 vertexPosition;

    in vec3 vertexNormale;

    in vec2 vertexTextureCoords;
    
    out vec2 vTextureCoordinate;

   // out vec3 vColor; 

out vec3 vnormal;
out vec3 vlightDir;
out vec3 vviewDir;
out float vdistance;

    void main() {
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

      vec4 vertex=vec4 (position, 1.0 );

       vec4 lightDir_1=lposition − vertex;
     

       vTextureCoordinate = vertexTextureCoords;
       vnormal=vertexNormale;
       vlightDir=vec3(lightDir_1);
       vviewDir=viewPosition-vertex;
       vdistance=length(lightDir);

        // TODO: надо переделать во всякие освещательные штуки
        //vColor = vertexNormale;

        // Присваиваем вершину волшебной переменной gl_Position
        gl_Position = vec4(position, 1.0);
    }
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
    #version 330 core
// параметры точечного источника освещения
uniform vec4 lposition;
uniform vec4 lambient;
uniform vec4 ldiffuse;
uniform vec4 lspecular;
uniform vec3 lattenuation;
uniform vec3 viewPosition;
uniform vec4 mambient;
uniform vec4 mdiffuse;
uniform vec4 mspecular;
uniform float mshininess;

in vec2 vTextureCoordinate;
in vec3 vnormal;
in vec3 vlightDir;
in vec3 vviewDir;
in float vdistance;
   // in vec3 vColor;

    // Цвет, который будем отрисовывать
    out vec4 color;

    void main() {
/*vec3 normal=normalize (vnormal );
//vec3 lightDir=normalize ( vlightDir );
//vec3 viewDir=normalize ( vviewDir );

//float attenuation =1.0 / ( lattenuation[0] +
//lattenuation[1] ∗ vdistance +
//lattenuation[2] ∗ vdistance ∗ vdistance);

//color+= mambient ∗ lambient ∗ attenuation;
//добавление рассеянного света
//float Ndot= max( dot ( vnormal , vlightDir) , 0.0);
//color+= mdiffuse ∗ ldiffuse ∗ Ndot ∗ attenuation;

//добавление отраженного света
//float RdotVpow= max ( pow(dot( reflect(−vlightDir, vnormal ) , vviewDir) ,mshininess ) , 0.0 );
//color+=mspecular ∗ lspecular ∗ RdotVpow ∗ attenuation;


       color = vec4( vnormal, 1);
    }
)";

std::vector<GLfloat> vertices {};

std::vector<GLuint>  indices {};

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

    std::vector<std::vector<float>> v{};
    std::vector<std::vector<float>> vt{};
    std::vector<std::vector<float>> vn{};

    std::vector <std::string> indexAccordance{};
    std::string line;
    while (std::getline(obj, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "v") {
            auto vertex = split(line, ' ');
            std::vector<float> cv{};
            for (size_t j = 1; j < vertex.size(); j++)
            {
                cv.push_back(std::stof(vertex[j]));
            }
            v.push_back(cv);
        }
        else if (type == "vn") {
            auto normale = split(line, ' ');
            std::vector<float> cvn{};
            for (size_t j = 1; j < normale.size(); j++)
            {
                cvn.push_back(std::stof(normale[j]));
            }
            vn.push_back(cvn);
        }
        else if (type == "vt") {
            auto texture = split(line, ' ');
            std::vector<float> cvt{};
            for (size_t j = 1; j < texture.size(); j++)
            {
                cvt.push_back(std::stof(texture[j]));
            }
            vt.push_back(cvt);
        }
        else if (type == "f") {
            auto splitted = split(line, ' ');
            for (size_t i = 1; i < splitted.size(); i++)
            {
                auto it = std::find(indexAccordance.begin(), indexAccordance.end(), splitted[i]);
                if (it == indexAccordance.end()) {
                    indexAccordance.push_back(splitted[i]);
                    indices.push_back(indexAccordance.size() - 1);
                }
                else {
                    indices.push_back(std::distance(indexAccordance.begin(),it));
                    continue;
                }
                auto triplet = split(splitted[i], '/');
                int positionIndex = std::stoi(triplet[0]) - 1;
                for (int j = 0; j < 3; j++) {
                    vertices.push_back(v[positionIndex][j]);
                }
                int normaleIndex = std::stoi(triplet[2]) - 1;
                for (int j = 0; j < 3; j++) {
                    vertices.push_back(vn[normaleIndex][j]);
                }
                int textureIndex = std::stoi(triplet[1]) - 1;
                for (int j = 0; j < 2; j++) {
                    vertices.push_back(vt[textureIndex][j]);
                }
            }
        }
        else {
            continue;
        }
    }
}

Vert4 lposition = Vert4{ -1, 0, -0.5,1.0 };
Vert4 lambient = Vert4{ 0.4, 0.7, 0.2,1.0 };
Vert4 ldiffuse = Vert4{ 0.5, 0.0, 0.0, 1.0 };
Vert4 lspecular = Vert4{ 0.7, 0.7, 0.0, 1.0 };
Vertex lattenuation = Vertex{ 1.0,0.0,1.0 };//конст, лин, кв
Vertex viewPosition = Vertex{ 0.0, 0.0, 1.0 };
Vert4 mambient = Vert4{ 0.05,0.05,0.06,1.0 };
Vert4 mdiffuse = Vert4{ 0.18,0.17,0.22,1.0 };
Vert4 mspecular = Vert4{ 0.33,0.32,0.36,1.0 };
float mshininess = 0.3;

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

void InitBuffers() {
    InitPositionBuffers();
}

void InitPositionBuffers()
{
    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    glUniform4f(Unif_lpos, lposition.a, lposition.b, lposition.c, lposition.d); 
    glUniform4f(Unif_lamb, lambient.a, lambient.b, lambient.c, lambient.d);
    glUniform4f(Unif_ldiff, ldiffuse.a, ldiffuse.b, ldiffuse.c, ldiffuse.d);
    glUniform4f(Unif_lspec, lspecular.a, lspecular.b, lspecular.c, lspecular.d);
    glUniform3f(Unif_latt, lattenuation.x, lattenuation.y, lattenuation.z);
    glUniform3f(Unif_viewpos, viewPosition.x, viewPosition.y, viewPosition.z);
    glUniform4f(Unif_mamb, mambient.a, mambient.b, mambient.c, mambient.d);
    glUniform4f(Unif_mdiff, mdiffuse.a, mdiffuse.b, mdiffuse.c, mdiffuse.d);
    glUniform4f(Unif_mspec, mspecular.a, mspecular.b, mspecular.c, mspecular.d);
    glUniform1f(Unif_mshine, mshininess);

    //Привязываем VAO
    glBindVertexArray(VAO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // Копируем наши индексы в в буфер для OpenGL
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // 3. Устанавливаем указатели на вершинные атрибуты
    // Атрибут с координатами
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    // Атрибут с цветом
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    // Атрибут с текстурой
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    //glEnableVertexAttribArray(0);
    
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
    const char* unif_name = "lposition";
    Unif_lpos = glGetUniformLocation(Program, unif_name);
    if (Unif_lpos < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
     unif_name = "lambient";
    Unif_lamb = glGetUniformLocation(Program, unif_name);
    if (Unif_lamb < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
    unif_name = "ldiffuse";
    Unif_ldiff = glGetUniformLocation(Program, unif_name);
    if (Unif_ldiff < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
     unif_name = "lspecular";
    Unif_lspec = glGetUniformLocation(Program, unif_name);
    if (Unif_lspec < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
    unif_name = "lattenuation";
    Unif_latt = glGetUniformLocation(Program, unif_name);
    if (Unif_latt < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
     unif_name = "viewPosition";
    Unif_viewpos = glGetUniformLocation(Program, unif_name);
    if (Unif_viewpos < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
     unif_name = "mambient";
    Unif_mamb = glGetUniformLocation(Program, unif_name); 
    if (Unif_mamb < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
   unif_name = "mdiffuse";
    Unif_mdiff = glGetUniformLocation(Program, unif_name);
    if (Unif_mdiff< -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
     unif_name = "mspecular";
    Unif_mspec = glGetUniformLocation(Program, unif_name);
    if (Unif_mspec < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
        return;
    }
    unif_name = "mshininess";
    Unif_mshine = glGetUniformLocation(Program, unif_name);
    if (Unif_mshine < -1)
    {
        std::cout << "could not bind uniform " << unif_name << std::endl;
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
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT,0);
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
