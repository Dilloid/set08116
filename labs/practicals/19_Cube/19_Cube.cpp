#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
effect eff;
target_camera cam;
float theta = 0.0f;
float rho = 0.0f;

vec3
v1 = vec3(-1, 1, 1),
v2 = vec3(-1, -1, 1),
v3 = vec3(1, -1, 1),
v4 = vec3(1, 1, 1),
v5 = vec3(1, 1, -1),
v6 = vec3(-1, 1, -1),
v7 = vec3(-1, -1, -1),
v8 = vec3(1, -1, -1);

bool load_content() {
  // Create cube data - twelve triangles triangles
  // Positions
  vector<vec3> positions{
      // *********************************
      // Add the position data for triangles here, (6 verts per side)
      // Front
      v3, v4, v1,
      v1, v2, v3,
      // Back
      v5, v8, v7,
      v7, v6, v5,
      // Right
      v5, v4, v3,
      v3, v8, v5,
      // Left
      v1, v6, v7,
      v7, v2, v1,
      // Top
      v1, v4, v5,
      v5, v6, v1,
      // Bottom
      v3, v2, v7,
      v7, v8, v3,
      // *********************************
  };
  // Colours
  vector<vec4> colours;
  for (auto i = 0; i < positions.size(); ++i) {
    colours.push_back(vec4(1.0, i % 2, 0.0f, 1.0f)); // Notice how I got those Rad colours?
  }
  // Add to the geometry
  geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
  geom.add_buffer(colours, BUFFER_INDEXES::COLOUR_BUFFER);

  // Load in shaders
  eff.add_shader("shaders/basic.vert", GL_VERTEX_SHADER);
  eff.add_shader("shaders/basic.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_position(vec3(10.0f, 10.0f, 10.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_UP)) {
    theta -= pi<float>() * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_DOWN)) {
    theta += pi<float>() * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_RIGHT)) {
    rho -= pi<float>() * delta_time;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT)) {
    rho += pi<float>() * delta_time;
  }
  // Update the camera
  cam.update(delta_time);
  return true;
}

bool render() {
  // Bind effect
  renderer::bind(eff);
  // Create MVP matrix
  mat4 M = eulerAngleXZ(theta, rho);
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto MVP = P * V * M;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // Render geometry
  renderer::render(geom);
  return true;
}

void main() {
  // Create application
  app application("19_Cube");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}