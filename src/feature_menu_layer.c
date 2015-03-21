#include "pebble.h"

#define NUM_MENU_SECTIONS 2
#define NUM_MENU_ICONS 3
#define NUM_FIRST_MENU_ITEMS 3
#define NUM_SECOND_MENU_ITEMS 1

static Window *s_main_window;
static MenuLayer *s_menu_layer[NUM_FIRST_MENU_ITEMS];
static GBitmap *s_menu_icons[NUM_MENU_ICONS];
static GBitmap *s_background_bitmap;

static char * words[5] = {"h", "e", "l", "l", "o"};

static int s_current_icon = 0;



// BEGIN CODE FOR SENDING MESSAGES
static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Text: SEND_CMD");

  if (iter == NULL) return;
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  app_message_outbox_send();
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *location_tuple = dict_read_first(iter); // Check for fields you expect to receive
  APP_LOG(APP_LOG_LEVEL_DEBUG, "RECEIVE: %s", location_tuple->value->cstring);
  
  if (location_tuple) { // Act on the found fields received
    words[0] = location_tuple->value->cstring;
  }
}


void in_dropped_handler(AppMessageResult reason, void *context) {
  // incoming message dropped
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message Dropped");
}
// END CODE FOR SENDING MESSAGES

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;
    case 1:
      return NUM_SECOND_MENU_ITEMS;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Some example items");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "One more");
      break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle
          menu_cell_basic_draw(ctx, cell_layer, words[0], "subs", NULL);
          break;
        case 1:
          // This is a basic menu icon with a cycling icon
          menu_cell_basic_draw(ctx, cell_layer, "Icon Item", "Select to cycle", s_menu_icons[s_current_icon]);
          break;
        case 2: 
          {
            // Here we use the graphics context to draw something different
            // In this case, we show a strip of a watchface's background
            GSize size = layer_get_frame(cell_layer).size;
            graphics_draw_bitmap_in_rect(ctx, s_background_bitmap, GRect(0, 0, size.w, size.h));
          }
          break;
      }
      break;
    case 1:
      switch (cell_index->row) {
        case 0:
          // There is title draw for something more simple than a basic menu item
          menu_cell_title_draw(ctx, cell_layer, "Final Item");
          break;
      }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    // This is the menu item with the cycling icon
    case 1:
      // Cycle the icon
      s_current_icon = (s_current_icon + 1) % NUM_MENU_ICONS;
      // After changing the icon, mark the layer to have it updated
      layer_mark_dirty(menu_layer_get_layer(menu_layer));
      break;
    case 0:
      //words[0] = "H";
      send_cmd();
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Before layer");
      layer_mark_dirty(menu_layer_get_layer(menu_layer));
  }

}

static void main_window_load(Window *window) {
  // Here we load the bitmap assets
  s_menu_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BIG_WATCH);
  s_menu_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_SECTOR_WATCH);
  s_menu_icons[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BINARY_WATCH);

  // And also load the background
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_BRAINS);

  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  s_menu_layer[0] = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer[0], NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer[0], window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer[0]));
}

static void main_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer[0]);

  // Cleanup the menu icons
  for (int i = 0; i < NUM_MENU_ICONS; i++) {
    gbitmap_destroy(s_menu_icons[i]);
  }

  gbitmap_destroy(s_background_bitmap);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  // START MESSAGE CODE
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  //app_message_register_outbox_sent(out_sent_handler);
  //app_message_register_outbox_failed(out_failed_handler);

  //const uint32_t inbound_size = 256;
  //const uint32_t outbound_size = 256;
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  // END MESSAGE CODE
  
  init();
  app_event_loop();
  deinit();
}
