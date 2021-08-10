#include "ysf_decoder.hpp"
#include "ysf_phase.hpp"
#include "ysf_meta.hpp"

using namespace Digiham::Ysf;

Decoder::Decoder(): Digiham::Decoder(new SyncPhase(), new MetaCollector()) {}