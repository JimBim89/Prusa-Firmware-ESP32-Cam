/**
   @file cfg.cpp

   @brief Library for save and load MCU configuration

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#include "cfg.h"

Configuration SystemConfig(&SystemLog);

/**
   @info Constructor
   @param Logs* - pointer to log module
   @return none
*/
Configuration::Configuration(Logs* i_log) {
  Log = i_log;
  WiFiMacAddress = WiFi.macAddress();
}

/**
   @info Init configuration module
   @param none
   @return none
*/
void Configuration::Init() {
  Log->AddEvent(LogLevel_Info, "Init cfg module: " + String(EEPROM_SIZE));
  //EEPROM.begin(EEPROM_SIZE);

  /* check, when it is first MCU start. If yes, then set default CFG */
  if (CheckFirstMcuStart() == true) {
    Log->AddEvent(LogLevel_Warning, "First MCU start! Set factory cfg");
    DefaultCfg();
    SaveFirstMcuStartFlag(CFG_FIRST_MCU_START_NAK);
    Log->SetLogLevel(LoadLogLevel());
  }

  /* set reset pin */
  pinMode(CFG_RESET_PIN, INPUT_PULLUP);
}

/**
   @info Read MCU cfg from EEPROM
   @param none
   @return none
*/
void Configuration::ReadCfg() {
  Log->AddEvent(LogLevel_Info, "Load CFG from EEPROM");
  LoadRefreshInterval();
  LoadToken();
  LoadFingerprint();
  LoadPhotoQuality();
  LoadFrameSize();
  LoadBrightness();
  LoadContrast();
  LoadSaturation();
  LoadHmirror();
  LoadVflip();
  LoadLensCorrect();
  LoadExposureCtrl();
  LoadAwb();
  LoadAwbGain();
  LoadAwbMode();
  LoadBpc();
  LoadWpc();
  LoadRawGama();
  LoadWifiSsid();
  LoadWifiPassowrd();
  LoadBasicAuthUsername();
  LoadBasicAuthPassword();
  LoadBasicAuthFlag();
  LoadCameraFlashEnable();
  LoadCameraFlashTime();
  LoadMdnsRecord();
  LoadAec2();
  LoadAeLevel();
  LoadAecValue();
  LoadGainCtrl();
  LoadAgcGain();
  LoadPrusaConnectHostname();
  Log->AddEvent(LogLevel_Info, "Active WiFi client cfg: " + String(CheckActifeWifiCfgFlag() ? "true" : "false"));
  Log->AddEvent(LogLevel_Info, "Load CFG from EEPROM done");
}

/**
   @info Function for check if it's first MCU start
   @param none
   @return bool - status
*/
bool Configuration::CheckFirstMcuStart() {
  Log->AddEvent(LogLevel_Info, "Read FirstMcuStart: ");
  uint8_t flag = EEPROM.read(EEPROM_ADDR_FIRST_MCU_START_FLAG_START);

  if (CFG_FIRST_MCU_START_NAK == flag) {
    Log->AddEvent(LogLevel_Info, "It's not first start MCU: " + String(flag));
    return false;
  } else {
    Log->AddEvent(LogLevel_Warning, "First start MCU!: " + String(flag));
    return true;
  }

  return false;
}

/**
   @info save flag about first MCU start. for settings basic auth and more
   @param uint8_t - flag
   @return none
*/
void Configuration::SaveFirstMcuStartFlag(uint8_t i_data) {
  Log->AddEvent(LogLevel_Info, "Save first MCU start flag: " + String(i_data));
  SaveUint8(EEPROM_ADDR_FIRST_MCU_START_FLAG_START, i_data);
}

/**
   @info Function for set default MCU cfg
   @param none
   @return none
*/
void Configuration::DefaultCfg() {
  Log->AddEvent(LogLevel_Warning, "+++++++++++++++++++++++++++");
  Log->AddEvent(LogLevel_Warning, "Start set factory cfg!");

  SaveRefreshInterval(FACTORY_CFG_PHOTO_REFRESH_INTERVAL);
  SaveToken("");
  GetFingerprint();
  SavePhotoQuality(FACTORY_CFG_PHOTO_QUALITY);
  SaveFrameSize(FACTORY_CFG_FRAME_SIZE);
  SaveBrightness(FACTORY_CFG_BRIGHTNESS);
  SaveContrast(FACTORY_CFG_CONTRAST);
  SaveSaturation(FACTORY_CFG_SATURATION);
  SaveHmirror(FACTORY_CFG_H_MIRROR);
  SaveVflip(FACTORY_CFG_V_FLIP);
  SaveLensCorrect(FACTORY_CFG_LENS_CORRECT);
  SaveExposureCtrl(FACTORY_CFG_EXPOSURE_CTRL);
  SaveAwb(FACTORY_CFG_AWB);
  SaveAwbGain(FACTORY_CFG_AWB_GAIN);
  SaveAwbMode(FACTORY_CFG_AWB_MODE);
  SaveBpc(FACTORY_CFG_BPC);
  SaveWpc(FACTORY_CFG_WPC);
  SaveRawGama(FACTORY_CFG_RAW_GAMA);
  SaveWifiCfgFlag(CFG_WIFI_SETTINGS_NOT_SAVED);
  SaveWifiPassword("");
  SaveWifiSsid("");
  SaveBasicAuthUsername(FACTORY_CFG_WEB_AUTH_USERNAME);
  SaveBasicAuthPassword(FACTORY_CFG_WEB_AUTH_PASSWORD);
  SaveBasicAuthFlag(FACTORY_CFG_WEB_AUTH_ENABLE);
  SaveCameraFlashEnable(FACTORY_CFG_CAMERA_FLASH_ENABLE);
  SaveCameraFlashTime(FACTORY_CFG_CAMERA_FLASH_TIME);
  SaveMdnsRecord(FACTORY_CFG_MDNS_RECORD_HOST);
  SaveAec2(FACTORY_CFG_AEC2);
  SaveAeLevel(FACTORY_CFG_AE_LEVEL);
  SaveAecValue(FACTORY_CFG_AEC_VALUE);
  SaveGainCtrl(FACTORY_CFG_GAIN_CTRL);
  SaveAgcGain(FACTORY_CFG_AGC_GAIN);
  SaveLogLevel(LogLevel_Info);
  SavePrusaConnectHostname(FACTORY_CFG_HOSTNAME);
  Log->AddEvent(LogLevel_Warning, "+++++++++++++++++++++++++++");
}

/**
   @info the function checks whether the configuration for connecting to WI-FI network  is saved 
   @param none
   @return bool - status
*/
bool Configuration::CheckActifeWifiCfgFlag() {
  uint8_t flag = EEPROM.read(EEPROM_ADDR_WIFI_ACTIVE_FLAG_START);
  Log->AddEvent(LogLevel_Verbose, "Read ActifeWifiCfgFlag: " + String(flag));

  if (CFG_WIFI_SETTINGS_SAVED == flag) {
    return true;
  } else {
    return false;
  }

  return false;
}

/**
   @info Function checks the GPIO pin, which is used to reset the CFG to default
   @param none
   @return none
*/
void Configuration::CheckResetCfg() {
  Log->AddEvent(LogLevel_Verbose, "Check reset MCU cfg");
  bool ResetPinStatus = digitalRead(CFG_RESET_PIN);

  /* wait 10s to pressed reset pin */
  uint8_t i = 0;
  for (i = 0; i < (CFG_RESET_TIME_WAIT / CFG_RESET_LOOP_DELAY); i++) {
    Log->AddEvent(LogLevel_Verbose, "Reset pin status: " + String(ResetPinStatus));
    if (ResetPinStatus == HIGH) {
      break;
    }

    delay(CFG_RESET_LOOP_DELAY);
    ResetPinStatus = digitalRead(CFG_RESET_PIN);
  }

  /* check if is reset pin grounded more at 10s */
  if (i == (CFG_RESET_TIME_WAIT / CFG_RESET_LOOP_DELAY)) {
    Log->AddEvent(LogLevel_Warning, "Reset MCU to factory CFG!");

    /* wait for ungrounded reset pin, and binking led */
    while (digitalRead(CFG_RESET_PIN) == LOW) {
      analogWrite(FLASH_GPIO_NUM, 20);
      delay(100);
      analogWrite(FLASH_GPIO_NUM, 0);
      delay(100);
    }

    /* turn off LED, reset cfg, reset MCU */
    analogWrite(FLASH_GPIO_NUM, 0);
    DefaultCfg();
    ESP.restart();

  } else {
    Log->AddEvent(LogLevel_Verbose, "Reset MCU cfg false");
  }
}

/**
   @info get Fingerprint value from system
   @param none
   @return none
*/
void Configuration::GetFingerprint() {
  String Id = "";
  for (size_t i = 0; i < UniqueIDsize; i++) {
    Id += String(UniqueID[i]);
    //Id += ".";
  }
  //String Random = String(esp_random());
  String encoded = base64::encode(Id + " " + WiFiMacAddress);
  SaveFingerprint(encoded);
  Log->AddEvent(LogLevel_Verbose, "UniqueID: " + Id);
  Log->AddEvent(LogLevel_Verbose, "WiFi MAC: " + WiFiMacAddress);
  //Log->AddEvent(LogLevel_Verbose, "Random number: " + Random);
  Log->AddEvent(LogLevel_Warning, "Calculated device fingerprint: " + encoded);
}

/**
   @info Function for save uint8_t to EEPROM
   @param uint16_t data address
   @param uint8_t data
   @return none
*/
void Configuration::SaveUint8(uint16_t address, uint8_t data) {
  EEPROM.write(address, data);
  EEPROM.commit();
}
/**
   @info Function for save int8_t to EEPROM
   @param uint16_t data address
   @param int8_t data
   @return none
*/
void Configuration::SaveInt8(uint16_t address, int8_t data) {
  EEPROM.write(address, data);
  EEPROM.commit();
}
/**
   @info Function for save bool to EEPROM
   @param uint16_t data address
   @param bool data
   @return none
*/
void Configuration::SaveBool(uint16_t address, bool data) {
  EEPROM.write(address, data);
  EEPROM.commit();
}
/**
   @info Function for save uint16_t to EEPROM
   @param uint16_t data address
   @param uint16_t data
   @return none
*/
void Configuration::SaveUint16(uint16_t address, uint16_t data) {
  uint8_t highByte = highByte(data);
  uint8_t lowByte = lowByte(data);

  EEPROM.write(address, highByte);
  EEPROM.write(address + 1, lowByte);
  EEPROM.commit();
}

/**
   @info Function for save string to EEPROM
   @param uint16_t data address
   @param uint16_t maximum data length
   @param String data
   @return none
*/
void Configuration::SaveString(uint16_t address, uint16_t max_length, String data) {
  if (data.length() < max_length) {
    /* save data length to first byte */
    EEPROM.write(address, data.length());

    /* save data */
    for (uint16_t i = address + 1, j = 0; j < data.length(); i++, j++) {
      EEPROM.write(i, data.charAt(j));
    }
    EEPROM.commit();
    Log->AddEvent(LogLevel_Verbose, "Write string done");
  } else {
    Log->AddEvent(LogLevel_Verbose, "Skip write string");
  }
}
/**
   @info Function for read uint16_t data from EEPROM
   @param uint16_t fist byte address
   @return uint16_t data
*/
uint16_t Configuration::LoadUint16(uint16_t address) {
  uint16_t tmp = uint16_t(EEPROM.read(address) << 8) | (EEPROM.read(address + 1));
  return tmp;
}
/**
   @info Function for load string from EEPROM
   @param uint16_t data address
   @param uint16_t maximum data length
   @param bool show/hide sensitive data
   @return String data
*/
String Configuration::LoadString(uint16_t address, uint16_t max_length, bool show_sensitive_data) {
  String tmp = "";
  uint8_t len = EEPROM.read(address);

  if ((len <= max_length) && (len > 0)) {
    for (uint16_t i = address + 1, j = 0; j < len; i++, j++) {
      tmp += (char)EEPROM.read(i);
    }
  }

  String LogMsg = "";
  if (false == show_sensitive_data) {
    for (uint16_t i = 0; i <= tmp.length(); i++) {
      LogMsg += "*";
    }
  } else {
    LogMsg = tmp;
  }
  Log->AddEvent(LogLevel_Info, LogMsg, true, false);
  
  return tmp;
}

/**
   @info save refresh interval to eeprom
   @param uint8_t - refresh interval
   @return none
*/
void Configuration::SaveRefreshInterval(uint8_t i_interval) {
  Log->AddEvent(LogLevel_Verbose, "Save RefreshInterval: " + String(i_interval));
  SaveUint8(EEPROM_ADDR_REFRESH_INTERVAL_START, i_interval);
}

/**
   @info save token to eeprom
   @param string - token
   @return none
*/
void Configuration::SaveToken(String i_token) {
  Log->AddEvent(LogLevel_Verbose, "Save Token[" + String(i_token.length()) + "]: " + i_token);
  SaveString(EEPROM_ADDR_TOKEN_START, EEPROM_ADDR_TOKEN_LENGTH, i_token);
}

/**
   @info save fingerprint to EEPROM
   @param String - fingerprint
   @return none
*/
void Configuration::SaveFingerprint(String i_fingerprint) {
  Log->AddEvent(LogLevel_Verbose, "Save Fingerprint[" + String(i_fingerprint.length()) + "]: " + i_fingerprint);
  SaveString(EEPROM_ADDR_FINGERPRINT_START, EEPROM_ADDR_FINGERPRINT_LENGTH, i_fingerprint);
}

/**
   @info save photo quality to EEPROM
   @param uint8_t - photo quality
   @return none
*/
void Configuration::SavePhotoQuality(uint8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save PhotoQuality: " + String(i_data));
  SaveUint8(EEPROM_ADDR_PHOTO_QUALITY_START, i_data);
}

/**
   @info save framesize to EEPROM
   @param uint8_t - framesize
   @return none
*/
void Configuration::SaveFrameSize(uint8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save FrameSize: " + String(i_data));
  SaveUint8(EEPROM_ADDR_FRAMESIZE_START, i_data);
}

/**
   @info save brightness to EEPROM
   @param uint8_t - brightness
   @return none
*/
void Configuration::SaveBrightness(int8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Brightness: " + String(i_data));
  SaveInt8(EEPROM_ADDR_BRIGHTNESS_START, i_data);
}

/**
   @info save contrast to EEPROM
   @param uint8_t - contrast
   @return none
*/
void Configuration::SaveContrast(int8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Contrast: " + String(i_data));
  SaveInt8(EEPROM_ADDR_CONTRAST_START, i_data);
}

/**
   @info save saturation to EEPROM
   @param int8_t - saturation
   @return none
*/
void Configuration::SaveSaturation(int8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Saturation: " + String(i_data));
  SaveInt8(EEPROM_ADDR_SATURATION_START, i_data);
}

/**
   @info save horizontal mirror to EEPROM
   @param uint8_t - hmirror value
   @return none
*/
void Configuration::SaveHmirror(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Hmirror: " + String(i_data));
  SaveBool(EEPROM_ADDR_HMIRROR_START, i_data);
}

/**
   @info save vertical flip to EEPROM
   @param uint8_t - vertical flip
   @return none
*/
void Configuration::SaveVflip(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save vflip: " + String(i_data));
  SaveBool(EEPROM_ADDR_VFLIP_START, i_data);
}

/**
   @info save lens correction to EEPROM
   @param uint8_t - lens correction
   @return none
*/
void Configuration::SaveLensCorrect(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save lensc: " + String(i_data));
  SaveBool(EEPROM_ADDR_LENSC_START, i_data);
}

/**
   @info save exposure ctrl to EEPROM
   @param uint8_t - exposure ctrl
   @return none
*/
void Configuration::SaveExposureCtrl(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save exposure_ctrl: " + String(i_data));
  SaveBool(EEPROM_ADDR_EXPOSURE_CTRL_START, i_data);
}

/**
   @info Save awb
   @param bool value
   @return none
*/
void Configuration::SaveAwb(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save awb: " + String(i_data));
  SaveBool(EEPROM_ADDR_AWB_ENABLE_START, i_data);
}

/**
   @info Save awb gain value
   @param bool value
   @return none
*/
void Configuration::SaveAwbGain(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save awb_gain: " + String(i_data));
  Configuration::SaveBool(EEPROM_ADDR_AWB_GAIN_ENABLE_START, i_data);
}

/**
   @info Save awb mode value
   @param uint8_t value
   @return none
*/
void Configuration::SaveAwbMode(uint8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save awb_mode: " + String(i_data));
  SaveUint8(EEPROM_ADDR_AWB_MODE_ENABLE_START, i_data);
}

/**
   @info Save bpc value
   @param bool value
   @return none
*/
void Configuration::SaveBpc(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save bpc: " + String(i_data));
  SaveBool(EEPROM_ADDR_BPC_ENABLE_START, i_data);
}

/**
   @info Save wpc value
   @param bool value
   @return none
*/
void Configuration::SaveWpc(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save wpc: " + String(i_data));
  SaveBool(EEPROM_ADDR_WPC_ENABLE_START, i_data);
}

/**
   @info Save raw gama value
   @param bool value
   @return none
*/
void Configuration::SaveRawGama(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save raw_gama: " + String(i_data));
  SaveBool(EEPROM_ADDR_RAW_GAMA_ENABLE_START, i_data);
}

/**
   @info save WI-FI ssid to EEPROM
   @param string - WI-FI SSID
   @return none
*/
void Configuration::SaveWifiSsid(String i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save WI-FI SSID[" + String(i_data.length()) + "]: " + i_data);
  SaveString(EEPROM_ADDR_WIFI_SSID_START, EEPROM_ADDR_WIFI_SSID_LENGTH, i_data);
}

/**
   @info save WI-FI password to EEPROM
   @param string - WI-FI password
   @return none
*/
void Configuration::SaveWifiPassword(String i_data) {
  //Log->AddEvent(LogLevel_Verbose, "Save WI-FI password[" + String(i_data.length()) + "]: " + i_data); /* SENSITIVE DATA! */
  Log->AddEvent(LogLevel_Verbose, "Save WI-FI password[" + String(i_data.length()) + "]");
  SaveString(EEPROM_ADDR_WIFI_PASSWORD_START, EEPROM_ADDR_WIFI_PASSWORD_LENGTH, i_data);
}

/**
   @info save flag for configuration to EEPROM
   @param uint8_t - cfg flag
   @return none
*/
void Configuration::SaveWifiCfgFlag(uint8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save active wifi cfg flag: " + String(i_data));
  SaveUint8(EEPROM_ADDR_WIFI_ACTIVE_FLAG_START, i_data);
}
/*
   @info save username fof BasicAuth to EEPROM
   @param string - username
   @return none
*/
void Configuration::SaveBasicAuthUsername(String i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save username BasicAuth[" + String(i_data.length()) + "]: " + i_data);
  SaveString(EEPROM_ADDR_BASIC_AUTH_USERNAME_START, EEPROM_ADDR_BASIC_AUTH_USERNAME_LENGTH, i_data);
}


/**
   @info save password fof BasicAuth to EEPROM
   @param string - password
   @return none
*/
void Configuration::SaveBasicAuthPassword(String i_data) {
  uint8_t len = i_data.length();
  Log->AddEvent(LogLevel_Verbose, "Save password BasicAuth[" + String(len) + "]: ");
  SaveString(EEPROM_ADDR_BASIC_AUTH_PASSWORD_START, EEPROM_ADDR_BASIC_AUTH_PASSWORD_LENGTH, i_data);
}

/**
   @info save status enable/disable BasicAuth to EEPROM
   @param bool - status
   @return none
*/
void Configuration::SaveBasicAuthFlag(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Enable/disable BasicAuth: " + String(i_data));
  SaveBool(EEPROM_ADDR_BASIC_AUTH_ENABLE_FLAG_START, i_data);
}

/**
   @info save status enable/disable photo flash to EEPROM
   @param bool - status
   @return none
*/
void Configuration::SaveCameraFlashEnable(uint8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Enable/disable camera flash: " + String(i_data));
  SaveUint8(EEPROM_ADDR_CAMERA_FLASH_ENABLE_START, i_data);
}

/**
   @info save time for photo flash to EEPROM
   @param bool - status
   @return none
*/
void Configuration::SaveCameraFlashTime(uint16_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save camera flash time: " + String(i_data));
  SaveUint16(EEPROM_ADDR_CAMERA_FLASH_TIME_START, i_data);
}

/**
   @info Save mdns record
   @param String - value
   @return none
*/
void Configuration::SaveMdnsRecord(String i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save mDNS record[" + String(i_data.length()) + "]: " + String(i_data));
  SaveString(EEPROM_ADDR_MDNS_RECORD_START, EEPROM_ADDR_MDNS_RECORD_LENGTH, i_data);
}

/**
   @info Save aec2 value
   @param bool - value
   @return none
*/
void Configuration::SaveAec2(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save Enable/disable AEC2: " + String(i_data));
  SaveBool(EEPROM_ADDR_AEC2_START, i_data);
}

/**
   @info Save ae level value
   @param int8_t - value
   @return none
*/
void Configuration::SaveAeLevel(int8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save ae_level: " + String(i_data));
  SaveBool(EEPROM_ADDR_AE_LEVEL_START, i_data);
}

/**
   @info Save aec value
   @param uint16_t - value
   @return none
*/
void Configuration::SaveAecValue(uint16_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save aec value time: " + String(i_data));
  SaveUint16(EEPROM_ADDR_AEC_VALUE_START, i_data);
}

/**
   @info Save gain ctrl
   @param bool - value
   @return none
*/
void Configuration::SaveGainCtrl(bool i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save gain_ctrl: " + String(i_data));
  SaveBool(EEPROM_ADDR_GAIN_CTRL_START, i_data);
}

/**
   @info Save agc gain 
   @param uint8_t - value
   @return none
*/
void Configuration::SaveAgcGain(uint8_t i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save agc_gain: " + String(i_data));
  SaveUint8(EEPROM_ADDR_AGC_GAIN_START, i_data);
}

/**
   @info Save log level
   @param LogLevel_enum - value
   @return none
*/
void Configuration::SaveLogLevel(LogLevel_enum i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save log level: " + String(i_data));
  SaveUint8(EEPROM_ADDR_LOG_LEVEL, i_data);
}

/** 
 * @info Save PrusaConnect hostname
 * @param String - hostname
 * @return none
*/
void Configuration::SavePrusaConnectHostname(String i_data) {
  Log->AddEvent(LogLevel_Verbose, "Save PrusaConnectHostanme[" + String(i_data.length()) + "]: " + i_data);
  SaveString(EEPROM_ADDR_HOSTNAME_START, EEPROM_ADDR_HOSTNAME_LENGTH, i_data);
}

/**
   @info load refresh interval from eeprom 
   @param none
   @return uint8_t - refresh interval
*/
uint8_t Configuration::LoadRefreshInterval() {
  uint8_t ret = EEPROM.read(EEPROM_ADDR_REFRESH_INTERVAL_START);
  Log->AddEvent(LogLevel_Info, "Refresh interval: " + String(ret));

  return ret;
}

/**
   @info load token from eeprom
   @param none
   @return String - token
*/
String Configuration::LoadToken() {
  Log->AddEvent(LogLevel_Info, "Token: ", false);
  String ret = LoadString(EEPROM_ADDR_TOKEN_START, EEPROM_ADDR_TOKEN_LENGTH, CONSOLE_VERBOSE_DEBUG);

  return ret;
}

/**
   @info load fingerprint from eeprom
   @param none
   @return String - fingerprint
*/
String Configuration::LoadFingerprint() {
  Log->AddEvent(LogLevel_Info, "Fingerprint: ", false);
  String ret = LoadString(EEPROM_ADDR_FINGERPRINT_START, EEPROM_ADDR_FINGERPRINT_LENGTH, true);

  return ret;
}

/**
   @info load photo quality cfg from eeprom
   @param none
   @return uint8_t - photo quality
*/
uint8_t Configuration::LoadPhotoQuality() {
  uint8_t ret = EEPROM.read(EEPROM_ADDR_PHOTO_QUALITY_START);
  Log->AddEvent(LogLevel_Info, "Photo quality: " + String(ret));

  return ret;
}

/**
   @info load framesize cfg from eeprom
   @param none
   @return uint8_t - framesize
*/
uint8_t Configuration::LoadFrameSize() {
  uint8_t ret = EEPROM.read(EEPROM_ADDR_FRAMESIZE_START);
  Log->AddEvent(LogLevel_Info, "Framesize: " + String(ret));
  return ret;
}

/**
   @info load Brightness cfg from eeprom
   @param none
   @return int8_t - brightness
*/
int8_t Configuration::LoadBrightness() {
  int8_t ret = EEPROM.read(EEPROM_ADDR_BRIGHTNESS_START);
  Log->AddEvent(LogLevel_Info, "brightness: " + String(ret));

  return ret;
}

/**
   @info load contrast cfg from eeprom
   @param none
   @return int8_t - contrast
*/
int8_t Configuration::LoadContrast() {
  int8_t ret = EEPROM.read(EEPROM_ADDR_CONTRAST_START);
  Log->AddEvent(LogLevel_Info, "contrast: " + String(ret));

  return ret;
}

/**
   @info load saturation cfg from eeprom
   @param none
   @return int8_t - saturation
*/
int8_t Configuration::LoadSaturation() {
  int8_t ret = EEPROM.read(EEPROM_ADDR_SATURATION_START);
  Log->AddEvent(LogLevel_Info, "saturation: " + String(ret));

  return ret;
}

/**
   @info load hmirror cfg from eeprom
   @param none
   @return bool - hmirror
*/
bool Configuration::LoadHmirror() {
  bool ret = EEPROM.read(EEPROM_ADDR_HMIRROR_START);
  Log->AddEvent(LogLevel_Info, "hmirror: " + String(ret));

  return ret;
}

/**
   @info load vflip cfg from eeprom
   @param none
   @return bool - vflip
*/
bool Configuration::LoadVflip() {
  bool ret = EEPROM.read(EEPROM_ADDR_VFLIP_START);
  Log->AddEvent(LogLevel_Info, "vflip: " + String(ret));

  return ret;
}

/**
   @info load lens corection cfg from eeprom
   @param none
   @return bool - lens correction
*/
bool Configuration::LoadLensCorrect() {
  bool ret = EEPROM.read(EEPROM_ADDR_LENSC_START);
  Log->AddEvent(LogLevel_Info, "lensc: " + String(ret));

  return ret;
}

/**
   @info load exposure ctrl cfg from eeprom
   @param none
   @return bool - exposure ctrl
*/
bool Configuration::LoadExposureCtrl() {
  bool ret = EEPROM.read(EEPROM_ADDR_EXPOSURE_CTRL_START);
  Log->AddEvent(LogLevel_Info, "exposure_ctrl: " + String(ret));

  return ret;
}

/**
   @info Load awb value
   @param none
   @return bool - awb
*/
bool Configuration::LoadAwb() {
  bool ret = EEPROM.read(EEPROM_ADDR_AWB_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "awb: " + String(ret));

  return ret;
}

/**
   @info Load awb gain value
   @param none
   @return bool - awb gain
*/
bool Configuration::LoadAwbGain() {
  bool ret = EEPROM.read(EEPROM_ADDR_AWB_GAIN_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "awb_gain: " + String(ret));

  return ret;
}

/**
   @info Load awb mode
   @param none
   @return uint8_t - awb mode
*/
uint8_t Configuration::LoadAwbMode() {
  uint8_t ret = EEPROM.read(EEPROM_ADDR_AWB_MODE_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "awb_mode: " + String(ret));

  return ret;
}

/**
   @info Load bpc value
   @param none
   @return bool - bpc
*/
bool Configuration::LoadBpc() {
  bool ret = EEPROM.read(EEPROM_ADDR_BPC_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "bpc: " + String(ret));

  return ret;
}

/**
   @info Load wpc value
   @param none
   @return bool - wpc
*/
bool Configuration::LoadWpc() {
  bool ret = EEPROM.read(EEPROM_ADDR_WPC_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "wpc: " + String(ret));

  return ret;
}

/**
   @info Load raw gama value
   @param none
   @return bool - raw gama
*/
bool Configuration::LoadRawGama() {
  bool ret = EEPROM.read(EEPROM_ADDR_RAW_GAMA_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "raw_gama: " + String(ret));

  return ret;
}

/**
   @info load WI-FI ssid cfg from eeprom
   @param none
   @return String - WI-FI SSID
*/
String Configuration::LoadWifiSsid() {
  Log->AddEvent(LogLevel_Info, "SSID: ", false);
  String ret = LoadString(EEPROM_ADDR_WIFI_SSID_START, EEPROM_ADDR_WIFI_SSID_LENGTH, true);

  return ret;
}

/**
   @info load Wi-FI password cfg from eeprom
   @param none
   @return String - WI-FI password
*/
String Configuration::LoadWifiPassowrd() {
  Log->AddEvent(LogLevel_Info, "WiFi password: ", false);
  String ret = LoadString(EEPROM_ADDR_WIFI_PASSWORD_START, EEPROM_ADDR_WIFI_PASSWORD_LENGTH, CONSOLE_VERBOSE_DEBUG);
  
  return ret;
}

/**
   @info Read username for basic authentification from eeprom
   @param none
   @return String - username
*/
String Configuration::LoadBasicAuthUsername() {
  Log->AddEvent(LogLevel_Info, "web auth user: ", false);
  String ret = LoadString(EEPROM_ADDR_BASIC_AUTH_USERNAME_START, EEPROM_ADDR_BASIC_AUTH_USERNAME_LENGTH, true);

  return ret;
}

/**
   @info Read password for basic authentification from eeprom
   @param none
   @return String - password
*/
String Configuration::LoadBasicAuthPassword() {
  Log->AddEvent(LogLevel_Info, "web auth pass: ", false);
  String ret = LoadString(EEPROM_ADDR_BASIC_AUTH_PASSWORD_START, EEPROM_ADDR_BASIC_AUTH_PASSWORD_LENGTH, CONSOLE_VERBOSE_DEBUG);

  return ret;
}

/**
   @info Load flag for enable/disable basic auth from eeprom
   @param none
   @return bool - status
*/
bool Configuration::LoadBasicAuthFlag() {
  bool ret = EEPROM.read(EEPROM_ADDR_BASIC_AUTH_ENABLE_FLAG_START);
  Log->AddEvent(LogLevel_Info, "web auth enable: " + String(ret));

  return ret;
}

/**
   @info Load flag for enable/disable flash from eeprom
   @param none
   @return bool - status
*/
bool Configuration::LoadCameraFlashEnable() {
  bool ret = EEPROM.read(EEPROM_ADDR_CAMERA_FLASH_ENABLE_START);
  Log->AddEvent(LogLevel_Info, "Camera flash: " + String(ret));

  return ret;
}

/**
   @info Load time for photo flash
   @param none
   @return uint16_t - time
*/
uint16_t Configuration::LoadCameraFlashTime() {
  uint16_t ret = LoadUint16(EEPROM_ADDR_CAMERA_FLASH_TIME_START);
  Log->AddEvent(LogLevel_Info, "Camera flash time: " + String(ret));

  return ret;
}

/**
   @info Load mdns record
   @param none
   @return String - mDNS record
*/
String Configuration::LoadMdnsRecord() {
  Log->AddEvent(LogLevel_Info, "mDNS: ", false);
  String ret = LoadString(EEPROM_ADDR_MDNS_RECORD_START, EEPROM_ADDR_MDNS_RECORD_LENGTH, true);

  return ret;
}

/**
   @info Load aec2 value
   @param none
   @return bool - value
*/
bool Configuration::LoadAec2() {
  bool ret = EEPROM.read(EEPROM_ADDR_AEC2_START);
  Log->AddEvent(LogLevel_Info, "aec2: " + String(ret));

  return ret;
}

/**
   @info Load ae level value
   @param none
   @return int8_t - value
*/
int8_t Configuration::LoadAeLevel() {
  int8_t ret = EEPROM.read(EEPROM_ADDR_AE_LEVEL_START);
  Log->AddEvent(LogLevel_Info, "ae_level: " + String(ret));

  return ret;
}

/**
   @info Load aec value
   @param none
   @return uint16_t - value
*/
uint16_t Configuration::LoadAecValue() {
  uint16_t ret = LoadUint16(EEPROM_ADDR_AEC_VALUE_START);
  Log->AddEvent(LogLevel_Info, "aec_value: " + String(ret));

  return ret;
}

/**
   @info Load fain ctrl value
   @param none
   @return bool - value
*/
bool Configuration::LoadGainCtrl() {
  bool ret = EEPROM.read(EEPROM_ADDR_GAIN_CTRL_START);
  Log->AddEvent(LogLevel_Info, "gain_ctrl: " + String(ret));

  return ret;
}

/**
   @info Load agc gain value
   @param none
   @return uint8_t - value
*/
uint8_t Configuration::LoadAgcGain() {
  uint8_t ret = EEPROM.read(EEPROM_ADDR_AGC_GAIN_START);
  Log->AddEvent(LogLevel_Info, "agc_gain: " + String(ret));

  return ret;
}

/**
 * @brief Load log level from EEPROM
 * 
 * @return LogLevel_enum - log level
 */
LogLevel_enum Configuration::LoadLogLevel() {
  LogLevel_enum ret = (LogLevel_enum) EEPROM.read(EEPROM_ADDR_LOG_LEVEL);
  Log->AddEvent(LogLevel_Info, "LogLevel: " + String(ret));
  
  return ret;
}

/**
 * @brief Load PrusaConnect hostname from EEPROM
 * 
 * @return String - hostname
 */
String Configuration::LoadPrusaConnectHostname() {
  Log->AddEvent(LogLevel_Info, "PrusaConnect hostname: ", false);
  String ret = LoadString(EEPROM_ADDR_HOSTNAME_START, EEPROM_ADDR_HOSTNAME_LENGTH, true);

  return ret;
}

/* EOF */