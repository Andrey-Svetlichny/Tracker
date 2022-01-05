#include "main.h"
#include "sim800.h"

static void sim800_response_clear(sim800_t *p)
{
  memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
  p->response_len = 0;
  p->response_match = false;
  p->result_code = 0;
  p->result_data = NULL;
}

// send command to SIM800L, return result_code (0 means OK)
static uint8_t sim800_cmd(sim800_t *p, char *cmd, bool result_expected)
{
  uint32_t timeout = 1000;
  uint8_t len = strlen(cmd);
  if (len + 2 > SIM800_MAX_COMMAND_LEN)
  {
    // cmd too long
    strcpy((char *)p->response, "cmd too long\n\r");
    p->response_len = 14;
    p->result_code = -3;
    p->onError(p);
    return p->result_code;
  }

  sim800_response_clear(p);
  strcpy((char *)p->command, cmd);
  strcpy((char *)p->command + len, "\n\r");
  p->result_expected = result_expected;
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

  if (!p->result_expected)
    return;

  // parse result_code & set result_data
  if (p->response_len < size + 3)
  {
    p->result_code = -2; // response too short
    return;
  }
  uint8_t *result = p->response + size + 1;
  char *tmp;
  p->result_code = strtol((char *)result, &tmp, 10);
  p->result_data = (uint8_t *)tmp;
}

// parse next response char
void sim800_response_char(uint8_t c, sim800_t *p)
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
  if (sim800_cmd(p, "AT", true))
    return false;

  // Set APN
  if (sim800_cmd(p, "AT+CSTT=\"TM\"", true))
    return false;

  // Bring up wireless connection with GPRS or CSD
  if (sim800_cmd(p, "AT+CIICR", true))
    return false;

  // Get local IP address
  sim800_cmd(p, "AT+CIFSR", false);
  HAL_Delay(1000);

  // Start Up TCP Connection
  if (sim800_cmd(p, "AT+CIPSTART=\"TCP\",\"mail-verif.com\",20300", true))
    return false;
  return true;
}

bool sim800_send(sim800_t *p, char *msg)
{
  char cmd[22];
  sprintf((char *)&cmd, "AT+CIPSEND=%d", strlen(msg));
  if (sim800_cmd(p, cmd, false))
    return false;

  HAL_Delay(1000);
  if (sim800_cmd(p, msg, true))
    return false;

  return true;
}

void sim800_disconnect(sim800_t *p)
{
  // Close TCP Connection
  sim800_cmd(p, "AT+CIPCLOSE", false);
  HAL_Delay(1000);

  // Deactivate GPRS PDP Context
  sim800_cmd(p, "AT+CIPSHUT", false);
}
