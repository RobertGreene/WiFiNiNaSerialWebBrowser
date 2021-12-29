#include <WiFiNINA.h>

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

String ssid = ""; // your default network SSID (name)
String pass = ""; // your default network password (use for WPA, or use as key for WEP)
int keyIndex = 0; // your default network key Index number (needed only for WEP)

// default looping command has the url as normally formed plus -d for deliminater to parse for us
String loop_command = "";
int loop_delay = 5000; // 5000 ms or 5 seconds between loop commands

// default ports
int port = 80;
int port_ssl = 443;

int status = WL_IDLE_STATUS;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 443 is default for HTTPS, and port 80 for HTTP):
WiFiSSLClient clientSSL;
WiFiClient client;

// global variables and functions
String ret = "";
String cmd = "";
String splitAt(String d1, String delim);
String splitBefore(String d1, String delim);
String checkStream(String delim);
void makeWifiConnection();
bool connectToHost(String host);
void makeGetRequest(String host, String request);
String doGetRequest(String delim, String host, String request);
String checkStreamSSL(String delim);
bool connectToSSLHost(String host);
void makeGetRequestSSL(String host, String request);
String doSSLGetRequest(String delim, String host, String request);
String getSerialCommand();
void processCommandsStreams();
void printWifiStatus();
void listNetworks();
bool hasConnected = false;
void printHelp();
void printMacAddress();
void printEncryptionType(int thisType);
bool logging = false;
bool defaultConnect = false;
bool firstRun = true;
String split_return[255];

// function to process the returned String from your web request
void returnCallback(String returned, String host, String req, String delim);

// help method or ? in the serial console
void printHelp(){
  Serial.println("Commands are:");
  Serial.println("getIP\tgets internet ip");
  Serial.println("getDate\tgets the date");
  Serial.println("getNow\tgets time and date");
  Serial.println("loop <command>\tie: loop https://gandpgaming.com:443/ip.asp?fromArduino=12 or loop singular to turn off the loop");
  Serial.println("loop_delay <blank/value>\tgets without a value and sets with");
  Serial.println("scan, list, networks\tprints out a list of networks");
  Serial.println("ssid <name of network>\tsets the Name of the Network to be connected");
  Serial.println("password <password of network>\tsets the Password of the Network to be connected");
  Serial.println("connect\tconnects to the network after you have set both SSID and Password for the network");
  Serial.println("http://host:port/page?field=value\tnormally formed urls ");
  Serial.println("https://host:port/page?field=value\tSSL formed urls ");
  Serial.println("wifi, status, network\tprint out your current network values");
  firstRun = false;  
}

// below are the serial commands and loop processing methods
void processCommandsStreams() {
  // printhelp when it first starts
  if (firstRun)
    printHelp();
  
  // start out with a fresh command
  cmd = "";

  // if the serial console is open
  if (Serial)
    cmd = getSerialCommand();

  if (cmd.equals("") && loop_command.equals("")) {
    // do nothing
  } else {

    // Serial command came in or we have a loop_command
    cmd.trim();
    if (!loop_command.equals("") && cmd.equals("") ) {
      delay(loop_delay);
      cmd = loop_command;
    }

    if (logging)
      Serial.println(cmd);

    // get your ip ie getip
    if (cmd.equalsIgnoreCase("getip")) {
      if (logging)
        Serial.println("Making request for IP");
      port = 80;
      ret = doGetRequest("Content-Length: ", "gandpgaming.com", "/ip.asp");
      ret = splitAt(ret, "\n");
      ret.trim();

      if (logging) {
        Serial.print("Your IP is ");
        Serial.println(ret);
        Serial.println("");
      }
    }

    // get date PST ie getdate
    if (cmd.equalsIgnoreCase("getdate")) {
      if (logging)
        Serial.println("Making request for DATE");
      port = 80;
      ret = doGetRequest("Content-Length: ", "gandpgaming.com", "/date.asp");
      ret = splitAt(ret, "\n");
      ret.trim();

      if (logging) {
        Serial.print("The date is ");
        Serial.print(ret);
        Serial.println(" PST");
      }
    }

    // get now PST ie getnow
    if (cmd.equalsIgnoreCase("getnow")) {
      if (logging)
        Serial.println("Making request to get Time and Date PST");
      port = 80;
      ret = doGetRequest("Content-Length: ", "gandpgaming.com", "/now.asp");
      if (logging) {
        Serial.print("It is ");
        ret = splitAt(ret, "\n");
        ret.trim();
        Serial.print(ret);
        Serial.println(" PST");
      }
    }

    // print wifi status ie: wifi, status, network
    if (cmd.equalsIgnoreCase("wifi") || cmd.equalsIgnoreCase("status") || cmd.equalsIgnoreCase("network")) {
      if (logging)
        printWifiStatus();
    }

    // print network list ie: list, scan, networks
    if (cmd.equalsIgnoreCase("list") || cmd.equalsIgnoreCase("networks") || cmd.equalsIgnoreCase("scan")) {
      if (logging)
        listNetworks();
    }

    // make a Wifi Connection ie: connect
    if (cmd.equalsIgnoreCase("connect") )
      makeWifiConnection();

    if (cmd.equalsIgnoreCase("help") || cmd.equals("?")){
      if (Serial)
        printHelp();
    }

    // print what the loop_delay is ie: loop_delay
    if (cmd.indexOf("loop_delay") == 0) {
      if (logging) {
        Serial.print("Is ");
        Serial.println(loop_delay);
      }
    }

    // set loop_delay <millisecond delay>
    if (cmd.indexOf("loop_delay ") == 0) {
      String delayStr = splitAt(cmd, "loop_delay ");

      int str_len = delayStr.length() + 1;
      char dbuf[str_len];

      delayStr.toCharArray(dbuf, str_len);
      loop_delay = atoi(dbuf);

      if (logging) {
        Serial.print("Now ");
        Serial.println(loop_delay);
      }
    }

    // set loop <command> ie loop https://gandpgaming.com/ip.asp?aid=1
    if (cmd.indexOf("loop") == 0 ) {
      if (cmd.equalsIgnoreCase("loop")) {
        // stop the loop command
        cmd = "";
        loop_command = "";
        if (logging)
          Serial.println("Turned the command loop off");
      } else {
        if (cmd.indexOf("loop ") == 0 ) {
          // set the loop command
          cmd = splitAt(cmd, "loop ");
          loop_command = cmd;
        }
      }
    }

    // ssid <name of network>
    if (cmd.indexOf("ssid") == 0 ) {
      if (cmd.equalsIgnoreCase("ssid")) {
        // stop the loop command
        cmd = "";
        ssid = "";
        if (logging)
          Serial.println("Turned off reconnect without SSID");
      } else {
        if (cmd.indexOf("ssid ") == 0 ) {
          // set the loop command
          cmd = splitAt(cmd, "ssid ");
          ssid = cmd;
        }
      }
    }

    // password <password>
    if (cmd.indexOf("password ") == 0 ) {
      if (cmd.equalsIgnoreCase("password")) {
        // stop the password
        cmd = "";
        pass = "";
        if (logging)
          Serial.println("Password for SSID set to nothing");
      } else {
        if (cmd.indexOf("password ") == 0 ) {
          // set the password command
          cmd = splitAt(cmd, "password ");
          pass = cmd;
        }
      }
    }

    // make a non-ssl request to server - http://host:port/page?field=value or http://host/page?field=value
    if (cmd.indexOf("http://") == 0) {
      String host = splitAt(cmd, "http://");
      String req = "/";
      String delim = "Content-Length: ";

      if (host.indexOf("/") > -1)
        req = "/" + splitAt(host, "/");

      if (cmd.indexOf(" -d") > -1) {
        delim = splitAt(cmd, " -d");
        delim.trim();
        if (delim.equals(""))
          delim = "Content-Length: ";
        else
          req = splitBefore(req, " -d");
      }

      if (host.indexOf("/") > -1)
        host = splitBefore(host, "/");

      if (host.indexOf(":") > -1) {
        String portStr = splitAt(host, ":");
        char ps[portStr.length() + 1];
        portStr.toCharArray(ps, portStr.length() + 1);
        port = atoi(ps);
        host = splitBefore(host, ":");
      } else {
        port = 80;
      }

      if (logging) {
        Serial.print("Host: ");
        Serial.print(host);
        Serial.print(", Port: ");
        Serial.print(port);
        Serial.print(",  Request: ");
        Serial.print(req);
        Serial.print(", Delim: ");
        Serial.println(delim);
      }

      ret = doGetRequest(delim, host, req);
      if (delim.equals("Content-Length: "))
        ret = splitAt(ret, "\n");
      ret.trim();

      if (logging) {
        Serial.print("Returned: ");
        Serial.println(ret);
        Serial.println("");
      }
      returnCallback(ret, host, req, delim);

    }

    // make a ssl request to server - https://host:port/page?field=value or https://host/page?field=value
    if (cmd.indexOf("https://") == 0) {
      String host = splitAt(cmd, "https://");
      String req = "/";
      String delim = "Content-Length: ";

      if (host.indexOf("/") > -1)
        req = "/" + splitAt(host, "/");

      if (cmd.indexOf(" -d") > -1) {
        delim = splitAt(cmd, " -d");
        delim.trim();
        if (delim.equals(""))
          delim = "Content-Length: ";
        else
          req = splitBefore(req, " -d");
      }

      if (host.indexOf("/") > -1)
        host = splitBefore(host, "/");

      if (host.indexOf(":") > -1) {
        String portStr = splitAt(host, ":");
        char ps[portStr.length() + 1];
        portStr.toCharArray(ps, portStr.length() + 1);
        port_ssl = atoi(ps);
        host = splitBefore(host, ":");
      } else {
        port_ssl = 443;
      }

      if (logging) {
        Serial.print("Host: ");
        Serial.print(host);
        Serial.print(", Port: ");
        Serial.print(port_ssl);
        Serial.print(",  Request: ");
        Serial.print(req);
        Serial.print(", Delim: ");
        Serial.println(delim);
      }

      ret = doSSLGetRequest(delim, host, req);
      if (delim.equals("Content-Length: "))
        ret = splitAt(ret, "\n");
      ret.trim();

      if (logging) {
        Serial.print("Returned: ");
        Serial.println(ret);
        Serial.println("");
      }
      returnCallback(ret, host, req, delim);

    }

  }

}

String getSerialCommand() {
  String retCmd = "";
  if (Serial.available())
    retCmd = Serial.readString();
  return retCmd;
}

String doSSLGetRequest(String delim, String host, String request) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!connectToSSLHost(host))
      return "Bad Request";
    makeGetRequestSSL(host, request);
    return checkStreamSSL(delim);
  }
  return "Not connected to Wifi(type scan then enter, ssid <name of the network> then enter, password <networks password>, connect then enter)";
}

String doGetRequest(String delim, String host, String request) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!connectToHost(host))
      return "Bad Request";
    makeGetRequest(host, request);
    return checkStream(delim);
  }
  return "Not connected to Wifi(type scan then enter, ssid <name of the network> then enter, password <networks password>, connect then enter)";
}

void makeWifiConnection() {

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    if (logging)
      Serial.println("WiFi shield not present");
    // don't continue:
    while (true)
      delay(1000);
  }

  // attempt to connect to Wifi network
  while (status != WL_CONNECTED) {
    if (logging) {
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
    }
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network
    char ss_id[ssid.length() + 1];
    ssid.toCharArray(ss_id, ssid.length() + 1);
    char pa_ss[pass.length() + 1];
    pass.toCharArray(pa_ss, pass.length() + 1);
    status = WiFi.begin(ss_id, pa_ss);
    // wait 10 seconds for connection:
    delay(10000);
  }

  if (logging)
    Serial.println("Connected to wifi");
  printWifiStatus();

}

bool connectToSSLHost(String host) {

  int str_len = host.length() + 1;
  char host_array[str_len];
  host.toCharArray(host_array, str_len);

  if (clientSSL.connect(host_array, port_ssl)) {
    if (logging) {
      Serial.print("Connected to ");
      Serial.println(host);
    }
  } else {
    if (logging) {
      Serial.print("Failed to connect to ");
      Serial.println(host);
      return false;
    }
  }
  return true;

}

bool connectToHost(String host) {

  int str_len = host.length() + 1;
  char host_array[str_len];
  host.toCharArray(host_array, str_len);

  if (client.connect(host_array, port)) {
    if (logging) {
      Serial.print("Connected to ");
      Serial.println(host);
    }
  } else {
    if (logging) {
      Serial.print("Failed to connect to ");
      Serial.println(host);
      return false;
    }
  }
  return true;

}

void makeGetRequestSSL(String host, String request) {
  // Make a HTTP request:
  clientSSL.print("GET ");
  clientSSL.print(request) ;
  clientSSL.print(" HTTP/1.1 \n");
  clientSSL.print("Host: ");
  clientSSL.print(host);
  clientSSL.print("\n");
  clientSSL.println("Connection: close");
  clientSSL.println();
  if (logging)
    Serial.println("Request sent");
}

String checkStreamSSL(String delim) {

  String retStr = "Request Timed Out";
  int i = 0;

  while (!clientSSL.available()) {
    // 140 seconds in all for request timeout
    delay(70);
    if (i == 2000) {
      clientSSL.stop();
      return retStr;
    }
    i++;
  }

  while (clientSSL.available()) {
    String readLine = clientSSL.readString();
    if (readLine.indexOf(delim) > -1) {
      retStr = splitAt(readLine, delim);
      continue;
    }
  }

  clientSSL.stop();
  return retStr;

}

void makeGetRequest(String host, String request) {
  // Make a HTTP request:
  client.print("GET ");
  client.print(request) ;
  client.print(" HTTP/1.1 \n");
  client.print("Host: ");
  client.print(host);
  client.print("\n");
  client.println("Connection: close");
  client.println();
  if (logging)
    Serial.println("Request sent");
}

String checkStream(String delim) {

  String retStr = "Request Timed Out";
  int i = 0;

  while (!client.available()) {
    delay(70);
    if (i == 20) {
      client.stop();
      return retStr;
    }
    i++;
  }

  while (client.available()) {
    String readLine = client.readString();
    if (readLine.indexOf(delim) > -1) {
      retStr = splitAt(readLine, delim);
      continue;
    }
  }

  client.stop();
  return retStr;

}

String splitAt(String d1, String delim) {
  int pos = d1.indexOf(delim) + delim.length();
  if (pos < -1)
    return "";
  return d1.substring(pos);
}

int split(String str, String delim){
  
  int stringCount=0;
  
  while (str.length() > 0)  {
    int index = str.indexOf(delim);
    if (index == -1) {
      split_return[stringCount++] = str;
      break;
    } else {
      split_return[stringCount++] = str.substring(0, index);
      str = str.substring(index+1);
    }
  }

  return stringCount-1;
    
}

String splitBefore(String d1, String delim) {
  int pos = d1.indexOf(delim);
  if (pos < -1)
    return "";
  return d1.substring(0, pos);
}

void printWifiStatus() {
  if (logging) {
    // print the SSID of the network you're attached to
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    printMacAddress();
    if (WiFi.status() == WL_CONNECTED)
      hasConnected = true;

  }
}

void listNetworks() {
  // will not be able to scan while connected
  if (WiFi.status() == WL_CONNECTED){
    Serial.println("Already connected");
    return;    
  }
  
  // scan for nearby networks
  Serial.println("Scanning Networks");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get a WiFi connection");
    return;
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);
  Serial.println("");

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print("\tNetwork Name: ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
  }
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
    case ENC_TYPE_UNKNOWN:
    default:
      Serial.println("Unknown");
      break;
  }
}

void printMacAddress() {
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
  Serial.println("");
}
