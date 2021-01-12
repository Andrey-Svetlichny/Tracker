/*
#define SIM800_MAX_COMMAND_LEN 255  ///< Maximum command length
#define SIM800_MAX_RESPONSE_LEN 255 ///< Maximum response length

typedef struct {
  uint8_t command[SIM800_MAX_COMMAND_LEN];   ///< command
  uint8_t response_len;                      ///< length of response
  uint8_t response[SIM800_MAX_RESPONSE_LEN]; ///< response
  bool response_match;                       ///< response match command
  uint8_t result_code;                       ///< response result code
  uint8_t *result_data;                      ///< response result data
} sim800_t;

static bool sim800_cmd(char* cmd, sim800_t *p, void (*transmit)());
static bool sim800_response_match_command(sim800_t *p);
static bool sim800_parse_char(uint8_t c, sim800_t *p);
static void sim800_response_clear(sim800_t *p);
*/