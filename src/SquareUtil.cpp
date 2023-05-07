#include "SquareUtil.h"

#define PIXELS_PER_ROW_PER_SQUARE (8)
#define PIXEL_PER_SQUARE_COLUMN (8)
#define PIXELS_PER_SQUARE (64)

SquareUtil::SquareUtil(uint16_t rows, uint16_t columns)
    : _rows(rows), _columns(columns), _leds_per_row(columns * PIXELS_PER_ROW_PER_SQUARE),
      _leds_per_column(rows * PIXEL_PER_SQUARE_COLUMN), _total_number_of_leds(rows * columns * PIXELS_PER_SQUARE) {}

uint16_t SquareUtil::stripPixel(uint16_t x, uint16_t y) {

  uint16_t square_column = x / PIXELS_PER_ROW_PER_SQUARE;

  uint16_t x_offset;
  uint16_t x_in_square = x % PIXELS_PER_ROW_PER_SQUARE;
  if (y % 2 == 0) {
    x_offset = x_in_square;
  } else {
    x_offset = PIXELS_PER_ROW_PER_SQUARE - x_in_square - 1;
  }

  // Adding square_column to correct for FastLED RGBW hack
  return (PIXELS_PER_SQUARE * square_column * _rows) + (y * PIXELS_PER_ROW_PER_SQUARE) + x_offset + square_column;
}

SquareUtil::Coordinates SquareUtil::getCoordinates(uint16_t strip_pixel) {
  Coordinates coorinates;

  uint16_t square_column = strip_pixel / (_rows * PIXELS_PER_SQUARE);

  uint16_t first_pixel = square_column * (_rows * PIXELS_PER_SQUARE);

  uint16_t diff = strip_pixel - first_pixel;
  coorinates.y = diff / PIXELS_PER_ROW_PER_SQUARE;

  uint16_t x_left = diff - (coorinates.y * PIXELS_PER_ROW_PER_SQUARE);
  if (coorinates.y % 2 == 0) {
    coorinates.x = (PIXELS_PER_ROW_PER_SQUARE * square_column) + x_left;
  } else {
    coorinates.x = (PIXELS_PER_ROW_PER_SQUARE * (square_column + 1)) - x_left - 1;
  }
  return coorinates;
}