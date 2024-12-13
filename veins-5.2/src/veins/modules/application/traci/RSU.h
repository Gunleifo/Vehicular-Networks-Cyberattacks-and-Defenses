#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace veins {

class RSU : public DemoBaseApplLayer {
public:
    RSU() : sendMessageEvt(nullptr),
                      isRSU(false), isAttacker(false),
                      hasAttacked(false), hasReceivedWarning(false),
                      hasSentMessage(false), nodeId(-1), senderNode(-1) {}

    virtual ~RSU() {
        cancelAndDelete(sendMessageEvt);
    }

protected:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void finish() override;

private:
    void sendNormalMessage();

    cMessage* sendMessageEvt;

    bool isRSU;
    bool isAttacker;
    bool hasAttacked;
    bool hasReceivedWarning;
    bool hasSentMessage;

    int nodeId;
    int senderNode;
    double attackTime;
};

}
