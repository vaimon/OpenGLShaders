#include "switcher.h"
#ifdef LIGHTING

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

GLint Unif_lamb;
GLint Unif_lpos;
GLint Unif_ldiff;

float lpos[4] = { 1.0f,0.5f,0.5f,1.0f };
float lambient[4] = { 0.4f, 0.7f, 0.2f, 1.0f };
float ldiffuse[4] = { 0.5f, 0.0f, 0.0f, 1.0f };
float lspecular[4] = { 0.7f, 0.7f, 0.0f, 1.0f };
float lattenuation[3] = { 1.0f,0.0f,1.0f };//конст, лин, кв
float viewPosition[3] = { 0.0f, 0.0f, 1.0f };
float mambient[4] = { 0.05f,0.05f,0.06f,1.0f };
float mdiffuse[4] = { 0.18f,0.17f,0.22f,1.0f };
float mspecular[4] = { 0.33f,0.32f,0.36f,1.0f };
float mshininess = 0.3f;

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

//uniform vec4 lspecular;
//uniform vec3 lattenuation;
//uniform vec3 viewPosition;
// Координаты вершины. Атрибут, инициализируется через буфер.
in vec3 vertexPosition;

in vec3 vertexNormale;

in vec2 vertexTextureCoords;

out vec2 vTextureCoordinate;
out vec3 vnormal;
//out vec3 vlightDir;
//out vec4 vColor;

uniform vec4 lpos;
uniform vec4 lambient;

void main() {
float x_angle = -1;
float y_angle = 1;
vec4 temp = lpos;
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

vTextureCoordinate = vertexTextureCoords;
vnormal=vertexNormale;
vec4 vpos = vec4(vertexPosition, 1.0);

//vec4 lightDir_1= lposition − vpos;
//vlightDir=vec3(lightDir_1);
// TODO: надо переделать во всякие освещательные штуки
// vColor = cococo;

// Присваиваем вершину волшебной переменной gl_Position
gl_Position = vec4(position, 1.0);
}
)";

// Исходный код фрагментного шейдера
const char* FragShaderSource = R"(
#version 330 core

uniform vec4 lpos;
uniform vec4 lambient;

in vec2 vTextureCoordinate;

// in vec4 vColor;

// Цвет, который будем отрисовывать
out vec4 color;

void main() {
color = lambient;
}
)";

std::vector<GLfloat> vertices{};

std::vector<GLuint> indices{};

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

	std::vector < std::vector < float>> v{};
	std::vector < std::vector < float>>vt{};
	std::vector < std::vector < float>> vn{};

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
					indices.push_back(std::distance(indexAccordance.begin(), it));
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

int task_main(std::string objFilename) {
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
		std::cout << "OpenGl error in " << place << "!: " << errCode << std::endl;
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
			std::cout <<  "ERROR: Could not allocate InfoLog buffer" << std::endl;
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

	// Линкуем шейдерную

	
		glLinkProgram(Program);
	// Проверяем статус сборки
	int link_ok;
	glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		std::cout << "error attach shaders \n";
		return;
	}


	const char* unif_name = "lpos";
	Unif_lpos = glGetUniformLocation(Program, unif_name);
	if (Unif_lpos == -1)
	{
		std::cout << "could not bind uniform " << unif_name << std::endl;
		return;
	}
	unif_name = "lambient";
	Unif_lamb = glGetUniformLocation(Program, unif_name);
	if (Unif_lamb == -1)
	{
		std::cout << "could not bind uniform " << unif_name << std::endl;
		return;
	}
	unif_name = "ldiffuse";
	Unif_ldiff = glGetUniformLocation(Program, unif_name);
	if (Unif_ldiff == -1)
	{
		std::cout << "could not bind uniform " <<unif_name << std::endl;
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
	glUniform4fv(Unif_lamb, 4, lambient);
	glUniform4fv(Unif_lpos, 4, lpos);
	//glUniform4fv(Unif_ldiff, 1, ldiffuse);

	// Привязываем вао
	glBindVertexArray(VAO);
	// Передаем данные на видеокарту(рисуем)
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
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

#endif
