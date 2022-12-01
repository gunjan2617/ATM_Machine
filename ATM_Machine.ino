//  Import all the libraries that are required for the code.
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// ssid and password for esp32 to connect to.
const char* ssid = "Redmi";
const char* password = "Radhekrishna@21";

const int pin_led_input = 2;

// Flags for various parameters
int login_status = 0;
int password_status = 0;
int loggedin_status = 0;
int withdraw_status = 0;
int deposit_status = 0;
int count = 0;
int open_bal = 15000;
int pin_update_status = 0;

//Globalizing  user_id and pin
String user_id = "";  // saving user id/account number in a variable
String pin = "";      // saving pin in a variable

//Gloabalizing balance
String amount = "";  // saving balance in a variable

// Initializing Telegram BOT
#define bot_token_from_telegram "5926470876:AAEgxMBb3YM8_fviAkkxa1jCYHf4a66TVWQ"  // My bot token obtained from bot father of telegram.

// Google script id for each operation
String GOOGLE_SCRIPT_IDupdate = "AKfycbzAPy6HfYkI3rFZ8P4pITwQS9JNcOZHvpxCkdLtGM6EbS9VH56pre-ba5vV2aeQ4tR5Tw";
String GOOGLE_SCRIPT_IDcheckUsr = "AKfycbwnsNDYXCqipEguRIxD4qylzNjDtygOloERDUs0WqtTczPOcJdKxLOLaBPOsLLXQyoLrA";
String GOOGLE_SCRIPT_IDcheckPIN = "AKfycbzNRU8QaoJTuEeGwONU-brODOujmYtGDBVnUiojXsfZ--XYzzSyGIM91zGOTbGyqYnIXQ";
String GOOGLE_SCRIPT_IDcheckBal = "AKfycbwDGPrabbyWgbXGBs8nURwgdrwFwFe3bCqg5XRVYxlKmBzzF8UUCvA_1T-fm3p8c24QMQ";
String GOOGLE_SCRIPT_IDchecktrans = "AKfycbwV5Mj98n_2_kuq5vSXzgI7ySem6KYTWGyMFTU-5JrzUMMmKuUde1tLokMDQlFrLwUiNA";
String GOOGLE_SCRIPT_IDupdatepin = "AKfycbxV7xV3NEBIKWZrxy3W0RdbYoBfS6zgTcgpD0NmWGbPhjAYJkTqyYN8yRT03tk7IK0W5Q";

#define chat_id_from_telegram "1664626527"  // my chat id, obtained from telegram

WiFiClientSecure client;
UniversalTelegramBot bot(bot_token_from_telegram, client);

// Checking the new messages on telegram after each second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


// This functions handle the received messages.
void Handling_new_messages_from_telegram(int new_msgs) {
  Serial.println("Handling_new_messages_from_telegram");
  Serial.println(String(new_msgs));

  for (int i = 0; i < new_msgs; i++) {
    String chat_id = String(bot.messages[i].chat_id);  // Chat id of the requester
    if (chat_id != chat_id_from_telegram) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;  // Printing the received message on the serial monitor
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    // Checking the recieved message if it matches with any of the commands

    if (text == "/start") {

      String welcome_message = "Hey, Welcome to ATM70, " + from_name + ".\n";
      welcome_message += "Use following command to login.\n\n";
      welcome_message += "/login to login to your account.\n\n";
      welcome_message += "Happy using ATM70! \n";
      bot.sendMessage(chat_id, welcome_message, "");
    }

    // Taking account number of the user as input and proceeding for password if it matches
    else if (text == "/login") {
      login_status = 1;
      bot.sendMessage(chat_id, "Enter your account number.", "");
    }

    else {
      if (login_status == 1) {
        HTTPClient http;
        String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDcheckUsr + "/exec?read";
        Serial.println("Making a request");
        http.begin(url.c_str());  // Specifying the URL and certificate
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        String read_data_from_sheet;
        if (httpCode > 0) {                         // Check for the returning code
          read_data_from_sheet = http.getString();  // Reading the data from the google spredsheet
          Serial.println(httpCode);
          Serial.println(read_data_from_sheet);
          if (read_data_from_sheet == text) {
            user_id = text;                                 //Input from the telegram bot is stored in a variable
            bot.sendMessage(chat_id, "Enter the pin", "");  // If valid account number show this message
            password_status = 1;
          } else {
            bot.sendMessage(chat_id, "Invalid account number. Try again!", "");  // If account number not found, show this message.
          }
        } else {
          bot.sendMessage(chat_id, "Couldn't process login request, Try again!", "");  //If timeout send this message
        }
        login_status = 0;  // Changing the flag
      }

      else if (password_status == 1) {
        HTTPClient http;
        String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDcheckPIN + "/exec?read";
        Serial.println("Making a request");
        http.begin(url.c_str());  // Specify the URL and certificate
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        String read_data_from_sheet;
        if (httpCode > 0) {  // Check for the returning code
          read_data_from_sheet = http.getString();
          Serial.println(httpCode);
          Serial.println(read_data_from_sheet);
          if (read_data_from_sheet == text) {
            pin = text;
            bot.sendMessage(chat_id, "Logged in successfully!!", "");  //If the pin is correct show this message
            digitalWrite(pin_led_input, HIGH);                         // Blink the LED after every successful operation
            delay(1000);                                               // Delay for blinking of the LED
            digitalWrite(pin_led_input, LOW);
            loggedin_status = 1;  //Changing the flag status
          } else {
            bot.sendMessage(chat_id, "Incorrect Pin. Try again!", "");  // If the pin is incorrect show this message
          }
        } else {
          bot.sendMessage(chat_id, "Couldn't process pin request. Try again!", "");  //If timeout send this message
        }
        password_status = 0;  //Chaning the flag
      }
      if (loggedin_status == 1) {
        if (text == "/logout") {
          if (loggedin_status == 1) {
            loggedin_status = 0;
            bot.sendMessage(chat_id, "Logged out successfully.", "");  // On logging out successfully
            digitalWrite(pin_led_input, HIGH);
            delay(1000);
            digitalWrite(pin_led_input, LOW);
          } else {
            bot.sendMessage(chat_id, "Login first!", "");  // If trying to log out without logging in
          }
        }
        // For checking the balance
        if (text == "/Balance") {
          HTTPClient http;
          String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDcheckBal + "/exec?read";
          Serial.println("Making a request");
          http.begin(url.c_str());  // Specify the URL and certificate
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          String read_data_from_sheet;
          if (httpCode > 0) {                         // Check for the returning code
            read_data_from_sheet = http.getString();  //Read data from the google spreadsheet
            Serial.println(httpCode);
            Serial.println(read_data_from_sheet);
            bot.sendMessage(chat_id, read_data_from_sheet, "");
            digitalWrite(pin_led_input, HIGH);
            delay(1000);
            digitalWrite(pin_led_input, LOW);
          } else {
            bot.sendMessage(chat_id, "Couldn't process balance request.", "");  // If timeout show this message
          }
        }
        // For withdrawing the money from the account
        if (text == "/Withdraw") {
          bot.sendMessage(chat_id, "Enter amount to be withdrawn.", "");
          withdraw_status = 1;
        } else if (withdraw_status == 1) {
          HTTPClient http;
          String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDcheckBal + "/exec?read";
          Serial.println("Making a request");
          http.begin(url.c_str());  // Specify the URL and certificate
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          String read_data_from_sheet;
          if (httpCode > 0) {  // Check for the returning code
            read_data_from_sheet = http.getString();
            Serial.println(httpCode);
            Serial.println(read_data_from_sheet);
            if (read_data_from_sheet.toInt() >= text.toInt()) {
              count++;  // Increasing the count for counting number of transactions
              int bal1 = ((read_data_from_sheet.toInt()) - (text.toInt()));
              bot.sendMessage(chat_id, "Withdrawn successfully!!", "");
              digitalWrite(pin_led_input, HIGH);
              delay(1000);
              digitalWrite(pin_led_input, LOW);
              String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDupdate + "/exec?" + "&usrname=" + user_id + "&pin=" + pin + "&bal=" + String(bal1) + "&deposit=0" + "&withdraw=" + text + "&num=" + String(count);
              Serial.print("POST data to spreadsheet:");
              Serial.println(urlFinal);
              HTTPClient http;
              http.begin(urlFinal.c_str());
              http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
              int httpCode = http.GET();
              Serial.print("HTTP Status Code: ");
              Serial.println(httpCode);
              http.end();
            } else {
              bot.sendMessage(chat_id, "Sorry! You don't have sufficient balance to withdraw.", "");  // If withdrawing amount is greater than the current balance
            }
          } else {
            bot.sendMessage(chat_id, "Couldn't process withdraw request.", "");  // If timeout send this message
          }
          withdraw_status = 0;  // Changine the flag
        }
        // Checking for the deposit of the money to the account
        if (text == "/Deposit") {
          bot.sendMessage(chat_id, "Enter amount to be deposited.", "");
          deposit_status = 1;
        } else if (deposit_status == 1) {
          HTTPClient http;
          String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDcheckBal + "/exec?read";
          Serial.println("Making a request");
          http.begin(url.c_str());  // Specify the URL and certificate
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          String read_data_from_sheet;
          if (httpCode > 0) {  // Check for the returning code
            read_data_from_sheet = http.getString();
            Serial.println(httpCode);
            Serial.println(read_data_from_sheet);
            count++;
            int bal1 = ((read_data_from_sheet.toInt()) + (text.toInt()));
            bot.sendMessage(chat_id, "Deposited successfully!!", "");
            digitalWrite(pin_led_input, HIGH);
            delay(1000);
            digitalWrite(pin_led_input, LOW);
            String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDupdate + "/exec?" + "&usrname=" + user_id + "&pin=" + pin + "&bal=" + String(bal1) + "&deposit=" + text + "&withdraw=0" + "&num=" + String(count);
            Serial.print("POST data to spreadsheet:");
            Serial.println(urlFinal);
            HTTPClient http;
            http.begin(urlFinal.c_str());
            http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
            int httpCode = http.GET();
            Serial.print("HTTP Status Code: ");
            Serial.println(httpCode);
            http.end();
          } else {
            bot.sendMessage(chat_id, "Couldn't process deposit request.", "");  //If timeout send this message
          }
          deposit_status = 0;  // Changing the flag
        }

        // For showing number of transactions
        if (text == "/Transactions") {
          HTTPClient http;
          String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDchecktrans + "/exec?read";
          Serial.println("Making a request");
          http.begin(url.c_str());  // Specify the URL and certificate
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          String dataFromSheet;
          if (httpCode > 0) {  // Check for the returning code
            dataFromSheet = http.getString();
            Serial.println(httpCode);
            Serial.println(dataFromSheet);
            bot.sendMessage(chat_id, dataFromSheet, "");
            digitalWrite(pin_led_input, HIGH);
            delay(1000);
            digitalWrite(pin_led_input, LOW);
          } else {
            bot.sendMessage(chat_id, "Couldn't process transactions request", "");
          }
        }
        // For updating the pin
        if (text == "/Update_pin") {
          bot.sendMessage(chat_id, "Enter new pin.", "");
          pin_update_status = 1;
        } else if (pin_update_status == 1) {
          pin = text;
          String urlFinal = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDupdate + "/exec?" + "&usrname=" + user_id + "&pin=" + pin + "&bal=" + String(amount) + "&deposit=0" + "&withdraw=0" + "&num=" + String(count);
          Serial.print("POST data to spreadsheet:");
          Serial.println(urlFinal);
          HTTPClient http;
          http.begin(urlFinal.c_str());
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          Serial.print("HTTP Status Code: ");
          Serial.println(httpCode);
          http.end();
          bot.sendMessage(chat_id, "Pin updated successfully!!", "");
          digitalWrite(pin_led_input, HIGH);
          delay(1000);
          digitalWrite(pin_led_input, LOW);
          pin_update_status = 0;  // Changing the flag
        }

        // Welcome message upon successul login and after each operation.
        if (loggedin_status == 1 && login_status == 0 && password_status == 0 && withdraw_status == 0 && deposit_status == 0) {
          String welcome_message = "Welcome, " + from_name + ".\n";
          welcome_message += "Following are the commands to operate the machine. \n\n";
          welcome_message += "/Balance to check your balance \n";
          welcome_message += "/Withdraw to withdraw the money  \n";
          welcome_message += "/Deposit to deposit the money  \n";
          welcome_message += "/Transactions to get how mant times transactions have taken place  \n";
          welcome_message += "/Update_pin to update your pin  \n";
          welcome_message += "/logout to logout from your account  \n";
          bot.sendMessage(chat_id, welcome_message, "");

          HTTPClient http;
          String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_IDcheckBal + "/exec?read";
          Serial.println("Making a request");
          http.begin(url.c_str());  // Specify the URL and certificate
          http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
          int httpCode = http.GET();
          String read_data_from_sheet;
          if (httpCode > 0) {  // Check for the returning code
            read_data_from_sheet = http.getString();
            Serial.println(httpCode);
            Serial.println(read_data_from_sheet);
            amount = read_data_from_sheet;
            http.end();
          }
        }
      }
    }
  }
}

void setup() {
  pinMode(pin_led_input, OUTPUT);  // Setting pin 2 as output mode
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  // connect to the WiFi

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");  // If WiFi not connected show this message
  }
  // Printing ESP32 Local IP Address in serial monitor
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int new_msgs = bot.getUpdates(bot.last_message_received + 1);

    while (new_msgs) {
      Serial.println("got response");
      Handling_new_messages_from_telegram(new_msgs);
      new_msgs = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}