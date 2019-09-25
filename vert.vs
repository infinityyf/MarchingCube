#version 330 core
layout (location = 0) in vec3 position;

out vec3 vertexColor;
out vec3 fragpos;
uniform mat4 model; //转化为世界坐标
uniform mat4 view; //转化为相机坐标
uniform mat4 projection;	//转化为投影坐标


void main()
{
    gl_Position =projection * view*model *vec4(position.x, position.y, position.z, 1.0);
	vertexColor = vec3(1.0f,1.0f,1.0f);
	fragpos = vec3(model * vec4(position, 1.0f));
}
