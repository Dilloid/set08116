#version 440

// Transformation matrix
uniform mat4 MVP;
// Model transformation matrix
uniform mat4 M;
// Normal matrix
uniform mat3 N;
// The light transformation matrix
uniform mat4 lightMVP;

// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 2) in vec3 normal;
// Incoming texture coordinate
layout (location = 10) in vec2 tex_coord_in;

// Outgoing position
layout (location = 0) out vec3 vertex_position;
// Outgoing transformed normal
layout (location = 1) out vec3 transformed_normal;
// Outgoing texture coordinate
layout (location = 2) out vec2 tex_coord_out;
// Outgoing position in light space
layout (location = 3) out vec4 vertex_light;

void main()
{
  // Calculate screen position
  gl_Position = MVP * vec4(position, 1.0);
  // Output other values to fragment shader
  vertex_position = (M * vec4(position, 1.0)).xyz;
  transformed_normal = N * normal;
  tex_coord_out = tex_coord_in;
  // Transform position into light space
  vertex_light = lightMVP * vec4(position, 1.0);
}