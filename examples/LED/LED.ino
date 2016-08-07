#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Hydra.h>
#include <RDF.h>
#include <RDFNTriplesParser.h>
#include <RDFNTriplesSerializer.h>

const char* ssid = "";
const char* password = "";

void initWifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

RDFString rdfType("http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
RDFString dhLightClass("http://ns.bergnet.org/dark-horse#Light");
RDFString dhOffClass("http://ns.bergnet.org/dark-horse#Off"); 
RDFString dhOnClass("http://ns.bergnet.org/dark-horse#On");
RDFString dhState("http://ns.bergnet.org/dark-horse#state");
AsyncWebServer server(80);
Hydra hydra;

class Led {
 public:
  Led(int pin): pin(pin) {}

  void begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  bool isOn() {
    return digitalRead(pin) == LOW;
  }

  void on() {
    digitalWrite(pin, LOW);
  }

  void off() {
    digitalWrite(pin, HIGH);
  }

  void get(AsyncWebServerRequest* request) {
    RDFDocument document;

    const RDFString* iri = document.string(hydra.absoluteUrl(request), true);
    const RDFNamedNode* subject = document.namedNode(iri);

    const RDFQuad* typeQuad = document.triple(subject, document.namedNode(&rdfType), document.namedNode(&dhLightClass));

    RDFString* status = isOn() ? &dhOnClass : &dhOffClass;
    const RDFQuad* statusQuad = document.triple(subject, document.namedNode(&dhState), document.namedNode(status));

    const uint8_t* content = NTriplesSerializer::serialize_static(&document);

    AsyncWebServerResponse* response = request->beginResponse(200, "application/n-triples", (const char*)content);
    hydra.handleRequest(request, response);
    request->send(response);

    delete[] content;
  }

  void put(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    RDFDocument document;

    const RDFString* content = document.string(data, len);
    NTriplesParser::parse_static(content, &document);
  
    const RDFQuad* state = document.find(0, document.namedNode(&dhState));
  
    if (state) {
      if (state->object->value->equals(&dhOnClass)) {
        on();
      } else if (state->object->value->equals(&dhOffClass)) {
        off();
      }
    }

    get(request);
  }

 protected:
  int pin;
};

Led led0(0);

void setup () {
  Serial.begin(115200);
  initWifi();
  SPIFFS.begin();
  hydra.begin("/api.nt", "api");
  led0.begin();

  server.on("/api", HTTP_GET, [&hydra](AsyncWebServerRequest* request) {
    Serial.println("GET /api");

    hydra.handleApiRequest(request);
  });

  server.on("/", HTTP_GET, [&led0](AsyncWebServerRequest* request) {
    Serial.println("GET /");

    led0.get(request);
  });

  server.on("/", HTTP_PUT, [](AsyncWebServerRequest* request) {
  }, [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
  }, [&led0](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    Serial.println("PUT /");

    led0.put(request, data, len);
  });

  server.begin();

  Serial.println("http://" + WiFi.localIP().toString() + "/");
}

void loop () {
}
