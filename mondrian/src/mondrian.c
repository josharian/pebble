#include <pebble.h>

// The "mondrian" watchface is an abstract watchface that
// shows time based on blocks of light/dark. The hours
// count from left to right, and the minutes count
// from bottom to top. AM/PM are distinguished by whether
// the counting up occurs in black or white. The face
// has notches around the edges to make it easier to
// pick out the time visually.

static Window *window;

InverterLayer *minute_layer;
InverterLayer *hour_layer;
Layer *notch_layer;

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168

static unsigned short window_color(unsigned short hour) {
  if (clock_is_24h_style()) {
    return GColorWhite;
  }
  return (hour < 12) ? GColorWhite : GColorBlack;
}

static unsigned short notch_color(unsigned short hour) {
  return (window_color(hour) == GColorBlack) ? GColorWhite : GColorBlack;
}

static unsigned short hour_x(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour * (PEBBLE_WIDTH / 24);
  }
  unsigned short display_hour = hour % 12;
  display_hour = display_hour ? display_hour : 12; // convert "0" to "12"
  return (display_hour * PEBBLE_WIDTH) / 12;
}

static unsigned short minute_y(unsigned short minute) {
  return (minute * PEBBLE_HEIGHT) / 60;
}

#define NOTCH_WIDTH 3
#define N_NOTCHES 3

static void update_notch_layer(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, notch_color(t->tm_hour));

  // draw little triangular ticks
  for (unsigned short i = 1; i <= N_NOTCHES; i++) {
    for (unsigned short in = 0; in < NOTCH_WIDTH; in++) {
      unsigned short inset = NOTCH_WIDTH - in - 1;
      for (short wide = -in*2; wide <= in*2; wide++) {
        unsigned short notch_y = wide + (i * PEBBLE_HEIGHT) / (N_NOTCHES + 1);
        graphics_draw_pixel(ctx, GPoint(inset, notch_y)); // left
        graphics_draw_pixel(ctx, GPoint(PEBBLE_WIDTH - 1 - inset, notch_y)); // right
        unsigned short notch_x = wide + (i * PEBBLE_WIDTH) / (N_NOTCHES + 1);
        graphics_draw_pixel(ctx, GPoint(notch_x, inset)); // top
        graphics_draw_pixel(ctx, GPoint(notch_x, PEBBLE_HEIGHT - 1 - inset)); // bottom
      }
    }
  }
}

static void hour_tick(struct tm *tick_time, TimeUnits units_changed) {
  unsigned int x = hour_x(tick_time->tm_hour);
  GRect frame = GRect(0, 0, x, PEBBLE_HEIGHT);
  layer_set_frame(inverter_layer_get_layer(hour_layer), frame);

  // redraw background color, notches
  window_set_background_color(window, window_color(tick_time->tm_hour));
  layer_mark_dirty(notch_layer);
}

static void minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  unsigned int y = minute_y(tick_time->tm_min);
  GRect frame = GRect(0, PEBBLE_HEIGHT - y, PEBBLE_WIDTH, y);
  layer_set_frame(inverter_layer_get_layer(minute_layer), frame);

  if (units_changed & HOUR_UNIT) {
    hour_tick(tick_time, units_changed);
  }
}

static void init() {
  window = window_create();
  window_stack_push(window, true);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  notch_layer = layer_create(bounds);
  layer_add_child(window_layer, notch_layer);
  layer_set_update_proc(notch_layer, update_notch_layer);

  hour_layer = inverter_layer_create(bounds);
  layer_add_child(window_layer, inverter_layer_get_layer(hour_layer));

  minute_layer = inverter_layer_create(bounds);
  layer_add_child(window_layer, inverter_layer_get_layer(minute_layer));

  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick);

  // avoid a blank screen on watch start
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
  minute_tick(tick_time, MINUTE_UNIT | HOUR_UNIT);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
  inverter_layer_destroy(minute_layer);
  inverter_layer_destroy(hour_layer);
  layer_destroy(notch_layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
