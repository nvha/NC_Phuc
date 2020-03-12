#ifndef NCSOCKETID_H
#define NCSOCKETID_H

#include "ns3/ptr.h"
#include "ns3/core-module.h"
#include "ns3/tcp-header.h"
#include "ns3/ipv4-header.h"
//#include "ns3/packet.h"
#include "nc-UnorderingEstimation.h"
#include "nc-LossEstimation.h"
#include "nc-combi-buffer.h"
#include "nc-encode.h"
#include "nc-decode.h"
#include "nc-buffer.h"
#include "nc-CheckControlPacket.h"
#include "nc-SeenUnseen.h"
#include "nc-smoothed_cal.h"
#include "nc-MakeSchedule.h"


#include <vector>

namespace ns3 {

class NcSocketID
{

public:
    NcSocketID();

    struct SocketInfo {
        uint16_t               socketID;
        NcUnorderingEstimation *UnorderEst;
        NcLossEstimation       *LossEst;
        NcCombiBuffer          *CombiBuffer;
        NcEncode               *EnCode;
        NcDecode               *DeCode;
        NcBuffer               *Buffer;
        uint32_t               BufferInSize;
        uint32_t               BufferOutSize;
        NcCheckControlPacket   *CheckControl;
        NcSeenUnseen           *SeenUnseen;
        EventId                *AckRetransEvent;
        bool                   *AckRetransEventFin;
        NcSmoothedCal          *SmoothCal;
        NcMakeSchedule         *MakeSchedule;
    };

    SocketInfo  GetSocketID         (Ipv4Address sip, Ipv4Address dip, uint16_t spo, uint16_t dpo, uint16_t pro=0);
    //
    void        UpdateNcBufferSize  (uint16_t socketID, uint32_t size, bool in_out);
    uint32_t    GetNcBufferSize     (bool in_out);
    //
//    void        UpdateAckRetransEvent (uint16_t socketID, EventId eventid);
//    void        SetAckRetransEventFin (uint16_t socketID);

private:
    std::vector<Ipv4Address> sip_SocketIdTable;
    std::vector<Ipv4Address> dip_SocketIdTable;
    std::vector<uint16_t>    spo_SocketIdTable;
    std::vector<uint16_t>    dpo_SocketIdTable;
    std::vector<uint16_t>    pro_SocketIdTable;
    std::vector<uint16_t>    sid_SocketIdTable;
    //
    std::vector<NcUnorderingEstimation *>   UnorderingEstimation_SocketIdTable;
    std::vector<NcLossEstimation *>         LossEstimation_SocketIdTable;
    std::vector<NcCombiBuffer *>            CombiBuffer_SocketIdTable;
    std::vector<NcEncode *>                 EnCode_SocketIdTable;
    std::vector<NcDecode *>                 DeCode_SocketIdTable;
    std::vector<NcBuffer *>                 Buffer_SocketIdTable;
    //
    std::vector<uint32_t>                   in_BuffSize_SocketIdTable;
    std::vector<uint32_t>                   outBuffSize_SocketIdTable;
    //
    std::vector<NcCheckControlPacket *>     CheckControl_SocketIdTable;
    std::vector<NcSeenUnseen *>             SeenUnseen_SocketIdTable;
    //
    std::vector<EventId *>                  AckRetransEvent_SocketIdTable;
    std::vector<bool *>                     AckRetransEventFin_SocketIdTable;
    //
    std::vector<NcSmoothedCal *>            SmoothCal_SocketIdTable;
    //
    std::vector<NcMakeSchedule *>           MakeSchedule_SocketIdTable;
};
}

#endif
