#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <iostream>
#include <fstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderTools.h"
#include "bufferTools.h"
#include "matrixs.h"
#include "data.h"



GLuint VBO;
GLuint VAO;

GLuint shaderProgram;


GLfloat deltaTime = 0.0f; //��ǰ֡����һ֡��ʱ���
GLfloat lastFrame = 0.0f;	//��һ֡ʱ��
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

int width, height;

const char *vertexShaderSource = "vert.vs";
const char *fragmentShaderSource = "frag.fs";


glm::mat4 model(1.0f);
glm::mat4 view(1.0f);
glm::mat4 proj(1.0f);


//��¼�����һϵ�в���
GLfloat aspect = 45.0f;
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//��¼������Ϣ
glm::vec3 LightColor = glm::vec3(1.0f,1.0f,1.0f);
glm::vec3 lightPos = glm::vec3(1.0f, 1.0f, 1.0f);

//�Ƚ���cubeindex 
int cubeindex = 0;
void prepare() {
	readData();
	voxels = (GLfloat*)malloc(sizeof(GLfloat) * Cubesize * Cubesize * Cubesize);
	vertices = (GLfloat*)malloc(sizeof(GLfloat) * Cubesize * Cubesize * Cubesize * 5);
	for (int i = 0; i < Cubesize * Cubesize * Cubesize; i++) {
		for (int j = 0; j < 5; j++) {
			vertices[i * 5 + j] = Alldata[i][j];
		}
	}
	for (int i = 0; i < Cubesize * Cubesize * Cubesize; i++) {
		voxels[i] = vertices[i * 5 + 3];
	}
	int bound = Cubesize - 1;
	for (int i = 0; i < bound * bound * bound; i++) {
		//����ÿһ����Ԫ������cubeindex
		//Ϊ�˷��������ı������Ӧ��ϵ����һ����Ե��˳�����ͳһ
		cubeindex = 0;
		if (voxels[i] < 1.0f)cubeindex |= 1;
		if (voxels[i + 1] < 1.0f)cubeindex |= 2;
		if (voxels[i + Cubesize] < 1.0f)cubeindex |= 8;
		if (voxels[i + Cubesize + 1] < 1.0f)cubeindex |= 4;
		if (voxels[i + Cubesize * Cubesize] < 1.0f)cubeindex |= 16;
		if (voxels[i + Cubesize * Cubesize + 1] < 1.0f)cubeindex |= 32;
		if (voxels[i + Cubesize * Cubesize + Cubesize] < 1.0f)cubeindex |= 128;
		if (voxels[i + Cubesize * Cubesize + Cubesize + 1] < 1.0f)cubeindex |= 64;
		//cout << voxels[i] << endl;
		//cout << (int)cubeindex << endl;
	}
	cout << sizeof(vertices) << endl;
}

void key_callback(GLFWwindow *window,int key,int scancode,int action,int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
	GLfloat cameraSpeed = 0.5f * deltaTime;
	if (key == GLFW_KEY_W)
		cameraPos += cameraSpeed * cameraFront;
	if (key == GLFW_KEY_S)
		cameraPos -= cameraSpeed * cameraFront;
	if (key == GLFW_KEY_A)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (key == GLFW_KEY_D)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // ע���������෴�ģ���Ϊy����ķ�Χ�Ǵ������ϵ�
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	//���Ƹ�����
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (aspect >= 1.0f && aspect <= 45.0f)
		aspect -= yoffset * 0.05F;
	if (aspect <= 1.0f)
		aspect = 1.0f;
	if (aspect >= 45.0f)
		aspect = 45.0f;

	
}

void renderScence() {

	GLfloat currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	//����view proj
	glm::mat4 model(1.0f);
	ModelMatrix(model);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	proj = glm::perspective(aspect, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);


	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
	//�������ôдÿ�ζ���Ҫ��VBO��������
	glBindVertexArray(VAO);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//�ӵ�ǰ�󶨵�EBO�л�ȡ���� ���ڰ���VAO,��VAO�ְ���EBO���Բ���Ҫ�ظ�����
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	//�ڶ������� ���Ƶ�ĸ���
	//���������� ����������
	//���ĸ����� ƫ����

	glDrawArrays(GL_POINTS, 0, Cubesize * Cubesize * Cubesize);
	//���ն����˳�򹹳������� �����ǰ�������
	glBindVertexArray(0);
}

int main() {
	prepare();


	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow *window = glfwCreateWindow(600, 600, "learnOpengl", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "fail to create glfw window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//����Ϊ��ǰ������
	glfwMakeContextCurrent(window);

	//ʹ��glew ����opengl ָ�뺯��
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "fail to init glew" << std::endl;
		return -1;
	}

	//�����ӿڴ�С
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	//������� ����׽λ��
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	//���ûص����� ����\���λ��\�����¼�
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	

	//��������������󣬸ò�����VBP��VAO�໥��
	glGenVertexArrays(1,&VAO);
	glBindVertexArray(VAO);
		//�������㻺�����
		CreateVertexBuffer(&VBO);
		//���������������
		//CreateIndexBuffer(&EBO);
		//��ʱVAO�Ѿ���VBO EBO �໥����
	glBindVertexArray(0);//���




	//������ɫ��
	GLuint vertexShader;
	GLuint fragmentShader;
	CompileShader(vertexShader, vertexShaderSource, GL_VERTEX_SHADER);
	CompileShader(fragmentShader,fragmentShaderSource,GL_FRAGMENT_SHADER);

	//������ɫ��������󣬲�����Ⱦʱ����
	CreateProgram(shaderProgram,vertexShader,fragmentShader);

	//compile ==> link
	glUseProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	
	//����
	glEnable(GL_DEPTH_TEST);




	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		renderScence();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
