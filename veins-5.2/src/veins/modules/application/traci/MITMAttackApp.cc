#include "veins/modules/application/traci/MITMAttackApp.h"
#include <algorithm>
#include <sstream>

using namespace veins;

Define_Module(MITMAttackApp);

void MITMAttackApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        // Key generation (simple example)
        modulus = 12;
        SymmetricKey = 13579;

        // Get configuration parameters
        nodeId = getParentModule()->getIndex(); // Get the vehicle's index as its ID
        isAttacker = par("isAttacker").boolValue(); // Define attacker based on node ID
        attackTime = par("attackTime").doubleValue();
        senderNode = par("senderNode").intValue();

        // Initialize status
        hasAttacked = false;
        hasReceivedWarning = false;
        hasSentMessage = false;

        if (isAttacker) {
            findHost()->getDisplayString().setTagArg("i", 1, "yellow");

            EV_INFO << "MITM Attack Vehicle initialized as Node " << nodeId << " at " << simTime() << endl;
        } else {
            // Only sender vehicle creates timer
            if (nodeId == senderNode) {
                sendMessageEvt = new cMessage("sendMessage");
                // Schedule message slightly before attack time
                scheduleAt(simTime() + (attackTime - 1), sendMessageEvt);

                // Make sender vehicle visually distinct
                findHost()->getDisplayString().setTagArg("i", 1, "blue");
                EV_INFO << "Sender vehicle " << nodeId << " initialized" << endl;
            }
        }
    }
}

void MITMAttackApp::handleSelfMsg(cMessage* msg)
{
    if (msg == sendMessageEvt && !hasSentMessage && nodeId == senderNode) {
        sendNormalMessage();
        hasSentMessage = true;
        EV_INFO << "Vehicle " << nodeId << " sent normal message at " << simTime() << endl;
    }
}

void MITMAttackApp::sendNormalMessage()
{
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    populateWSM(wsm);

    std::string message = "STATUS:normal";
    int hash = simpleHash(message);
    int signature = EncryptDecrypt(hash);

    wsm->setDemoData(message.c_str());
    wsm->addPar("signature") = signature;

    sendDown(wsm);
    EV_INFO << "Vehicle " << nodeId << " sent signed message with hash " << hash
            << " and signature " << signature << " at " << simTime() << endl;
}

void MITMAttackApp::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    std::string receivedMessage = wsm->getDemoData();
    int receivedSignature = wsm->par("signature");


    if (isAttacker && !hasAttacked) {
        if (receivedMessage.find("STATUS:normal") != std::string::npos) {
            TraCIDemo11pMessage* modifiedMsg = wsm->dup();
            modifiedMsg->setDemoData("WARNING:accident");
            modifiedMsg->addPar("signature") = receivedSignature;
            sendDown(modifiedMsg);
            hasAttacked = true;
            EV_INFO << "Vehicle " << nodeId << " (Attacker) modified and forwarded message at " << simTime() << endl;
        }
    } else if (!isAttacker && !hasReceivedWarning && nodeId != senderNode) {
        if (receivedMessage.find("WARNING:accident") != std::string::npos) {

            if (verifySignature(receivedMessage, receivedSignature) == false) {
                //
            } else if ((verifySignature(receivedMessage, receivedSignature) == true)){
                hasReceivedWarning = true;
                findHost()->getDisplayString().setTagArg("i", 1, "red");

                if (traciVehicle) {
                    traciVehicle->setSpeed(0);
                    traciVehicle->setColor(TraCIColor(255, 0, 0, 255));
                }

                EV_INFO << "Vehicle " << nodeId << " stopped due to accident warning at " << simTime() << endl;
            }
        }
    }
}

void MITMAttackApp::finish()
{
    if (isAttacker) {
        EV_INFO << "Attacker vehicle " << nodeId << " attack completed at " << simTime() << endl;
    } else if (hasReceivedWarning) {
        EV_INFO << "Vehicle " << nodeId << " was affected by the attack" << endl;
    }

    DemoBaseApplLayer::finish();
}

int MITMAttackApp::simpleHash(const std::string& message)
{
    int hash = 0;
    int prime=31;
    for (char c : message) {
        hash = (hash * prime + c) % modulus;
    }
    return hash;
}

int MITMAttackApp::EncryptDecrypt(int hash)
{
    return hash ^ SymmetricKey;
}

bool MITMAttackApp::verifySignature(const std::string& message, int signature)
{
    int newHash = simpleHash(message);
    int expectedHash = EncryptDecrypt(signature);
    return newHash == expectedHash;
}
