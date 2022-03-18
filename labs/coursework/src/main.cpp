#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

effect eff;
chase_camera cam;
directional_light light_d;
map<string, mesh> meshes;
map<string, vec4> colours;
map<string, texture> textures;
map<string, material> materials;

bool load_content()
{
	// Define necessary colours
	colours["black"] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	colours["white"] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	colours["grey"] = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	colours["red"] = vec4(1.0f, 0.0f, 0.0f, 1.0f);

	// Create or load each object and set properties
	textures["plane"] = texture("res/textures/ground.jpg");
	materials["plane"] = material(colours["black"], colours["white"], colours["white"], 100.0f);
	meshes["plane"] = mesh(geometry_builder::create_plane());

	textures["pyramid"] = texture("res/textures/ground.jpg");
	materials["pyramid"] = material(colours["black"], colours["white"], colours["white"], 100.0f);
	meshes["pyramid"] = mesh(geometry_builder::create_pyramid(vec3(5.0f, 5.0f, 5.0f)));
	meshes["pyramid"].get_transform().translate(vec3(0.0f, 2.5f, 0.0f));

	textures["sphere"] = texture("res/textures/marble.jpg");
	materials["sphere"] = material(colours["black"], colours["white"], colours["white"], 10.0f);
	meshes["sphere"] = mesh(geometry_builder::create_sphere());
	meshes["sphere"].get_transform().scale = vec3(2.0f, 2.0f, 2.0f);
	meshes["sphere"].get_transform().translate(vec3(0.0f, 8.0f, 0.0f));

	textures["box"] = texture("res/textures/check_1.png");
	materials["box"] = material(colours["black"], colours["white"], colours["white"], 20.0f);
	meshes["box"] = mesh(geometry("res/models/box.obj"));
	meshes["teapot"].get_transform().scale = vec3(2.0f, 2.0f, 2.0f);
	meshes["box"].get_transform().translate(vec3(0.0f, 1.0f, 10.0f));

	textures["teapot"] = texture("res/textures/metal_smooth.jpg");
	materials["teapot"] = material(colours["black"], colours["white"], colours["white"], 20.0f);
	meshes["teapot"] = mesh(geometry("res/models/teapot_s2.obj"));
	meshes["teapot"].get_transform().scale = vec3(20.0f, 20.0f, 20.0f);
	meshes["teapot"].get_transform().translate(vec3(5.0f, 0.0f, 5.0f));

	textures["car"] = texture("res/textures/metal_tread.jpg");
	materials["car"] = material(colours["black"], colours["red"], colours["white"], 20.0f);
	meshes["car"] = mesh(geometry("res/models/car.obj"));
	meshes["car"].get_transform().scale = vec3(0.05f, 0.05f, 0.05f);
	meshes["car"].get_transform().translate(vec3(-5.0f, 0.0f, -5.0f));

	// Set the material for each mesh
	for (auto &e : meshes) {
		auto name = e.first;
		meshes[name].set_material(materials[name]);
	}

	// Set light properties
	light_d.set_ambient_intensity(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	light_d.set_light_colour(colours["white"]);
	light_d.set_direction(vec3(0.0f, 1.0f, -1.0f));

	// Load shaders
	eff.add_shader("res/shaders/phong.vert", GL_VERTEX_SHADER);
	eff.add_shader("res/shaders/phong.frag", GL_FRAGMENT_SHADER);
	
	// Build effects
	eff.build();
	
	// Set camera properties
	cam.set_pos_offset(vec3(0.0f, 5.0f, 30.0f));
	cam.set_springiness(0.5f);
	cam.move(meshes["sphere"].get_transform().position, eulerAngles(meshes["sphere"].get_transform().orientation));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;
}

bool update(float delta_time)
{
	// Rotate the sphere, move the camera to match
	meshes["sphere"].get_transform().rotate(vec3(0.0f, (half_pi<float>() / 2), 0.0f) * delta_time);
	cam.move(meshes["sphere"].get_transform().position, eulerAngles(meshes["sphere"].get_transform().orientation));

	// Update the camera
	cam.update(delta_time);
	return true;
}

bool render()
{
	// Bind effect
	renderer::bind(eff);
	
	// For each mesh
	for (auto &e : meshes)
	{
		auto m = e.second;

		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;

		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

		// Set normal matrix uniform
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(m.get_transform().get_normal_matrix()));
		
		// Bind shader properties
		renderer::bind(m.get_material(), "mat");
		renderer::bind(light_d, "light");
		renderer::bind(textures[e.first], 0);

		// Set texture uniform
		glUniform1i(eff.get_uniform_location("tex"), 0);

		// Set eye position uniform
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1,
			value_ptr(cam.get_position()));

		// Render mesh
		renderer::render(m);
	}
return true;
}

void main()
{
	// Create application
	app application("Graphics Coursework");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}