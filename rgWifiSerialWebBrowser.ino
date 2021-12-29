#include "rgWiFiSerialWebBrowser.h"

/*******************************************************************
    Arduino WiFiNina Serial Web Browser

    Author: Robert Maxwell Greene
    Email: details@gandpgaming.com
    Text: 423 572 1618
    Date: 12/29/2021
    
    Permission to use, copy, modify, and/or distribute this software
    for any purpose with or without fee is hereby granted.

   Copyright (c) 2021-2022 Robert M. Greene
   
   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
   THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
   FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
   CONTRACT,NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *******************************************************************/


void setup() {

  // Network
  ssid = ""; // your default network SSID (name)
  pass = ""; // your default network password (use for WPA, or use as key for WEP)
  keyIndex = 0; // your default network key Index number (needed only for WEP)

  // Default loop command 
  loop_command = ""; // url to hit make sure it starts with http:// or https:// as normally formed
  loop_delay = 5000; // 5000 ms or 5 seconds between loop commands
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  if (logging) {
    while (!Serial) {
      // wait for serial port to connect. Needed for native USB port only
    }
  }

  // trim and setup defaultConnect
  loop_command.trim();
  ssid.trim();
  pass.trim();
  defaultConnect = (!pass.equals("") && !ssid.equals(""));

}

void loop() {

  // when serial is up and connected then talk to the serial
  if (Serial && !logging)
    logging = true;
  else if (!Serial && logging)
    logging = false;

  // process commands and streams
  processCommandsStreams();
  
  // no wifi connection so we connect if ssid is set and has been connected after boot
  if (WiFi.status() != WL_CONNECTED) {
    if (hasConnected || defaultConnect ) {
      if (!ssid.equals(""))
        makeWifiConnection();
    }
  }

  delay(25);
}

// function to process the returned String from your web request
void returnCallback(String returned, String host, String req, String delim) {
  // process the returned wifi client web call that was made
  // splitAt does everything after deliminater and splitBefore does whats before the deliminater, between these we can parse the results
  // split returns to the substrings of split_return[255]; max 255 splits the split returns the string count
  // https://gandpgaming.com/ip.asp
  // example
  if (req.indexOf("ip.asp")>-1){  
    int split_count = split(returned, ".");
    if (split_count>0){
      for (int i=0; i<=split_count;i++)
        Serial.println(split_return[i]);
    }
  }
  
  Serial.println("Returned from callback function: ");
  Serial.println(returned);
  

}
