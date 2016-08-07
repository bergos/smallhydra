/* Copyright 2016 Thomas Bergwinkl. All Rights Reserved.

 */

#include "Hydra.h"

#include <RDFNTriplesParser.h>
#include <RDFNTriplesSerializer.h>

RDFString localhostApi("http://localhost/api");

Hydra::Hydra()
    : raw(0),
      api(0) {
}

void Hydra::begin(String filename, String path, String hostname) {
  this->path = path;

  readApiDocument(filename);
}

String Hydra::absoluteUrl(AsyncWebServerRequest* request) {
  String url = String("http://");

  if (request->host().length() != 0) {
    url += request->host();
  } else {
    url += WiFi.localIP().toString();
  }

  url += request->url();

  return url;
}

void Hydra::handleRequest(AsyncWebServerRequest* request,
                          AsyncWebServerResponse* response) {
  response->addHeader(
      "Link",
      "<" + absoluteUrl(request) + path
          + ">; rel=\"http://www.w3.org/ns/hydra/core#apiDocumentation\"");
}

void Hydra::handleApiRequest(AsyncWebServerRequest* request) {
  if (!api) {
    String hostname = request->host();

    if (hostname.length() == 0) {
      hostname = WiFi.localIP().toString();
    }

    api = patchApiDocument(hostname);
  }

  const uint8_t* content = NTriplesSerializer::serialize_static(api);

  request->send(200, "application/n-triples", (const char*) content);

  delete[] content;
}

void Hydra::readApiDocument(String path) {
  File file = SPIFFS.open(path, "r");

  if (!file) {
    return;
  }

  NTriplesParser::parse_static(document.string(file.readString(), true),
                               &document);

  raw = document.dataset();

  for (int i = 0; i < document.quads.length; i++) {
    raw->quads.add(document.quads.get(i));
  }
}

RDFDataset* Hydra::patchApiDocument(String hostname) {
  hostname = hostname.length() != 0 ? hostname : WiFi.localIP().toString();

  const RDFTerm* apiTerm = document.namedNode(
      document.string("http://" + hostname + "/" + path, true));

  RDFDataset* patched = document.dataset();

  if (!raw) {
    return patched;
  }

  for (int i = 0; i < raw->quads.length; i++) {
    const RDFQuad* triple = raw->quads.get(i);

    const RDFTerm* subject = triple->subject;
    const RDFTerm* predicate = triple->predicate;
    const RDFTerm* object = triple->object;

    if (subject->value->equals(localhostApi)) {
      subject = apiTerm;
    }

    if (object->value->equals(localhostApi)) {
      object = apiTerm;
    }

    patched->quads.add(document.triple(subject, predicate, object));
  }

  return patched;
}
