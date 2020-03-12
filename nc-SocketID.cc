#include "nc-SocketID.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <fstream>

namespace ns3 {

NcSocketID::NcSocketID(){
}

NcSocketID::SocketInfo NcSocketID::GetSocketID(Ipv4Address sip, Ipv4Address dip, uint16_t spo, uint16_t dpo, uint16_t pro){
    NcSocketID::SocketInfo returnValue;

    for (uint16_t i=0; i<sid_SocketIdTable.size(); i++){
        if ((sip_SocketIdTable[i]==sip && dip_SocketIdTable[i]==dip && spo_SocketIdTable[i]==spo && dpo_SocketIdTable[i]==dpo /*&& pro_SocketIdTable[i]==pro*/) ||
            (sip_SocketIdTable[i]==dip && dip_SocketIdTable[i]==sip && spo_SocketIdTable[i]==dpo && dpo_SocketIdTable[i]==spo /*&& pro_SocketIdTable[i]==pro*/)) {
            //
            returnValue.socketID        = sid_SocketIdTable[i];
            returnValue.UnorderEst      = UnorderingEstimation_SocketIdTable[i];
            returnValue.LossEst         = LossEstimation_SocketIdTable[i];
            returnValue.CombiBuffer     = CombiBuffer_SocketIdTable[i];
            returnValue.EnCode          = EnCode_SocketIdTable[i];
            returnValue.DeCode          = DeCode_SocketIdTable[i];
            returnValue.Buffer          = Buffer_SocketIdTable[i];
            returnValue.BufferInSize    = GetNcBufferSize(true);
            returnValue.BufferOutSize   = GetNcBufferSize(false);
            returnValue.CheckControl    = CheckControl_SocketIdTable[i];
            returnValue.SeenUnseen      = SeenUnseen_SocketIdTable[i];
            //
            returnValue.AckRetransEvent    = AckRetransEvent_SocketIdTable   [i];
            returnValue.AckRetransEventFin = AckRetransEventFin_SocketIdTable[i];
            //
            returnValue.SmoothCal          = SmoothCal_SocketIdTable[i];
            //
            returnValue.MakeSchedule       = MakeSchedule_SocketIdTable[i];

            return returnValue;
        }
    }

    sip_SocketIdTable.push_back(sip);
    dip_SocketIdTable.push_back(dip);
    spo_SocketIdTable.push_back(spo);
    dpo_SocketIdTable.push_back(dpo);
    pro_SocketIdTable.push_back(pro);
    //
    uint16_t t_sid = sid_SocketIdTable.size();
    sid_SocketIdTable.push_back(t_sid);
    //
    NcUnorderingEstimation *t_NcUnorderingEstimation = new NcUnorderingEstimation;
    UnorderingEstimation_SocketIdTable.push_back(t_NcUnorderingEstimation);
    //
    NcLossEstimation *t_NcLossEstimation = new NcLossEstimation;
    LossEstimation_SocketIdTable.push_back(t_NcLossEstimation);
    //
    NcCombiBuffer *t_NcCombiBuffer = new NcCombiBuffer;
    CombiBuffer_SocketIdTable.push_back(t_NcCombiBuffer);
    //
    NcEncode *t_NcEncode = new NcEncode;
    EnCode_SocketIdTable.push_back(t_NcEncode);
    //
    NcDecode *t_NcDecode = new NcDecode;
    DeCode_SocketIdTable.push_back(t_NcDecode);
    //
    NcBuffer *t_NcBuffer = new NcBuffer;
    Buffer_SocketIdTable.push_back(t_NcBuffer);

    returnValue.socketID    = t_sid;
    returnValue.UnorderEst  = t_NcUnorderingEstimation;
    returnValue.LossEst     = t_NcLossEstimation;
    returnValue.CombiBuffer = t_NcCombiBuffer;
    returnValue.EnCode      = t_NcEncode;
    returnValue.DeCode      = t_NcDecode;
    returnValue.Buffer      = t_NcBuffer;

    //
    in_BuffSize_SocketIdTable.push_back(0);
    outBuffSize_SocketIdTable.push_back(0);
    returnValue.BufferInSize  = 0;
    returnValue.BufferOutSize = 0;

    //
    NcCheckControlPacket *t_NcCheckControlPacket = new NcCheckControlPacket;
    CheckControl_SocketIdTable.push_back(t_NcCheckControlPacket);
    returnValue.CheckControl = t_NcCheckControlPacket;

    //
    NcSeenUnseen *t_NcSeenUnseen = new NcSeenUnseen;
    SeenUnseen_SocketIdTable.push_back(t_NcSeenUnseen);
    returnValue.SeenUnseen = t_NcSeenUnseen;

    //
    EventId *t_AckRetransEvent = new EventId;
    AckRetransEvent_SocketIdTable.push_back(t_AckRetransEvent);
    bool *t_AckRetransEventFin = new bool;
    *t_AckRetransEventFin      = false;
    AckRetransEventFin_SocketIdTable.push_back(t_AckRetransEventFin);

    //
    NcSmoothedCal * t_NcSmoothedCal = new NcSmoothedCal;
    SmoothCal_SocketIdTable.push_back(t_NcSmoothedCal);
    returnValue.SmoothCal = t_NcSmoothedCal;

    //
    NcMakeSchedule * t_NcMakeSchedule = new NcMakeSchedule;
    MakeSchedule_SocketIdTable.push_back(t_NcMakeSchedule);
    returnValue.MakeSchedule = t_NcMakeSchedule;

    return returnValue;
}


void NcSocketID::UpdateNcBufferSize (uint16_t socketID, uint32_t size, bool in_out){
    for (uint32_t i=0; i<sid_SocketIdTable.size(); i++) {
        if (sid_SocketIdTable[i] == socketID) {
            if (in_out)
                in_BuffSize_SocketIdTable[i] = size;
            else
                outBuffSize_SocketIdTable[i] = size;

            return;
        }
    }
}

uint32_t NcSocketID::GetNcBufferSize(bool in_out){
    uint32_t t_size = 0;
    for (uint32_t i=0; i<sid_SocketIdTable.size(); i++) {
        if (in_out)
            t_size = t_size + in_BuffSize_SocketIdTable[i];
        else
            t_size = t_size + outBuffSize_SocketIdTable[i];
    }

    return t_size;
}

//void NcSocketID::UpdateAckRetransEvent(uint16_t socketID, EventId eventid) {
//    for (uint32_t i=0; i<sid_SocketIdTable.size(); i++) {
//        if (sid_SocketIdTable[i] == socketID) {
//            AckRetransEvent_SocketIdTable[i] = eventid;
//            return;
//        }
//    }
//}

//void NcSocketID::SetAckRetransEventFin(uint16_t socketID) {
//    for (uint32_t i=0; i<sid_SocketIdTable.size(); i++) {
//        if (sid_SocketIdTable[i] == socketID) {
//            AckRetransEventFin_SocketIdTable[i] = true;
//            return;
//        }
//    }
//}


} // end namespace

