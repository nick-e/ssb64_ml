#ifndef gamepad_visualizer_H
#define gamepad_visualizer_H

#include <iostream>
#include <mutex>

#include <gtkmm-3.0/gtkmm.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/gamepad.h"
#include "../core/shader.h"
#include "../core/texture.h"
#include "../core/threadsafe_gamepad.h"
#include "../core/util.h"

namespace ssbml
{
  namespace gui
  {
    class gamepad_visualizer : public Gtk::Box, private gamepad
    {
    public:
      virtual void update(const compressed &c);

      gamepad_visualizer& operator<<(const compressed &c);

      gamepad_visualizer(uint64_t size);

    private:
      struct button
      {
        static GLuint vao;
        bool activated;
        float b;
        float g;
        float r;
        float radius;
        float x;
        float y;
        glm::mat4 model;

        void set_position(float x, float y);
        void set_color(float r, float g, float b);

        button(float radius, float x, float y, float r, float g, float b);
      };

      struct dpad
      {
        static GLuint solidVao;
        static GLuint hollowVao;
        static GLuint solidVbo;
        static GLuint hollowVbo;
        bool activated;
        float b;
        float g;
        float r;
        float rotation;
        float scale;
        float x;
        float y;
        glm::mat4 model;

        void set_position(float x, float y);
        void set_color(float r, float g, float b);

        dpad(float scale, float x, float y, float rotation, float r, float g, float b);
      };

      struct bumper
      {
        static GLuint solidVao;
        static GLuint hollowVao;
        static GLuint solidVbo;
        static GLuint hollowVbo;
        bool activated;
        bool flip;
        float b;
        float g;
        float r;
        float scale;
        float x;
        float y;
        glm::mat4 model;

        void set_position(float x, float y);
        void set_color(float r, float g, float b);

        bumper(float scale, float x, float y, bool flip, float r, float g, float b);
      };

      Gtk::GLArea area;
      shader *circleShader;
      shader *lineShader;
      GLint circleColorUniform;
      GLint circleModelUniform;
      GLint circleProjectionUniform;
      GLint circleRadiusUniform;
      GLint lineColorUniform;
      GLint lineModelUniform;
      GLint lineProjectionUniform;
      bool initialized;
      float r;
      float g;
      float b;
      uint64_t size;
      glm::mat4 projection;

      button *aButton;
      button *bButton;
      button *leftAnalog;
      button *leftAnalogBack;
      button *modeButton;
      button *rightAnalog;
      button *rightAnalogBack;
      button *selectButton;
      button *startButton;
      button *xButton;
      button *yButton;

      dpad *upDpad;
      dpad *leftDpad;
      dpad *rightDpad;
      dpad *downDpad;

      bumper *leftBumper;
      bumper *rightBumper;

      void init();
      void terminate();
      bool draw(const Glib::RefPtr<Gdk::GLContext>& context);
      void draw_button(button &c);
      void draw_dpad(dpad &d);
      void draw_bumper(bumper &b);
    };
  }
}

#endif
