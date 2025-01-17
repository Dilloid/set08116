#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect eff;
texture tex;
target_camera cam;
point_light light;

bool load_content() {
  // Create plane mesh
  meshes["plane"] = mesh(geometry_builder::create_plane());

  // Create scene
  meshes["box"] = mesh(geometry_builder::create_box());
  meshes["tetra"] = mesh(geometry_builder::create_tetrahedron());
  meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
  meshes["disk"] = mesh(geometry_builder::create_disk(20));
  meshes["cylinder"] = mesh(geometry_builder::create_cylinder(20, 20));
  meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
  meshes["torus"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));

  // Transform objects
  meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
  meshes["tetra"].get_transform().scale = vec3(4.0f, 4.0f, 4.0f);
  meshes["tetra"].get_transform().translate(vec3(-30.0f, 10.0f, -10.0f));
  meshes["pyramid"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["pyramid"].get_transform().translate(vec3(-10.0f, 7.5f, -30.0f));
  meshes["disk"].get_transform().scale = vec3(3.0f, 1.0f, 3.0f);
  meshes["disk"].get_transform().translate(vec3(-10.0f, 11.5f, -30.0f));
  meshes["disk"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  meshes["cylinder"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["cylinder"].get_transform().translate(vec3(-25.0f, 2.5f, -25.0f));
  meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
  meshes["sphere"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));

  // *********************************
  // Set materials
  // - all emissive is black
  // - all specular is white
  // - all shininess is 25
  vec4 black = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  float shininess = 25.0f;

  // Red box
  vec4 red = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  material mat_red = material(black, red, white, shininess);
  meshes["box"].set_material(mat_red);
  // Green tetra
  vec4 green = vec4(0.0f, 1.0f, 0.0f, 1.0f);
  material mat_green = material(black, green, white, shininess);
  meshes["tetra"].set_material(mat_green);
  // Blue pyramid
  vec4 blue = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  material mat_blue = material(black, blue, white, shininess);
  meshes["pyramid"].set_material(mat_blue);
  // Yellow disk
  vec4 yellow = vec4(1.0f, 1.0f, 0.0f, 1.0f);
  material mat_yellow = material(black, yellow, white, shininess);
  meshes["disk"].set_material(mat_yellow);
  // Magenta cylinder
  vec4 magenta = vec4(1.0f, 0.0f, 1.0f, 1.0f);
  material mat_magenta = material(black, magenta, white, shininess);
  meshes["cylinder"].set_material(mat_magenta);
  // Cyan sphere
  vec4 cyan = vec4(0.0f, 1.0f, 1.0f, 1.0f);
  material mat_cyan = material(black, cyan, white, shininess);
  meshes["sphere"].set_material(mat_cyan);
  // White torus
  material mat_white = material(black, white, white, shininess);
  meshes["torus"].set_material(mat_white);
  // *********************************

  // Load texture
  tex = texture("textures/checker.png");

  // *********************************
  // Set lighting values, Position (-25, 10, -10)
  light.set_position(vec3(-25, 10, -10));
  // Light colour white
  light.set_light_colour(white);
  // Set range to 20
  light.set_range(20);
  // Load in shaders
  eff.add_shader("49_Point_Light/point.vert", GL_VERTEX_SHADER);
  eff.add_shader("49_Point_Light/point.frag", GL_FRAGMENT_SHADER);
  // Build effect
  eff.build();
  // *********************************

  // Set camera properties
  cam.set_position(vec3(50.0f, 10.0f, 50.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool update(float delta_time) {
  // Range of the point light
  static float range = 20.0f;

  // *********************************

  // WSAD to move point light
  vec3 translation = vec3(0.0f, 0.0f, 0.0f);

  if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
      translation.z -= 1.0f;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
      translation.z += 1.0f;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
      translation.x -= 1.0f;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
      translation.x += 1.0f;
  }

  // O and P to change range
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_O)) {
      range -= 1.0f;
  }
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_P)) {
      range += 1.0f;
  }

  if (range < 0) range = 0;
  // *********************************

  // Move point light
  light.move(translation);

  // Set range
  light.set_range(range);

  // Rotate the sphere
  meshes["sphere"].get_transform().rotate(vec3(0.0f, half_pi<float>(), 0.0f) * delta_time);

  cam.update(delta_time);

  return true;
}

bool render() {
  // Render meshes
  for (auto &e : meshes) {
    auto m = e.second;
    // Bind effect
    renderer::bind(eff);
    // Create MVP matrix
    auto M = m.get_transform().get_transform_matrix();
    auto V = cam.get_view();
    auto P = cam.get_projection();
    auto MVP = P * V * M;
    // Set MVP matrix uniform
    glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
                       1,                               // Number of values - 1 mat4
                       GL_FALSE,                        // Transpose the matrix?
                       value_ptr(MVP));                 // Pointer to matrix data

    // *********************************
    // Set M matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
    // Set N matrix uniform - remember - 3x3 matrix
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(m.get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(light, "point");
	// Bind texture
	renderer::bind(tex, 0);
	// Set tex uniform
	glUniform1i(eff.get_uniform_location("tex"), 0);
	// Set eye position - Get this from active camera
	glUniform3fv(eff.get_uniform_location("eye_pos"), 1,
		value_ptr(cam.get_position()));
    // Render mesh
	renderer::render(m);
    // *********************************
  }

  return true;
}

void main() {
  // Create application
  app application("49_Point_Light");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}