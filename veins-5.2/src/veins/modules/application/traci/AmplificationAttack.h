#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace veins {

class AmplificationAttack : public DemoBaseApplLayer {
public:
    AmplificationAttack() : sendMessageEvt(nullptr), 
                            resetSpeedEvt(nullptr), 
                            originalSpeed(0.0),
                            isRSU(false), isAttacker(false),
                            hasAttacked(false), hasReceivedWarning(false),
                            hasSentMessage(false), nodeId(-1), senderNode(-1),
                            targetVehicle(-1), attackTime(0.0) {}

    virtual ~AmplificationAttack() {
        cancelAndDelete(sendMessageEvt);
        cancelAndDelete(resetSpeedEvt);
    }

protected:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void finish() override;

private:
    void sendSmallRequest();
    void resetSpeed();

    cMessage* sendMessageEvt;  // Timer for sending the small request
    cMessage* resetSpeedEvt;   // Timer for resetting the vehicle's speed
    double originalSpeed;

    bool isRSU;
    bool isAttacker;
    bool hasAttacked;
    bool hasReceivedWarning;
    bool hasSentMessage;

    int nodeId;
    int senderNode;
    int targetVehicle;
    double attackTime;
};

}
