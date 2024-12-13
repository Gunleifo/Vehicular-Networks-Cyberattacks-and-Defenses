#include "veins/modules/application/traci/RSU.h"
#include <algorithm>
#include <sstream>

using namespace veins;

Define_Module(RSU);

void RSU::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        // Get configuration parameters
        isRSU = par("isRSU").boolValue();
        isAttacker = isRSU && par("isAttacker").boolValue();
        attackTime = par("attackTime").doubleValue();
        senderNode = par("senderNode").intValue();

        nodeId = getParentModule()->getIndex();

        // Initialize status
        hasAttacked = false;
        hasReceivedWarning = false;
        hasSentMessage = false;

        if (isRSU) {
            if (isAttacker) {
                findHost()->getDisplayString().setTagArg("i", 1, "red");
                findHost()->getDisplayString().setTagArg("is", 0, "2");

                EV_INFO << "MITM Attack RSU initialized at " << simTime() << endl;
            }
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

void RSU::handleSelfMsg(cMessage* msg)
{
    if (msg == sendMessageEvt && !hasSentMessage && nodeId == senderNode) {
        sendNormalMessage();
        hasSentMessage = true;
        EV_INFO << "Vehicle " << nodeId << " sent normal message at " << simTime() << endl;
    }
}

void RSU::sendNormalMessage()
{
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    populateWSM(wsm);
    wsm->setDemoData("STATUS:normal");
    sendDown(wsm);
}

void RSU::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    std::string message = wsm->getDemoData();

    if (isRSU && isAttacker && !hasAttacked) {
        if (message.find("STATUS:normal") != std::string::npos) {
            // RSU modifies and forwards the message
            TraCIDemo11pMessage* modifiedMsg = wsm->dup();
            modifiedMsg->setDemoData("stop");
            sendDown(modifiedMsg);

            hasAttacked = true;
            EV_INFO << "RSU modified and forwarded message at " << simTime() << endl;
        }
    }
    else if (!isRSU && !hasReceivedWarning && nodeId != senderNode) {
        // Other vehicles receive the modified message
        if (message.find("WARNING:accident") != std::string::npos) {
            hasReceivedWarning = true;

            // Visual feedback
            findHost()->getDisplayString().setTagArg("i", 1, "red");

            if (traciVehicle) {
                traciVehicle->setSpeed(0);
                traciVehicle->setColor(TraCIColor(255, 0, 0, 255));
            }

            EV_INFO << "Vehicle " << nodeId << " stopped due to accident warning at " << simTime() << endl;
        }
    }
    else if(nodeId == senderNode){
        //findHost()->getDisplayString().setTagArg("i", 1, "blue");
        if (message.find("stop") != std::string::npos) {
            if (traciVehicle) {
                traciVehicle->setSpeed(0);
                traciVehicle->setColor(TraCIColor(255, 0, 0, 255));
            }
         }

    }
}

void RSU::finish()
{
    if (isRSU && isAttacker) {
        EV_INFO << "RSU attack completed at " << simTime() << endl;
    }
    else if (hasReceivedWarning) {
        EV_INFO << "Vehicle " << nodeId << " was affected by the attack" << endl;
    }

    DemoBaseApplLayer::finish();
}
