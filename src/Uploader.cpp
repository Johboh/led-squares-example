#include "Uploader.h"

Uploader::Uploader(ImageStore &image_store, std::function<void(String, uint16_t)> on_image_selected,
                   std::function<void(uint8_t)> on_brightness)
    : _image_store(image_store), _server(80), _on_image_selected(on_image_selected), _on_brightness(on_brightness) {}

void Uploader::setup() {
  _server.on("/", [&]() {
    _server.send(200, "text/html",
                 "/clear<br>/select?filename=/example.gif&blendDuration=<i>milliseconds to blend or 0/absent for no "
                 "blending</i><br>/list=<br>/df<br>/brightness?value=0-255<br>/upload/off<br>");
  });
  _server.onNotFound([&]() { _server.send(404, "text/plain", "Not found"); });
  _server.on("/off", [&]() {
    if (_on_image_selected) {
      _on_image_selected("/black.img", 0);
    }
    _server.send(200, "text/plain", "OK");
  });
  _server.on("/clear", [&]() {
    if (PSRamFS.format()) {
      _server.send(200, "text/plain", "OK");
    } else {
      _server.send(500, "text/plain", "Format failed.");
    }
    if (_on_image_selected) {
      _on_image_selected("", 0);
    }
  });
  _server.on("/select", [&]() {
    if (_server.hasArg("filename")) {
      auto filename = _server.arg("filename");
      if (!PSRamFS.exists(filename)) {
        _server.send(400, "text/plain", "File " + String(filename) + " does not exists.");
      } else {
        _server.send(200, "text/plain", "OK");
        if (_on_image_selected) {
          uint16_t blend_duration = _server.hasArg("blendDuration") ? _server.arg("blendDuration").toInt() : 0;
          _on_image_selected(filename, blend_duration);
        }
      }
    } else {
      _server.send(400, "text/plain", "Missing query parameter filename");
    }
  });
  _server.on("/list", [&]() {
    struct PSRAMFILE {
      int file_id;
      char *name;
      char *bytes;
      uint32_t size;
      uint32_t memsize;
      uint32_t index;
      int dir_id;
    };
    PSRAMFILE **files = (PSRAMFILE **)PSRamFS.getFiles();
    size_t files_count = PSRamFS.getFilesCount();
    if (files != NULL) {
      if (files_count > 0) {
        String response;
        for (size_t i = 0; i < files_count; ++i) {
          if (files[i]->name != NULL) {
            response += String(files[i]->name) + ": " + String(files[i]->size) + " bytes<br>\n";
          }
        }
        _server.send(200, "text/html", response);
      } else {
        _server.send(200, "text/html", "No files.");
      }
    }
  });
  _server.on("/df", [&]() {
    _server.send(200, "text/plain",
                 "Using " + String(PSRamFS.usedBytes()) + " bytes out of " + String(PSRamFS.totalBytes()));
  });
  _server.on("/brightness", [&]() {
    if (_server.hasArg("value")) {
      auto value = _server.arg("value");
      if (_on_brightness) {
        _on_brightness(value.toInt());
      }
      _server.send(200, "text/plain", "OK");
    } else {
      _server.send(400, "text/plain", "Missing query parameter value");
    }
  });
  _server.on(
      "/upload", HTTP_POST, [&]() {},
      [&]() {
        HTTPUpload &upload = _server.upload();
        String path = String("/") + upload.filename;

        switch (upload.status) {
        case UPLOAD_FILE_START:
          Serial.println("Upload for file " + path + " started.");

          PSRamFS.remove(path);
          _file = PSRamFS.open(path.c_str(), FILE_WRITE);
          if (!_file) {
            _server.send(500, "text/html", "Failed to open file for writing");
            Serial.println("failed to open file for writing");
          }
          break;
        case UPLOAD_FILE_WRITE:
          if (_file && _file.write(upload.buf, upload.currentSize) != upload.currentSize) {
            _server.send(500, "text/html", "Failed to write");
            Serial.println("failed to write");
          } else {
            Serial.println("Wrote " + String(upload.currentSize));
          }
          break;
        case UPLOAD_FILE_END:
          if (_file) {
            Serial.println("Upload for file " + path + " ended.");
            _file.close();
            _server.send(200, "text/html", path);
          } else {
            Serial.println("End without file for " + path);
          }
          break;
        case UPLOAD_FILE_ABORTED:
          Serial.println("Upload for file " + path + " aborted.");
          break;
        default:
          break;
        }
      });
  _server.begin();
}

void Uploader::handle() { _server.handleClient(); }