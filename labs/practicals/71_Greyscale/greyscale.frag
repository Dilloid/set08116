#version 430 core

// Incoming texture containing frame information
uniform sampler2D tex;

// Our colour filter - calculates colour intensity
const vec3 intensity = vec3(0.299, 0.587, 0.184);

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample texture colour
  colour = texture(tex, tex_coord);
  // Calculate grey value
  float grey = dot(intensity, colour.rgb);
  // Use greyscale to as final colour
  // - ensure alpha is 1
  colour = vec4(vec3(grey), 1.0);
  // *********************************
}