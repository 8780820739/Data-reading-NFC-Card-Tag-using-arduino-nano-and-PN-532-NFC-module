#include <Wire.h>
#include <nfc.h>

/** define an nfc class */
NFC_Module nfc;

void setup(void) {
  Serial.begin(9600);
  nfc.begin();
  Serial.println("MF1S50 Reader Demo From Elechouse!");

  uint32_t versiondata = nfc.get_version();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  /** Set normal mode, and disable SAM */
  nfc.SAMConfiguration();
}

void loop(void) {
  u8 buf[32], sta;
  unsigned long start_time, end_time;

  /** Polling the Mifare card, buf[0] is the length of the UID */
  sta = nfc.InListPassiveTarget(buf);

  /** check state and UID length */
  if (sta && buf[0] == 4) {
    // The card may be a Mifare Classic card, try to read all the blocks
    Serial.print("UUID length: ");
    Serial.println(buf[0], DEC);
    Serial.print("UUID: ");
    nfc.puthex(buf + 1, buf[0]);
    Serial.println();

    // Factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
    u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Iterate through all blocks (0 to 63) and read their contents
    for (u8 blocknum = 0; blocknum < 64; blocknum++) {
      // Authenticate the block
      sta = nfc.MifareAuthentication(0, blocknum, buf + 1, buf[0], key);
      if (sta) {
        // Save read block data
        u8 block[16];
        Serial.print("Authentication success for block ");
        Serial.println(blocknum);

        // Read the block and measure the time taken
        start_time = millis();
        sta = nfc.MifareReadBlock(blocknum, block);
        end_time = millis();

        if (sta) {
          Serial.print("Read block ");
          Serial.print(blocknum);
          Serial.print(" successfully in ");
          Serial.print(end_time - start_time);
          Serial.println(" ms:");

          // Convert the hexadecimal values to ASCII characters
          for (int i = 0; i < 16; i++) {
            Serial.write((char)block[i]);
          }

          Serial.println();
        }
      }
    }
  }
}