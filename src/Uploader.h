#ifndef __UPLOADER_H__
#define __UPLOADER_H__

#include "ImageStore.h"
#include <Arduino.h>
#include <PSRamFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <functional>

class Uploader {
public:
  /**
   * @brief Construct a new Uploader object
   */
  Uploader(ImageStore &image_store, std::function<void(String path, uint16_t blend_duration)> on_image_selected,
           std::function<void(uint8_t)> on_brightness = {});

public:
  void setup();
  void handle();

private:
  ImageStore &_image_store;
  WebServer _server;
  File _file;
  std::function<void(String, uint16_t)> _on_image_selected;
  std::function<void(uint8_t)> _on_brightness;
};

#endif // __UPLOADER_H__