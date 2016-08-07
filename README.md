# SmallHydra

A small Hydra library for the ESP8266 using the ESPAsyncWebServer.
It handles reading the API graph from a SPIFFS file and sending the Link header.

## Requirements

SmallHydra uses the [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) to handle requests and [SmallRDF](https://github.com/bergos/smallrdf) to handle the RDF graph data.

## Example

The examples folder contains an example how to switch a LED on GPIO 0 on and off.
