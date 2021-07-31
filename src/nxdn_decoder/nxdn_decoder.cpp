#include "nxdn_decoder.hpp"
#include "nxdn_meta.hpp"
#include "nxdn_phase.hpp"

using namespace Digiham::Nxdn;

Decoder::Decoder(): Digiham::Decoder(new SyncPhase, new MetaCollector) {}

