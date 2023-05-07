#pragma once

typedef struct {
	const char *ssid;
	const char *password;
	const char *client_hostname;
} local_wifi_sta_config_t;

typedef struct  SplitText_T
{
  uint displayNum;
  String top;
  String bottom;
} GenericSplitText_T;

typedef struct  NormalText_T
{
  uint displayNum;
  char chr;
} GenericNormalText_T;