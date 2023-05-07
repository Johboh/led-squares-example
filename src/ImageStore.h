#ifndef __IMAGE_STORE_H__
#define __IMAGE_STORE_H__

#include "FS.h"
#include <Arduino.h>
#include <vector>

#define RANDOM_FILE_PATH "/random.img"

class ImageStore {
public:
  /**
   * @brief Construct a new Image Store object
   *
   * @param canvas_width canvas width, in pixels.
   * @param canvas_height canvas height, in pixels.
   */
  ImageStore(uint16_t canvas_width, uint16_t canvas_height);

public:
  struct RGB {
    uint8_t r, g, b;
  };

public:
  void setup();

  /**
   * @brief Select the media.
   *
   * @param path path to media. Or empty string to not select any media.
   * @return true on media selection success, or false if media cannot be found.
   */
  bool selectMedia(String path = RANDOM_FILE_PATH);

  /**
   * @brief Get number of frames for current selected media.
   *
   * @return  number of frames. 0 if frame information is not available, or if no media is selected.
   */
  uint16_t getNumberOfFrames() { return _number_of_frames; }

  /**
   * @brief How long to delay the given frame until switching to next one.
   *
   * @param frame frame number.
   * @return number of miliseconds to delay, or 0 to delay forever. Also 0 if frame information is not available,
   * or if no media is selected.
   */
  uint16_t delayFrameFor(uint16_t frame);

  RGB getPixel(uint16_t frame, uint16_t x, uint16_t y);

private:
  void createImage(String path, uint16_t pixel_data);

private:
  uint16_t _canvas_width;
  size_t _canvas_size_in_bytes;

private:
  File _selected_file;
  uint16_t _number_of_frames;
  std::vector<uint16_t> _frame_delays;
  std::vector<size_t> _frame_image_data_position;
};

#endif // __IMAGE_STORE_H__