#include "display.h"
#include "main.h"
#include "sim800.h"

// send command to SIM800L, return: success
static bool sim800_cmd(sim800_t *p, char *cmd, void (*responseParser)(sim800_t *self), uint32_t timeout)
{
  uint8_t len = strlen(cmd);

  // validate cmd
  if (p->executing)
    return SIM800_RESULT_ANOTHER_EXECUTING;

  if (len + 2 > SIM800_MAX_COMMAND_LEN)
  {
    strcpy((char *)p->response, "cmd too long\n\r");
    p->response_len = 14;
    p->result = SIM800_RESULT_COMMAND_TOO_LONG;
    p->onError(p);
    return p->result;
  }

  // prepare p
  strcpy((char *)p->command, cmd);
  strcpy((char *)p->command + len, "\n\r");
  memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
  p->response_len = 0;
  p->result = SIM800_RESULT_NO_RESULT_YET;
  p->resultData = NULL;
  p->parse = responseParser;

  // transmit
  p->executing = true;
  p->transmit((char *)p->command);

  // wait for result from p->validate
  uint32_t tickstart = HAL_GetTick();
  while (HAL_GetTick() - tickstart < timeout)
  {
    if (!p->executing && p->result != SIM800_RESULT_NO_RESULT_YET)
    {
      if (p->result == SIM800_RESULT_SUCCESS)
        return true;

      p->onError(p);
      return false;
    }
  }

  // timeout
  p->executing = false;
  strcpy((char *)p->response, "timeout\n\r");
  p->response_len = 9;
  p->result = SIM800_RESULT_TIMEOUT;
  p->onError(p);
  return false;
}

// next char received from UART
void sim800_receiveChar(uint8_t c, sim800_t *p)
{
  if (!p->executing)
  {
    // ignore
    return;
  }

  if (p->response_len == SIM800_MAX_RESPONSE_LEN)
  {
    // response buffer full
    p->executing = false;
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
  }

  p->response[p->response_len++] = c;
  if (p->response_len > 1 && p->response[p->response_len - 1] == '\n' && p->response[p->response_len - 2] == '\r')
  {
    // new line received, parse response
    p->parse(p);
  }
}

static void sim800_parse_ok(sim800_t *p)
{
  if (!strcmp("\r\nOK\r\n", (char *)p->response))
  {
    p->executing = false;
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }
  if (!strcmp("\r\nERROR\r\n", (char *)p->response))
  {
    p->executing = false;
    p->result = SIM800_RESULT_ERROR;
    return;
  }
  if (strlen((char *)p->response) > 9)
  {
    p->executing = false;
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
    return;
  }
}

static void sim800_parse_status(sim800_t *p)
{
  if (strlen((char *)p->response) < 15)
    return;

  if (strncmp("\r\nOK\r\n\r\nSTATE: ", (char *)p->response, 6))
  {
    p->executing = false;
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
    return;
  }
  p->executing = false;
  p->resultData = p->response + 15;
  p->result = SIM800_RESULT_SUCCESS;
}

static void sim800_showStatus(sim800_t *p)
{
  // Query Current Connection Status
  display("AT+CIPSTATUS");
  HAL_Delay(500);
  if (!sim800_cmd(p, "AT+CIPSTATUS", sim800_parse_status, 1000))
    return;
  display((char *)p->resultData);
  HAL_Delay(1000);
}

bool sim800_connect(sim800_t *p)
{
  // Check if SIM800 ok?
  display("AT");
  HAL_Delay(500);
  if (!sim800_cmd(p, "AT", sim800_parse_ok, 1000))
    return false;
  display("AT OK");
  HAL_Delay(1000);

  sim800_showStatus(p);

  // Set APN;
  // IP INITIAL => IP START
  display("Set APN");
  HAL_Delay(500);
  if (!sim800_cmd(p, "AT+CSTT=\"TM\"", sim800_parse_ok, 1000))
    return false;
  display("Set APN OK");
  HAL_Delay(1000);

  sim800_showStatus(p);

  // Bring up wireless connection with GPRS or CSD
  // IP START => IP CONFIG => IP GPRSACT
  display("GPRS");
  HAL_Delay(500);
  if (!sim800_cmd(p, "AT+CIICR", sim800_parse_ok, 10000))
    return false;
  display("GPRS OK");
  HAL_Delay(1000);

  /*
    // Get local IP address
    display("Get IP");
    HAL_Delay(500);
    // expected for example 10.4.100.193 => IP STATUS
    if (!sim800_cmd(p, "AT+CIFSR", sim800_parse_ok, 1000))
      return false;
    display("IP = ");
    HAL_Delay(1000);

    // Start Up TCP Connection
    display("Connect TCP");
    HAL_Delay(500);
    // expected "OK"
    if (sim800_cmd(p, "AT+CIPSTART=\"TCP\",\"mail-verif.com\",20300", true))
      return false;
    display("Connect TCP OK");
    HAL_Delay(1000);
   */
  return true;
}

bool sim800_send(sim800_t *p, char *msg)
{
  // char cmd[22];
  // sprintf((char *)&cmd, "AT+CIPSEND=%d", strlen(msg));
  // expected ">"
  // if (sim800_cmd(p, cmd, false))
  //   return false;

  // HAL_Delay(1000);
  // expected "SEND OK"
  // if (sim800_cmd(p, msg, true))
  //   return false;

  return true;
}

void sim800_disconnect(sim800_t *p)
{
  // Close TCP Connection adnd deactivate GPRS PDP Context
  // expected "SHUT OK" => IP INITIAL
  sim800_cmd(p, "AT+CIPSHUT", sim800_parse_ok, 10000);
}
