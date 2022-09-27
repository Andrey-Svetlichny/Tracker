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
    strcpy(p->response, "cmd too long\n\r");
    p->response_len = 14;
    p->result = SIM800_RESULT_COMMAND_TOO_LONG;
    p->onError(p);
    return p->result;
  }

  // prepare p
  strcpy(p->command, cmd);
  strcpy(p->command + len, "\n\r");
  memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
  p->response_len = 0;
  p->result = SIM800_RESULT_NO_RESULT_YET;
  p->resultData = NULL;
  p->parse = responseParser;

  // transmit
  p->executing = true;
  p->transmit(p->command);

  // wait for result from p->validate
  uint32_t tickstart = HAL_GetTick();
  while (HAL_GetTick() - tickstart < timeout)
  {
    if (!p->response_len)
      continue;

    p->parse(p);

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
  strcpy(p->response, "timeout\n\r");
  p->response_len = 9;
  p->result = SIM800_RESULT_TIMEOUT;
  p->onError(p);
  return false;
}

static void sim800_parse_ok(sim800_t *p)
{
  p->executing = false;
  if (!strcmp("\r\nOK\r\n", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }
  if (!strcmp("\r\nERROR\r\n", p->response))
  {
    p->result = SIM800_RESULT_ERROR;
    return;
  }
  p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parse_status(sim800_t *p)
{
  p->executing = false;
  if (!strncmp("\r\nOK\r\n\r\nSTATE: ", p->response, 6))
  {
    p->resultData = p->response + 15;
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }
  p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parseIP(sim800_t *p)
{
  p->executing = false;
  if (strlen(p->response) < 5 || strlen(p->response) > 19 || p->response[p->response_len - 2] != '\r' || p->response[p->response_len - 1] != '\n')
  {
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
    return;
  }

  if (!strcmp("\r\nERROR\r\n", p->response))
  {
    p->result = SIM800_RESULT_ERROR;
    return;
  }

  p->resultData = p->response + 2;
  p->result = SIM800_RESULT_SUCCESS;
}

static void sim800_parse_connectTCP(sim800_t *p)
{
  p->executing = false;
  if (!strcmp("\r\nOK\r\n", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }

  if (!strcmp("\r\nALREADY CONNECT\r\n", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }

  if (!strcmp("\r\nCONNECT OK\r\n", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }

  if (!strncmp("\r\n+CME ERROR ", p->response, 13))
  {
    p->result = SIM800_RESULT_ERROR;
    return;
  }

  p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parse_send1(sim800_t *p)
{
  p->executing = false;
  if (!strcmp("\r\n> ", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }

  p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parse_send2(sim800_t *p)
{
  p->executing = false;
  if (!strcmp("\r\nSEND OK\r\n", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }

  p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parse_shut(sim800_t *p)
{
  p->executing = false;
  if (!strcmp("\r\nSHUT OK\r\n", p->response))
  {
    p->result = SIM800_RESULT_SUCCESS;
    return;
  }
  if (!strcmp("\r\nERROR\r\n", p->response))
  {
    p->result = SIM800_RESULT_ERROR;
    return;
  }
  p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

// Query Current Connection Status
static char *sim800_status(sim800_t *p)
{
  for (int i = 0;; i++)
  {
    if (sim800_cmd(p, "AT+CIPSTATUS", sim800_parse_status, 1000))
      return p->resultData;
    if (i == 3)
      break;
    HAL_Delay(1000);
  }
  return "AT+CIPSTATUS ERROR";
}

static void sim800_showStatus(sim800_t *p)
{
  char *status = sim800_status(p);
  display(status);
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

  // check status
  {
    char *status = sim800_status(p);
    display(status);
    HAL_Delay(1000);
    if (!strcmp("CONNECT OK", status))
      return true;
  }

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
  // Max Response Time 85 seconds
  // IP START => IP CONFIG => IP GPRSACT
  display("GPRS");
  HAL_Delay(500);
  if (!sim800_cmd(p, "AT+CIICR", sim800_parse_ok, 10000))
  {
    // wait
    display("waiting 3sec");
    HAL_Delay(3000);
  }

  // check status
  {
    char *status = sim800_status(p);
    display(status);
    HAL_Delay(1000);

    if (strcmp("IP GPRSACT", status))
      return false;
    display("GPRS OK");
    HAL_Delay(1000);
  }

  // Get local IP address
  display("Get IP");
  HAL_Delay(500);
  // expected for example 10.4.100.193 => IP STATUS
  if (!sim800_cmd(p, "AT+CIFSR", sim800_parseIP, 1000))
    return false;
  display(p->resultData);
  HAL_Delay(1000);

  sim800_showStatus(p);

  // Start Up TCP Connection
  // Max Response Time 160 seconds
  display("Connect TCP");
  HAL_Delay(500);
  // expected "CONNECT OK" || "STATE: <state>"
  if (!sim800_cmd(p, "AT+CIPSTART=\"TCP\",\"mail-verif.com\",20300", sim800_parse_connectTCP, 5000))
  {
    // wait
    display("waiting 5sec");
    HAL_Delay(5000);
  }
  // check status
  {
    char *status = sim800_status(p);
    display(status);
    HAL_Delay(1000);

    if (strcmp("CONNECT OK", status))
      return false;
    display("Connect TCP OK");
    HAL_Delay(1000);
  }

  display("Connect OK !!!");
  HAL_Delay(5000);

  return true;
}

bool sim800_send(sim800_t *p, char *msg)
{
  char cmd[22];
  sprintf((char *)&cmd, "AT+CIPSEND=%d", strlen(msg));
  display(cmd);
  // expected ">"
  if (!sim800_cmd(p, cmd, sim800_parse_send1, 100000))
    return false;

  HAL_Delay(1000);
  display(">");

  HAL_Delay(1000);
  // expected "SEND OK"
  display(msg);
  if (!sim800_cmd(p, msg, sim800_parse_send2, 5000))
    return false;
  // {
  //   // wait
  //   display("waiting 3sec");
  //   HAL_Delay(3000);

  //   // check status
  //   char *status = sim800_status(p);
  //   display(status);
  //   HAL_Delay(1000);

  //   if (strcmp("SEND OK", status))
  //     return false;
  //   display("SEND OK");
  //   HAL_Delay(1000);
  // }

  return true;
}

void sim800_disconnect(sim800_t *p)
{
  // Close TCP Connection adnd deactivate GPRS PDP Context
  // Max Response Time 65 seconds
  // expected "SHUT OK" => IP INITIAL
  if (!sim800_cmd(p, "AT+CIPSHUT", sim800_parse_shut, 10000))
  {
    // wait
    display("waiting 3sec");
    HAL_Delay(3000);

    // check status
    char *status = sim800_status(p);
    display(status);
    HAL_Delay(1000);

    if (strcmp("IP INITIAL", status))
      return;
  }

  display("Disonnect OK !!!");
  HAL_Delay(5000);
}
