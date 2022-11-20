#include <EEPROM.h>
#include <MemoryFree.h>
#include <avr/wdt.h>// watchdog timer
#include <SDI12.h>

//FIRMWARE VERSION.
#define F_Version 1

#define SERIAL_BAUD 115200 /*!< The baud rate for the output serial port */
#define DATA_PIN 69         /*!< The pin of the SDI-12 data bus -  PIN 69 (A15) SI ES CON LA UNIFICADA V2*/
#define POWER_PIN -1       /*!< The sensor power pin (or -1 if not switching power) */
#define WAKE_DELAY 0       /*!< Extra time needed for the sensor to wake (0-100ms) */

/** Define the SDI-12 bus */
SDI12 mySDI12(DATA_PIN);

/**
  '?' is a wildcard character which asks any and all sensors to respond
  'I' indicates that the myCommand wants information about the sensor
  '!' finishes the myCommand
*/
String myCommand;
unsigned char query[75];    //Datos SDI Recibidos
unsigned char query1[75];    //Datos SDI Para transmitir
float query2[18];   //Datos recibidos de la consulta
unsigned char query3[50];   //Datos preparados para transmisión
int bytes_received = 0;
int bytes_tx = 0;
byte Est_SDI = 0;
int numResults = 0;
uint8_t wait = 0;
String addr = "0";
uint8_t resultsReceived = 0;
uint8_t resultsExpected = 7;
uint8_t resultsTx = 0;
uint8_t cmd_number      = 0;
uint8_t dato_index      = 0;

void setup() {
  //desactiva el watchdog timer
  wdt_disable();

  Serial.begin(SERIAL_BAUD);  //Comunicación USB con PC: Serial0
  while (!Serial);
  
  Serial3.begin(1200);		//Comunicación con UIP: Serial1
  while (!Serial3);

  Serial.println("Opening SDI-12 bus...");
  mySDI12.begin();
  delay(500);  // allow things to settle

  // Power the sensors;
  if (POWER_PIN > 0) {
    Serial.println("Powering up sensors...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(200);
  }

	wdt_enable(WDTO_4S);
}

void loop() {
	
	wdt_reset();
	
	//Serial.print(F("FreeMemory = "));
	//Serial.println(freeMemory());
	
	ModoAT();
	Rx_UIP();
	Rx_SDI();
	
	if (Est_SDI == 0 || Est_SDI == 4)
	{
		  myCommand = "0M!";
		  Serial.println(F("Iniciando medición."));
		  mySDI12.sendCommand(myCommand);
		  Est_SDI = 1;
	}

}

void ModoAT()
{
	String archivo = "";
	String cadena = "";
	//String datos = "";
	
	if (Serial.available())
	{
		cadena = Serial.readString();
		if (cadena != "AT+DEBUG")
		{
			return;
		}
	}
	else
		return;
	
	Serial.println(F("Entrando a Modo AT"));
	//cli();
	long TimeoutAT = 120000;
	unsigned long TiempoAT = millis();
	while (TimeoutAT > millis() - TiempoAT)
	{
		wdt_reset();
		if(Serial.available())
		{
			cadena = Serial.readString();
			
			if (cadena == "AT")
			{
				TiempoAT = millis();
				Serial.println(F("OK"));
			}

			if (cadena == "AT+SDICMD")
			{
				TiempoAT = millis();
				Serial.println(cadena);
				while (Serial.available()<1 && millis() - TiempoAT < 60000)
				wdt_reset();
				delay(30);
				String cmd = Serial.readString();
				
				if (cmd == "0OOV!")
				{
					mySDI12.sendCommand(cmd);
					delay(30);
					
				}
				
			}			
			//if(cadena == "AT+DIR" || cadena == "AT+EDIR")
			//{
				//TiempoAT = millis();
				//File dir = SD.open("/");
				//if (dir != NULL)
				//{
					//File entry =  dir.openNextFile();
					//while (1)
					//{
						//wdt_reset();
						//entry =  dir.openNextFile();
						//if (entry == NULL)
						//break;
						//archivo = entry.name();
						//if (cadena == "AT+EDIR")
						//SD.remove(archivo);
						//else
						//Serial.println(archivo);
						//entry.close();
					//}
					//if (cadena == "AT+EDIR")
					//Serial.println(F("Directorio eliminado"));
					//else
					//Serial.println(F("terminado"));
				//}
			//}
			
			//if (cadena == "AT+DFILEX")
			//{
				//TiempoAT = millis();
				//Serial.println(cadena);
				//while (Serial.available()<11 && millis() - TiempoAT < 60000)
				//wdt_reset();
				//delay(30);
				//archivo = Serial.readString();
				////Serial.println(archivo);
				//File f = SD.open(archivo,FILE_READ);
				//Serial.print(F("Tamaño del archivo (bytes): "));
				//Serial.println(f.size());
				//
				//if (archivo == "POR_ENV_.TXT" || archivo == "TEMPO.TXT" )
				//{
//
					//Serial.print(F("Longit.: "));
					//Serial.println(pStation.get_clienteDataSize());
					//for (int i=0;  i < pStation.get_clienteDataSize() ; i++)
					//{
						//
						//Serial.print(F("Dato "));
						//Serial.print(i);
						//Serial.print(F(" : "));
						//unsigned int dato2 = f.read();
						//Serial.println(dato2);
					//}
					//
				//}
				//else
				//{
					//while (f.available())
					//{
						////datos = f.readStringUntil('\n');
						//Serial.println(f.readStringUntil('\n'));
						////delay(10);
					//}
					//
				//}
				//f.close();
				//Serial.println(F("terminado"));
			//}
			
			if (cadena == "AT+EEPROMW")
			{
				TiempoAT = millis();
				Serial.println(cadena);
				while (Serial.available()<3 && millis() - TiempoAT < 60000)
				wdt_reset();
				delay(30);
				String pos = Serial.readStringUntil(',');
				String data = Serial.readString();
				EEPROM.write(pos.toInt(), data.toInt());
				Serial.println(F("Hecho, no olvide reiniciar"));
			}
			
			if (cadena == "AT+EEPROMR")
			{
				TiempoAT = millis();
				Serial.println(cadena);
				while (Serial.available()<1 && millis() - TiempoAT < 60000)
				wdt_reset();
				delay(30);
				String pos = Serial.readString();
				
				if (pos.toInt() == 5)
				{
					Serial.println(F_Version);
				}
				else
				{
					int valor = EEPROM.read(pos.toInt());
					Serial.println(String(valor));
				}
			}
			
			if (cadena == "AT+EEREADALL")
			{
				TiempoAT = millis();
				Serial.println(F("Contenido de la EEPROM:"));
				for (int i= 1; i < 75 ; i++)
				{
					int valor = EEPROM.read(i);
					Serial.print(i);
					Serial.print(": ");
					Serial.println(String(valor));
					delay(20);
				}

			}
			
			//if (cadena == "AT+EFILEX")
			//{
				//TiempoAT = millis();
				//Serial.println(cadena);
				//while (Serial.available()<11 && millis() - TiempoAT < 60000)
				//wdt_reset();
				//delay(30);
				//archivo = Serial.readString();
				//SD.remove(archivo);
				//Serial.println(F("Archivo eliminado"));
			//}
			//
			//if (cadena == "AT+RTCW")
			//{
				//TiempoAT = millis();
				//Serial.println(cadena);
				//while (Serial.available()<10 && millis() - TiempoAT < 60000)
				//wdt_reset();
				//delay(30);
				//
				//unsigned int yearw = Serial.readStringUntil(':').toInt();
				//unsigned int monthw = Serial.readStringUntil(':').toInt();
				//unsigned int dayw = Serial.readStringUntil(' ').toInt();
				//unsigned int hourw = Serial.readStringUntil(':').toInt();
				//unsigned int minutew = Serial.readStringUntil(':').toInt();
				//unsigned int secondw = Serial.readString().toInt();
				//if (yearw < 0 || yearw > 99 || monthw < 1 || monthw > 12 || dayw < 1 || dayw > 31 || hourw < 0 || hourw > 23 || minutew < 0 || minutew > 59 || secondw < 0 || secondw > 59)
				//{
					//
					//Serial.println(F("Error en algún valor en el formato de fecha u hora (YY:MM:DD HH:MM:SS)."));
//
				//}
				//else
				//{
					//RTC.adjust(DateTime(yearw+2000, monthw, dayw, hourw, minutew, secondw));
					////pStation.set_recordCount(Function_Last_Idrecord(get_Month())+1);
					//Serial.print(yearw+2000);
					//Serial.print(F(":"));
					//Serial.print(monthw);
					//Serial.print(F(":"));
					//Serial.print(dayw);
					//Serial.print(F(" "));
					//Serial.print(hourw);
					//Serial.print(F(":"));
					//Serial.print(minutew);
					//Serial.print(F(":"));
					//Serial.println(secondw);
					//Serial.println(F("Hora actualizada, reiniciando"));
					//delay(1000);
					//cadena = "AT+RESET";
				//}
//
			//}
			//
			//if (cadena == "AT+RTCR")
			//{
				//DateTime now1 = RTC.now();
				//unsigned int actday, actmonth, actyear, acthour, actminute, actsecond;
//
				//actday=now1.day();
				//actmonth=now1.month();
//
				//if (now1.year() >= 2000)
				//actyear = now1.year()-2000;
				//else
				//actyear = now1.year()-1900;
				//acthour=now1.hour();
				//actminute=now1.minute();
				//actsecond=now1.second();
				//String hora = String(actyear)+"/"+String(actmonth)+"/"+String(actday)
				//+" "+String(acthour)+":"+String(actminute)+":"+String(actsecond);
				//Serial.println(hora);
			//}
			
			if (cadena == "AT+RESET")
			{
				wdt_enable(WDTO_15MS);
				while(1);
			}
			
			if (cadena == "AT+FIN")
			{
				Serial.println(F("Saliendo de Modo AT"));
				//sei();
				Serial.print(F("freeMemory(SaleAT)= "));
				Serial.println(freeMemory());
				return;
			}
		}
	}
	//sei();
	Serial.println(F("Saliendo de Modo AT"));
}

void Rx_UIP()
{
	String cadena = "";
	//String datos = "";
	
	if (Serial3.available())
	{
		//query[bytes_received] = Serial1.read();
		delay(100);
		cadena = Serial3.readString();
	}
	
	if (cadena == "") return;
	
	//Serial.print(F("Desde UIP: "));
	//Serial.println(cadena);
	
	wdt_reset();
	
	if (cadena == "CONSULTA") // 67 79 78 83 85 76 84 65
	{
		//Transmite los datos en memoria
		Serial.println(F("Se recibe consulta desde UIP, se transmiten los datos SDI12."));
		Serial3.flush();
		Serial3.write(query3,resultsTx);
		//bytes_tx = 0;
	}
	else
	{
		return;
	}
	
}

void Rx_SDI()
{
	String cadena = "";
	boolean datosSDI = false;
	
	//String datos = "";
	
	if (mySDI12.available())
	{
		if (Est_SDI == 1)
		{
			delay(100);
			cadena = mySDI12.readStringUntil('\n');
			cadena.trim();
			mySDI12.clearBuffer();
		}
		
		if (Est_SDI == 2)
		{
			delay(30);
			cadena = mySDI12.readStringUntil('\n');
			cadena.trim();
			mySDI12.clearBuffer();
		}
				
		if (Est_SDI == 3)
		{
			delay(30);
		    mySDI12.read();           // ignore the repeated SDI12 address
			char c = mySDI12.peek();  // check if there's a '+' and toss if so
			if (c == '+') { mySDI12.read(); }
		    while (mySDI12.available()) 
			{
				char c = mySDI12.peek();
				if (c == '-' || (c >= '0' && c <= '9') || c == '.') 
				{
					float result = mySDI12.parseFloat(SKIP_NONE);
					Serial.print(String(result, 10));
					query2[dato_index] = result;
					dato_index++;
					if (result != -9999) { resultsReceived++; }
				} 
				else if (c == '+') 
				{
					mySDI12.read();
					Serial.print(", ");
				} 
				else 
				{
					mySDI12.read();
				}
				delay(10);  // 1 character ~ 7.5ms
			}
			if (cmd_number < resultsExpected-1) 
			{ 
				Est_SDI = 2;
				Serial.print(", ");
				cmd_number++; 
			}
			
			//query[bytes_received] = mySDI12.read();
			//bytes_received++;
		}

		datosSDI = true;
		
	}
	
	if (!datosSDI) return;
	
	Serial.print(F("Desde sensor SDI12: "));
	Serial.println(cadena);
	
	wdt_reset();
	
	if (datosSDI)
	{
		switch (Est_SDI)
		{
			case (1):
			{
				//Procesa respuesta a Inicio de medición
				addr = cadena.substring(0, 1);
				Serial.print(addr);
				Serial.print(", ");

				// find out how long we have to wait (in seconds).
				wait = cadena.substring(1, 4).toInt();
				Serial.print(wait);
				Serial.print(", ");

				// Set up the number of results to expect
				numResults = cadena.substring(4).toInt();
				Serial.println(numResults);				
				Est_SDI = 2;
				break;
			}
			case (2):
			{
				if (cmd_number < resultsExpected) 
				{
					myCommand = "";
					myCommand += addr;
					myCommand += "D";
					myCommand += cmd_number;
					myCommand += "!";  // SDI-12 myCommand to get data [address][D][dataOption][!]
					mySDI12.sendCommand(myCommand, WAKE_DELAY);
					
					//myCommand = "0D0!";
					Serial.print(F("Solicitanto datos SDI12: "));
					Serial.println(myCommand);
					Est_SDI = 3;
					//mySDI12.sendCommand(myCommand);
				}
				else
				{
					Serial.println();
					mySDI12.clearBuffer();
					Est_SDI = 3;
					
				}
				
				break;
			}
			case (3):
			{
				Serial.println(F("Actualizando datos SDI12..."));
				uint8_t cuenta = 0;
				resultsTx = 0;
 				while (cuenta<resultsReceived)
 				{
 					 //query3[i]= int(query2[i]*10000);
					  
					query3[resultsTx] = uint16_t(query2[resultsTx]*10) & 0xFF;
					Serial.println(int(query3[resultsTx]));
					resultsTx++;
					query3[resultsTx] = uint16_t(query2[resultsTx]*10) >> 8;
					Serial.println(int(query3[resultsTx]));
					resultsTx++;
					cuenta++;
				}
// 				bytes_tx = bytes_received;
// 				bytes_received = 0;
//				resultsTx = resultsReceived*2;
				resultsReceived = 0;
				cmd_number = 0;
				Est_SDI = 4;
				break;
			}			
		}
	
		cadena = "";
	}
	else
	{
		return;
	}
	

	//Serial.println(F("Se recibe respuesta SDI12"));
	
}