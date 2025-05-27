#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRutils.h>

// Pin definitions
const uint16_t kRecvPin = 15; // IR receiver pin
const uint16_t kSendPin = 4;  // IR LED pin

// IR objects
IRrecv irrecv(kRecvPin);
IRsend irsend(kSendPin);
decode_results results;

// Protocol state
decode_type_t detectedProtocol = UNKNOWN;
bool protocolSupported = false;

// AC state
IRac ac(kSendPin);

void detectProtocol() {
  if (irrecv.decode(&results)) {
    String protoName = typeToString(results.decode_type);
    Serial.print("Detected protocol: ");
    Serial.println(protoName);

    if (IRac::isProtocolSupported(results.decode_type)) {
      detectedProtocol = results.decode_type;
      protocolSupported = true;
      Serial.println("Protocol supported! You can now send commands via Serial (on/off/temp XX).");
      ac.next.protocol = detectedProtocol;
      ac.next.model = 1; // Default model, may need adjustment for some brands
      ac.next.power = true;
      ac.next.degrees = 24;
      ac.next.mode = stdAc::opmode_t::kCool;
      ac.next.fanspeed = stdAc::fanspeed_t::kAuto;
      ac.next.swingv = stdAc::swingv_t::kAuto;
      ac.next.swingh = stdAc::swingh_t::kAuto;
    } else {
      detectedProtocol = UNKNOWN;
      protocolSupported = false;
      Serial.println("Protocol not supported by IRac.");
    }
    irrecv.resume();
  }
}

void handleSerialCommand() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  // Only allow sending if protocol is valid and supported
  if (protocolSupported && detectedProtocol != UNKNOWN) {
    if (cmd.equalsIgnoreCase("on")) {
      ac.next.power = true;
      Serial.println("Sending AC ON command...");
      ac.sendAc();
    } else if (cmd.equalsIgnoreCase("off")) {
      ac.next.power = false;
      Serial.println("Sending AC OFF command...");
      ac.sendAc();
    } else if (cmd.startsWith("temp ")) {
      int temp = cmd.substring(5).toInt();
      if (temp >= 16 && temp <= 30){
        ac.next.degrees = temp;
        Serial.print("Set temperature to ");
        Serial.println(temp);
        ac.sendAc();
      } else {
        Serial.println("Temperature out of range (16-30.");
      }
    } else {
      Serial.println("Unknown command. Use 'on', 'off', or 'temp XX'.");
    }
  } else {
    Serial.println("No valid AC protocol detected. Please point your AC remote and press a button first.");
  }

  // Always clear the input buffer
  while (Serial.available()) Serial.read();
}

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  irsend.begin();
  Serial.println("Point your AC remote and press a button to detect protocol.");
  Serial.println("After detection, use Serial commands: on, off, temp XX");
}

void loop() {
  detectProtocol();
  handleSerialCommand();
}