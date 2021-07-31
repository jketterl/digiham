#include "dstar_decoder.hpp"
#include "dstar_phase.hpp"
#include "dstar_meta.hpp"

using namespace Digiham::DStar;

Decoder::Decoder():
    Digiham::Decoder(new SyncPhase(), new MetaCollector())
{}