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
    strcpy(p->response, "cmd too long\r\n");
    p->response_len = 14;
    p->result = SIM800_RESULT_COMMAND_TOO_LONG;
    p->onError(p);
    return p->result;
  }

  // prepare p
  strcpy(p->command, cmd);
  strcpy(p->command + len, "\r\n");
  memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
  p->response_len = 0;
  p->result = SIM800_RESULT_NO_RESULT_YET;
  p->resultData = NULL;

  // transmit
  p->executing = true;
  p->transmit(p->command);

  // waiting for result
  uint32_t tickstart = HAL_GetTick();
  while (HAL_GetTick() - tickstart < timeout)
  {
    if (!p->response_len)
      continue;

    // result received
    p->executing = false;
    responseParser(p);
    if (p->result == SIM800_RESULT_SUCCESS)
      return true;

    p->onError(p);
    return false;
  }

  // timeout
  p->executing = false;
  strcpy(p->response, "timeout\r\n");
  p->response_len = 9;
  p->result = SIM800_RESULT_TIMEOUT;
  p->onError(p);
  return false;
}

static void sim800_parseOk(sim800_t *p)
{
  if (!strcmp("\r\nOK\r\n", p->response))
    p->result = SIM800_RESULT_SUCCESS;
  else if (!strcmp("\r\nERROR\r\n", p->response))
    p->result = SIM800_RESULT_ERROR;
  else
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parseStatus(sim800_t *p)
{
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
  if (p->response_len < 5 ||
      p->response_len > 19 ||
      p->response[p->response_len - 2] != '\r' ||
      p->response[p->response_len - 1] != '\n')
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
  else if (!strcmp("\r\nERROR\r\n", p->response))
    p->result = SIM800_RESULT_ERROR;
  else
  {
    p->resultData = p->response + 2;
    p->result = SIM800_RESULT_SUCCESS;
  }
}

static void sim800_parseConnectTCP(sim800_t *p)
{
  if (!strcmp("\r\nOK\r\n", p->response) ||
      !strcmp("\r\nALREADY CONNECT\r\n", p->response) ||
      !strcmp("\r\nCONNECT OK\r\n", p->response))
    p->result = SIM800_RESULT_SUCCESS;
  else if (!strncmp("\r\n+CME ERROR ", p->response, 13))
    p->result = SIM800_RESULT_ERROR;
  else
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parseSend1(sim800_t *p)
{
  if (!strcmp("\r\n> ", p->response))
    p->result = SIM800_RESULT_SUCCESS;
  else if (!strcmp("\r\nERROR\r\n", p->response))
    p->result = SIM800_RESULT_ERROR;
  else
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parseSend2(sim800_t *p)
{
  if (!strcmp("\r\nSEND OK\r\n", p->response))
    p->result = SIM800_RESULT_SUCCESS;
  else
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

static void sim800_parseShut(sim800_t *p)
{
  if (!strcmp("\r\nSHUT OK\r\n", p->response))
    p->result = SIM800_RESULT_SUCCESS;
  else if (!strcmp("\r\nERROR\r\n", p->response))
    p->result = SIM800_RESULT_ERROR;
  else
    p->result = SIM800_RESULT_RESPONSE_NOT_RECOGNIZED;
}

// Query Current Connection Status (with 3 retry)
static char *sim800_status(sim800_t *p)
{
  for (int i = 3;; i--)
  {
    if (sim800_cmd(p, "AT+CIPSTATUS", sim800_parseStatus, 1000))
      return p->resultData;
    if (!i)
      break;
    HAL_Delay(1000);
  }
  return "AT+CIPSTATUS ERROR";
}

// start TCP Connection
bool sim800_connect(sim800_t *p)
{
  // check SIM800 health
  if (!sim800_cmd(p, "AT", sim800_parseOk, 1000))
    return false;
  display("AT OK");
  HAL_Delay(1000);

  // check status
  char *status = sim800_status(p);
  display(status);
  HAL_Delay(1000);

  // if already connected, exit
  if (!strcmp("CONNECT OK", status))
    return true;

  // Set APN;
  // IP INITIAL => IP START
  if (!strcmp("IP INITIAL", status))
  {
    display("Set APN");
    HAL_Delay(500);
    if (!sim800_cmd(p, "AT+CSTT=\"TM\"", sim800_parseOk, 1000))
      return false;
    display("Set APN OK");
    HAL_Delay(1000);

    // check status
    status = sim800_status(p);
    display(status);
    HAL_Delay(1000);
  }

  // Activate GPRS or CSD
  // Max Response Time 85 seconds
  // IP START => IP CONFIG => IP GPRSACT
  if (!strcmp("IP START", status))
  {
    display("GPRS");
    HAL_Delay(500);
    if (!sim800_cmd(p, "AT+CIICR", sim800_parseOk, 30000)) // don't wait 85 sec
    {
      // wait
      display("waiting 3sec");
      HAL_Delay(3000);
    }

    // check status
    status = sim800_status(p);
    display(status);
    HAL_Delay(1000);

    if (strcmp("IP GPRSACT", status))
      return false;
    display("GPRS OK");
    HAL_Delay(1000);
  }

  // Get local IP address
  if (!strcmp("IP GPRSACT", status))
  {
    display("Get IP");
    HAL_Delay(500);
    // expected for example 10.4.100.193 => IP STATUS
    if (!sim800_cmd(p, "AT+CIFSR", sim800_parseIP, 1000))
      return false;
    display(p->resultData);
    HAL_Delay(1000);

    // check status
    status = sim800_status(p);
    display(status);
    HAL_Delay(1000);
  }

  // Start Up TCP Connection
  // Max Response Time 160 seconds
  if (!strcmp("IP STATUS", status))
  {
    display("Connect TCP");
    HAL_Delay(500);
    if (!sim800_cmd(p, "AT+CIPSTART=\"TCP\",\"mail-verif.com\",20300", sim800_parseConnectTCP, 30000))
    {
      // wait
      display("waiting 5sec");
      HAL_Delay(5000);
    }
    // check status
    status = sim800_status(p);
    display(status);
    HAL_Delay(1000);

    if (strcmp("CONNECT OK", status))
      return false;
  }

  display("Connect TCP OK");
  HAL_Delay(2000);
  return true;
}

bool sim800_send(sim800_t *p, char *msg)
{
  char cmd[22];
  sprintf((char *)&cmd, "AT+CIPSEND=%d", strlen(msg));
  display(cmd);
  if (!sim800_cmd(p, cmd, sim800_parseSend1, 1000))
    return false;
  // Max Response Time 645 seconds
  if (!sim800_cmd(p, msg, sim800_parseSend2, 5000))
    return false;

  return true;
}

// Close TCP Connection and deactivate GPRS PDP Context
void sim800_disconnect(sim800_t *p)
{
  // Max Response Time 65 seconds
  // => IP INITIAL
  if (!sim800_cmd(p, "AT+CIPSHUT", sim800_parseShut, 10000))
  {
    // check status
    char *status = sim800_status(p);
    display(status);
    HAL_Delay(1000);

    if (strcmp("IP INITIAL", status))
      return;
  }

  display("Disonnect OK");
  HAL_Delay(3000);
}
