#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SIM800_MAX_COMMAND_LEN 255  ///< Maximum command length
#define SIM800_MAX_RESPONSE_LEN 255 ///< Maximum response length

typedef struct sim800_t sim800_t;
typedef struct sim800_t
{
    uint8_t command[SIM800_MAX_COMMAND_LEN];   ///< command
    bool result_expected;                      ///< expected result_code/result_data
    uint8_t response_len;                      ///< length of response
    uint8_t response[SIM800_MAX_RESPONSE_LEN]; ///< response
    bool response_match;                       ///< response match command
    uint8_t result_code;                       ///< response result code
    uint8_t *result_data;                      ///< response result data
    void (*onError)(sim800_t *self);
    void (*transmit)(char *cmd);
} sim800_t;

void sim800_response_char(uint8_t c, sim800_t *p);
bool sim800_connect(sim800_t *p);
bool sim800_send(sim800_t *p, char *msg);
void sim800_disconnect(sim800_t *p);
