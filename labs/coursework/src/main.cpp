#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

effect eff, sky_eff, terr_eff, shadow_eff, post_eff;
chase_camera cam;
spot_light spot;
directional_light light;
vector<point_light> points(3);
map<string, mesh> meshes;
map<string, vec4> colours;
map<string, texture> textures;
map<string, material> materials;

mesh skybox, terr;
cubemap cube_map;
texture terrain_tex;
shadow_map shadow;

frame_buffer frame;
geometry screen_quad;

void generate_terrain(geometry& geom, const texture& height_map,
	unsigned int width, unsigned int depth, float height_scale)
{
	// Contains our position data
	vector<vec3> positions;
	// Contains our normal data
	vector<vec3> normals;
	// Contains our texture coordinate data
	vector<vec2> tex_coords;
	// Contains our index data
	vector<unsigned int> indices;

	// Extract the texture data from the image
	glBindTexture(GL_TEXTURE_2D, height_map.get_id());
	auto data = new vec4[height_map.get_width() * height_map.get_height()];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void*)data);

	// Determine ratio of height map to geometry
	float width_point = static_cast<float>(width) / static_cast<float>(height_map.get_width());
	float depth_point = static_cast<float>(depth) / static_cast<float>(height_map.get_height());

	// Point to work on
	vec3 point;

	// Part 1 - Iterate through each point, calculate vertex and add to vector
	for (int x = 0; x < height_map.get_width(); ++x) {
		// Calculate x position of point
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (int z = 0; z < height_map.get_height(); ++z) {
			// *********************************
			// Calculate z position of point
			point.z = -(depth / 2.0f) + (depth_point * z);
			// *********************************
			// Y position based on red component of height map data
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			// Add point to position data
			positions.push_back(point);
		}
	}

	// Part 1 - Add index data
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x) {
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y) {
			// Get four corners of patch
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			// *********************************
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_height()) + x + 1;
			// *********************************
			// Push back indices for triangle 1 (tl,br,bl)
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			// Push back indices for triangle 2 (tl,tr,br)
			// *********************************
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
			// *********************************
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());

	// Part 2 - Calculate normals for the height map
	for (unsigned int i = 0; i < indices.size() / 3; ++i) {
		// Get indices for the triangle
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];

		// Calculate two sides of the triangle
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];

		// Normal is normal(cross product) of these two sides
		// *********************************
		vec3 n = normalize(side2 * side1);

		// Add to normals in the normal buffer using the indices for the triangle
		normals[idx1] += n;
		normals[idx2] += n;
		normals[idx3] += n;
		// *********************************
	}

	// Normalize all the normals
	for (auto& n : normals) {
		// *********************************
		n = normalize(n);
		// *********************************
	}

	// Part 3 - Add texture coordinates for geometry
	for (unsigned int x = 0; x < height_map.get_width(); ++x) {
		for (unsigned int z = 0; z < height_map.get_height(); ++z) {
			tex_coords.push_back(vec2(width_point * x, depth_point * z));
		}
	}

	// Add necessary buffers to the geometry
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_index_buffer(indices);

	// Delete data
	delete[] data;
}

bool load_content()
{
	renderer::setClearColour(0.0f, 0.0f, 0.0f);

	// Create screen quad
	vector<vec3> positions{
		vec3(-0.8f, -0.8f, 0.0f), vec3(0.8f, -0.8f, 0.0f),
		vec3(-0.8f, 0.8f, 0.0f), vec3(0.8f, 0.8f, 0.0f)
	};
	vector<vec2> tex_coords{
		vec2(0.0, 0.0), vec2(1.0f, 0.0f),
		vec2(0.0f, 1.0f), vec2(1.0f, 1.0f)
	};

	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);

	frame = frame_buffer(
		renderer::get_screen_width(),
		renderer::get_screen_height()
	);

	shadow = shadow_map(
		renderer::get_screen_width(),
		renderer::get_screen_height()
	);

	skybox = mesh(geometry_builder::create_box());
	skybox.get_transform().scale = vec3(100.0, 100.0, 100.0);

	array<string, 6> path{
		"res/textures/posx.jpg", "res/textures/negx.jpg",
		"res/textures/posy.jpg", "res/textures/negy.jpg",
		"res/textures/posz.jpg", "res/textures/negz.jpg"
	};

	array<string, 6> cave{
		"res/textures/cave3_ft.png", "res/textures/cave3_bk.png",
		"res/textures/cave3_up.png", "res/textures/cave3_dn.png",
		"res/textures/cave3_rt.png", "res/textures/cave3_lf.png"
	};

	array<string, 6> corona{
		"res/textures/corona_ft.png", "res/textures/corona_bk.png",
		"res/textures/corona_up.png", "res/textures/corona_dn.png",
		"res/textures/corona_rt.png", "res/textures/corona_lf.png"
	};

	array<string, 6> redeclipse{
		"res/textures/redeclipse_ft.png", "res/textures/redeclipse_bk.png",
		"res/textures/redeclipse_up.png", "res/textures/redeclipse_dn.png",
		"res/textures/redeclipse_rt.png", "res/textures/redeclipse_lf.png"
	};

	cube_map = cubemap(corona);

	// Define necessary colours
	colours["black"] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	colours["white"] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	colours["grey"] = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	colours["red"] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	colours["green"] = vec4(0.0f, 0.8f, 0.0f, 1.0f);
	colours["blue"] = vec4(0.0f, 0.0f, 1.0f, 0.5f);

	// Create or load each object and set properties
	textures["pyramid"] = texture("res/textures/ground.jpg");
	materials["pyramid"] = material(colours["black"], colours["white"], colours["white"], 100.0f);
	meshes["pyramid"] = mesh(geometry_builder::create_pyramid(vec3(5.0f, 5.0f, 5.0f)));
	meshes["pyramid"].get_transform().translate(vec3(0.0f, 2.5f, 0.0f));

	textures["sphere"] = texture("res/textures/marble.jpg");
	materials["sphere"] = material(colours["black"], colours["white"], colours["white"], 10.0f);
	meshes["sphere"] = mesh(geometry_builder::create_sphere(25, 25));
	meshes["sphere"].get_transform().scale = vec3(6.0f, 6.0f, 6.0f);
	meshes["sphere"].get_transform().translate(vec3(0.0f, 12.0f, 0.0f));

	textures["box"] = texture("res/textures/check_1.png");
	materials["box"] = material(colours["black"], colours["white"], colours["white"], 20.0f);
	meshes["box"] = mesh(geometry("res/models/box.obj"));
	meshes["teapot"].get_transform().scale = vec3(2.0f, 2.0f, 2.0f);
	meshes["box"].get_transform().translate(vec3(0.0f, 1.0f, 10.0f));

	textures["teapot"] = texture("res/textures/metal_smooth.jpg");
	materials["teapot"] = material(colours["black"], colours["white"], colours["white"], 30.0f);
	meshes["teapot"] = mesh(geometry("res/models/teapot_s2.obj"));
	meshes["teapot"].get_transform().scale = vec3(25.0f, 25.0f, 25.0f);
	meshes["teapot"].get_transform().translate(vec3(5.0f, 0.0f, 5.0f));
	meshes["teapot"].get_transform().rotate(vec3(1.0f, 0.0f, 0.0f) * 15.0f);

	textures["car"] = texture("res/textures/metal_tread.jpg");
	materials["car"] = material(colours["black"], colours["red"], colours["white"], 20.0f);
	meshes["car"] = mesh(geometry("res/models/car2.obj"));
	meshes["car"].get_transform().scale = vec3(0.05f, 0.05f, 0.05f);
	meshes["car"].get_transform().translate(vec3(-10.0f, 0.0f, -5.0f));
	meshes["car"].get_transform().rotate(vec3(0.0f, 0.0f, half_pi<float>() / 2.0f) * 50.0f);

	// Set the material for each mesh
	for (auto &e : meshes) {
		auto name = e.first;
		meshes[name].set_material(materials[name]);
	}

	geometry geom;
	texture height_map("res/textures/sinemap2.png");
	terrain_tex = texture("res/textures/grid3.png");
	generate_terrain(geom, height_map, 45.0f, 45.0f, 3.0f);
	terr = mesh(geom);
	terr.get_transform().position = vec3(0.0f, -5.0f, 0.0f);
	terr.set_material(material(colours["black"], colours["white"], colours["white"], 20.0f));

	// Set light properties
	light.set_ambient_intensity(vec4(0.1f, 0.1f, 0.1f, 0.5f));
	light.set_light_colour(colours["green"]);
	light.set_direction(vec3(0.0f, -1.0f, 0.0f));

	points[0].set_position(vec3(0.0f, -8.0f, 0.0f));
	points[0].set_light_colour(colours["white"]);
	points[0].set_range(0.0f);

	points[1].set_position(vec3(-11.0f, 0.0f, -8.0f));
	points[1].set_light_colour(colours["white"]);
	points[1].set_range(3.0f);

	points[2].set_position(vec3(-9.0f, 0.0f, -8.0f));
	points[2].set_light_colour(colours["white"]);
	points[2].set_range(3.0f);

	spot.set_position(vec3(25.0f, 50.0f, 0.0f));
	spot.set_light_colour(colours["blue"]);
	spot.set_direction(vec3(0.0f, -1.0f, 1.0f));
	spot.set_range(500.0f);
	spot.set_power(10.0f);

	// Load shaders
	eff.add_shader("res/shaders/shader.vert", GL_VERTEX_SHADER);
	eff.add_shader("res/shaders/shader.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("res/shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("res/shaders/part_point.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("res/shaders/part_spot.frag", GL_FRAGMENT_SHADER);
	eff.add_shader("res/shaders/part_shadow.frag", GL_FRAGMENT_SHADER);
	sky_eff.add_shader("res/shaders/skybox.vert", GL_VERTEX_SHADER);
	sky_eff.add_shader("res/shaders/skybox.frag", GL_FRAGMENT_SHADER);
	terr_eff.add_shader("res/shaders/terrain.vert", GL_VERTEX_SHADER);
	terr_eff.add_shader("res/shaders/terrain.frag", GL_FRAGMENT_SHADER);
	shadow_eff.add_shader("res/shaders/shader.vert", GL_VERTEX_SHADER);
	shadow_eff.add_shader("res/shaders/shader.frag", GL_FRAGMENT_SHADER);
	shadow_eff.add_shader("res/shaders/part_direction.frag", GL_FRAGMENT_SHADER);
	shadow_eff.add_shader("res/shaders/part_point.frag", GL_FRAGMENT_SHADER);
	shadow_eff.add_shader("res/shaders/part_spot.frag", GL_FRAGMENT_SHADER);
	shadow_eff.add_shader("res/shaders/part_shadow.frag", GL_FRAGMENT_SHADER);
	post_eff.add_shader("res/shaders/simple_texture.vert", GL_VERTEX_SHADER);
	post_eff.add_shader("res/shaders/simple_texture.frag", GL_FRAGMENT_SHADER);

	// Build effects
	eff.build();
	sky_eff.build();
	terr_eff.build();
	shadow_eff.build();
	post_eff.build();

	// Set camera properties
	cam.set_pos_offset(vec3(0.0f, 0.0f, 60.0f));
	cam.set_springiness(0.5f);
	cam.move(meshes["sphere"].get_transform().position, eulerAngles(meshes["sphere"].get_transform().orientation));
	auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
	cam.set_projection(quarter_pi<float>(), aspect, 0.1f, 1000.0f);

	return true;
}

bool update(float delta_time)
{
	// Rotate the sphere, move the camera to match
	meshes["sphere"].get_transform().rotate(vec3(0.0f, (half_pi<float>() / 2), 0.0f) * delta_time);
	cam.move(meshes["sphere"].get_transform().position, eulerAngles(meshes["sphere"].get_transform().orientation));
	cam.update(delta_time);

	skybox.get_transform().position = cam.get_position();

	// Update the shadow map light_position from the spot light
	shadow.light_position = spot.get_position();
	// do the same for light_dir property
	shadow.light_dir = spot.get_direction();

	return true;
}

void render_skybox()
{
	// Bind skybox effect
	renderer::bind(sky_eff);
	// Calculate MVP for the skybox
	auto M = skybox.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(sky_eff.get_uniform_location("MVP"),
		1, GL_FALSE, value_ptr(MVP));
	// Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(sky_eff.get_uniform_location("cubemap"), 0);
	// Render skybox
	renderer::render(skybox);
}

void render_terrain()
{
	// Bind terrain effect
	renderer::bind(terr_eff);
	// Calculate MVP
	auto M = terr.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(terr_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set normal matrix uniform
	glUniformMatrix3fv(terr_eff.get_uniform_location("N"), 1, GL_FALSE,
		value_ptr(terr.get_transform().get_normal_matrix()));
	// Bind shader properties
	renderer::bind(terr.get_material(), "mat");
	// Bind lights
	renderer::bind(light, "light");
	// Bind texture
	renderer::bind(terrain_tex, 0);
	// Set texture uniform
	glUniform1i(terr_eff.get_uniform_location("tex"), 0);
	// Set eye position uniform
	glUniform3fv(terr_eff.get_uniform_location("eye_pos"), 1,
		value_ptr(cam.get_position()));
	// Render terrain
	renderer::render(terr);
}

mat4 LightProjectionMat;
void render_shadows()
{
    // Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);

	// We could just use the Camera's projection, 
	// but that has a narrower FoV than the cone of the spot light, so we would get clipping.
	// so we have yo create a new Proj Mat with a field of view of 90.
	LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 1000.f);

	// Bind shader
	renderer::bind(shadow_eff);
	// Render meshes
	for (auto& e : meshes) {
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		// View matrix taken from shadow map
		auto V = shadow.get_view();
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(shadow_eff.get_uniform_location("MVP"),
			1, GL_FALSE, value_ptr(MVP));
		// Render mesh
		renderer::render(m);
	}
	// Set render target back to the frame
	renderer::set_render_target(frame);
	// Set face cull mode to back
	glCullFace(GL_BACK);
}

void render_meshes()
{
	// For each mesh
	for (auto& e : meshes)
	{
		auto m = e.second;

		// Bind main effect
		renderer::bind(eff);

		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;

		// Set MVP matrix uniform
		glUniformMatrix4fv(eff.get_uniform_location("MVP"),
			1, GL_FALSE, value_ptr(MVP));

		// Set normal matrix uniform
		glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
			value_ptr(m.get_transform().get_normal_matrix()));

		// Set lightMVP uniform, using:
		// View matrix from the shadow map
		auto lightV = shadow.get_view();
		// Multiply together with LightProjectionMat
		auto lightMVP = LightProjectionMat * lightV * M;
		// Set uniform
		glUniformMatrix4fv(eff.get_uniform_location("lightMVP"),
			1, GL_FALSE, value_ptr(lightMVP));

		// Bind shader properties
		renderer::bind(m.get_material(), "mat");
		// Bind lights
		renderer::bind(light, "light");
		renderer::bind(points, "points");
		renderer::bind(spot, "spot");
		// Bind textures
		renderer::bind(textures[e.first], 0);

		// Set texture uniform
		glUniform1i(eff.get_uniform_location("tex"), 0);

		// Set eye position uniform
		glUniform3fv(eff.get_uniform_location("eye_pos"),
			1, value_ptr(cam.get_position()));

		// Bind shadow map texture - use texture unit 1
		renderer::bind(shadow.buffer->get_depth(), 1);
		// Set the shadow_map uniform
		glUniform1i(eff.get_uniform_location("shadow_map"), 1);

		// Render mesh
		renderer::render(m);
	}
}

bool render()
{
	// Set render target to frame buffer
	renderer::set_render_target(frame);
	// Clear frame
	renderer::clear();

	// Disable depth test,depth mask,face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Render skybox and terrain
	render_skybox();
	render_terrain();
	// Enable depth test,depth mask,face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	// Render shadows and meshes
	render_shadows();
	render_meshes();

	// Set render target back to the screen
	renderer::set_render_target();
	// Bind Tex effect
	renderer::bind(post_eff);
	// MVP is now the identity matrix
	auto MVP = mat4(1.0);
	// Set MVP matrix uniform
	glUniformMatrix4fv(post_eff.get_uniform_location("MVP"),
		1, GL_FALSE, value_ptr(MVP));
	// Bind texture from frame buffer
	renderer::bind(frame.get_frame(), 0);
	// Set the tex uniform
	glUniform1i(post_eff.get_uniform_location("tex"), 0);
	// Render the screen quad
	renderer::render(screen_quad);

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