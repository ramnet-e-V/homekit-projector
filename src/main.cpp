#include "HomeSpan.h" // include the HomeSpan library
#include <SoftwareSerial.h>

struct Projector : Service::Outlet
{

  SpanCharacteristic *projector;   // store a reference to the On Characteristic
  SoftwareSerial *projectorSerial; // store a reference to the serial port for the projector

  Projector() : Service::Outlet()
  {
    projectorSerial = new SoftwareSerial(4, 5); // RX, TX
    projectorSerial->begin(9600);
    projector = new Characteristic::On(); // instantiate the On Characteristic and save it as projector
  }

  boolean update()
  { // update() method
    if (projector->getNewVal())
    {
      // Turn projector on
      projectorSerial->print("\r*pow=on#\r");
      LOG1("Sent power ON command to projector\n");
    }
    else
    {
      // Turn projector off
      projectorSerial->print("\r*pow=off#\r");
      LOG1("Sent power OFF command to projector\n");
    }
    return (true); // indicate update was successful
  }

  void loop()
  {

    if (projector->timeVal() > 10000)
    { // check time elapsed since last update, must be greater than 5 seconds

      // Query projector power state
      projectorSerial->print("\r*pow=?#\r");
      LOG1("Sent power status query to projector\n");

      delay(100); // small delay to allow response to arrive

      // Read response from the projector
      String response = "";
      while (projectorSerial->available())
      {
        char c = projectorSerial->read();
        response += c;
      }

      response.trim(); // remove any extra spaces/newlines
      LOG1("Projector Response: ");
      LOG1(response);
      LOG1("\n");

      int powerState = 0; // assume off by default

      // Check if response contains "*POW=ON#" or "*POW=OFF#"
      if (response.indexOf("*POW=ON#") != -1)
      {
        powerState = 1; // projector is on
      }
      else if (response.indexOf("*POW=OFF#") != -1)
      {
        powerState = 0; // projector is off
      }

      projector->setVal(powerState); // update HomeKit characteristic

      LOG1("Power State Updated: ");
      LOG1(powerState);
      LOG1("\n");
    }

  } // end loop
};

void setup()
{
  Serial.begin(115200); // start the Serial interface

  homeSpan.begin(Category::Outlets, "Projector", "Projector"); // initialize HomeSpan

  new SpanAccessory(); // Projector Accessory

  new Service::AccessoryInformation(); // HAP requires every Accessory to implement an AccessoryInformation Service
  new Characteristic::Identify();      // HAP requires the Accessory Information Service to include the Identify Characteristic

  new Projector(); // Create the Projector service

} // end of setup()

void loop()
{
  homeSpan.poll(); // HomeSpan polling loop
} // end of loop