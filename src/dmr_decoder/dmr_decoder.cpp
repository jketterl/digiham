#include "dmr_decoder.hpp"
#include "dmr_phase.hpp"
#include "dmr_meta.hpp"

using namespace Digiham::Dmr;

Decoder::Decoder(): Digiham::Decoder(new SyncPhase(), new MetaCollector()) {}