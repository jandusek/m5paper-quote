/*
*******************************************************************************
* Copyright (c) 2023 by Jan Dusek
*
* Describe: Display random quotes
* Date: 2023/01/26
*******************************************************************************
*/

#include <WiFi.h>
#include <M5Unified.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "env.h"
#include "bariol_regular_webfont24pt7b.h"
#include "bariol_bold_webfont24pt7b.h"

#define MAX_LINE_LENGTH 46
#define SLEEP_HRS 6 // display new quote every SLEEP_HRS hours

// Splits the input string into multiple lines, preserving whole words
// Returns an array of lines and number of lines (by reference via 2nd param)
char** splitIntoLines(char* str, int* total_lines) {
  int len = strlen(str);
  int num_lines = 1;         // start with 1 line
  int last_whitespace = -1;  // index of the last whitespace character
  int line_start = 0;
  int line_char = 0;

  // Count the total number of lines we'll need
  for (int i = 0; i < len; i++, line_char++) {
    if (str[i] == ' ') last_whitespace = i;
    if (line_char == MAX_LINE_LENGTH) {
      line_start = last_whitespace + 1;
      i = last_whitespace;
      line_char = 0;
      num_lines++;
    }
  }
  *total_lines = num_lines;

  // Allocate memory for the lines
  char** lines = (char**)malloc(num_lines * sizeof(char*));
  for (int i = 0; i < num_lines; i++) {
    lines[i] = (char*)malloc((MAX_LINE_LENGTH + 1) * sizeof(char));
  }

  int cur_line = 0;
  last_whitespace = -1;  // index of the last whitespace character
  line_start = 0;
  line_char = 0;
  for (int i = 0; i < len; i++, line_char++) {
    if (str[i] == ' ') last_whitespace = i;
    if (line_char == MAX_LINE_LENGTH) {
      memcpy(lines[cur_line], &str[line_start], last_whitespace - line_start);
      lines[cur_line][last_whitespace - line_start] = '\0';
      line_start = last_whitespace + 1;
      i = last_whitespace;
      line_char = 0;
      cur_line++;
    }
  }
  memcpy(lines[cur_line], &str[line_start], len - line_start);
  lines[cur_line][len - line_start] = '\0';

  return lines;
}

// removes all ocurrences of a specific character from a string, in place
void removeAllChars(char* str, char c) {
  char *pr = str, *pw = str;
  while (*pr) {
    *pw = *pr++;
    pw += (*pw != c);
  }
  *pw = '\0';
}

bool displayQuote(String payload) {
  // Deserialize the JSON document
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, payload);
  int w = M5.Display.width();
  int h = M5.Display.height();
  M5.Display.startWrite();
  M5.Display.fillRect(0, 0, (int)(M5.Display.width()), (int)(M5.Display.height() - 10), 0xFFFF);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  const char* _quote = doc["quote"];
  char* quote = (char*)malloc(strlen(_quote) + 1);
  char* followup;
  strcpy(quote, _quote);
  
  // remove Markdown formatting from the quote
  removeAllChars(quote, '*');
  removeAllChars(quote, '_');
  
  int numLinesQuote = 0;
  int numLinesFollowup = 0;
  char** quoteLines = splitIntoLines(quote, &numLinesQuote);
  char** followupLines;
  bool hasFollowup = doc.containsKey("followup");

  if (hasFollowup) {
    const char* _followup = doc["followup"];
    followup = (char*)malloc(strlen(_followup) + 1);
    strcpy(followup, _followup);

    // remove Markdown formatting from the followup
    removeAllChars(followup, '*');
    removeAllChars(followup, '_');
    
    followupLines = splitIntoLines(followup, &numLinesFollowup);
  }

  const char* author = doc["author"];
  /*Serial.print(quote);
  Serial.print(" --");
  Serial.println(author);*/
  char buf[32] = { 0 };
  M5.Display.setFreeFont(&bariol_regular_webfont24pt7b);
  M5.Display.setTextSize(1);

  M5.Display.setTextDatum(MC_DATUM);
  int spread = (int)(M5.Display.height() / (numLinesQuote + numLinesFollowup + 3));
  for (int i = 0; i < numLinesQuote; i++) {
    M5.Display.drawString(quoteLines[i], (int)(M5.Display.width() / 2), spread * (i + 1.1));
  }
  for (int i = 0; i < numLinesFollowup; i++) {
    M5.Display.drawString(followupLines[i], (int)(M5.Display.width() / 2), spread * (numLinesQuote + i + 1.2));
  }

  M5.Display.setFreeFont(&bariol_bold_webfont24pt7b);
  M5.Display.setTextSize(1);
  M5.Display.drawString(author, (int)(M5.Display.width() / 2), spread * (numLinesQuote + numLinesFollowup + 1.3) + 35);
  M5.Display.endWrite();

  // free quote memory
  for (int i = 0; i < numLinesQuote; i++) {
    free(quoteLines[i]);
  }
  free(quoteLines);
  free(quote);

  // free followup memory
  if (hasFollowup) {
    for (int i = 0; i < numLinesFollowup; i++) {
      free(followupLines[i]);
    }
    free(followupLines);
    free(followup);
  }
  return true;
}

void showQuote() {
  HTTPClient http;
  String payload;

  // The quote API's URL or IP address with path
  http.begin(ENV_QUOTE_URL);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.println(payload);
    displayQuote(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return;
}

void setup(void) {
  auto cfg = M5.config();

  cfg.external_rtc = true;
  cfg.clear_display = false;

  M5.begin(cfg);
  Serial.print('Connecting to WiFi...');

  WiFi.begin(ENV_WIFI_SSID, ENV_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("\r\n WiFi Connected.");
  Serial.print("Battery: ");
  Serial.println(M5.Power.getBatteryLevel());
  M5.Display.setRotation(1);
  M5.Display.fillRect(0, 0, (int)(M5.Display.width()), (int)(M5.Display.height()), 0xFFFF);
  M5.Display.progressBar(0, (int)(M5.Display.height() - 6), (int)(M5.Display.width()), 6, (int)(M5.Power.getBatteryLevel()));

  showQuote();

  Serial.println("Going to deep sleep.");
  M5.Display.sleep();
  M5.Rtc.clearIRQ();
  M5.Rtc.setAlarmIRQ(3600 * SLEEP_HRS);
  M5.Power.powerOff();
}

void loop(void) {
  Serial.println('Loop... It should never get here.');
}
