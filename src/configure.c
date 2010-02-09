/* MegaZeux
 *
 * Copyright (C) 2004 Gilead Kutnick <exophase@adelphia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// Config file options, which can be given either through config.txt
// or at the command line.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "configure.h"
#include "counter.h"
#include "event.h"
#include "rasm.h"
#include "fsafeopen.h"
#include "util.h"

#if defined(CONFIG_NDS)
#define VIDEO_OUTPUT_DEFAULT "nds"
#elif defined(CONFIG_GP2X)
#define VIDEO_OUTPUT_DEFAULT "gp2x"
#define AUDIO_BUFFER_SIZE 128
#elif defined(CONFIG_PSP)
#define FORCE_BPP_DEFAULT 8
#elif defined(CONFIG_WII)
#define AUDIO_SAMPLE_RATE 48000
#endif

#ifndef FORCE_BPP_DEFAULT
#define FORCE_BPP_DEFAULT 32
#endif

#ifndef VIDEO_OUTPUT_DEFAULT
#define VIDEO_OUTPUT_DEFAULT "software"
#endif

#ifndef AUDIO_BUFFER_SIZE
#define AUDIO_BUFFER_SIZE 4096
#endif

#ifndef AUDIO_SAMPLE_RATE
#define AUDIO_SAMPLE_RATE 44100
#endif

typedef struct
{
  char option_name[OPTION_NAME_LEN];
  config_function change_option;
} config_entry;

static void config_disassemble_extras(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->disassemble_extras = strtol(value, NULL, 10);
}

static void config_disassemble_base(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int new_base = strtol(value, NULL, 10);

  if((new_base == 10) || (new_base == 16))
    conf->disassemble_base = new_base;
}

static void config_set_audio_buffer(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->buffer_size = strtol(value, NULL, 10);
}

static void config_set_resolution(config_info *conf, char *name, char *value,
 char *extended_data)
{
  char *next;
  conf->resolution_width = strtol(value, &next, 10);
  conf->resolution_height = strtol(next + 1, NULL, 10);
}

static void config_set_fullscreen(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->fullscreen = strtol(value, NULL, 10);
}

static void config_set_music(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->music_on = strtol(value, NULL, 10);
}

static void config_set_mod_volume(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int new_volume = strtol(value, NULL, 10);

  if(new_volume < 1)
    new_volume = 1;

  if(new_volume > 8)
    new_volume = 8;

  conf->music_volume = new_volume;
}

static void config_set_mzx_speed(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int new_speed = strtol(value, NULL, 10);

  if(new_speed < 1)
    new_speed = 1;

  if(new_speed > 9)
    new_speed = 9;

  conf->mzx_speed = new_speed;
}

static void config_set_pc_speaker(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->pc_speaker_on = strtol(value, NULL, 10);
}

static void config_set_sam_volume(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int new_volume = strtol(value, NULL, 10);

  if(new_volume < 1)
    new_volume = 1;

  if(new_volume > 8)
    new_volume = 8;

  conf->sam_volume = new_volume;
}

static void config_save_file(config_info *conf, char *name, char *value,
 char *extended_data)
{
  strncpy(conf->default_save_name, value, 256);
}

static void config_startup_file(config_info *conf, char *name, char *value,
 char *extended_data)
{
  strncpy(conf->startup_file, value, 256);
}

static void config_enable_oversampling(config_info *conf, char *name,
 char *value, char *extended_data)
{
  conf->oversampling_on = strtol(value, NULL, 10);
}

static void config_resample_mode(config_info *conf, char *name, char *value,
 char *extended_data)
{
  if(!strcasecmp(value, "none"))
  {
    conf->resample_mode = 0;
  }
  else

  if(!strcasecmp(value, "linear"))
  {
    conf->resample_mode = 1;
  }
  else

  if(!strcasecmp(value, "cubic"))
  {
    conf->resample_mode = 2;
  }
}

static void config_mp_resample_mode(config_info *conf, char *name,
 char *value, char *extended_data)
{
  if(!strcasecmp(value, "none"))
  {
    conf->modplug_resample_mode = 0;
  }
  else

  if(!strcasecmp(value, "linear"))
  {
    conf->modplug_resample_mode = 1;
  }
  else

  if(!strcasecmp(value, "cubic"))
  {
    conf->modplug_resample_mode = 2;
  }
  else

  if(!strcasecmp(value, "fir"))
  {
    conf->modplug_resample_mode = 3;
  }
}

static void joy_axis_set(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int joy_num, joy_axis;
  int joy_key_min, joy_key_max;

  sscanf(name, "joy%daxis%d", &joy_num, &joy_axis);
  sscanf(value, "%d, %d", &joy_key_min, &joy_key_max);

  if(joy_num < 1)
    joy_num = 1;

  if(joy_num > 16)
    joy_num = 16;

  if(joy_axis < 1)
    joy_axis = 1;

  if(joy_axis > 16)
    joy_axis = 16;

  map_joystick_axis(joy_num - 1, joy_axis - 1, (keycode)joy_key_min,
   (keycode)joy_key_max);
}

static void joy_button_set(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int joy_num, joy_button;
  int joy_key;

  sscanf(name, "joy%dbutton%d", &joy_num, &joy_button);
  joy_key = (keycode)strtol(value, NULL, 10);

  if(joy_num < 1)
    joy_num = 1;

  if(joy_num > 16)
    joy_num = 16;

  map_joystick_button(joy_num - 1, joy_button - 1, (keycode)joy_key);
}

static void pause_on_unfocus(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int int_val = strtol(value, NULL, 10);

  set_refocus_pause(int_val);
}

static void include_config(config_info *conf, char *name, char *value,
 char *extended_data)
{
  // This one's for the original include N form
  set_config_from_file(conf, name + 7);
}

static void include2_config(config_info *conf, char *name, char *value,
 char *extended_data)
{
  // This one's for the include = N form
  set_config_from_file(conf, value);
}

static void config_set_sfx_volume(config_info *conf, char *name, char *value,
 char *extended_data)
{
  int new_volume = strtol(value, NULL, 10);

  if(new_volume < 1)
    new_volume = 1;

  if(new_volume > 8)
    new_volume = 8;

  conf->pc_speaker_volume = new_volume;
}

static void config_mask_midchars(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->mask_midchars = strtol(value, NULL, 10);
}

static void config_set_audio_freq(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->output_frequency = strtol(value, NULL, 10);
}

static void config_force_bpp(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->force_bpp = strtol(value, NULL, 10);
}

static void config_window_resolution(config_info *conf, char *name, char *value,
 char *extended_data)
{
  char *next;
  conf->window_width = strtol(value, &next, 10);
  conf->window_height = strtol(next + 1, NULL, 10);
}

static void config_set_video_output(config_info *conf, char *name, char *value,
 char *extended_data)
{
  strncpy(conf->video_output, value, 16);
}

static void config_enable_resizing(config_info *conf, char *name, char *value,
 char *extended_data)
{
  conf->allow_resize = strtol(value, NULL, 10);
}

static void config_set_gl_filter_method(config_info *conf, char *name,
 char *value, char *extended_data)
{
  strncpy(conf->gl_filter_method, value, 16);
}

static void config_gl_vsync(config_info *conf, char *name,
 char *value, char *extended_data)
{
  conf->gl_vsync = strtol(value, NULL, 10);
}

static void config_set_gl_tilemap_fragment_shader(config_info *conf, char *name,
 char *value, char *extended_data)
{
  strncpy(conf->gl_tilemap_fragment_shader, value, 42);
}

static void config_set_gl_tilemap_vertex_shader(config_info *conf, char *name,
 char *value, char *extended_data)
{
  strncpy(conf->gl_tilemap_vertex_shader, value, 42);
}

static void config_set_gl_scaling_fragment_shader(config_info *conf, char *name,
 char *value, char *extended_data)
{
  strncpy(conf->gl_scaling_fragment_shader, value, 42);
}

static void config_set_gl_scaling_vertex_shader(config_info *conf, char *name,
 char *value, char *extended_data)
{
  strncpy(conf->gl_scaling_vertex_shader, value, 42);
}

static void config_startup_editor(config_info *conf, char *name,
 char *value, char *extended_data)
{
  conf->startup_editor = strtol(value, NULL, 10);
}

/* FAT NOTE: This is searched as a binary tree, the nodes must be
 *           sorted alphabetically, or they risk being ignored.
 */
static const config_entry config_options[] =
{
  { "audio_buffer", config_set_audio_buffer },
  { "audio_sample_rate", config_set_audio_freq },
  { "disassemble_base", config_disassemble_base },
  { "disassemble_extras", config_disassemble_extras },
  { "enable_oversampling", config_enable_oversampling },
  { "enable_resizing", config_enable_resizing },
  { "force_bpp", config_force_bpp },
  { "force_resolution", config_set_resolution }, /* backwards compatibility */
  { "fullscreen", config_set_fullscreen },
  { "fullscreen_resolution", config_set_resolution },
  { "gl_filter_method", config_set_gl_filter_method },
  { "gl_scaling_fragment_shader", config_set_gl_scaling_fragment_shader },
  { "gl_scaling_vertex_shader", config_set_gl_scaling_vertex_shader },
  { "gl_tilemap_fragment_shader", config_set_gl_tilemap_fragment_shader },
  { "gl_tilemap_vertex_shader", config_set_gl_tilemap_vertex_shader },
  { "gl_vsync", config_gl_vsync },
  { "include", include2_config },
  { "include*", include_config },
  { "joy!axis!", joy_axis_set },
  { "joy!button!", joy_button_set },
  { "mask_midchars", config_mask_midchars },
  { "modplug_resample_mode", config_mp_resample_mode },
  { "music_on", config_set_music },
  { "music_volume", config_set_mod_volume },
  { "mzx_speed", config_set_mzx_speed },
  { "pause_on_unfocus", pause_on_unfocus },
  { "pc_speaker_on", config_set_pc_speaker },
  { "pc_speaker_volume", config_set_sfx_volume },
  { "resample_mode", config_resample_mode },
  { "sample_volume", config_set_sam_volume },
  { "save_file", config_save_file },
  { "startup_editor", config_startup_editor },
  { "startup_file", config_startup_file },
  { "video_output", config_set_video_output },
  { "window_resolution", config_window_resolution }
};

static const int num_config_options =
 sizeof(config_options) / sizeof(config_entry);

static const config_entry *find_option(char *name,
 const config_entry options[], int num_options)
{
  int cmpval, top = num_options - 1, middle, bottom = 0;
  const config_entry *base = options;

  while(bottom <= top)
  {
    middle = (top + bottom) / 2;
    cmpval = match_function_counter(name, (base[middle]).option_name);

    if(cmpval > 0)
      bottom = middle + 1;
    else

    if(cmpval < 0)
      top = middle - 1;
    else
      return base + middle;
  }

  return NULL;
}

static config_info default_options =
{
  // Video options
  0,                            // fullscreen
  640,                          // resolution_width
  480,                          // resolution_height
  640,                          // window_width
  350,                          // window_height
  0,                            // allow_resize
  VIDEO_OUTPUT_DEFAULT,         // video_output
  FORCE_BPP_DEFAULT,            // force_bpp
  "linear",                     // opengl filter method
  "shaders/tilemap.vert",       // opengl tilemap vertex shader
  "shaders/tilemap.frag",       // opengl tilemap fragment shader
  "shaders/simple.vert",        // opengl scaling vertex shader
  "shaders/hqscale.frag",       // opengl scaling fragment shader
  0,                            // opengl vsync mode

  // Audio options
  AUDIO_SAMPLE_RATE,            // output_frequency
  AUDIO_BUFFER_SIZE,            // buffer_size
  0,                            // oversampling_on
  1,                            // resample_mode
  2,                            // modplug_resample_mode
  8,                            // music_volume
  8,                            // sam_volume
  8,                            // pc_speaker_volume
  1,                            // music_on
  1,                            // pc_speaker_on

  // Game options
  "caverns.mzx",                // startup_file
  "saved.sav",                  // default_save_name
  4,                            // mzx_speed
  1,                            // disassemble_extras
  10,                           // disassemble_base
  0,                            // startup_editor

  1,                            // mask_midchars
};

static void config_change_option(void *conf, char *name, char *value,
 char *extended_data)
{
  const config_entry *current_option = find_option(name,
   config_options, num_config_options);

  if(current_option)
    current_option->change_option(conf, name, value, extended_data);
}

__editor_maybe_static void __set_config_from_file(
 find_change_option find_change_handler, void *conf, const char *conf_file_name)
{
  char current_char, *input_position, *output_position, *use_extended_buffer;
  int line_size, extended_size, extended_allocate_size = 512;
  char line_buffer_alternate[256], line_buffer[256];
  int extended_buffer_offset, peek_char;
  char *extended_buffer;
  char *equals_position, *value;
  FILE *conf_file;

  conf_file = fopen(conf_file_name, "rb");
  if(!conf_file)
    return;

  extended_buffer = malloc(extended_allocate_size);

  while(fsafegets(line_buffer_alternate, 255, conf_file))
  {
    if(line_buffer_alternate[0] != '#')
    {
      input_position = line_buffer_alternate;
      output_position = line_buffer;
      equals_position = NULL;

      do
      {
        current_char = *input_position;

        if(!isspace(current_char))
        {
          if((current_char == '\\') &&
            (input_position[1] == 's'))
          {
            input_position++;
            current_char = ' ';
          }

          if((current_char == '=') && (equals_position == NULL))
            equals_position = output_position;

          *output_position = current_char;
          output_position++;
        }
        input_position++;
      } while(current_char);

      if(equals_position)
      {
        *equals_position = 0;
        value = equals_position + 1;
      }
      else
      {
        value = (char *)"1";
      }

      if(line_buffer[0])
      {
        // There might be extended information too - get it.
        peek_char = fgetc(conf_file);
        extended_size = 0;
        extended_buffer_offset = 0;
        use_extended_buffer = NULL;

        while((peek_char == ' ') || (peek_char == '\t'))
        {
          // Extended data line
          use_extended_buffer = extended_buffer;
          if(fsafegets(line_buffer_alternate, 254, conf_file))
          {
            line_size = strlen(line_buffer_alternate);
            line_buffer_alternate[line_size] = '\n';
            line_size++;

            extended_size += line_size;
            if(extended_size >= extended_allocate_size)
            {
              extended_allocate_size *= 2;
              extended_buffer = realloc(extended_buffer,
                extended_allocate_size);
            }

            strcpy(extended_buffer + extended_buffer_offset,
              line_buffer_alternate);
            extended_buffer_offset += line_size;
          }

          peek_char = fgetc(conf_file);
        }
        ungetc(peek_char, conf_file);

        find_change_handler(conf, line_buffer, value, use_extended_buffer);
      }
    }
  }

  free(extended_buffer);
  fclose(conf_file);
}

__editor_maybe_static void __set_config_from_command_line(
 find_change_option find_change_handler, void *conf, int argc, char *argv[])
{
  char current_char, *input_position, *output_position;
  char *equals_position, line_buffer[256], *value;
  int i;

  for(i = 1; i < argc; i++)
  {
    input_position = argv[i];
    output_position = line_buffer;
    equals_position = NULL;

    do
    {
      current_char = *input_position;

      if((current_char == '\\') &&
       (input_position[1] == 's'))
      {
        input_position++;
        current_char = ' ';
      }

      if((current_char == '=') && (equals_position == NULL))
        equals_position = output_position;

      *output_position = current_char;
      output_position++;
      input_position++;
    } while(current_char);

    if(equals_position)
    {
      *equals_position = 0;
      value = equals_position + 1;
    }
    else
    {
      value = (char *)"1";
    }

    if(line_buffer[0])
      find_change_handler(conf, line_buffer, value, NULL);
  }
}

void set_config_from_file(config_info *conf, const char *conf_file_name)
{
  __set_config_from_file(config_change_option, conf, conf_file_name);
}

void default_config(config_info *conf)
{
  memcpy(conf, &default_options, sizeof(config_info));
}

void set_config_from_command_line(config_info *conf, int argc, char *argv[])
{
  __set_config_from_command_line(config_change_option, conf, argc, argv);
}
