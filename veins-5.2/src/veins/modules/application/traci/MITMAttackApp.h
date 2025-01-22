#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace veins {

class MITMAttackApp : public DemoBaseApplLayer {
public:
    MITMAttackApp() : sendMessageEvt(nullptr),
                      isAttacker(false),
                      hasAttacked(false), hasReceivedWarning(false),
                      hasSentMessage(false), nodeId(-1), senderNode(-1),
                      attackTime(0){}

protected:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void finish() override;

private:
    void sendNormalMessage();
    int simpleHash(const std::string& message);
    int EncryptDecrypt(int hash);
    int signMessage(int hash);
    bool verifySignature(const std::string& message, int signature);

    cMessage* sendMessageEvt;

    bool isAttacker;
    bool hasAttacked;
    bool hasReceivedWarning;
    bool hasSentMessage;

    int SymmetricKey;
    int modulus;

    int nodeId;
    int senderNode;
    double attackTime;

    int attackType;
    int attackerNodeId;
    int digitalsignature;
};

}
