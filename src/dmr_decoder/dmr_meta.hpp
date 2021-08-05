#pragma once

#include "meta.hpp"

#include <map>
#include <string>
#include <functional>

namespace Digiham::Dmr {

    class Slot {
        public:
            void setSync(int sync);
            void reset();
            bool isDirty();
            void setClean();
            std::map<std::string, std::string> collect();
        private:
            void setDirty();
            std::string getSyncName() const;
            bool dirty = false;
            int sync = -1;
    };

    class MetaCollector: public Digiham::MetaCollector {
        public:
            MetaCollector();
            ~MetaCollector() override;
            void withSlot(int slot, const std::function<void(Slot*)>& callback);
            void sendMetaData() override;
            void sendMetaDataForSlot(int slot);
            void reset();
        protected:
            std::string getProtocol() override;
            Slot* slots[2];
    };

}