#pragma once

#include "meta.hpp"
#include "lc.hpp"

#include <map>
#include <string>
#include <functional>

namespace Digiham::Dmr {

    class Slot {
        public:
            void setSync(int sync);
            void setType(int type);
            void setSource(uint32_t source);
            void setTarget(uint32_t target);
            void setFromLc(Lc* lc);
            void reset();
            bool isDirty();
            void setClean();
            std::map<std::string, std::string> collect();
        private:
            void setDirty();
            std::string getSyncName() const;
            std::string getTypeName() const;
            bool dirty = false;
            int sync = -1;
            int type = -1;
            uint32_t source;
            uint32_t target;
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