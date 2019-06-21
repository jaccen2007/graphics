#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <chrono>

#include "Camera.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// 2D
//#define GLTF_MODEL "BoxTextured/TriangleWithoutIndices.gltf"
//#define GLTF_MODEL "BoxTextured/Triangle.gltf"
//#define GLTF_MODEL "BoxTextured/SimpleMeshes.gltf"

//<NOT DRAW ALL>#define GLTF_MODEL "BoxTextured/SimpleMorph.gltf"
//<CLIPING SPACE PROBLEM>#define GLTF_MODEL "BoxTextured/SimpleSparseAccessor.gltf"

// 3D

//#define GLTF_MODEL "BoxTextured/Box.gltf"
//#define GLTF_MODEL "BoxTextured/BoxInterleaved.gltf"
//#define GLTF_MODEL "BoxTextured/BoxTextured.gltf"
//#define GLTF_MODEL "BoxTextured/BoxTexturedNonPowerOfTwo.gltf"
//<NOT DRAWING COLORFUL> #define GLTF_MODEL "BoxTextured/BoxVertexColors.gltf"
//#define GLTF_MODEL "BoxTextured/Duck.gltf"
//<NOT WORKING>#define GLTF_MODEL "BoxTextured/2CylinderEngine.gltf"


//test_gltf에서 들어가기
// #define GLTF_MODEL "test_gltf/01_TriangleWithoutIndices/glTF/TriangleWithoutIndices.gltf"
// #define GLTF_MODEL "test_gltf/02_Cameras/glTF/Cameras.gltf"
// #define GLTF_MODEL "test_gltf/03_Box/glTF/Box.gltf"
// #define GLTF_MODEL "test_gltf/04_BoxTextured/glTF/BoxTextured.gltf"
// #define GLTF_MODEL "test_gltf/05_BoxVertexColors/glTF/BoxVertexColors.gltf" // 컬러가 안나옴
// #define GLTF_MODEL "test_gltf/06_Duck/glTF/Duck.gltf"
// #define GLTF_MODEL "test_gltf/07_BrainStem/glTF/BrainStem.gltf"
// #define GLTF_MODEL "test_gltf/08_Lantern/glTF/Lantern.gltf" //오류난다
// #define GLTF_MODEL "test_gltf/09_TextureSettingsTest/glTF/TextureSettingsTest.gltf"
// #define GLTF_MODEL "test_gltf/10_Sponza/glTF/Sponza.gltf" // 아무것도 안보임

#include "../glTF/tiny_gltf.h"
#define BUFFER_OFFSET(i) ((char*)0 + (i))

#include "../common/transform.hpp"

namespace kmuvcl {
  namespace math {
    template <typename T>
    inline mat4x4f quat2mat(T x, T y, T z, T w)
    {
      T xx = x * x;
      T xy = x * y;
      T xz = x * z;
      T xw = x * w;

      T yy = y * y;
      T yz = y * z;
      T yw = y * w;

      T zz = z * z;
      T zw = z * w;

      mat4x4f mat_rot;
      mat_rot(0, 0) = 1.0f - 2.0f*(yy + zz);
      mat_rot(0, 1) = 2.0f*(xy - zw);
      mat_rot(0, 2) = 2.0f*(xz + yw);

      mat_rot(1, 0) = 2.0f*(xy + zw);
      mat_rot(1, 1) = 1.0f - 2.0f*(xx + zz);
      mat_rot(1, 2) = 2.0f*(yz - xw);

      mat_rot(2, 0) = 2.0f*(xz - yw);
      mat_rot(2, 1) = 2.0f*(yz + xw);
      mat_rot(2, 2) = 1.0f - 2.0f*(xx + yy);

      mat_rot(3, 3) = 1.0f;
      return mat_rot;
    }
    const float MATH_PI = 3.14159265358979323846f;

    template <typename T>
    inline T rad2deg(T deg)
    {
      T rad = deg * (180.0f / MATH_PI);
      return rad;
    }

    template <typename T>
    inline T deg2rad(T rad)
    {
      T deg = rad * (MATH_PI / 180.0f);
      return deg;
    }
  } // math
} // kmuvcl

////////////////////////////////////////////////////////////////////////////////
/// OpenGL state 초기화 관련 함수
////////////////////////////////////////////////////////////////////////////////
void init_state();
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// 쉐이더 관련 변수 및 함수
////////////////////////////////////////////////////////////////////////////////
GLuint  program;          // 쉐이더 프로그램 객체의 레퍼런스 값
GLint   loc_a_position;
GLint   loc_a_texcoord;

GLint   loc_u_PVM;
GLint   loc_u_texture;
GLint   loc_a_color;      // attribute 변수 a_color 위치

GLuint create_shader_from_file(const std::string& filename, GLuint shader_type);
void init_shader_program();
////////////////////////////////////////////////////////////////////////////////
GLint   loc_a_normal;
GLint   loc_u_M;

GLint   loc_u_view_position_wc;
GLint   loc_u_light_position_wc;

GLint   loc_u_light_diffuse;
GLint   loc_u_light_specular;

GLint   loc_u_material_specular;
GLint   loc_u_material_shininess;

GLint   loc_u_diffuse_texture;
GLuint  diffuse_texid;

kmuvcl::math::vec3f view_position_wc;

kmuvcl::math::vec3f light_position_wc = kmuvcl::math::vec3f(0.0f, 0.8f, 0.8f);
kmuvcl::math::vec4f light_diffuse = kmuvcl::math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
kmuvcl::math::vec4f light_specular = kmuvcl::math::vec4f(0.1f, 0.1f, 0.1f, 0.1f);

kmuvcl::math::vec4f material_specular = kmuvcl::math::vec4f(1.0f, 1.0f, 1.0f, 1.0f);
float               material_shininess = 1.2f;

////////////////////////////////////////////////////////////////////////////////
/// 변환 관련 변수 및 함수
////////////////////////////////////////////////////////////////////////////////
kmuvcl::math::mat4x4f   mat_view, mat_proj;
kmuvcl::math::mat4x4f   mat_PVM;

void set_transform();
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// 카메라 관련 변수
////////////////////////////////////////////////////////////////////////////////
Camera  camera;

float   g_aspect = 1.0f;
float   g_angle = 0.0;
int     camera_index = 0;
bool    g_is_animation = false;
std::chrono::time_point<std::chrono::system_clock> prev, curr;
////////////////////////////////////////////////////////////////////////////////
/// 렌더링 관련 변수 및 함수
////////////////////////////////////////////////////////////////////////////////
tinygltf::Model model;

GLuint position_buffer;
GLuint normal_buffer;
GLuint texcoord_buffer;
GLuint index_buffer;
GLuint color_buffer; // GPU 메모리에서 color_buffer의 위치

GLuint texid;

bool load_model(tinygltf::Model &model, const std::string filename);
void init_buffer_objects();     // VBO init 함수: GPU의 VBO를 초기화하는 함수.
void init_texture_objects();

void draw_scene();
void draw_node(const tinygltf::Node& node, kmuvcl::math::mat4f mat_view);
void draw_mesh(const tinygltf::Mesh& mesh, const kmuvcl::math::mat4f& mat_model);
////////////////////////////////////////////////////////////////////////////////
bool witch = false;

void init_state()
{
  glEnable(GL_DEPTH_TEST);
  camera.set_near(0.001f);
  camera.set_far(1000.0f);
}

// GLSL 파일을 읽어서 컴파일한 후 쉐이더 객체를 생성하는 함수
GLuint create_shader_from_file(const std::string& filename, GLuint shader_type)
{
  GLuint shader = 0;

  shader = glCreateShader(shader_type);

  std::ifstream shader_file(filename.c_str());
  std::string shader_string;

  shader_string.assign(
    (std::istreambuf_iterator<char>(shader_file)),
    std::istreambuf_iterator<char>());

  // Get rid of BOM in the head of shader_string
  // Because, some GLSL compiler (e.g., Mesa Shader compiler) cannot handle UTF-8 with BOM
  if (shader_string.compare(0, 3, "\xEF\xBB\xBF") == 0)  // Is the file marked as UTF-8?
  {
    std::cout << "Shader code (" << filename << ") is written in UTF-8 with BOM" << std::endl;
    std::cout << "  When we pass the shader code to GLSL compiler, we temporarily get rid of BOM" << std::endl;
    shader_string.erase(0, 3);                  // Now get rid of the BOM.
  }

  const GLchar* shader_src = shader_string.c_str();
  glShaderSource(shader, 1, (const GLchar * *)& shader_src, NULL);
  glCompileShader(shader);

  GLint is_compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled != GL_TRUE)
  {
    std::cout << "Shader COMPILE error: " << std::endl;

    GLint buf_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buf_len);

    std::string log_string(1 + buf_len, '\0');
    glGetShaderInfoLog(shader, buf_len, 0, (GLchar *)log_string.c_str());

    std::cout << "error_log: " << log_string << std::endl;

    glDeleteShader(shader);
    shader = 0;
  }

  return shader;
}

// vertex shader와 fragment shader를 링크시켜 program을 생성하는 함수
void init_shader_program()
{
  GLuint vertex_shader;
  GLuint fragment_shader;

  if(witch)
  {
   vertex_shader //= create_shader_from_file("./shader/vertex.glsl", GL_VERTEX_SHADER);
    = create_shader_from_file("./shader/vertex.glsl", GL_VERTEX_SHADER);

  std::cout << "vertex_shader id: " << vertex_shader << std::endl;
  assert(vertex_shader != 0);

   fragment_shader //=create_shader_from_file("./shader/fragment.glsl", GL_FRAGMENT_SHADER);
    = create_shader_from_file("./shader/fragment.glsl", GL_FRAGMENT_SHADER);

  std::cout << "fragment_shader id: " << fragment_shader << std::endl;
  assert(fragment_shader != 0);
  }
  else
  {
   vertex_shader //= create_shader_from_file("./shader/vertex.glsl", GL_VERTEX_SHADER);
    = create_shader_from_file("./shader/TriangleWithoutIndices_vertex.glsl", GL_VERTEX_SHADER);

  std::cout << "vertex_shader id: " << vertex_shader << std::endl;
  assert(vertex_shader != 0);

   fragment_shader //=create_shader_from_file("./shader/fragment.glsl", GL_FRAGMENT_SHADER);
    = create_shader_from_file("./shader/TriangleWithoutIndices_fragment.glsl", GL_FRAGMENT_SHADER);

  std::cout << "fragment_shader id: " << fragment_shader << std::endl;
  assert(fragment_shader != 0);
  }
  

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint is_linked;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
  if (is_linked != GL_TRUE)
  {
    std::cout << "Shader LINK error: " << std::endl;

    GLint buf_len;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_len);

    std::string log_string(1 + buf_len, '\0');
    glGetProgramInfoLog(program, buf_len, 0, (GLchar *)log_string.c_str());

    std::cout << "error_log: " << log_string << std::endl;

    glDeleteProgram(program);
    program = 0;
  }

  std::cout << "program id: " << program << std::endl;
  assert(program != 0);

  loc_u_PVM = glGetUniformLocation(program, "u_PVM");
  loc_u_M = glGetUniformLocation(program, "u_M");

  loc_u_texture = glGetUniformLocation(program, "u_texture");
  loc_a_color = glGetAttribLocation(program, "a_color");
  loc_a_position = glGetAttribLocation(program, "a_position");
  loc_a_texcoord = glGetAttribLocation(program, "a_texcoord");

  loc_u_view_position_wc = glGetUniformLocation(program, "u_view_position_wc");
  loc_u_light_position_wc = glGetUniformLocation(program, "u_light_position_wc");

  loc_u_light_diffuse = glGetUniformLocation(program, "u_light_diffuse");
  loc_u_light_specular = glGetUniformLocation(program, "u_light_specular");

  loc_u_material_specular = glGetUniformLocation(program, "u_material_specular");
  loc_u_material_shininess = glGetUniformLocation(program, "u_material_shininess");

  loc_u_diffuse_texture = glGetUniformLocation(program, "u_diffuse_texture");

  loc_a_normal = glGetAttribLocation(program, "a_normal");

}

bool load_model(tinygltf::Model &model, const std::string filename)
{
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
  if (!warn.empty())
  {
    std::cout << "WARNING: " << warn << std::endl;
  }

  if (!err.empty())
  {
    std::cout << "ERROR: " << err << std::endl;
  }

  if (!res)
  {
    std::cout << "Failed to load glTF: " << filename << std::endl;
  }
  else
  {
    std::cout << "Loaded glTF: " << filename << std::endl;
  }

  std::cout << std::endl;

  return res;
}

void init_buffer_objects()
{

  const std::vector<tinygltf::Mesh>& meshes = model.meshes;
  const std::vector<tinygltf::Accessor>& accessors = model.accessors;
  const std::vector<tinygltf::BufferView>& bufferViews = model.bufferViews;
  const std::vector<tinygltf::Buffer>& buffers = model.buffers;

  for (const tinygltf::Mesh& mesh : meshes)
  {
    for (const tinygltf::Primitive& primitive : mesh.primitives)
    {
      if(primitive.indices != -1)
      {  const tinygltf::Accessor& accessor = accessors[primitive.indices];
        const tinygltf::BufferView& bufferView = bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = buffers[bufferView.buffer];


        glGenBuffers(1, &index_buffer);
        glBindBuffer(bufferView.target, index_buffer);
        glBufferData(bufferView.target, bufferView.byteLength,
            &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);}

      for (const auto& attrib : primitive.attributes)
      {
        const tinygltf::Accessor& accessor = accessors[attrib.second];
        const tinygltf::BufferView& bufferView = bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = buffers[bufferView.buffer];

        if (attrib.first.compare("POSITION") == 0)
        {
          glGenBuffers(1, &position_buffer);
          glBindBuffer(bufferView.target, position_buffer);
          glBufferData(bufferView.target, bufferView.byteLength,
            &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        }
        else if (attrib.first.compare("NORMAL") == 0)
        {
          glGenBuffers(1, &normal_buffer);
          glBindBuffer(bufferView.target, normal_buffer);
          glBufferData(bufferView.target, bufferView.byteLength,
            &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        }
        else if (attrib.first.compare("TEXCOORD_0") == 0)
        {
          glGenBuffers(1, &texcoord_buffer);
          glBindBuffer(bufferView.target, texcoord_buffer);
          glBufferData(bufferView.target, bufferView.byteLength,
            &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        }
        else if (attrib.first.compare("COLOR_0") == 0)
        {
            glGenBuffers(1, &color_buffer);
            glBindBuffer(bufferView.target, color_buffer);
            glBufferData(bufferView.target, bufferView.byteLength,
                &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        }
      }
    }
  }
}

void init_texture_objects()
{
  const std::vector<tinygltf::Texture>& textures = model.textures;
  const std::vector<tinygltf::Image>& images = model.images;
  const std::vector<tinygltf::Sampler>& samplers = model.samplers;

  for (const tinygltf::Texture& texture : textures)
  {
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);

    const tinygltf::Image& image = images[texture.source];
    const tinygltf::Sampler& sampler = samplers[texture.sampler];

    GLenum format = GL_RGBA;
    if (image.component == 1) {
      format = GL_RED;
    }
    else if (image.component == 2) {
      format = GL_RG;
    }
    else if (image.component == 3) {
      format = GL_RGB;
    }

    GLenum type = GL_UNSIGNED_BYTE;
    if (image.bits == 16) {
      type = GL_UNSIGNED_SHORT;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
      image.width, image.height, 0, format, type, &image.image[0]);

    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);

    //glGenerateMipmap(GL_TEXTURE_2D);
  }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_P && action == GLFW_PRESS)
  {
    g_is_animation = !g_is_animation;
    std::cout << (g_is_animation ? "animation" : "no animation") << std::endl;
  }

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		camera_index = camera_index == 0 ? 1 : 0;
	}

  if (key == GLFW_KEY_A && action == GLFW_PRESS)
      camera.move_left(0.1f);
  if (key == GLFW_KEY_D && action == GLFW_PRESS)
      camera.move_right(0.1f);

  if (key == GLFW_KEY_Z && action == GLFW_PRESS)
      camera.move_forward(0.1f);
  if (key == GLFW_KEY_X && action == GLFW_PRESS)
      camera.move_backward(0.1f);

  if (key == GLFW_KEY_W && action == GLFW_PRESS)
      camera.move_up(0.1f);
  if (key == GLFW_KEY_S && action == GLFW_PRESS)
      camera.move_down(0.1f);

  if (key == GLFW_KEY_H && action == GLFW_PRESS)
	   light_position_wc += kmuvcl::math::vec3f(10.0f, 0.0f, 0.0f);
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		 light_position_wc -= kmuvcl::math::vec3f(10.0f, 0.0f, 0.0f);
	if (key == GLFW_KEY_J && action == GLFW_PRESS)
		 light_position_wc += kmuvcl::math::vec3f(0.0f, 10.0f, 0.0f);
	if (key == GLFW_KEY_K && action == GLFW_PRESS)
		 light_position_wc -= kmuvcl::math::vec3f(0.0f, 10.0f, 0.0f);
	if (key == GLFW_KEY_U && action == GLFW_PRESS)
		 light_position_wc += kmuvcl::math::vec3f(0.0f, 0.0f, 10.0f);
	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		 light_position_wc -= kmuvcl::math::vec3f(0.0f, 0.0f, 10.0f);

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		material_shininess += 0.5f;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		material_shininess -= 0.5f;

  std::cout << camera.position() << std::endl;    
}

void frambuffer_size_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);

	g_aspect = (float)width / (float)height;
}

void set_transform()
{
  
  kmuvcl::math::vec3f eye = camera.position();
	kmuvcl::math::vec3f up = camera.up_direction();
	kmuvcl::math::vec3f center = eye + camera.front_direction();

	mat_view = kmuvcl::math::lookAt(eye[0], eye[1], eye[2],
  center[0], center[1], center[2],
  up[0], up[1], up[2]);

  float n = camera.near();
  float f = camera.far();    

	if (camera.mode() == Camera::kOrtho)
	{
		float l = camera.left();
		float r = camera.right();
		float b = camera.bottom();
		float t = camera.top();

		mat_proj = kmuvcl::math::ortho(l, r, b, t, n, f);
	}
	else if (camera.mode() == Camera::kPerspective)
	{
		mat_proj = kmuvcl::math::perspective(camera.fovy(), g_aspect, n, f);
	}

  ////////////////////////////////////////////////////////////////////////

  const std::vector<tinygltf::Node>& nodes = model.nodes;
  if (model.cameras.size() > 0)
  {
    const std::vector<tinygltf::Camera>& cameras = model.cameras;
    const tinygltf::Camera& camera = cameras[camera_index];
    if (camera.type.compare("perspective") == 0)
    {
      float fovy = kmuvcl::math::rad2deg(camera.perspective.yfov);
      float aspectRatio = camera.perspective.aspectRatio;
      float znear = camera.perspective.znear;
      float zfar = camera.perspective.zfar;

      /*std::cout << "(camera.mode() == Camera::kPerspective)" << std::endl;
      std::cout << "(fovy, aspect, n, f): " << fovy << ", " << aspectRatio << ", " << znear << ", " << zfar << std::endl;*/
      mat_proj = kmuvcl::math::perspective(fovy, aspectRatio, znear, zfar);
    }
    else // (camera.type.compare("orthographic") == 0)
    {
      float xmag = camera.orthographic.xmag;
      float ymag = camera.orthographic.ymag;
      float znear = camera.orthographic.znear;
      float zfar = camera.orthographic.zfar;

      /*std::cout << "(camera.mode() == Camera::kOrtho)" << std::endl;
      std::cout << "(xmag, ymag, n, f): " << xmag << ", " << ymag << ", " << znear << ", " << zfar << std::endl;*/
      mat_proj = kmuvcl::math::ortho(-xmag, xmag, -ymag, ymag, znear, zfar);
    }

  }
  else
  {
    //mat_view.set_to_identity();
    mat_view = kmuvcl::math::translate(0.0f, 0.0f, -2.0f);

    //mat_proj.set_to_identity();
    float fovy = 70.0f;
    float aspectRatio = 1.0f;
    float znear = 0.01f;
    float zfar = 100.0f;

    mat_proj = kmuvcl::math::perspective(fovy, aspectRatio, znear, zfar);
  }
  
}

void draw_node(const tinygltf::Node& node, kmuvcl::math::mat4f mat_model)
{
  
  const std::vector<tinygltf::Node>& nodes = model.nodes;
  const std::vector<tinygltf::Mesh>& meshes = model.meshes;

  if (node.scale.size() == 3) {
    mat_model = mat_model * kmuvcl::math::scale<float>(
      node.scale[0], node.scale[1], node.scale[2]);
  }

  if (node.rotation.size() == 4) {
    mat_model = mat_model * kmuvcl::math::quat2mat(
      node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
  }

  if (node.translation.size() == 3) {
    mat_model = mat_model * kmuvcl::math::translate<float>(
      node.translation[0], node.translation[1], node.translation[2]);
  }
  
  if (node.matrix.size() == 16)
  {
    kmuvcl::math::mat4f mat_node;
    mat_node(0, 0) = node.matrix[0];
    mat_node(0, 1) = node.matrix[1];
    mat_node(0, 2) = node.matrix[2];
    mat_node(0, 3) = node.matrix[3];

    mat_node(1, 0) = node.matrix[4];
    mat_node(1, 1) = node.matrix[5];
    mat_node(1, 2) = node.matrix[6];
    mat_node(1, 3) = node.matrix[7];

    mat_node(2, 0) = node.matrix[8];
    mat_node(2, 1) = node.matrix[9];
    mat_node(2, 2) = node.matrix[10];
    mat_node(2, 3) = node.matrix[11];

    mat_node(3, 0) = node.matrix[12];
    mat_node(3, 1) = node.matrix[13];
    mat_node(3, 2) = node.matrix[14];
    mat_node(3, 3) = node.matrix[15];

    mat_model = mat_model * mat_node;
  }

  if (node.mesh > -1)
  {
    draw_mesh(meshes[node.mesh], mat_model);

  }

  for (size_t i = 0; i < node.children.size(); ++i)
  {
    draw_node(nodes[node.children[i]], mat_model);
  }
}

void draw_mesh(const tinygltf::Mesh& mesh, const kmuvcl::math::mat4f& mat_model)
{
  const std::vector<tinygltf::Material>& materials = model.materials;
  const std::vector<tinygltf::Texture>& textures = model.textures;
  const std::vector<tinygltf::Accessor>& accessors = model.accessors;
  const std::vector<tinygltf::BufferView>& bufferViews = model.bufferViews;

  glUseProgram(program);
  mat_PVM = mat_proj * mat_view * mat_model;

  glUniformMatrix4fv(loc_u_PVM, 1, GL_FALSE, mat_PVM);
  glUniformMatrix4fv(loc_u_M, 1, GL_FALSE, mat_model);

  view_position_wc = camera.position();
  glUniform3fv(loc_u_view_position_wc, 1, view_position_wc);
  glUniform3fv(loc_u_light_position_wc, 1, light_position_wc);

  glUniform4fv(loc_u_light_diffuse, 1, light_diffuse);
  glUniform4fv(loc_u_light_specular, 1, light_specular);

  glUniform4fv(loc_u_material_specular, 1, material_specular);
  glUniform1f(loc_u_material_shininess, material_shininess);
  for (const tinygltf::Primitive& primitive : mesh.primitives)
  {
    if (primitive.material > -1)
    {
      const tinygltf::Material& material = materials[primitive.material];
      for (const std::pair<std::string, tinygltf::Parameter> parameter : material.values)
      {
        if (parameter.first.compare("baseColorTexture") == 0)
        {
          if (parameter.second.TextureIndex() > -1)
          {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texid);

            glUniform1i(loc_u_texture, 0);
          }
        }
        else{

        }
      }
    }
  
    for (const std::pair<std::string, int>& attrib : primitive.attributes)
    {
      const int accessor_index = attrib.second;
      const tinygltf::Accessor& accessor = accessors[accessor_index];
      const tinygltf::BufferView& bufferView = bufferViews[accessor.bufferView];      
      const int byteStride = accessor.ByteStride(bufferView);

      if (attrib.first.compare("POSITION") == 0)
      {
        glBindBuffer(bufferView.target, position_buffer);
        glEnableVertexAttribArray(loc_a_position);
        glVertexAttribPointer(loc_a_position,
          accessor.type, accessor.componentType,
          accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
          BUFFER_OFFSET(accessor.byteOffset));
      }
      else if (attrib.first.compare("NORMAL") == 0)
      {
        glBindBuffer(bufferView.target, normal_buffer);
        glEnableVertexAttribArray(loc_a_normal);
        glVertexAttribPointer(loc_a_normal,
          accessor.type, accessor.componentType,
          accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
          BUFFER_OFFSET(accessor.byteOffset));
      }
      else if (attrib.first.compare("TEXCOORD_0") == 0)
      {
        glBindBuffer(bufferView.target, texcoord_buffer);
        glEnableVertexAttribArray(loc_a_texcoord);
        glVertexAttribPointer(loc_a_texcoord,
          accessor.type, accessor.componentType,
          accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
          BUFFER_OFFSET(accessor.byteOffset));
      }
    }

    const tinygltf::Accessor& index_accessor = accessors[primitive.indices];
    const tinygltf::BufferView& bufferView = bufferViews[index_accessor.bufferView];    

    glBindBuffer(bufferView.target, index_buffer);

    glDrawElements(primitive.mode,
      index_accessor.count,
      index_accessor.componentType,
      BUFFER_OFFSET(index_accessor.byteOffset));    

    // 정점 attribute 배열 비활성화
    glDisableVertexAttribArray(loc_a_texcoord);
    glDisableVertexAttribArray(loc_a_normal);
    glDisableVertexAttribArray(loc_a_position);

  }
  glUseProgram(0);
}

//2D render
void render_object()
{
    // 특정 쉐이더 프로그램 사용
    glUseProgram(program);

    const std::vector<tinygltf::Mesh>& meshes = model.meshes;
    const std::vector<tinygltf::Accessor>& accessors = model.accessors;
    const std::vector<tinygltf::BufferView>& bufferViews = model.bufferViews;
    
    for (size_t i = 0; i < meshes.size(); ++i)
    {
        const tinygltf::Mesh& mesh = meshes[i];

          for (size_t j = 0; j < mesh.primitives.size(); ++j)
          {
              const tinygltf::Primitive& primitive = mesh.primitives[j];

              int count = 0;

              for (std::map<std::string, int>::const_iterator it = primitive.attributes.cbegin();
                  it != primitive.attributes.cend();
                  ++it)
              {
                  const std::pair<std::string, int>& attrib = *it;

                  const int accessor_index = attrib.second;
                  const tinygltf::Accessor& accessor = accessors[accessor_index];

                  count = accessor.count;

                  const tinygltf::BufferView& bufferView = bufferViews[accessor.bufferView];

                  if (attrib.first.compare("POSITION") == 0)
                  {
                      glBindBuffer(bufferView.target, position_buffer);
                      glEnableVertexAttribArray(loc_a_position);
                      glVertexAttribPointer(loc_a_position,
                          accessor.type, accessor.componentType,
                          accessor.normalized ? GL_TRUE : GL_FALSE, 0,
                          BUFFER_OFFSET(accessor.byteOffset));
                  }
                  else if (attrib.first.compare("COLOR_0") == 0)
                  {
                      glBindBuffer(bufferView.target, color_buffer);
                      glEnableVertexAttribArray(loc_a_color);
                      glVertexAttribPointer(loc_a_color,
                          accessor.type, accessor.componentType,
                          accessor.normalized ? GL_TRUE : GL_FALSE, 0,
                          BUFFER_OFFSET(accessor.byteOffset));
                  }
              }
              glDrawArrays(primitive.mode, 0, count);

              // 정점 attribute 배열 비활성화
              glDisableVertexAttribArray(loc_a_position);
              glDisableVertexAttribArray(loc_a_color);
          } 
    }
    // 쉐이더 프로그램 사용해제
    glUseProgram(0);
}

void draw_scene()
{
  const std::vector<tinygltf::Node>& nodes = model.nodes;

  kmuvcl::math::mat4f mat_model;
  mat_model.set_to_identity();

  for (const tinygltf::Scene& scene : model.scenes)
  {
    for (size_t i = 0; i < scene.nodes.size(); ++i)
    {
      const tinygltf::Node& node = nodes[scene.nodes[i]];
      curr = std::chrono::system_clock::now();
      std::chrono::duration<float> elaped_seconds = (curr - prev);
      prev = curr;
      if (g_is_animation)
      {            
        g_angle += 30.0f * elaped_seconds.count();
        if (g_angle > 25200.0f)
        {
          g_angle = 0.0f;
        }
      }
      // mat_model = kmuvcl::math::rotate(g_angle*0.7f, 0.0f, 0.0f, 1.0f);
      // mat_model = kmuvcl::math::rotate(g_angle*0.5f, 1.0f, 0.0f, 0.0f)*mat_model;
      // mat_model = kmuvcl::math::translate(0.0f, 0.0f, -4.0f)*mat_model;
      mat_model = kmuvcl::math::rotate(g_angle*1.0f, 0.0f, 1.0f, 0.0f)*mat_model;
      draw_node(node, mat_model);
    }
  }
}


int main(int argc, char** argv)
{
  GLFWwindow* window;

  // Initialize GLFW library
  if (!glfwInit())
    return -1;

  // Create a GLFW window containing a OpenGL context
  window = glfwCreateWindow(500, 500, "Hello Texture with glTF 2.0", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    return -1;
  }

  // Make the current OpenGL context as one in the window
  glfwMakeContextCurrent(window);
  // std::cout << "a1" << std::endl;

  // Initialize GLEW library
  if (glewInit() != GLEW_OK)
    std::cout << "GLEW Init Error!" << std::endl;

  // Print out the OpenGL version supported by the graphics card in my PC
  std::cout << glGetString(GL_VERSION) << std::endl;
  std::string argument = argv[1];
  // for(int i=0; i < argc ; i++)
  // {
  //   argument.append(argv[i]);
  // }
  load_model(model, argument );
   const std::vector<tinygltf::Node>& nodes = model.nodes;

  for (const tinygltf::Scene& scene : model.scenes)
  {
    for (size_t i = 0; i < scene.nodes.size(); ++i)
    {
      const tinygltf::Node& node = nodes[scene.nodes[i]];
      if( (node.children.size() < 0 || node.children.size() == 0) && nodes.size() < 2 )
      {
        witch = false;
      }
      else
      {
        witch = true;
      }
    }
  }

  init_state();
  init_shader_program();

  // GPU의 VBO를 초기화하는 함수 호출
  
  init_buffer_objects();
  if(witch)
  {
    init_texture_objects();
  }

  // key_callback
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, frambuffer_size_callback);

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window))
  {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(witch)
    {
      set_transform();
      draw_scene();
    }
    else{
      render_object();
    }

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}
