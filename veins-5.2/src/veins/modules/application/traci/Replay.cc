#include "veins/modules/application/traci/Replay.h"
#include <algorithm>
#include <sstream>

using namespace veins;

Define_Module(Replay);

void Replay::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        // Get configuration parameters
        nodeId = getParentModule()->getIndex(); // Get the vehicle's index as its ID
        isAttacker = par("isAttacker").boolValue(); // Define attacker based on node ID
        attackTime = par("attackTime").doubleValue();
        senderNode = par("senderNode").intValue();
        //attackType = par("attackType").intValue(); // Attack type: 1 = Drop, 2 = Tamper


        // Initialize status
        hasAttacked = false;
        hasReceivedWarning = false;
        hasSentMessage = false;


        if (isAttacker) {
            findHost()->getDisplayString().setTagArg("i", 1, "yellow");

            EV_INFO << "MITM Attack Vehicle initialized as Node " << nodeId << " at " << simTime() << endl;

        }
        else {
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

void Replay::handleSelfMsg(cMessage* msg)
{
    if (msg == sendMessageEvt && !hasSentMessage && nodeId == senderNode) {
        sendNormalMessage();
        hasSentMessage = true;
        EV_INFO << "Vehicle " << nodeId << " sent normal message at " << simTime() << endl;
    }
}

void Replay::sendNormalMessage()
{
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    populateWSM(wsm);
    wsm->setDemoData("Emergency");
    sendDown(wsm);
    EV_INFO << "Vehicle " << nodeId << " sent message to attacker " << attackerNodeId << " at " << simTime() << endl;
}

void Replay::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    std::string message = wsm->getDemoData();

    if (isAttacker && !hasAttacked) {

        if (message.find("Emergency") != std::string::npos) {
            TraCIDemo11pMessage* replayMsg = wsm->dup();
            replayMsg->setDemoData("Emergency");
            sendDown(replayMsg);
            hasAttacked = true;
            EV_INFO << "Vehicle " << nodeId << " (Attacker) modified and forwarded message at " << simTime() << endl;
        }

    }
    else if (!isAttacker && !hasReceivedWarning && nodeId != senderNode) {
        // Other vehicles receive the modified message
        if (message.find("replay") != std::string::npos) {
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
}

void Replay::finish()
{
    if (isAttacker) {
        EV_INFO << "Attacker vehicle " << nodeId << " attack completed at " << simTime() << endl;
    }
    else if (hasReceivedWarning) {
        EV_INFO << "Vehicle " << nodeId << " was affected by the attack" << endl;
    }

    DemoBaseApplLayer::finish();
}
