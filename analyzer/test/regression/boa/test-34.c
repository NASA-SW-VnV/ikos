// SAFE
typedef enum {
  CONST_4_I = 3,
  CONST_3_I,
  CONST_2_I,
  CONST_1_I,
} cnst_t;

typedef unsigned int __uint32_t;
typedef long unsigned int size_t;
typedef unsigned short int __uint16_t;
typedef __uint16_t env_t;

static const char PACKET_URL[] = "foo.org";

typedef struct {
  char url[sizeof PACKET_URL];
  __uint32_t timestamp;
} pkt_hdr_t;

typedef env_t pkt_env_tbl_t[4];

typedef struct {
  pkt_hdr_t header;
  pkt_env_tbl_t environment_table;
} types_packet_t;

static struct { types_packet_t packet_structure; } g;

__uint16_t abs_bb_get(const cnst_t cnst_id, const int err);

void fill_env_tbl(void) {
  static const cnst_t CONSTANTS[] = {CONST_1_I,
                                     CONST_2_I,
                                     CONST_3_I,
                                     CONST_4_I};
  static const size_t N = sizeof(CONSTANTS) / sizeof(cnst_t);

  size_t i = 0;
  for (; i < N; i++) {
    g.packet_structure.environment_table[i] = abs_bb_get(CONSTANTS[i], 0);
  }
}

// BEGIN MODIFICATION

// We need to have a main function, otherwise the variable N will be
// ignored so the value of "i" cannot be determined.
// The variable N is declared as static so it's translated by the
// front-end as a global variable. If N wouldn't be static then we
// wouldn't need the main function.
int main() {
  fill_env_tbl();
  return 42;
}
// END MODIFICATION
