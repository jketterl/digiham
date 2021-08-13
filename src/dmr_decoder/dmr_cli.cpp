#include "dmr_cli.hpp"

using namespace Digiham::Dmr;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Cli::~Cli() {
    if (fifo != nullptr) {
        fclose(fifo);
    }
    if (fifoReader != nullptr) {
        fifoReader->join();
        delete fifoReader;
        fifoReader = nullptr;
    }
}

std::string Cli::getName() {
    return "dmr_decoder";
}

Decoder* Cli::buildModule() {
    decoder = new Decoder();
    decoder->setMetaWriter(metaWriter);
    return decoder;
}

std::stringstream Cli::getUsageString() {
    std::stringstream ss = Digiham::DecoderCli::getUsageString();
    ss << " -c, --control-fifo  read control messages from this file\n";
    return ss;
}

std::vector<struct option> Cli::getOptions() {
    std::vector<struct option> options = Digiham::DecoderCli::getOptions();
    options.push_back({"control-fifo", required_argument, NULL, 'c'});
    return options;
}

bool Cli::receiveOption(int c, char* optarg) {
    switch (c) {
        case 'c': {
            fifo = fopen(optarg, "r");
            if (fifo == nullptr) break;
            fifoReader = new std::thread([this] () { fifoLoop(); });
            break;
        }
        default:
            return Digiham::DecoderCli::receiveOption(c, optarg);
    }
    return true;
}

void Cli::fifoLoop() {
    size_t control_bufsize = 32;
    char* control_line = (char*) malloc(sizeof(char) * control_bufsize);

    while (true) {
        int error;
        while (!(error = ferror(fifo)) && fread(control_line, sizeof(char), 2, fifo) >= 2) {
            if (control_line[1] == '\n') {
                unsigned char slot_filter = control_line[0] - '0';
                decoder->setSlotFilter(slot_filter);
            }
        }

        // the control fifo is in non-blocking mode so EAGAIN is expected
        if (error) {
            fclose(fifo);
            fifo = NULL;
            break;
        } else {
            clearerr(fifo);
        }
    }
}