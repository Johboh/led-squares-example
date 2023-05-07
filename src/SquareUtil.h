#ifndef __SQUARE_UTIL_H__
#define __SQUARE_UTIL_H__

#include <Arduino.h>

/**
 * Assuming the following:
 * - First pixel of a square is top left, and then snake connected downward.
 * - Each square is connected column per column, where end of first column is connected to top most square in second
 * column and so on.
 */
class SquareUtil {
public:
  /**
   * @brief Construct a new Square Util object
   *
   * @param rows number of squares per row (note: squares, not pixels).
   * @param columns number of squares per column (note: squares, not pixels).
   */
  SquareUtil(uint16_t rows, uint16_t columns);

public:
  /**
   * @brief Convert the x/y coordinates to the strip pixel.
   * top left is 0, 0.
   *
   * @param x coordinate.
   * @param y coordinate.
   * @return uint16_t strip pixel.
   */
  uint16_t stripPixel(uint16_t x, uint16_t y);

  struct Coordinates {
    uint16_t x;
    uint16_t y;
  };

  /**
   * @brief Given a strip pixel, return the x/y coordinate.
   *
   * @param stripPixel the strip pixel index.
   * @return Coordinate
   */
  Coordinates getCoordinates(uint16_t strip_pixel);

  uint16_t ledsPerRow() { return _leds_per_row; }

  uint16_t ledsPerColumn() { return _leds_per_column; }

  uint16_t totalNumberOfLeds() { return _total_number_of_leds; }

private:
  uint16_t _rows;
  uint16_t _columns;
  uint16_t _leds_per_row;
  uint16_t _leds_per_column;
  uint16_t _total_number_of_leds;
};

#endif // __SQUARE_UTIL_H__