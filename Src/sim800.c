#include "main.h"
#include "sim800.h"

#define SIM800_MAX_COMMAND_LEN 255  ///< Maximum command length
#define SIM800_MAX_RESPONSE_LEN 255 ///< Maximum response length

typedef struct sim800_t sim800_t;
typedef struct sim800_t
{
  uint8_t command[SIM800_MAX_COMMAND_LEN];   ///< command
  uint8_t response_len;                      ///< length of response
  uint8_t response[SIM800_MAX_RESPONSE_LEN]; ///< response
  bool response_match;                       ///< response match command
  uint8_t result_code;                       ///< response result code
  uint8_t *result_data;                      ///< response result data
  void (*onError)(sim800_t *self);
  void (*transmit)(char *cmd);
} sim800_t;

static void sim800_response_clear(sim800_t *p)
{
  memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
  p->response_len = 0;
  p->response_match = false;
  p->result_code = 0;
  p->result_data = NULL;
}

// send command to SIM800L, return result_code (0 means OK)
static uint8_t sim800_cmd(sim800_t *p, char *cmd)
{
  uint32_t timeout = 1000;
  uint8_t len = strlen(cmd);
  if (len + 2 > SIM800_MAX_COMMAND_LEN)
  {
    // cmd too long
    return -1;
  }

  sim800_response_clear(p);
  strcpy((char *)p->command, cmd);
  strcpy((char *)p->command + len, "\n\r");
  p->transmit((char *)p->command);

  uint32_t tickstart = HAL_GetTick();
  while (HAL_GetTick() - tickstart < timeout)
  {
    if (p->response_match)
    {
      if (p->result_code)
      {
        p->onError(p);
      }

      return p->result_code;
    }
  }

  // timeout
  strcpy((char *)p->response, "timeout\n\r");
  p->response_len = 9;
  p->result_code = -1;
  p->onError(p);
  return p->result_code;
}

// if match, set response_match and fill result_code and result_data
static void sim800_response_received(sim800_t *p)
{
  uint16_t size = strlen((char *)p->command) - 2; // command length without "\n\r"

  // compare response with command
  if (strncmp((char *)p->command, (char *)p->response, size))
  {
    // not match
    sim800_response_clear(p);
    return;
  }
  p->response_match = true;

  // parse result_code & set result_data
  if (p->response_len < size + 3)
  {
    // ignore result code for command
    if (!strncmp((char *)p->command, "AT+CIFSR", 8))
      return;

    // ignore result code for command
    if (!strncmp((char *)p->command, "AT+CIPSEND", 10))
      return;

    // ignore result code for command
    if (!strncmp((char *)p->command, "AT+CIPCLOSE", 11))
      return;

    // ignore result code for command
    if (!strncmp((char *)p->command, "AT+CIPSHUT", 10))
      return;

    p->result_code = -1; // response too short
    return;
  }
  uint8_t *result = p->response + size + 1;
  char *tmp;
  p->result_code = strtol((char *)result, &tmp, 10);
  p->result_data = (uint8_t *)tmp;
}

// parse next response char
void sim800_parse_char(uint8_t c, sim800_t *p)
{
  p->response[p->response_len] = c;
  if (p->response_len > 0 && p->response[p->response_len] == '\n' && p->response[p->response_len - 1] == '\r')
  {
    // string received
    sim800_response_received(p);
  }
  if (++p->response_len == SIM800_MAX_RESPONSE_LEN)
  {
    // response buffer full
    sim800_response_clear(p);
  }
}

bool sim800_connect(sim800_t *p)
{
  // Check if SIM800 ok?
  if (sim800_cmd(p, "AT"))
    return false;

  // Set APN
  if (sim800_cmd(p, "AT+CSTT=\"TM\""))
    return false;

  // Bring up wireless connection with GPRS or CSD
  if (sim800_cmd(p, "AT+CIICR"))
    return false;

  // Get local IP address, ignore result
  sim800_cmd(p, "AT+CIFSR");
  HAL_Delay(1000);

  // Start Up TCP Connection
  if (sim800_cmd(p, "AT+CIPSTART=\"TCP\",\"mail-verif.com\",20300"))
    return false;
  return true;
}

bool sim800send(sim800_t *p, char *msg)
{
  char cmd[22];
  sprintf((char *)&cmd, "AT+CIPSEND=%d", strlen(msg));
  if (sim800_cmd(p, cmd))
    return false;

  HAL_Delay(1000);
  if (sim800_cmd(p, msg))
    return false;

  return true;
}

void sim800disconnect(sim800_t *p)
{
  // Close TCP Connection - ignore error
  sim800_cmd(p, "AT+CIPCLOSE");
  HAL_Delay(1000);
  // Deactivate GPRS PDP Context
  sim800_cmd(p, "AT+CIPSHUT");
}
