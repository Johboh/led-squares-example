#include "ImageStore.h"
#include "esp_random.h"
#include <PSRamFS.h>

#define BYTES_PER_PIXEL 3 // 24 bit deps (r,g,b each 8 bytes)

#pragma pack(1)

// All files must have this header at the start of the file.
struct MediaFileHeader {
  uint16_t number_of_frames = 0;
};

// All frames in the media file must have this header before start of each frame.
struct FrameHeader {
  uint16_t delay_ms = 0;
};

#pragma pack(0)

ImageStore::ImageStore(uint16_t canvas_width, uint16_t canvas_height)
    : _canvas_width(canvas_width), _canvas_size_in_bytes(canvas_width * canvas_height * BYTES_PER_PIXEL) {}

void ImageStore::setup() {
  if (!PSRamFS.begin()) {
    Serial.println("ImageStore::setup(): PSRamFS mount Failed");
    return;
  }

  createImage(RANDOM_FILE_PATH, 0xFFFF);
  createImage("/white.img", 0xFF);
  createImage("/black.img", 0x00);
}

bool ImageStore::selectMedia(String path) {
  Serial.println("ImageStore::selectMedia(" + path + ")");

  if (path == RANDOM_FILE_PATH) {
    createImage(RANDOM_FILE_PATH, 0xFFFF); // Will randomize with a new random file.
  }

  // Pre-caculate some data so we don't have to find them over and over again later on.
  _frame_delays.clear();
  _frame_image_data_position.clear();

  if (_selected_file) {
    _selected_file.close();
  }

  _selected_file = PSRamFS.open(path.c_str(), FILE_READ);
  if (_selected_file) {
    MediaFileHeader media_header;
    _selected_file.read((uint8_t *)&media_header, sizeof(MediaFileHeader));
    _number_of_frames = media_header.number_of_frames;

    Serial.println("ImageStore::selectMedia(" + path + "): _number_of_frames = " + String(_number_of_frames));

    uint32_t offset_from_last_image = sizeof(MediaFileHeader);
    for (uint16_t i = 0; i < _number_of_frames; ++i) {
      _selected_file.seek(offset_from_last_image);

      FrameHeader frame_header;
      _selected_file.read((uint8_t *)&frame_header, sizeof(FrameHeader));

      uint32_t image_data_position = offset_from_last_image + sizeof(FrameHeader);

      Serial.println("ImageStore::selectMedia(" + path + "): frame = " + String(i) + ": delay_ms: " +
                     String(frame_header.delay_ms) + " image_data_position: " + String(image_data_position));

      _frame_delays.emplace_back(frame_header.delay_ms);
      _frame_image_data_position.emplace_back(image_data_position);

      offset_from_last_image = image_data_position + _canvas_size_in_bytes;
    }

    return true;
  }

  Serial.println("ImageStore::selectMedia(" + path + "): Unable to select media, file not found.");
  return false;
}

uint16_t ImageStore::delayFrameFor(uint16_t frame) {
  if (frame < _frame_delays.size()) {
    Serial.println("ImageStore::delayFrameFor(frame=" + String(frame) + "): " + String(_frame_delays[frame]));
    return _frame_delays[frame];
  } else {
    Serial.println("ImageStore::delayFrameFor(frame=" + String(frame) + "): frame number is out of bounds.");
    return 0;
  }
}

ImageStore::RGB ImageStore::getPixel(uint16_t frame, uint16_t x, uint16_t y) {
  uint16_t r = y * _canvas_width * BYTES_PER_PIXEL + x * BYTES_PER_PIXEL;

  if (frame >= _frame_image_data_position.size()) {
    Serial.println("ImageStore::getPixel(frame=" + String(frame) + ", x=" + String(x) + ", y=" + String(y) +
                   "): frame number is out of bounds.");
  }

  if (_selected_file) {
    _selected_file.seek(_frame_image_data_position[frame] + r);
    uint8_t rgb[3];
    _selected_file.read(rgb, 3);
    return RGB{
        .r = rgb[0],
        .g = rgb[1],
        .b = rgb[2],
    };
  } else {
    Serial.println("ImageStore::getPixel(frame=" + String(frame) + ", x=" + String(x) + ", y=" + String(y) +
                   "): No selected file.");
  }

  return RGB{0x00};
}

void ImageStore::createImage(String path, uint16_t pixel_data) {
  PSRamFS.remove(path);
  File file = PSRamFS.open(path, FILE_WRITE);
  uint8_t image[_canvas_size_in_bytes + sizeof(MediaFileHeader) + sizeof(FrameHeader)];

  MediaFileHeader *media_file_header = (MediaFileHeader *)image;
  media_file_header->number_of_frames = 1;

  FrameHeader *frame_header = (FrameHeader *)(image + sizeof(MediaFileHeader));
  frame_header->delay_ms = 0;

  if (pixel_data <= 0xFF) {
    memset(image + sizeof(MediaFileHeader) + sizeof(FrameHeader), (uint8_t)pixel_data, _canvas_size_in_bytes);
  } else {
    esp_fill_random(image + sizeof(MediaFileHeader) + sizeof(FrameHeader), _canvas_size_in_bytes);
  }
  file.write(image, _canvas_size_in_bytes);
  file.close();
}