import os

class Converter():
  def __init__(self, width_in_pixels, height_in_pixels):
    self.width_in_pixels = width_in_pixels
    self.height_in_pixels = height_in_pixels

  def read_rows(self, path):
      image_file = open(path, "rb")
      # Blindly skip the BMP header.
      image_file.seek(54)

      # We need to read pixels in as rows to later swap the order
      # since BMP stores pixels starting at the bottom left.
      rows = []
      row = []
      pixel_index = 0

      while True:
          if pixel_index == self.width_in_pixels:
              pixel_index = 0
              rows.insert(0, row)
              if len(row) != self.width_in_pixels * 3:
                  raise Exception("Row length is not %d*3 but %d / 3.0 = %d" % (self.width_in_pixels, len(row), len(row) / 3.0))
              row = []
          pixel_index += 1

          r_string = image_file.read(1)
          g_string = image_file.read(1)
          b_string = image_file.read(1)

          if len(r_string) == 0:
              # This is expected to happen when we've read everything.
              if len(rows) != self.height_in_pixels:
                  print("Warning!!! Read to the end of the file at the correct sub-pixel (red) but we've not read %d rows! Read %d rows." % (self.height_in_pixels, len(rows)))
              break

          if len(g_string) == 0:
              print("Warning!!! Got 0 length string for green. Breaking.")
              break

          if len(b_string) == 0:
              print("Warning!!! Got 0 length string for blue. Breaking.")
              break

          r = ord(r_string)
          g = ord(g_string)
          b = ord(b_string)

          row.append(b)
          row.append(g)
          row.append(r)

      image_file.close()

      return rows

  def repack_sub_pixels(self, sub_pixels, rows):
      for row in rows:
          for sub_pixel in row:
              sub_pixels.append(sub_pixel)