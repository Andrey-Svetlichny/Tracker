#include "main.h"
#include "sim800.h"

#define SIM800_MAX_COMMAND_LEN 255 ///< Maximum command length
#define SIM800_MAX_RESPONSE_LEN 255 ///< Maximum response length

typedef struct {
  uint8_t command_len;            ///< Length of command
  uint8_t command[SIM800_MAX_COMMAND_LEN]; ///< command
  uint8_t response_len;            ///< Length of response
  uint8_t response[SIM800_MAX_RESPONSE_LEN]; ///< response
  bool response_received;
} sim800_t;


static void sim800_response_clear(sim800_t *p)
{
    memset(p->response, 0x00, SIM800_MAX_RESPONSE_LEN);
    p->response_len = 0;
}

static void sim800_parse_response(sim800_t *p)
{
  // compare response with command
  uint16_t size = p->command_len;
  if (!strncmp((char*)p->command, (char*)p->response, size) && p->response[p->response_len] == '\r')
  {
    // display(p->response);
    // uint8_t result = p->response + size + 1;
    // response match command
    p->response_received = true;
  } else {
    display("not match");
  }
}


// send command to SIM800L, return response
static char* sim800_cmd(char* cmd, sim800_t *p, void (*transmit)())
{
  uint32_t timeout = 1000;
  p->command_len = strlen(cmd);
  if (p->command_len + 2 > SIM800_MAX_COMMAND_LEN)
  {
    return "cmd too long";
  }
  
  strcpy((char*)p->command, cmd);
  strcpy((char*)p->command + p->command_len, "\n\r");
  transmit(p->command);

  uint32_t tickstart = HAL_GetTick();
  while (HAL_GetTick() - tickstart < timeout)
  {
    if (p->response_received)
    {
      p->response_received = false;
      // verify result
      display((char*)p->response);
    }
  }

  // timeout
  return "timeout";  
}

static void sim800_parse_char(uint8_t c, sim800_t *p)
{
  p->response[p->response_len] = c;
  if (p->response_len > 0 && p->response[p->response_len] == '\n' && p->response[p->response_len-1] == '\r')
  {
    // string received
    sim800_parse_response(p);
  }
  else if (++p->response_len == SIM800_MAX_RESPONSE_LEN) 
  {
    // response buffer full
    sim800_response_clear(p);
  }
}
