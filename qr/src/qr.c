#include <pebble.h>

// World's most useful watch face.

static Window *window;
static ResHandle qr_handle;

#define PACKED_QR_SIZE 56 // == ceil(21*21/8)

static uint8_t active_qr[PACKED_QR_SIZE];

Layer *qr_layer;

#define PEBBLE_WIDTH 144
#define PEBBLE_HEIGHT 168

#define QR_DIM 21 // small qr codes are 21x21
#define QR_X_PAD 1.5 // 1.5 + 21 + 1.5 == 24, which divides nicely into the pebble screen
#define N_DOTS (QR_DIM + QR_X_PAD * 2)

#define PIX_PER_DOT (PEBBLE_WIDTH / N_DOTS)
#define QR_Y_PAD (((PEBBLE_HEIGHT - QR_DIM * PIX_PER_DOT) / PIX_PER_DOT) / 2.0)

static void update_qr_layer(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned int min_elapsed = t->tm_hour * 60 + t->tm_min;
  resource_load_byte_range(qr_handle, PACKED_QR_SIZE * min_elapsed, active_qr, PACKED_QR_SIZE);
  // APP_LOG(APP_LOG_LEVEL_INFO, "UPDATING %i %i == %i -- %i", (int)(t->tm_hour), (int)(t->tm_min), min_elapsed, PACKED_QR_SIZE * min_elapsed);

  graphics_context_set_fill_color(ctx, GColorBlack);

  uint16_t idx = 0;
  uint8_t byt = 0;
  for (unsigned int y = 0; y < QR_DIM; y++) {
    for (unsigned int x = 0; x < QR_DIM; x++) {
      if (idx % 8 == 0) {
        byt = active_qr[idx/8];
      }
        //APP_LOG(APP_LOG_LEVEL_INFO, "TEST: %d %d", idx, byt);
      if (byt & 128) {
        GRect dot = GRect((x + QR_X_PAD) * PIX_PER_DOT, (y + QR_Y_PAD) * PIX_PER_DOT, PIX_PER_DOT, PIX_PER_DOT);
        //APP_LOG(APP_LOG_LEVEL_INFO, "ON: %d %d %d %d", dot.origin.x, dot.origin.y, dot.size.w, dot.size.h);
        graphics_fill_rect(ctx, dot, 0, GCornerNone);
      }
      byt = byt << 1;
      idx++;
    }
  }
}

static void minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(qr_layer);
}

static void init() {
  window = window_create();
  window_stack_push(window, true);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  qr_handle = resource_get_handle(RESOURCE_ID_QR);

  qr_layer = layer_create(bounds);
  layer_add_child(window_layer, qr_layer);
  layer_set_update_proc(qr_layer, update_qr_layer);
  layer_mark_dirty(qr_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(window);
  layer_destroy(qr_layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
