#include <filesystem>
#include <fstream>
#include <string>

#include <apclient.hpp>
#include <apuuid.hpp>
#include "archipelago.h"
extern "C" {
#include "player.h"
#include "tools.h"
}

extern Player player_struct;

static APClient* ap;
static std::string uri = "ws://localhost:38281";
static std::string slot = "SED4906";
static std::string password = "";

void questbit_on_for_real(short qnum) {
    printf("questbit %d on, for real\n", qnum);
    player_struct.questbits[((qnum) / 8u)] |= 1u << ((qnum) % 8u);
}

void questbit_off_for_real(short qnum) {
    printf("questbit %d off, for real\n", qnum);
    player_struct.questbits[((qnum) / 8u)] &= ~(1u << ((qnum) % 8u));
}

void she_get_high(char which) {
    player_struct.drugs[which]++;
}

void archipelago_begin() {
    std::ifstream ifs("archipelago.conf");
    std::getline(ifs, uri);
    std::getline(ifs, slot);
    std::getline(ifs, password);
    ifs.close();
    ap = new APClient(ap_get_uuid("archipelago.uuid"), "System Shock", uri);
    ap->set_socket_connected_handler([&]() {
        printf("archipelago connected\n");
    });
    ap->set_socket_error_handler([&](const std::string& msg) {
        printf("socket error: %s\n", msg.c_str());
    });
    ap->set_socket_disconnected_handler([]() {
        printf("socket disconnected\n");
    });
    ap->set_room_info_handler([&]() {
        printf("archipelago roominfo\n");
        ap->ConnectSlot(slot, password, 7);
    });
    ap->set_room_update_handler([&]() {
        printf("archipelago room update\n");
    });
    ap->set_items_received_handler([&](const std::list<APClient::NetworkItem>& items) {
        printf("items received\n");
        for (auto i : items) {
            switch (i.item) {
                case 0x6:
                    message_info("got Radiation Shield Active");
                    questbit_on_for_real(6);
                    break;
                case 0x7:
                    message_info("got Laser Safety Interlock Disabled");
                    questbit_on_for_real(7);
                    break;
                case 0x8:
                    message_info("got Laser Destroyed");
                    questbit_on_for_real(8);
                    break;
                case 0xa:
                    message_info("got Delta Grove Safety Interlock Disabled");
                    questbit_on_for_real(0xa);
                    break;
                case 0xb:
                    message_info("got Alpha Grove Safety Interlock Disabled");
                    questbit_on_for_real(0xb);
                    break;
                case 0xc:
                    message_info("got Beta Grove Safety Interlock Disabled");
                    questbit_on_for_real(0xc);
                    break;
                case 0xf:
                    message_info("got Beta Grove Jettisoned");
                    questbit_on_for_real(0xf);
                    ap->LocationChecks({0xc}); // this became out of logic! check it
                    break;
                case 0x10:
                    message_info("got Robot Charge Interrupt");
                    questbit_off_for_real(0x10);
                    break;
                case 0x11:
                    message_info("got Laser Safety Interlock Access");
                    questbit_on_for_real(0x11);
                    break;
                case 0x14:
                    message_info("got Reactor Auto-Destruct Activated");
                    questbit_on_for_real(0x14);
                    break;
                case 0x1e:
                    message_info("got Medical Armory");
                    questbit_on_for_real(0x1e);
                    break;
                case 0x20:
                    message_info("got Flight Bay 3");
                    questbit_off_for_real(0x20);
                    break;
                case 0x75:
                    message_info("got Beta Grove Elevator");
                    questbit_off_for_real(0x75);
                    break;
                case 0x99:
                    message_info("got Level 8 Access");
                    questbit_on_for_real(0x99);
                    break;
                case 0xc0:
                    message_info("got Edward Diego's Storage Closet");
                    questbit_off_for_real(0xc0);
                    break;
                case 0xc3:
                    message_info("got Flight Bay Armory");
                    questbit_off_for_real(0xc3);
                    break;
                case 0xf7:
                    message_info("got Robot Access Panel Unlocked");
                    questbit_off_for_real(0xf7);
                    break;
                case 0xf9:
                    message_info("got Robot Production Deactivated");
                    questbit_off_for_real(0xf9);
                    break;
                case 0xfb:
                    message_info("got Blast Door");
                    questbit_off_for_real(0xfb);
                    break;
                case 0xfc:
                    message_info("got Puzzle Lift");
                    questbit_on_for_real(0xfc);
                    break;
                case 0xfe:
                    message_info("got Armory");
                    questbit_on_for_real(0xfe);
                    break;
                case 0x10e:
                    message_info("got Level 4 Force Bridge");
                    questbit_off_for_real(0x10e);
                    break;
                case 1000:
                    message_info("got a Staminup");
                    she_get_high(0);
                    break;
                case 1001:
                    message_info("got a Sight Patch");
                    she_get_high(1);
                    break;
                case 1002:
                    message_info("got a B'serk");
                    she_get_high(2);
                    break;
                case 1003:
                    message_info("got a Medipatch");
                    she_get_high(3);
                    break;
                case 1004:
                    message_info("got a Reflex Patch");
                    she_get_high(4);
                    break;
                case 1005:
                    message_info("got a Genius Patch");
                    she_get_high(5);
                    break;
                case 1006:
                    message_info("got a Detox");
                    she_get_high(6);
                    break;
                default:
                    message_info("got laid (received an unknown item)");
                    printf("unknown item %ld\n", i.item);
                    break;
            }
        }
    });
    ap->set_location_checked_handler([&](const std::list<long int>& locations) {
        printf("location checked\n");
    });
}

void archipelago_update() {
    ap->poll();
}

void archipelago_reset() {
    ap->reset();
}

bool archipelago_intercept_questbit_on(short qnum) {
    switch(qnum) {
        case 0x06: // radiation shield active // switch
            message_info("found Radiation Shield Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x07: // laser safety interlock disabled // switch
            message_info("found Laser Safety Interlock Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x08: // laser destroyed
            message_info("found Laser Button");
            ap->LocationChecks({qnum});
            return false;
        case 0x0a: // delta grove safety interlock // switch
            message_info("found Delta Grove Safety Interlock Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x0b: // alpha grove safety interlock // switch
            message_info("found Alpha Grove Safety Interlock Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x0c: // beta grove safety interlock // switch
            message_info("found Beta Grove Safety Interlock Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x0e: // relay 428 repaired
            message_info("found Relay 428 Repaired");
            ap->LocationChecks({qnum});
            return true;
        case 0x0f: // beta grove jettisoned
            message_info("found Beta Grove Jettison Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x11: // laser safety interlock access // switch
            message_info("found Laser Safety Interlock Access Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0x13: // last three digits of code entered
            message_info("found Systems Authorization Code (Last 3 Digits)");
            ap->LocationChecks({qnum});
            return true;
        case 0x14: // reactor auto destruct activated!!! // systems authorization code
            message_info("found Systems Authorization Code");
            ap->LocationChecks({qnum});
            return false;
        case 0x1c: // "camera activating security door"
            message_info("found \"Camera Activating Security Door\"");
            ap->LocationChecks({qnum});
            return true;
        case 0x1e: // hospital armory unlocked // level 1 switching node
            message_info("found Level 1 Switching Node");
            ap->LocationChecks({qnum});
            return false;
        case 0x1f: // "step right into my trap, little hacker!"
            message_info("found \"Step right into my trap, little hacker!\"");
            ap->LocationChecks({qnum});
            return true;
        case 0x23: // "you are not welcome here. remove yourself."
            message_info("found \"You are not welcome here. Remove yourself.\"");
            ap->LocationChecks({qnum});
            return true;
        case 0x64: // level 1 respawns
            message_info("found Level 1 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0x65: // level 3 respawns
            message_info("found Level 3 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0x66: // level 6 respawns
            message_info("found Level 6 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0x92: // level 4 respawns
            message_info("found Level 4 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0x93: // level 5 respawns
            message_info("found Level 5 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0x99: // level 8 access // all antennas destroyed
            message_info("found Antennas Destroyed");
            ap->LocationChecks({qnum});
            return false;
        case 0xf0: // first three digits of code entered
            message_info("found Systems Authorization Code (First 3 Digits)");
            ap->LocationChecks({qnum});
            return true;
        case 0xf6: // level 2 respawns
            message_info("found Level 2 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0xfa: // level 7 respawns
            message_info("found Level 7 Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0xfc: // lift // puzzle
            message_info("found Lift Puzzle");
            ap->LocationChecks({qnum});
            return false;
        case 0x100: // level R respawns
            message_info("found Level R Respawns");
            ap->LocationChecks({qnum});
            return true;
        case 0x10e: // level 4 force bridge // switch
            message_info("found Level 4 Force Bridge Switch");
            ap->LocationChecks({qnum});
            return false;
        default:
            break;
    }
    return true;
}

bool archipelago_intercept_questbit_off(short qnum) {
    switch(qnum) {
        case 0x10: // robot charge interrupt // button
            message_info("found Robot Charge Interrupt Button");
            ap->LocationChecks({qnum});
            return false;
        case 0x20: // flight bay 3 // level 5 switching node
            message_info("found Level 5 Switching Node 1");
            ap->LocationChecks({qnum});
            return false;
        case 0x75: // beta grove elevator // level 6 switching node
            message_info("found Level 6 Switching Node 2");
            ap->LocationChecks({qnum});
            return false;
        case 0xc0: // edward diego's storage closet // level 6 switching node
            message_info("found Level 6 Switching Node 1");
            ap->LocationChecks({qnum});
            return false;
        case 0xc3: // flight bay armory // level 5 switching node
            message_info("found Level 5 Switching Node 2");
            ap->LocationChecks({qnum});
            return false;
        case 0xf3: // "welcome to my death machine"
            message_info("found \"Welcome to my death machine!\"");
            ap->LocationChecks({qnum});
            return true;
        case 0xf7: // robot access panel // switch
            message_info("found Robot Access Panel Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0xf9: // robot production deactivated // switch
            message_info("Robot Production Switch");
            ap->LocationChecks({qnum});
            return false;
        case 0xfb: // blast door // level R switching node
            message_info("found Level R Switching Node 1");
            ap->LocationChecks({qnum});
            return false;
        case 0xfe: // armory // level R switching node
            message_info("found Level R Switching Node 2");
            ap->LocationChecks({qnum});
            return false;
        default:
            break;
    }
    return true;
}

short archipelago_intercept_questvar_set(short qnum, short x, const char* func_name) {
    if (strcmp(func_name, "update_shodometer"))
        return x;
    switch(qnum) {
        case 0x10:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x11:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x12:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x13:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x14:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x15:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x16:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x17:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        case 0x18:
            if (x == 0)
                ap->LocationChecks({qnum + 0x200});
            return x;
        default:
            break;
    }
    return x;
}
