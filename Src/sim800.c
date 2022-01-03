#include "main.h"
#include "sim800.h"

#define SIM800_MAX_COMMAND_LEN 255  ///< Maximum command length
#define SIM800_MAX_RESPONSE_LEN 255 ///< Maximum response length

typedef struct
{
  uint8_t command[SIM800_MAX_COMMAND_LEN];   ///< command
  uint8_t response_len;                      ///< length of response
  uint8_t response[SIM800_MAX_RESPONSE_LEN]; ///< response
  bool response_match;                       ///< response match command
  uint8_t result_code;                       ///< response result code
  uint8_t *result_data;                      ///< response result data
} sim800_t;

static void sim800_response_clear(sim800_t *p)
{
  memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
  p->response_len = 0;
  p->response_match = false;
  p->result_code = 0;
  p->result_data = NULL;
}

// send command to SIM800L, return result_code == 0 (OK)
static uint8_t sim800_cmd(char *cmd, sim800_t *p, void (*transmit)())
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
  transmit(p->command);

  uint32_t tickstart = HAL_GetTick();
  while (HAL_GetTick() - tickstart < timeout)
  {
    if (p->response_match)
    {
      return p->result_code;
    }
  }

  // timeout
  return -1;
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
static void sim800_parse_char(uint8_t c, sim800_t *p)
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
