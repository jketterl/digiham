// derived from the trellis state machine described in the ysf spec, Appendix B
uint8_t trellis_transitions[16][2] = {
    {0b00, 0b11}, // 0000
    {0b11, 0b00}, // 0001
    {0b10, 0b01}, // 0010
    {0b01, 0b10}, // 0011
    {0b01, 0b10}, // 0100
    {0b10, 0b01}, // 0101
    {0b11, 0b00}, // 0110
    {0b00, 0b11}, // 0111
    {0b01, 0b10}, // 1000
    {0b10, 0b01}, // 1001
    {0b11, 0b00}, // 1010
    {0b00, 0b11}, // 1011
    {0b00, 0b11}, // 1100
    {0b11, 0b00}, // 1101
    {0b10, 0b01}, // 1110
    {0b01, 0b10}  // 1111
};
