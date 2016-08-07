/* Copyright 2016 Thomas Bergwinkl. All Rights Reserved.

 */

#ifndef HYDRA_H_
#define HYDRA_H_

#include <ESPAsyncWebServer.h>
#include <RDF.h>

class Hydra {
 public:
  Hydra();

  void begin(String filename, String path, String hostname = "");

  String absoluteUrl(AsyncWebServerRequest* request);
  void handleRequest(AsyncWebServerRequest* request,
                     AsyncWebServerResponse* response);
  void handleApiRequest(AsyncWebServerRequest* request);

 protected:
  RDFDocument document;
  RDFDataset* raw;
  String path;
  RDFDataset* api;

  void readApiDocument(String filename);
  RDFDataset* patchApiDocument(String hostname);
};

#endif  // HYDRA_H_
