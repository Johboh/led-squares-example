#include "ExternalBrightness.h"
#include "FastLED.h"
#include "FastLED_RGBW.h"
#include "ImageStore.h"
#include "SquareUtil.h"
#include "Uploader.h"
#include "credentials.h"
#include <OtaHelper.h>
#include <RGBWConverter.h>
#include <WiFiHelper.h>

// Number of tiles per row and column
#define NUM_COLUMNS (7)
#define NUM_ROWS (5)

#define LEDS_PER_TILE_ROW (8)
#define LEDS_PER_TILE_COLUMN (8)
#define LEDS_PER_TILE (64)
#define LEDS_ON_X (LEDS_PER_TILE_ROW * NUM_COLUMNS)
#define LEDS_ON_Y (LEDS_PER_TILE_COLUMN * NUM_ROWS)

#define BLEND_REFRESH_RATE_MS 50

RGBWConverter converter(230, 220, 120, false);

#define PIN_LDR GPIO_NUM_7

// Adding 1 to correct for FastLED RGBW hack
#define NUM_LEDS_PER_STRIP ((LEDS_PER_TILE * NUM_ROWS) + 1)
#define DEFAULT_BRIGHTNESS 50

#define NUM_LEDS (NUM_LEDS_PER_STRIP * NUM_COLUMNS)

CRGBW _leds[NUM_LEDS];
CRGB *_ledsRGB = (CRGB *)&_leds[0]; // RGBW hack.

OtaHelper _ota_helper(hostname);
WiFiHelper _wifi_helper(wifi_ssid, wifi_password, hostname);

std::optional<String> _image_to_select = std::optional<String>{""};
std::optional<uint16_t> _blend_duration_ms = std::nullopt;

SquareUtil _square_util(NUM_ROWS, NUM_COLUMNS);
ImageStore _image_store(_square_util.ledsPerRow(), _square_util.ledsPerColumn());

Uploader _uploader(
    _image_store,
    [](String path, uint16_t blend_duration) {
      _image_to_select = std::optional<String>{path};
      _blend_duration_ms = blend_duration > 0 ? std::optional<uint16_t>{blend_duration} : std::nullopt;
    },
    [](uint8_t brightness) {
      FastLED.setBrightness(brightness);
      FastLED.show();
    });

ExternalBrightness _external_brightness(PIN_LDR);

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0); // Prevent delays when not connected to USB/CDC.

  _wifi_helper.connect();
  _ota_helper.setup();

  auto rbgw_size = getRGBWsize(NUM_LEDS_PER_STRIP); // RGBW hack.
  // Add each column as separate data line.
  FastLED.addLeds<SK6812, GPIO_NUM_34, RGB>(_ledsRGB, 0 * rbgw_size, rbgw_size);
  FastLED.addLeds<SK6812, GPIO_NUM_40, RGB>(_ledsRGB, 1 * rbgw_size, rbgw_size);
  FastLED.addLeds<SK6812, GPIO_NUM_38, RGB>(_ledsRGB, 2 * rbgw_size, rbgw_size);
  FastLED.addLeds<SK6812, GPIO_NUM_39, RGB>(_ledsRGB, 3 * rbgw_size, rbgw_size);
  FastLED.addLeds<SK6812, GPIO_NUM_36, RGB>(_ledsRGB, 4 * rbgw_size, rbgw_size);
  FastLED.addLeds<SK6812, GPIO_NUM_35, RGB>(_ledsRGB, 5 * rbgw_size, rbgw_size);
  FastLED.addLeds<SK6812, GPIO_NUM_37, RGB>(_ledsRGB, 6 * rbgw_size, rbgw_size);
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);
  FastLED.setMaxPowerInMilliWatts(85000); // Adjust accordingly.

  for (uint16_t x = 0; x < LEDS_ON_X; ++x) {
    for (uint16_t y = 0; y < LEDS_ON_Y; ++y) {
      _leds[_square_util.stripPixel(x, y)] = CRGBW(0, 0, 0, 0);
    }
  }

  FastLED.show();

  _image_store.setup();
  _uploader.setup();

  pinMode(GPIO_NUM_15, OUTPUT);
  digitalWrite(GPIO_NUM_15, 1);
}

size_t _current_media = 0;
uint16_t _current_frame = 0;
unsigned long _draw_next_frame_at = 0;
unsigned long _blend_last_refresh_ms = 0;

unsigned long _blend_target_time = 0;
CRGB _blend_from[NUM_LEDS];

void loop() {
  _wifi_helper.handle();
  _ota_helper.handle();
  _uploader.handle();
  _external_brightness.handle();

  bool should_draw = false;
  auto now = millis();

  // Have new image?
  if (_image_to_select) {
    String image = *_image_to_select;

    _image_store.selectMedia(image);

    if (_blend_duration_ms) {
      // We need to blend.
      // Take snapshot.
      for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        auto led = _leds[i];
        _blend_from[i] = CRGB(led.r, led.g, led.b);
      }
      _blend_target_time = now + *_blend_duration_ms;
      _blend_last_refresh_ms = 0;
    } else {
      // No blending. Just set.
      _draw_next_frame_at = now;
    }

    _image_to_select = std::nullopt;
  }

  // Are we blending?
  if (_blend_target_time > 0 && _blend_duration_ms) {
    // Yes.

    // Done blending?
    if (now >= _blend_target_time) {
      // Yes.
      _blend_target_time = 0;
      _blend_last_refresh_ms = 0;
      _draw_next_frame_at = now;
    } else {
      // Not done, blend.

      // Refresh now?
      if (now - _blend_last_refresh_ms > BLEND_REFRESH_RATE_MS) {
        float progress = 1.0 - (((float)_blend_target_time - (float)now) / (float)*_blend_duration_ms);

        // Blend each pixel in the matrix.
        for (uint16_t x = 0; x < _square_util.ledsPerRow(); ++x) {
          for (uint16_t y = 0; y < _square_util.ledsPerColumn(); ++y) {
            auto rgb = _image_store.getPixel(0, x, y);
            uint16_t i = _square_util.stripPixel(x, y);
            auto blended = blend(_blend_from[i], CRGB(rgb.r, rgb.g, rgb.b), progress * 255);
            auto rgbwc = converter.RGBToRGBW(blended.r, blended.g, blended.b);
            auto rgbw = CRGBW(rgbwc.r, rgbwc.g, rgbwc.b, rgbwc.w);
            _leds[i] = rgbw;
          }
        }
        FastLED.show();

        _blend_last_refresh_ms = now;
      }
    }
  } else {
    // No blending, normal draw.

    // Advance to next frame?
    if (_draw_next_frame_at > 0 && now >= _draw_next_frame_at) {
      auto number_of_frames = _image_store.getNumberOfFrames();
      if (++_current_frame >= number_of_frames) {
        _current_frame = 0;
      }
      auto delay = _image_store.delayFrameFor(_current_frame);
      if (delay > 0) {
        _draw_next_frame_at = now + delay;
      } else {
        _draw_next_frame_at = 0;
      }

      should_draw = true;
    }

    if (should_draw) {
      for (uint16_t x = 0; x < _square_util.ledsPerRow(); ++x) {
        for (uint16_t y = 0; y < _square_util.ledsPerColumn(); ++y) {
          auto rgb = _image_store.getPixel(_current_frame, x, y);
          auto rgbw = converter.RGBToRGBW(rgb.r, rgb.g, rgb.b);
          auto pixel = _square_util.stripPixel(x, y);
          _leds[pixel] = CRGBW(rgbw.r, rgbw.g, rgbw.b, rgbw.w);
        }
      }
      FastLED.show();
    }
  }
}