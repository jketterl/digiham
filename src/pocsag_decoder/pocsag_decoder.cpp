#include "pocsag_decoder.hpp"
#include "pocsag_phase.hpp"

using namespace Digiham::Pocsag;

Decoder::Decoder(): Digiham::Decoder(new SyncPhase()) {}