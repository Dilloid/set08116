#version 440

// Spot light data
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};

// Material data
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Spot light being used in the scene
uniform spot_light spot;
// Material of the object being rendered
uniform material mat;
// Position of the eye (camera)
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  
  // Calculate direction to the light
  vec3 light_dir = normalize(spot.position - position);
  
  // Calculate distance to light
  float d = distance(spot.position, position);

  // Calculate spot light intensity
  float intensity = pow(max(dot(-light_dir, spot.direction), 0), spot.power);
  
  // Calculate attenuation value
  float a = (intensity / (spot.constant + (spot.linear * d) + (spot.quadratic * pow(d, 2))));

  // Calculate light colour
  vec4 light_colour = spot.light_colour * a;

  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);

  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient

  // Calculate diffuse component
  vec4 diffuse = max(dot(normal, light_dir), 0) * (mat.diffuse_reflection * light_colour);

  // Calculate half vector
  vec3 h = normalize(light_dir + view_dir);

  // Calculate specular component
  vec4 specular = pow(max(dot(normal, h), 0), mat.shininess) * (mat.specular_reflection * light_colour);
  
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord);

  // Calculate primary colour component
  vec4 primary = mat.emissive + diffuse;

  // Calculate final colour - remember alpha
  colour = primary * tex_colour + specular;
  colour.a = 1.0f;

  // *********************************
}