#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "textfile.h"
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLM_FORCE_SWIZZLE // Or defined when building (e.g. -DGLM_FORCE_SWIZZLE)
#include <glm/glm.hpp>
#include "glm/gtc/type_ptr.hpp"

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

#define deg2rad(x) ((x)*((3.1415926f)/(180.0f)))

using namespace glm;
using namespace std;
const double PI = 3.1415926;
// Default window size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
// current window size
int screenWidth = WINDOW_WIDTH, screenHeight = WINDOW_HEIGHT;
bool pause = false;
vector<string> filenames; // .obj filename list

typedef struct
{
	GLuint diffuseTexture;
} PhongMaterial;

typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint vboTex;
	GLuint ebo;
	GLuint p_color;
	int vertex_count;
	GLuint p_normal;
	GLuint p_texCoord;
	PhongMaterial material;
	int indexCount;
} Shape;

struct model
{
	vec3 position = vec3(0, 0, 0);
	vec3 scale = vec3(1, 1, 1);
	vec3 rotation = vec3(0, 0, 0);	// Euler form

	vector<Shape> shapes;
	vector<PhongMaterial> materials;
};
vector<model> models;

struct camera
{
	vec3 position;
	vec3 center;
	vec3 up_vector;
};
camera main_camera;

struct project_setting
{
	GLfloat nearClip, farClip;
	GLfloat fovy;
	GLfloat aspect;
};
project_setting proj;

Shape m_shape;
// vector<Shape> m_shape_list;

int cur_idx = 0; // represent which model should be rendered now
vector<string> model_list{
	"../objects/T.obj",
	"../objects/L.obj",
	"../objects/J.obj",
	"../objects/S.obj",
	"../objects/Z.obj",
	"../objects/I.obj",
	"../objects/O.obj",
};

GLuint program;

GLuint iLocP;
GLuint iLocV;
GLuint iLocM;

double zOffset = 4.0f;

// Call back function for window reshape
void ChangeSize(GLFWwindow* window, int width, int height)
{
	// glViewport(0, 0, width, height);
	proj.aspect = (float)(width) / (float)height;
	screenWidth = width;
	screenHeight = height;

	glViewport(0, 0, screenWidth, screenHeight);
}

mat4 CalRotation(GLfloat x, GLfloat y, GLfloat z) {
	mat4 R = rotate(mat4(1.0), radians(x), vec3(1.0, 0.0, 0.0));
	R = R * rotate(mat4(1.0), radians(y), vec3(0.0, 1.0, 0.0));
	R = R * rotate(mat4(1.0), radians(z), vec3(0.0, 0.0, 1.0));
	return R;
}

void renderModel(mat4 model_matrix, int idx) {
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix));

	for (int i = 0; i < models[idx].shapes.size(); i++)
	{
		glBindVertexArray(models[idx].shapes[i].vao);
		//glUniform1i(glGetUniformLocation(program, "uTexture"), 0); 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, models[idx].materials[i].diffuseTexture);

		glDrawArrays(GL_TRIANGLES, 0, models[idx].shapes[i].vertex_count);
	}
}

// Render function for display rendering
void RenderScene() {
	// clear canvas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	mat4 project_matrix;
	// perspective(fov, aspect_ratio, near_plane_distance, far_plane_distance)
	// ps. fov = field of view, it represent how much range(degree) is this camera could see 
	project_matrix = perspective(deg2rad(proj.fovy), proj.aspect, proj.nearClip, proj.farClip);

	mat4 view_matrix;
	// lookAt(camera_position, camera_viewing_vector, up_vector)
	// up_vector represent the vector which define the direction of 'up'
	view_matrix = lookAt(main_camera.position, main_camera.center, main_camera.up_vector);
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));

	// render
	double curTime = glfwGetTime();
	static double prevTime = 0;
	static double accuTime = 0;
	if(!pause)
		accuTime += curTime - prevTime;
	prevTime = curTime;

	// body
	models[cur_idx].position.y = sin(accuTime * 6) / 10;
	//cout << sin(accuTime * 6) << '\n';
	mat4 T_body = translate(mat4(1.0), vec3(models[cur_idx].position));
	mat4 R_body = CalRotation(110, 180, sin(-accuTime * 3)*10);
	mat4 S_body = scale(mat4(1.0), vec3(0.5, 1, 0.7));
	mat4 model_matrix_body = T_body * R_body * S_body;
	renderModel(model_matrix_body, 0);
	
	// head
	mat4 T_head = T_body * translate(mat4(1.0), vec3(0, 0.8, 0));
	mat4 R_head = R_body * CalRotation(10, 0, 0);
	mat4 S_head = scale(mat4(1.0), vec3(0.3));

	mat4 model_matrix_head = T_head * R_head * S_head;
	renderModel(model_matrix_head, 6);

	// left arm
	mat4 T_leftarm = T_body * translate(mat4(1.0), vec3(0.8, 0.1, 0.1));
	mat4 R_leftarm = R_body*CalRotation(15, 10, 90);
	mat4 S_leftarm = scale(mat4(1.0), vec3(0.9));

	mat4 model_matrix_leftarm = T_leftarm * R_leftarm * S_leftarm;
	renderModel(model_matrix_leftarm, 1);

	// left hand
	mat4 T_lefthand = T_body * translate(mat4(1.0), vec3(1, -0.6,  0.1));
	mat4 R_lefthand = R_leftarm * CalRotation(0, 90, 0);
	mat4 S_lefthand = scale(mat4(1.0), vec3(0.9));
	R_lefthand *= rotate(mat4(1.0), GLfloat(-sin(accuTime * 3) / 5), vec3(1, 0, 0));
	mat4 model_matrix_lefthand = model_matrix_leftarm * R_lefthand * inverse(model_matrix_leftarm) *T_lefthand  * S_lefthand;
	renderModel(model_matrix_lefthand, 5);


	// right arm
	mat4 T_rightarm = T_body * translate(mat4(1.0), vec3(-0.8, 0.1, 0.1));
	mat4 R_rightarm = R_body * CalRotation(15, -10, 90);
	mat4 S_rightarm = scale(mat4(1.0), vec3(0.9));

	mat4 model_matrix_rightarm = T_rightarm * R_rightarm * S_rightarm;
	renderModel(model_matrix_rightarm, 2);

	// right hand
	mat4 T_righthand = T_body * translate(mat4(1.0), vec3(-1, -0.6, -0.1));
	mat4 R_righthand = R_rightarm * CalRotation(90, 90, 90);
	mat4 S_righthand = scale(mat4(1.0), vec3(0.9));
	R_righthand *= rotate(mat4(1.0), GLfloat(sin(accuTime * 3) / 5), vec3(1, 0, 0));
	mat4 model_matrix_righthand = model_matrix_rightarm * R_righthand * inverse(model_matrix_rightarm) * T_righthand  * S_righthand;
	renderModel(model_matrix_righthand, 5);

	// right leg
	mat4 T_rightleg= T_body * translate(mat4(1.0), vec3(-0.4, -0.8, -0.1));
	mat4 R_rightleg= R_body * CalRotation(120, 0, 100);
	mat4 S_rightleg = scale(mat4(1.0), vec3(0.6));
	
	R_rightleg *= rotate(mat4(1.0), GLfloat(PI + sin(accuTime * 3)/2), vec3(0, 1, 0));
	mat4 model_matrix_rightleg = T_rightleg * R_rightleg * S_rightleg;
	renderModel(model_matrix_rightleg, 3);

	// right foot
	mat4 T_rightfoot = T_body * translate(mat4(1.0), vec3(1, -0.8, -0.3));
	mat4 R_rightfoot = R_rightleg * CalRotation(0, 0, -5);
	mat4 S_rightfoot = scale(mat4(1.0), vec3(0.9));

	R_rightfoot *= rotate(mat4(1.0), GLfloat(sin(accuTime * 3) / 5), vec3(1, 0, 0));
	R_rightfoot *= rotate(mat4(1.0), GLfloat(sin(accuTime * 3) / 5), vec3(0, 1, 0));
	mat4 model_matrix_rightfoot = model_matrix_rightleg * R_rightfoot * inverse(model_matrix_rightleg) * T_rightfoot * S_rightfoot;
	renderModel(model_matrix_rightfoot, 5);

	// left leg
	mat4 T_leftleg = T_body * translate(mat4(1.0), vec3(0.4, -0.8, -0.3));
	mat4 R_leftleg = R_body * CalRotation(120, 180, 280);
	mat4 S_leftleg = scale(mat4(1.0), vec3(0.6));

	R_leftleg *= rotate(mat4(1.0), GLfloat(PI - sin(accuTime *3)/2), vec3(0, 1, 0));
	mat4 model_matrix_leftleg = T_leftleg * R_leftleg * S_leftleg;
	renderModel(model_matrix_leftleg, 4);

	// leftfoot
	mat4 T_leftfoot = T_body * translate(mat4(1.0), vec3(-1, -0.8, -0.3));
	mat4 R_leftfoot = R_leftleg * CalRotation(20, 20, -5);
	mat4 S_leftfoot = scale(mat4(1.0), vec3(0.9));

	R_leftfoot *= rotate(mat4(1.0), GLfloat(sin(accuTime * 3) / 5), vec3(0, 0, 1));
	R_leftfoot *= rotate(mat4(1.0), GLfloat(sin(accuTime * 3) / 5), vec3(0, 1, 0));
	mat4 model_matrix_leftfoot = model_matrix_leftleg * R_leftfoot * inverse(model_matrix_leftleg) *T_leftfoot * S_leftfoot;
	renderModel(model_matrix_leftfoot, 5);
}

// Call back function for keyboard
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_D:
			printf("press d!\n");
			models[cur_idx].position.x += 1;
			break;
		case GLFW_KEY_A:
			printf("press a!\n");
			models[cur_idx].position.x -= 1;
			break;
		case GLFW_KEY_W:
			printf("press w!\n");
			models[cur_idx].position.z -= 1;
			break;
		case GLFW_KEY_S:
			printf("press s!\n");
			models[cur_idx].position.z += 1;
			break;
		case GLFW_KEY_SPACE:
			printf("press space!\n");
			pause = !pause;
			break;
		default:
			break;
		}
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// scroll up positive, otherwise it would be negtive
	double newZoffset = zOffset - yoffset * 0.2;
	main_camera.position.z = main_camera.position.z / zOffset * (newZoffset);
	main_camera.position.y = main_camera.position.y / zOffset * (newZoffset);
	zOffset = newZoffset;

	printf("Scroll Event: (%f, %f)\n", xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		printf("Click Mouse Left button!\n");
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		printf("Release Mouse Left button!\n");
	}
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{

	static int x = 400;
	static int y = 300;

	float delta_x = xpos - x;
	float delta_y = ypos - y;

	float sensitivityX = 360.0 / WINDOW_WIDTH;
	float sensitivityY = 180.0 / WINDOW_HEIGHT;
	main_camera.position.x = sin(radians(delta_x * sensitivityX)) * zOffset;
	main_camera.position.z = cos(radians(delta_x * sensitivityX)) * zOffset;
}

void setShaders()
{
	GLuint v, f, p;
	char* vs = NULL;
	char* fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = textFileRead("shader.vs.glsl");
	fs = textFileRead("shader.fs.glsl");

	glShaderSource(v, 1, (const GLchar**)&vs, NULL);
	glShaderSource(f, 1, (const GLchar**)&fs, NULL);

	free(vs);
	free(fs);

	GLint success;
	char infoLog[1000];
	// compile vertex shader
	glCompileShader(v);
	// check for shader compile errors
	glGetShaderiv(v, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(v, 1000, NULL, infoLog);
		std::cout << "ERROR: VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// compile fragment shader
	glCompileShader(f);
	// check for shader compile errors
	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(f, 1000, NULL, infoLog);
		std::cout << "ERROR: FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// create program object
	p = glCreateProgram();

	// attach shaders to program object
	glAttachShader(p, f);
	glAttachShader(p, v);

	// link program
	glLinkProgram(p);
	// check for linking errors
	glGetProgramiv(p, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(p, 1000, NULL, infoLog);
		std::cout << "ERROR: SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(v);
	glDeleteShader(f);

	if (success)
		glUseProgram(p);
	else
	{
		system("pause");
		exit(123);
	}

	program = p;
}

void normalization(tinyobj::attrib_t* attrib, vector<GLfloat>& vertices, vector<GLfloat>& colors, vector<GLfloat>& normals, vector<GLfloat>& textureCoords, vector<int>& material_id, tinyobj::shape_t* shape)
{
	vector<float> xVector, yVector, zVector;
	float minX = 10000, maxX = -10000, minY = 10000, maxY = -10000, minZ = 10000, maxZ = -10000;

	// find out min and max value of X, Y and Z axis
	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		//maxs = max(maxs, attrib->vertices.at(i));
		if (i % 3 == 0)
		{

			xVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minX)
			{
				minX = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxX)
			{
				maxX = attrib->vertices.at(i);
			}
		}
		else if (i % 3 == 1)
		{
			yVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minY)
			{
				minY = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxY)
			{
				maxY = attrib->vertices.at(i);
			}
		}
		else if (i % 3 == 2)
		{
			zVector.push_back(attrib->vertices.at(i));

			if (attrib->vertices.at(i) < minZ)
			{
				minZ = attrib->vertices.at(i);
			}

			if (attrib->vertices.at(i) > maxZ)
			{
				maxZ = attrib->vertices.at(i);
			}
		}
	}

	float offsetX = (maxX + minX) / 2;
	float offsetY = (maxY + minY) / 2;
	float offsetZ = (maxZ + minZ) / 2;

	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		if (offsetX != 0 && i % 3 == 0)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetX;
		}
		else if (offsetY != 0 && i % 3 == 1)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetY;
		}
		else if (offsetZ != 0 && i % 3 == 2)
		{
			attrib->vertices.at(i) = attrib->vertices.at(i) - offsetZ;
		}
	}

	float greatestAxis = maxX - minX;
	float distanceOfYAxis = maxY - minY;
	float distanceOfZAxis = maxZ - minZ;

	if (distanceOfYAxis > greatestAxis)
	{
		greatestAxis = distanceOfYAxis;
	}

	if (distanceOfZAxis > greatestAxis)
	{
		greatestAxis = distanceOfZAxis;
	}

	float scale = greatestAxis / 2;

	for (int i = 0; i < attrib->vertices.size(); i++)
	{
		//std::cout << i << " = " << (double)(attrib.vertices.at(i) / greatestAxis) << std::endl;
		attrib->vertices.at(i) = attrib->vertices.at(i) / scale;
	}
	size_t index_offset = 0;
	for (size_t f = 0; f < shape->mesh.num_face_vertices.size(); f++) {
		int fv = shape->mesh.num_face_vertices[f];

		// Loop over vertices in the face.
		for (size_t v = 0; v < fv; v++) {
			// access to vertex
			tinyobj::index_t idx = shape->mesh.indices[index_offset + v];
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 0]);
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 1]);
			vertices.push_back(attrib->vertices[3 * idx.vertex_index + 2]);
			// Optional: vertex colors
			colors.push_back(attrib->colors[3 * idx.vertex_index + 0]);
			colors.push_back(attrib->colors[3 * idx.vertex_index + 1]);
			colors.push_back(attrib->colors[3 * idx.vertex_index + 2]);
			// Optional: vertex normals
			normals.push_back(attrib->normals[3 * idx.normal_index + 0]);
			normals.push_back(attrib->normals[3 * idx.normal_index + 1]);
			normals.push_back(attrib->normals[3 * idx.normal_index + 2]);
			// Optional: texture coordinate
			textureCoords.push_back(attrib->texcoords[2 * idx.texcoord_index + 0]);
			textureCoords.push_back(attrib->texcoords[2 * idx.texcoord_index + 1]);
			// The material of this vertex
			material_id.push_back(shape->mesh.material_ids[f]);
		}
		index_offset += fv;
	}
}

static string GetBaseDir(const string& filepath) {
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}


GLuint LoadTextureImage(string image_path)
{
	int channel, width, height;
	int require_channel = 4;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(image_path.c_str(), &width, &height, &channel, require_channel);
	if (data != NULL)
	{
		GLuint tex;

		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		stbi_image_free(data);
		return tex;
	}
	else
	{
		cout << "LoadTextureImage: Cannot load image from " << image_path << endl;
		return -1;
	}
}

vector<Shape> SplitShapeByMaterial(vector<GLfloat>& vertices, vector<GLfloat>& colors, vector<GLfloat>& normals, vector<GLfloat>& textureCoords, vector<int>& material_id, vector<PhongMaterial>& materials)
{
	vector<Shape> res;
	for (int m = 0; m < materials.size(); m++)
	{
		vector<GLfloat> m_vertices, m_colors, m_normals, m_textureCoords;
		for (int v = 0; v < material_id.size(); v++)
		{
			// extract all vertices with same material id and create a new shape for it.
			if (material_id[v] == m)
			{
				m_vertices.push_back(vertices[v * 3 + 0]);
				m_vertices.push_back(vertices[v * 3 + 1]);
				m_vertices.push_back(vertices[v * 3 + 2]);

				m_colors.push_back(colors[v * 3 + 0]);
				m_colors.push_back(colors[v * 3 + 1]);
				m_colors.push_back(colors[v * 3 + 2]);

				m_normals.push_back(normals[v * 3 + 0]);
				m_normals.push_back(normals[v * 3 + 1]);
				m_normals.push_back(normals[v * 3 + 2]);

				m_textureCoords.push_back(textureCoords[v * 2 + 0]);
				m_textureCoords.push_back(textureCoords[v * 2 + 1]);
			}
		}

		if (!m_vertices.empty())
		{
			Shape tmp_shape;
			glGenVertexArrays(1, &tmp_shape.vao);
			glBindVertexArray(tmp_shape.vao);

			glGenBuffers(1, &tmp_shape.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.vbo);
			glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(GL_FLOAT), &m_vertices.at(0), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
			tmp_shape.vertex_count = m_vertices.size() / 3;

			glGenBuffers(1, &tmp_shape.p_color);
			glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_color);
			glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(GL_FLOAT), &m_colors.at(0), GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glGenBuffers(1, &tmp_shape.p_normal);
			glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_normal);
			glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(GL_FLOAT), &m_normals.at(0), GL_STATIC_DRAW);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glGenBuffers(1, &tmp_shape.p_texCoord);
			glBindBuffer(GL_ARRAY_BUFFER, tmp_shape.p_texCoord);
			glBufferData(GL_ARRAY_BUFFER, m_textureCoords.size() * sizeof(GL_FLOAT), &m_textureCoords.at(0), GL_STATIC_DRAW);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);

			tmp_shape.material = materials[m];
			res.push_back(tmp_shape);
		}
	}

	return res;
}

void LoadTexturedModels(string model_path)
{
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	tinyobj::attrib_t attrib;
	vector<GLfloat> vertices;
	vector<GLfloat> colors;
	vector<GLfloat> normals;
	vector<GLfloat> textureCoords;
	vector<int> material_id;

	string err;
	string warn;

	string base_dir = GetBaseDir(model_path); // handle .mtl with relative path

#ifdef _WIN32
	base_dir += "\\";
#else
	base_dir += "/";
#endif

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str(), base_dir.c_str());

	if (!warn.empty()) {
		cout << warn << std::endl;
	}

	if (!err.empty()) {
		cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	printf("Load Models Success ! Shapes size %d Material size %d\n", shapes.size(), materials.size());
	model tmp_model;

	vector<PhongMaterial> allMaterial;
	for (int i = 0; i < materials.size(); i++)
	{
		PhongMaterial material;
		material.diffuseTexture = LoadTextureImage(base_dir + string(materials[i].diffuse_texname));
		if (material.diffuseTexture == -1)
		{
			cout << "LoadTexturedModels: Fail to load model's material " << i << endl;
			system("pause");

		}

		allMaterial.push_back(material);
	}

	for (int i = 0; i < shapes.size(); i++)
	{

		vertices.clear();
		colors.clear();
		normals.clear();
		textureCoords.clear();
		material_id.clear();
		normalization(&attrib, vertices, colors, normals, textureCoords, material_id, &shapes[i]);
		// printf("Vertices size: %d", vertices.size() / 3);

		// split current shape into multiple shapes base on material_id.
		vector<Shape> splitedShapeByMaterial = SplitShapeByMaterial(vertices, colors, normals, textureCoords, material_id, allMaterial);

		// concatenate splited shape to model's shape list
		tmp_model.materials.push_back(allMaterial[material_id[0]]);
		tmp_model.shapes.insert(tmp_model.shapes.end(), splitedShapeByMaterial.begin(), splitedShapeByMaterial.end());
	}
	shapes.clear();
	materials.clear();
	models.push_back(tmp_model);
}

void initParameter()
{
	proj.nearClip = 0.001;
	proj.farClip = 1000.0;
	proj.fovy = 80;
	proj.aspect = (float)(WINDOW_WIDTH) / (float)WINDOW_HEIGHT; // adjust width for side by side view

	main_camera.position = vec3(0.0f, 0.0f, 2.0f);
	main_camera.center = vec3(0.0f, 0.0f, 0.0f);
	main_camera.up_vector = vec3(0.0f, 1.0f, 0.0f);
}

void setUniformVariables()
{
	iLocP = glGetUniformLocation(program, "um4p");
	iLocV = glGetUniformLocation(program, "um4v");
	iLocM = glGetUniformLocation(program, "um4m");
}

void setupRC()
{
	// setup shaders
	setShaders();
	initParameter();
	setUniformVariables();

	// OpenGL States and Values
	glClearColor(0, 0, 0, 1.0);

	for (string model_path : model_list) {
		LoadTexturedModels(model_path);
	}
}

void glPrintContextInfo(bool printExtension)
{
	cout << "GL_VENDOR = " << (const char*)glGetString(GL_VENDOR) << endl;
	cout << "GL_RENDERER = " << (const char*)glGetString(GL_RENDERER) << endl;
	cout << "GL_VERSION = " << (const char*)glGetString(GL_VERSION) << endl;
	cout << "GL_SHADING_LANGUAGE_VERSION = " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	if (printExtension)
	{
		GLint numExt;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
		cout << "GL_EXTENSIONS =" << endl;
		for (GLint i = 0; i < numExt; i++)
		{
			cout << "\t" << (const char*)glGetStringi(GL_EXTENSIONS, i) << endl;
		}
	}
}

int main(int argc, char** argv)
{

	// initial glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // fix compilation on OS X
#endif


	// create window
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Student_ID CG Training Coarse", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	// load OpenGL function pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glPrintContextInfo(false);

	// register glfw callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetFramebufferSizeCallback(window, ChangeSize);

	glEnable(GL_DEPTH_TEST);
	// Setup render context
	setupRC();

	// main loop
	while (!glfwWindowShouldClose(window))
	{
		// Render
		RenderScene();

		// swap buffer from back to front
		glfwSwapBuffers(window);

		// Poll input event
		glfwPollEvents();
	}

	// just for compatibiliy purposes
	return 0;
}
