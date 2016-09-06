/*
  Copyright (c) 2012, 2016 Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

  MySQL_Connection.cpp - Library for communicating with a MySQL Server over
                         Ethernet. (formerly mysql.cpp)

  Change History:

  Version 1.0.0a Created by Dr. Charles A. Bell, April 2012.
  Version 1.0.0b Updated by Dr. Charles A. Bell, October 2013.
  Version 1.0.1b Updated by Dr. Charles A. Bell, February 2014.
  Version 1.0.2b Updated by Dr. Charles A. Bell, April 2014.
  Version 1.0.3rc Updated by Dr. Charles A. Bell, March 2015.
  Version 1.0.4ga Updated by Dr. Charles A. Bell, July 2015.
  Version 1.1.0a Created by Dr. Charles A. Bell, January 2016.
  Version 1.1.1a Created by Dr. Charles A. Bell, January 2016.
*/
#include <Arduino.h>
#include <EMySQL_Connection.h>
#include <EMySQL_Encrypt_Sha1.h>

#define MAX_CONNECT_ATTEMPTS 10

const char CONNECTED[] PROGMEM = "*** Connected to server version ";
const char DISCONNECTED[] PROGMEM = "*** Disconnected.";

/*
  connect - Connect to a MySQL server.

  This method is used to connect to a MySQL server. It will attempt to
  connect to the server as a client retrying up to MAX_CONNECT_ATTEMPTS.
  This permits the possibility of longer than normal network lag times
  for wireless networks. You can adjust MAX_CONNECT_ATTEMPTS to suit
  your environment.

  server[in]      IP address of the server as IPAddress type
  port[in]        port number of the server
  user[in]        user name
  password[in]    (optional) user password

  Returns boolean - True = connection succeeded
*/
boolean MySQL_Connection::connect(IPAddress server, int port, char *user,
                                  char *password)
{
  int connected = 0;
  int i = -1;

  // Retry up to MAX_CONNECT_ATTEMPTS times 1 second apart.
  do {
    mysdelay(1000);
    connected = client->connect(server, port);
    i++;
  } while (i < MAX_CONNECT_ATTEMPTS && !connected);

  if (connected) {
    //Serial.print("[0]");
    read_packet();
    if (buffer == NULL) return false;
    //Serial.print("[1]");
    parse_handshake_packet();
    //Serial.print("[2]");
    send_authentication_packet(user, password);
    //Serial.print("[3]");
    read_packet();
    if (buffer == NULL) return false;
    //Serial.print("[4]");
    if (check_ok_packet() != 0) {
      parse_error_packet();
      //Serial.println("#1");
      return false;
    }
    //#%$ show_error(CONNECTED);Serial.println(server_version);
    free(server_version); // don't need it anymore
    //Serial.println("#2");
    return true;
  }
  //Serial.println("#3");
  return false;
}

/*
  close - cancel the connection

  This method closes the connection to the server and frees up any memory
  used in the buffer.
*/
void MySQL_Connection::close()
{
  if (connected())
  {
    client->flush();
    client->stop();
    //№;%show_error(DISCONNECTED, true);
  }
}
