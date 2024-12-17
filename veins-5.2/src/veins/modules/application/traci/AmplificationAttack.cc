#include "AmplificationAttack.h"
#include <algorithm>
#include <sstream>

using namespace veins;

Define_Module(AmplificationAttack);

void AmplificationAttack::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);

    if (stage == 0) {
        isRSU = par("isRSU").boolValue();
        isAttacker = par("isRSU").boolValue() && par("isAttacker").boolValue();
        attackTime = par("attackTime").doubleValue();
        senderNode = par("senderNode").intValue();
        targetVehicle = par("targetVehicle").intValue();

        nodeId = getParentModule()->getIndex();

        // Initialize
        hasAttacked = false;
        hasReceivedWarning = false;
        hasSentMessage = false;
        resetSpeedEvt = nullptr;
        originalSpeed = 0;

        EV_INFO << "Node " << nodeId 
                << " isRSU: " << isRSU 
                << " isAttacker: " << isAttacker 
                << " targetVehicle: " << targetVehicle 
                << " senderNode: " << senderNode << endl;

        if (isRSU) {
            if (isAttacker) {
                findHost()->getDisplayString().setTagArg("i", 1, "red");
                findHost()->getDisplayString().setTagArg("is", 0, "2");

                EV_INFO << "Amplification Attack RSU initialized at " << simTime() << endl;
            }
        } else {
            if (nodeId == senderNode) {
                sendMessageEvt = new cMessage("sendSmallRequest");
                scheduleAt(simTime() + (attackTime - 1), sendMessageEvt);

                // Visual indication for Attacker
                findHost()->getDisplayString().setTagArg("i", 1, "blue");
                EV_INFO << "Sender vehicle " << nodeId << " initialized" << endl;
            }
        }
    }
}

void AmplificationAttack::handleSelfMsg(cMessage* msg)
{
    if (msg == sendMessageEvt && !hasSentMessage && nodeId == senderNode) {
        // Send small initial request
        sendSmallRequest();
        hasSentMessage = true;
        EV_INFO << "Vehicle " << nodeId << " sent small request at " << simTime() << endl;
    } else if (msg == resetSpeedEvt) {
        // Recovery to normal speed
        if (traciVehicle) {
            traciVehicle->setSpeed(originalSpeed);
            EV_INFO << "Vehicle " << nodeId 
                    << " restored to normal speed at " << simTime() << endl;
        }

        // Clean up the timer
        cancelAndDelete(resetSpeedEvt);
        resetSpeedEvt = nullptr;
    }
}

void AmplificationAttack::sendSmallRequest()
{
    // Send a small trigger message
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    populateWSM(wsm);
    
    // Broadcast the message to ensure it reaches the RSU
    wsm->setRecipientAddress(-1);  // Broadcast address
    wsm->setDemoData("SmallRequest");
    wsm->setByteLength(1024); //1 kilobyte
    sendDown(wsm);
}

void AmplificationAttack::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    std::string message = wsm->getDemoData();

    EV_INFO << "Node " << nodeId 
            << " received message: " << message
            << endl;

    // RSU logic
    if (isRSU && isAttacker && !hasAttacked) {
        // Check for small trigger message
        if (message.find("SmallRequest") != std::string::npos) {
            // Send large amplified message to target vehicle
            TraCIDemo11pMessage* amplifiedMsg = new TraCIDemo11pMessage();
            populateWSM(amplifiedMsg);
            
            // Broadcast to ensure message reaches target vehicle
            amplifiedMsg->setRecipientAddress(-1);  // Broadcast address
            
            std::string largeMessage = "AmplificationMessage:" + std::to_string(targetVehicle);
            
            amplifiedMsg->setDemoData(largeMessage.c_str());
            amplifiedMsg->setByteLength(1073741824); //1 gigabyte
            sendDown(amplifiedMsg);

            hasAttacked = true;
            EV_INFO << "RSU sent amplified message with content: " << largeMessage 
                    << " at " << simTime() << endl;
        }
    }
    // Target vehicle logic
    else if (!isRSU) {
        // Check for amplification message
        std::string amplificationPrefix = "AmplificationMessage:" + std::to_string(nodeId);
        
        EV_INFO << "Checking amplification condition for vehicle " << nodeId 
                << " with message: " << message 
                << " Looking for: " << amplificationPrefix << endl;

        if (message.find(amplificationPrefix) != std::string::npos) {
            EV_INFO << "Amplification message detected for vehicle " << nodeId << endl;

            if (traciVehicle) {
                originalSpeed = traciVehicle->getSpeed();

                // Slow down the vehicle to 20% of its current speed
                double reducedSpeed = originalSpeed * 0.2;
                traciVehicle->setSpeed(reducedSpeed);

                // Speed recovery after 15 seconds
                if (!resetSpeedEvt) {
                    resetSpeedEvt = new cMessage("resetSpeed");
                    scheduleAt(simTime() + 15, resetSpeedEvt);
                }

                EV_INFO << "Target vehicle " << nodeId 
                        << " slowed down by amplification attack at " 
                        << simTime() << ". New speed: " << reducedSpeed << endl;
            } else {
                EV_ERROR << "TraCI Vehicle interface is null for node " << nodeId << endl;
            }
        }
    }
}

void AmplificationAttack::finish()
{
    if (isRSU && isAttacker) {
        EV_INFO << "Amplification Attack completed at " << simTime() << endl;
    }
    
    // Clean up
    if (sendMessageEvt && sendMessageEvt->isScheduled()) {
        cancelAndDelete(sendMessageEvt);
    }
    if (resetSpeedEvt && resetSpeedEvt->isScheduled()) {
        cancelAndDelete(resetSpeedEvt);
    }

    DemoBaseApplLayer::finish();
}
