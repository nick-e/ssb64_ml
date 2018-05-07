#include "gamepad_visualizer.h"

GLuint ssbml::gui::gamepad_visualizer::button::vao = 0;
GLuint ssbml::gui::gamepad_visualizer::dpad::solidVao = 0;
GLuint ssbml::gui::gamepad_visualizer::dpad::solidVbo = 0;
GLuint ssbml::gui::gamepad_visualizer::dpad::hollowVao = 0;
GLuint ssbml::gui::gamepad_visualizer::dpad::hollowVbo = 0;
GLuint ssbml::gui::gamepad_visualizer::bumper::solidVao = 0;
GLuint ssbml::gui::gamepad_visualizer::bumper::solidVbo = 0;
GLuint ssbml::gui::gamepad_visualizer::bumper::hollowVao = 0;
GLuint ssbml::gui::gamepad_visualizer::bumper::hollowVbo = 0;


ssbml::gui::gamepad_visualizer::gamepad_visualizer(uint64_t size) :
  initialized(false),
  r(0.90980392156),
  g(0.90980392156),
  b(0.90588235294),
  size(size),
  projection(glm::ortho(-50.0f, 50.0f, -25.0f, 25.0f, -1.0f, 1.0f))
{
  set_halign(Gtk::ALIGN_START);
  set_valign(Gtk::ALIGN_START);
  set_size_request(size, size / 2);

  area.set_hexpand(true);
  area.set_vexpand(true);
  add(area);

  area.signal_realize().connect(sigc::mem_fun(*this, &gamepad_visualizer::init));
  area.signal_render().connect(sigc::mem_fun(*this, &gamepad_visualizer::draw));
}

void ssbml::gui::gamepad_visualizer::init()
{
  area.make_current();
  area.throw_if_error();

  GLenum ret = glewInit();
  if (ret != GLEW_OK)
  {
    std::cerr << "glewInit(): " << glewGetErrorString(ret) << std::endl;
  }

  glEnable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glViewport(0, 0, size, size / 2);

  circleShader = new shader("../src/gui/shaders/circle.vert",
    "../src/gui/shaders/circle.frag");
  circleShader->use();
  circleColorUniform = circleShader->get_uniform_location("color");
  circleModelUniform = circleShader->get_uniform_location("model");
  circleProjectionUniform = circleShader->get_uniform_location("projection");
  circleRadiusUniform = circleShader->get_uniform_location("radius");
  shader::set_uniform(circleProjectionUniform, projection);

  lineShader = new shader("../src/gui/shaders/line.vert",
    "../src/gui/shaders/line.frag");
  lineShader->use();
  lineColorUniform = lineShader->get_uniform_location("color");
  lineModelUniform = lineShader->get_uniform_location("model");
  lineProjectionUniform = lineShader->get_uniform_location("projection");
  shader::set_uniform(lineProjectionUniform, projection);

  // 3.5, 1.25
  float analogBackSize = 4;
  float analogSize = 3.5;
  float smallButtonSize = 1.25;
  float buttonSize = 2.0;
  float largeButtonSize = 3.0;
  float scale = 3.0;

  float r = 0.0;
  float g = 0.0;
  float b = 0.0;

  leftAnalogBack = new button(analogBackSize * scale, -7.75 * scale, 0.0, 0.5,
    0.5, 0.5);
  leftAnalogBack->activated = true;
  rightAnalogBack = new button(analogBackSize * scale, 4 * scale, -4.5 * scale,
    0.5, 0.5, 0.5);
  rightAnalogBack->activated = true;
  leftAnalog = new button(analogSize * scale, -7.75 * scale, 0.0, r, g, b);
  rightAnalog = new button(analogSize * scale, 4 * scale, -4.5 * scale, r, g,
    b);
  selectButton = new button(smallButtonSize * scale, 2.25 * -scale, 0.0, r, g,
    b);
  startButton = new button(smallButtonSize * scale, 2.25 * scale, 0.0, r, g, b);
  modeButton = new button(largeButtonSize * scale, 0.0, 3.25 * scale, r, g, b);
  xButton = new button(buttonSize * scale, 5.75 * scale , 0.0, r, g, b);
  bButton = new button(buttonSize * scale, 9.75 * scale, 0.0, r, g, b);
  aButton = new button(buttonSize * scale, 7.75 * scale, -2.0 * scale, r, g, b);
  yButton = new button(buttonSize * scale, 7.75 * scale, 2.0 * scale, r, g, b);
  upDpad = new dpad(scale, -4.0 * scale, -4.5 * scale, 0, r, g, b);
  leftDpad = new dpad(scale, -4.0 * scale, -4.5 * scale, 90, r, g, b);
  rightDpad = new dpad(scale, -4.0 * scale, -4.5 * scale, 270, r, g, b);
  downDpad = new dpad(scale, -4.0 * scale, -4.5 * scale, 180, r, g, b);
  leftBumper = new bumper(scale, -4.0 * scale, 5.0 * scale, true, r, g, b);
  rightBumper = new bumper(scale, 4.0 * scale, 5.0 * scale, false, r, g, b);

  glGenVertexArrays(1, &button::vao);

  GLfloat dpadSolidVertices[] =
  {
    0.0, 0.0,
    -0.75, 0.75,
    0.75, 0.75,
    -0.75, 2.25,
    0.75, 2.25
  };
  glGenVertexArrays(1, &dpad::solidVao);
  glGenBuffers(1, &dpad::solidVbo);
  glBindVertexArray(dpad::solidVao);
  glBindBuffer(GL_ARRAY_BUFFER, dpad::solidVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(dpadSolidVertices), dpadSolidVertices,
    GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  GLfloat dpadHollowVertices[] =
  {
    -0.75, 0.75,
    -0.75, 2.25,
    0.75, 2.25,
    0.75, 0.75
  };
  glGenVertexArrays(1, &dpad::hollowVao);
  glGenBuffers(1, &dpad::hollowVbo);
  glBindVertexArray(dpad::hollowVao);
  glBindBuffer(GL_ARRAY_BUFFER, dpad::hollowVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(dpadHollowVertices), dpadHollowVertices,
    GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  GLfloat bumperSolidVertices[] =
  {
    0.0, 0.0,
    0.0, 0.5,
    0.5, 0.0,
    1.5, 1.3,
    1.5, 0.3,
    6.5, 0.2,
    6.75, -0.8
  };
  glGenVertexArrays(1, &bumper::solidVao);
  glGenBuffers(1, &bumper::solidVbo);
  glBindVertexArray(bumper::solidVao);
  glBindBuffer(GL_ARRAY_BUFFER, bumper::solidVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bumperSolidVertices), bumperSolidVertices,
    GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  GLfloat bumperHollowVertices[] =
  {
    0.0, 0.0,
    0.0, 0.5,
    1.5, 1.3,
    6.5, 0.2,
    6.75, -0.8,
    1.5, 0.3,
    0.5, 0.0
  };
  glGenVertexArrays(1, &bumper::hollowVao);
  glGenBuffers(1, &bumper::hollowVbo);
  glBindVertexArray(bumper::hollowVao);
  glBindBuffer(GL_ARRAY_BUFFER, bumper::hollowVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(bumperHollowVertices),
    bumperHollowVertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  initialized = true;
}

ssbml::gui::gamepad_visualizer::button::button(float radius,float x, float y,
   float r, float g, float b) :
  activated(false),
  radius(radius)
{
  set_color(r, g, b);
  set_position(x, y);
}

void ssbml::gui::gamepad_visualizer::button::set_position(float x, float y)
{
  this->x = x;
  this->y = y;
  model = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
  model = glm::translate(model, glm::vec3(x, y, 0.0f));
  model = glm::scale(model, glm::vec3(radius * 2, radius * 2, 1.0f));
}

void ssbml::gui::gamepad_visualizer::button::set_color(float r, float g, float b)
{
  this->r = r;
  this->g = g;
  this->b = b;
}

void ssbml::gui::gamepad_visualizer::dpad::set_position(float x, float y)
{
  this->x = x;
  this->y = y;
  model = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
  model = glm::translate(model, glm::vec3(x, y, 0.0f));
  model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0, 0.0, 1.0));
  model = glm::scale(model, glm::vec3(scale, scale, 1.0f));
}

void ssbml::gui::gamepad_visualizer::dpad::set_color(float r, float g, float b)
{
  this->r = r;
  this->g = g;
  this->b = b;
}

ssbml::gui::gamepad_visualizer::dpad::dpad(float scale, float x, float y,
  float rotation, float r, float g, float b) :
  activated(false),
  rotation(rotation),
  scale(scale)
{
  set_color(r, g, b);
  set_position(x, y);
}

void ssbml::gui::gamepad_visualizer::bumper::set_position(float x, float y)
{
  this->x = x;
  this->y = y;
  model = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
  model = glm::translate(model, glm::vec3(x, y, 0.0f));
  model = glm::scale(model, glm::vec3(flip ? -scale : scale, scale, 1.0f));
}

void ssbml::gui::gamepad_visualizer::bumper::set_color(float r, float g, float b)
{
  this->r = r;
  this->g = g;
  this->b = b;
}

ssbml::gui::gamepad_visualizer::bumper::bumper(float scale, float x, float y,
  bool flip, float r, float g, float b) :
  activated(false),
  flip(flip),
  scale(scale)
{
  set_color(r, g, b);
  set_position(x, y);
}

void ssbml::gui::gamepad_visualizer::terminate()
{
  area.make_current();
  area.throw_if_error();

  delete circleShader;
  delete lineShader;

  delete leftAnalogBack;
  delete rightAnalogBack;
  delete leftAnalog;
  delete rightAnalog;
  delete selectButton;
  delete startButton;
  delete modeButton;
  delete xButton;
  delete aButton;
  delete bButton;
  delete yButton;

  delete upDpad;
  delete leftDpad;
  delete rightDpad;
  delete downDpad;

  delete leftBumper;
  delete rightBumper;

  glDeleteVertexArrays(1, &button::vao);

  glDeleteVertexArrays(1, &dpad::solidVao);
  glDeleteVertexArrays(1, &dpad::hollowVao);
  glDeleteBuffers(1, &dpad::solidVbo);
  glDeleteBuffers(1, &dpad::hollowVbo);

  glDeleteVertexArrays(1, &bumper::solidVao);
  glDeleteVertexArrays(1, &bumper::hollowVao);
  glDeleteBuffers(1, &bumper::solidVbo);
  glDeleteBuffers(1, &bumper::hollowVbo);

  initialized = false;
}

bool ssbml::gui::gamepad_visualizer::draw(
  const Glib::RefPtr<Gdk::GLContext>& context)
{
  glClearColor (r, g, b, 1.0);
  glClear (GL_COLOR_BUFFER_BIT);

  circleShader->use();
  glBindVertexArray(button::vao);
  draw_button(*leftAnalogBack);
  draw_button(*leftAnalog);
  draw_button(*rightAnalogBack);
  draw_button(*rightAnalog);
  draw_button(*selectButton);
  draw_button(*startButton);
  draw_button(*modeButton);
  draw_button(*aButton);
  draw_button(*bButton);
  draw_button(*xButton);
  draw_button(*yButton);

  lineShader->use();
  draw_dpad(*upDpad);
  draw_dpad(*leftDpad);
  draw_dpad(*rightDpad);
  draw_dpad(*downDpad);

  lineShader->use();
  draw_bumper(*leftBumper);
  draw_bumper(*rightBumper);

  return false;
}

void ssbml::gui::gamepad_visualizer::update(const compressed &c)
{
  buttons.a = c.buttons & 0x1;
  buttons.b = (c.buttons >> 1) & 0x1;
  buttons.x = (c.buttons >> 2) & 0x1;
  buttons.y = (c.buttons >> 3) & 0x1;
  buttons.tr = (c.buttons >> 4) & 0x1;
  buttons.tl = (c.buttons >> 5) & 0x1;
  buttons.thumbr = (c.buttons >> 6) & 0x1;
  buttons.thumbl = (c.buttons >> 7) & 0x1;
  buttons.select = (c.buttons >> 8) & 0x1;
  buttons.start = (c.buttons >> 9) & 0x1;
  buttons.mode = (c.buttons >> 10) & 0x1;
  analogs = c.analogs;

  if (!initialized)
  {
    return;
  }

  float scale = 3.0;
  float multiplier = (float)3 / 32768;
  this->leftAnalog->set_position(-7.75 * scale + analogs.x * multiplier,
    analogs.y * -multiplier);
  this->leftAnalog->activated = buttons.thumbl;

  this->rightAnalog->set_position(4 * scale + analogs.rx * multiplier,
    -4.5 * scale + analogs.ry * -multiplier);
  this->rightAnalog->activated = buttons.thumbr;

  selectButton->activated = buttons.select;
  startButton->activated = buttons.start;
  modeButton->activated = buttons.mode;
  aButton->activated = buttons.a;
  bButton->activated = buttons.b;
  xButton->activated = buttons.x;
  yButton->activated = buttons.y;

  upDpad->activated = analogs.hat0y == -1;
  downDpad->activated = analogs.hat0y == 1;
  leftDpad->activated = analogs.hat0x == -1;
  rightDpad->activated = analogs.hat0x == 1;

  leftBumper->activated = buttons.tl;
  rightBumper->activated = buttons.tr;

  area.queue_render();
}

ssbml::gui::gamepad_visualizer& ssbml::gui::gamepad_visualizer::operator<<(
  const compressed &c)
{
  update(c);
  return *this;
}

void ssbml::gui::gamepad_visualizer::draw_button(button &c)
{
  shader::set_uniform(circleModelUniform, c.model);
  shader::set_uniform(circleColorUniform, c.r, c.g, c.b);
  glPointSize(c.radius * 2);
  glDrawArrays(GL_POINTS, 0, 1);

  if (!c.activated)
  {
    shader::set_uniform(circleColorUniform, r, g, b);
    glPointSize(c.radius * 2 - 2);
    glDrawArrays(GL_POINTS, 0, 1);
  }
}

void ssbml::gui::gamepad_visualizer::draw_dpad(dpad &d)
{
  glBindVertexArray(dpad::hollowVao);
  shader::set_uniform(lineModelUniform, d.model);
  shader::set_uniform(lineColorUniform, d.r, d.g, d.b);
  glLineWidth(1);
  glDrawArrays(GL_LINE_STRIP, 0, 4);

  if (d.activated)
  {
    glBindVertexArray(dpad::solidVao);
    shader::set_uniform(lineModelUniform, d.model);
    shader::set_uniform(lineColorUniform, d.r, d.g, d.b);
    glLineWidth(1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
  }
}

void ssbml::gui::gamepad_visualizer::draw_bumper(bumper &b)
{
  glBindVertexArray(bumper::hollowVao);
  shader::set_uniform(lineModelUniform, b.model);
  shader::set_uniform(lineColorUniform, b.r, b.g, b.b);
  glLineWidth(1);
  glDrawArrays(GL_LINE_LOOP, 0, 7);

  if (b.activated)
  {
    glBindVertexArray(bumper::solidVao);
    shader::set_uniform(lineModelUniform, b.model);
    shader::set_uniform(lineColorUniform, b.r, b.g, b.b);
    glLineWidth(1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 7);
  }
}
