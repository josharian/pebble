#include <pebble.h>
#include <pebble_fonts.h>

/*
from __future__ import division
import math

def radians(theta):
  return math.pi * theta / 180

def minutes(radius, trigf):
  return [int(round(radius * trigf(radians((90 - theta))))) for theta in xrange(0, 360, 6)]

def declare(name, radius, trigf):
  return "static int16_t {0}[60] = {{{1}}};".format(name, ", ".join(str(n) for n in minutes(radius, trigf)))

def printn(n):
  print declare("x{0}".format(n), n, math.cos)
  print declare("y{0}".format(n), n, math.sin)

for n in (12, 15, 20, 24, 30, 36, 40, 45, 48, 60):
  printn(n)

*/

static Window *window;
static Layer *layer;
static GFont font;

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168
// Font details determined experimentally;
// getting them right is important to getting each
// character to render in the right place, even
// with the request to center the drawn text.
#define LETTER_WIDTH 12
#define LINE_HEIGHT 34

static const char * hours[12] = {
  "TWELVE", "ONE", "TWO", "THREE",
  "FOUR", "FIVE", "SIX", "SEVEN",
  "EIGHT", "NINE", "TEN", "ELEVEN"
};

static int16_t x12[60] = {0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 10, 11, 11, 12, 12, 12, 12, 12, 11, 11, 10, 10, 9, 8, 7, 6, 5, 4, 2, 1, 0, -1, -2, -4, -5, -6, -7, -8, -9, -10, -10, -11, -11, -12, -12, -12, -12, -12, -11, -11, -10, -10, -9, -8, -7, -6, -5, -4, -2, -1};
static int16_t y12[60] = {12, 12, 12, 11, 11, 10, 10, 9, 8, 7, 6, 5, 4, 2, 1, 0, -1, -2, -4, -5, -6, -7, -8, -9, -10, -10, -11, -11, -12, -12, -12, -12, -12, -11, -11, -10, -10, -9, -8, -7, -6, -5, -4, -2, -1, 0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 10, 11, 11, 12, 12};
static int16_t x15[60] = {0, 2, 3, 5, 6, 8, 9, 10, 11, 12, 13, 14, 14, 15, 15, 15, 15, 15, 14, 14, 13, 12, 11, 10, 9, 8, 6, 5, 3, 2, 0, -2, -3, -5, -6, -7, -9, -10, -11, -12, -13, -14, -14, -15, -15, -15, -15, -15, -14, -14, -13, -12, -11, -10, -9, -8, -6, -5, -3, -2};
static int16_t y15[60] = {15, 15, 15, 14, 14, 13, 12, 11, 10, 9, 7, 6, 5, 3, 2, 0, -2, -3, -5, -6, -7, -9, -10, -11, -12, -13, -14, -14, -15, -15, -15, -15, -15, -14, -14, -13, -12, -11, -10, -9, -7, -6, -5, -3, -2, 0, 2, 3, 5, 6, 8, 9, 10, 11, 12, 13, 14, 14, 15, 15};
static int16_t x20[60] = {0, 2, 4, 6, 8, 10, 12, 13, 15, 16, 17, 18, 19, 20, 20, 20, 20, 20, 19, 18, 17, 16, 15, 13, 12, 10, 8, 6, 4, 2, 0, -2, -4, -6, -8, -10, -12, -13, -15, -16, -17, -18, -19, -20, -20, -20, -20, -20, -19, -18, -17, -16, -15, -13, -12, -10, -8, -6, -4, -2};
static int16_t y20[60] = {20, 20, 20, 19, 18, 17, 16, 15, 13, 12, 10, 8, 6, 4, 2, 0, -2, -4, -6, -8, -10, -12, -13, -15, -16, -17, -18, -19, -20, -20, -20, -20, -20, -19, -18, -17, -16, -15, -13, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 13, 15, 16, 17, 18, 19, 20, 20};
static int16_t x24[60] = {0, 3, 5, 7, 10, 12, 14, 16, 18, 19, 21, 22, 23, 23, 24, 24, 24, 23, 23, 22, 21, 19, 18, 16, 14, 12, 10, 7, 5, 3, 0, -3, -5, -7, -10, -12, -14, -16, -18, -19, -21, -22, -23, -23, -24, -24, -24, -23, -23, -22, -21, -19, -18, -16, -14, -12, -10, -7, -5, -3};
static int16_t y24[60] = {24, 24, 23, 23, 22, 21, 19, 18, 16, 14, 12, 10, 7, 5, 3, 0, -3, -5, -7, -10, -12, -14, -16, -18, -19, -21, -22, -23, -23, -24, -24, -24, -23, -23, -22, -21, -19, -18, -16, -14, -12, -10, -7, -5, -3, 0, 3, 5, 7, 10, 12, 14, 16, 18, 19, 21, 22, 23, 23, 24};
static int16_t x30[60] = {0, 3, 6, 9, 12, 15, 18, 20, 22, 24, 26, 27, 29, 29, 30, 30, 30, 29, 29, 27, 26, 24, 22, 20, 18, 15, 12, 9, 6, 3, 0, -3, -6, -9, -12, -15, -18, -20, -22, -24, -26, -27, -29, -29, -30, -30, -30, -29, -29, -27, -26, -24, -22, -20, -18, -15, -12, -9, -6, -3};
static int16_t y30[60] = {30, 30, 29, 29, 27, 26, 24, 22, 20, 18, 15, 12, 9, 6, 3, 0, -3, -6, -9, -12, -15, -18, -20, -22, -24, -26, -27, -29, -29, -30, -30, -30, -29, -29, -27, -26, -24, -22, -20, -18, -15, -12, -9, -6, -3, 0, 3, 6, 9, 12, 15, 18, 20, 22, 24, 26, 27, 29, 29, 30};
static int16_t x36[60] = {0, 4, 7, 11, 15, 18, 21, 24, 27, 29, 31, 33, 34, 35, 36, 36, 36, 35, 34, 33, 31, 29, 27, 24, 21, 18, 15, 11, 7, 4, 0, -4, -7, -11, -15, -18, -21, -24, -27, -29, -31, -33, -34, -35, -36, -36, -36, -35, -34, -33, -31, -29, -27, -24, -21, -18, -15, -11, -7, -4};
static int16_t y36[60] = {36, 36, 35, 34, 33, 31, 29, 27, 24, 21, 18, 15, 11, 7, 4, 0, -4, -7, -11, -15, -18, -21, -24, -27, -29, -31, -33, -34, -35, -36, -36, -36, -35, -34, -33, -31, -29, -27, -24, -21, -18, -15, -11, -7, -4, 0, 4, 7, 11, 15, 18, 21, 24, 27, 29, 31, 33, 34, 35, 36};
static int16_t x40[60] = {0, 4, 8, 12, 16, 20, 24, 27, 30, 32, 35, 37, 38, 39, 40, 40, 40, 39, 38, 37, 35, 32, 30, 27, 24, 20, 16, 12, 8, 4, 0, -4, -8, -12, -16, -20, -24, -27, -30, -32, -35, -37, -38, -39, -40, -40, -40, -39, -38, -37, -35, -32, -30, -27, -24, -20, -16, -12, -8, -4};
static int16_t y40[60] = {40, 40, 39, 38, 37, 35, 32, 30, 27, 24, 20, 16, 12, 8, 4, 0, -4, -8, -12, -16, -20, -24, -27, -30, -32, -35, -37, -38, -39, -40, -40, -40, -39, -38, -37, -35, -32, -30, -27, -24, -20, -16, -12, -8, -4, 0, 4, 8, 12, 16, 20, 24, 27, 30, 32, 35, 37, 38, 39, 40};
static int16_t x45[60] = {0, 5, 9, 14, 18, 23, 26, 30, 33, 36, 39, 41, 43, 44, 45, 45, 45, 44, 43, 41, 39, 36, 33, 30, 26, 23, 18, 14, 9, 5, 0, -5, -9, -14, -18, -22, -26, -30, -33, -36, -39, -41, -43, -44, -45, -45, -45, -44, -43, -41, -39, -36, -33, -30, -26, -23, -18, -14, -9, -5};
static int16_t y45[60] = {45, 45, 44, 43, 41, 39, 36, 33, 30, 26, 22, 18, 14, 9, 5, 0, -5, -9, -14, -18, -22, -26, -30, -33, -36, -39, -41, -43, -44, -45, -45, -45, -44, -43, -41, -39, -36, -33, -30, -26, -22, -18, -14, -9, -5, 0, 5, 9, 14, 18, 23, 26, 30, 33, 36, 39, 41, 43, 44, 45};
static int16_t x48[60] = {0, 5, 10, 15, 20, 24, 28, 32, 36, 39, 42, 44, 46, 47, 48, 48, 48, 47, 46, 44, 42, 39, 36, 32, 28, 24, 20, 15, 10, 5, 0, -5, -10, -15, -20, -24, -28, -32, -36, -39, -42, -44, -46, -47, -48, -48, -48, -47, -46, -44, -42, -39, -36, -32, -28, -24, -20, -15, -10, -5};
static int16_t y48[60] = {48, 48, 47, 46, 44, 42, 39, 36, 32, 28, 24, 20, 15, 10, 5, 0, -5, -10, -15, -20, -24, -28, -32, -36, -39, -42, -44, -46, -47, -48, -48, -48, -47, -46, -44, -42, -39, -36, -32, -28, -24, -20, -15, -10, -5, 0, 5, 10, 15, 20, 24, 28, 32, 36, 39, 42, 44, 46, 47, 48};
static int16_t x60[60] = {0, 6, 12, 19, 24, 30, 35, 40, 45, 49, 52, 55, 57, 59, 60, 60, 60, 59, 57, 55, 52, 49, 45, 40, 35, 30, 24, 19, 12, 6, 0, -6, -12, -19, -24, -30, -35, -40, -45, -49, -52, -55, -57, -59, -60, -60, -60, -59, -57, -55, -52, -49, -45, -40, -35, -30, -24, -19, -12, -6};
static int16_t y60[60] = {60, 60, 59, 57, 55, 52, 49, 45, 40, 35, 30, 24, 19, 12, 6, 0, -6, -12, -19, -24, -30, -35, -40, -45, -49, -52, -55, -57, -59, -60, -60, -60, -59, -57, -55, -52, -49, -45, -40, -35, -30, -24, -19, -12, -6, 0, 6, 12, 19, 24, 30, 35, 40, 45, 49, 52, 55, 57, 59, 60};

static int16_t *offsets_x[4][5] = {
  {x30,  x60, NULL, NULL, NULL}, // Three letters
  {x20,  x40,  x60, NULL, NULL},
  {x15,  x30,  x45,  x60, NULL},
  {x12,  x24,  x36,  x48,  x60}  // Six letters
};

static int16_t *offsets_y[4][5] = {
  {y30,  y60, NULL, NULL, NULL}, // Three letters
  {y20,  y40,  y60, NULL, NULL},
  {y15,  y30,  y45,  y60, NULL},
  {y12,  y24,  y36,  y48,  y60}  // Six letters
};

static void draw_letter_at(GContext *ctx, char letter, int16_t x_off, int16_t y_off) {
  char str[2] = {letter, 0};
  GRect box = GRect(x_off + (PEBBLE_WIDTH - LETTER_WIDTH) / 2,
                    y_off + (PEBBLE_HEIGHT - LINE_HEIGHT) / 2,
                    LETTER_WIDTH, LINE_HEIGHT);
  graphics_draw_text(ctx, str, font, box, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  // following lines used to determine font sizing
  // graphics_context_set_stroke_color(ctx, GColorWhite);
  // graphics_draw_rect(ctx, box);
}

static void update_layer(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  const char *hour = hours[t->tm_hour % 12];

  uint8_t l = strlen(hour);
  int16_t **offx = offsets_x[l-3];
  int16_t **offy = offsets_y[l-3];

  draw_letter_at(ctx, hour[0], 0, 0);
  for(uint8_t i = 1; i < l; i++) {
    int16_t *xx = offx[i-1];
    int16_t *yy = offy[i-1];
    draw_letter_at(ctx, hour[i], xx[t->tm_min], -yy[t->tm_min]); // y axis is inverted, so -yy
  }
}

static void minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(layer);
}

static void init() {
  window = window_create();
  window_stack_push(window, true);

  window_set_background_color(window, GColorBlack);

  font = fonts_get_system_font(FONT_KEY_GOTHIC_24);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  layer = layer_create(bounds);
  layer_add_child(window_layer, layer);
  layer_set_update_proc(layer, update_layer);
  layer_mark_dirty(layer);

  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
  layer_destroy(layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
