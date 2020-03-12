/* This code for adding new layer between Transport layer (e.g., tcp-l4-protocol) and IP layer (ipv4-l3-protocol).
 * Made by Nguyen Viet Ha (nvha.dtvt@gmail.com) - 2018/03/14
 */

#ifndef NC_L4_PROTOCOL_H
#define NC_L4_PROTOCOL_H

//#include "../../../../build/ns3/callback.h"
//#include "../ipv4-header.h"
//#include "../ipv4-l3-protocol.h"
//#include "../tcp-header.h"
//#include "../ipv4-interface.h"
//#include "../ip-l4-protocol.h"

#include "ns3/callback.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/tcp-header.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ip-l4-protocol.h"

#include "nc-header.h"
#include "nc-buffer.h"
#include "nc-encode.h"
#include "nc-decode.h"
#include "nc-CheckControlPacket.h"
#include "nc-combi-buffer.h"
#include "nc-LossEstimation.h"
#include "nc-Create-Header.h"
#include "nc-SeenUnseen.h"

#include "nc-define-parameters.h"

#include "nc-UnorderingEstimation.h"

#include "nc-SocketID.h"

#include <fstream>
#include <sstream>


namespace ns3 {

class Packet;
class Ipv4Address;

class NcL4Protocol : public Object
{
public:
    static TypeId GetTypeId (void);

    NcL4Protocol ();
    virtual ~NcL4Protocol ();



    /*=============================================
     *                                           ||
     * FOR RECEIVING A PACKET FROM LOWER LAYER   ||
     *                                           ||
     * ===========================================VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
     */
public:
    void RegGlobalInt (Ptr<NetDevice> nd);
private:
    std::vector<Ptr<NetDevice>> m_GlobalInt;

public:
    /* These functions is called when the ipv4-l3-protocol receive the packet.
     */
    /* Pass here from IpForard, IpMulticastForward, and LocalDeliver from ipv4-l3-protocol
     *      IpForard:           the device receive the packet which forward to another device (unicast)
     *      IpMulticastForward: the device receive the packet which forward to other devices (multicast)
     *      LocalDeliver:       the device receive the packet which is for this devices
     *
     *      IpForard:           Can use for Re-Encode at Intermediate Nodes
     *      IpMulticastForward: Can use for Re-Encode at Intermediate Nodes
     *      LocalDeliver:       Can use for Decode at Sinks
     */
    void nc_IpForward          (Ptr<Ipv4Route> rtentry, Ptr<const Packet> packet, const Ipv4Header &header);
    void nc_IpMulticastForward (Ptr<Ipv4MulticastRoute> mrtentry, Ptr<const Packet> packet, const Ipv4Header &header);
    void nc_LocalDeliver       (Ptr<const Packet> packet, Ipv4Header const&ip, uint32_t iif);

    /* Make CALLBACK to call the IpForard, IpMulticastForward, and LocalDeliver from ipv4-l3-protocol
     * Description: After process the packet receive from ipv4-l3-protocol,
     *              this layer should return the packet back to ipv4-l3-protocol
     */
    typedef Callback<void, Ptr<Ipv4Route>, Ptr<const Packet>, const Ipv4Header &>          IpForwardCallback;
    typedef Callback<void, Ptr<Ipv4MulticastRoute>, Ptr<const Packet>, const Ipv4Header &> IpMulticastForwardCallback;
    typedef Callback<void, Ptr<const Packet>, Ipv4Header const&, uint32_t>                 LocalDeliverCallback;

    /*
     * Create the function to lets ipv4-l3-protocol transfer its functions
     *(IpForard, IpMulticastForward, and LocalDeliver) to this layer
     */
    void SetIpForwardCallback              (IpForwardCallback cb);
    void SetIpMulticastForwardCallback     (IpMulticastForwardCallback cb);
    void SetLocalDeliverCallback           (LocalDeliverCallback cb);

private:
    IpForwardCallback          m_IpForward;
    IpMulticastForwardCallback m_IpMulticastForward;
    LocalDeliverCallback       m_LocalDeliver;

    /*
     * This function is called from nc_LocalDeliver function if the protocol is TCP.
     */
    void ForTCP_LocalReceive(Ptr<Packet> p, Ipv4Header const&ip, uint32_t iif);

    /*
     * This function will call ForTCP_Receive to add some LOG file.
     */
    void ForwardToLocalDeliver (Ptr<const Packet> packet, Ipv4Header const&ip, uint32_t iif);

    /*
     * This function is called from nc_IpForward function if the protocol is TCP.
     */
    void ForTCP_UnicastForward(Ptr<Ipv4Route> rtentry, Ptr<Packet> packet, const Ipv4Header &header);

public:
    /*
     *  This function is called from simulation file to send a packet to TCP/NC tunnel
     */
    bool GetControlPacketStatus ();




    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^





    /*=============================================
     *                                           ||
     * FOR RECEIVING A PACKET FROM UPPER LAYER   ||
     *                                           ||
     * ============================================VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
     */
public:
    /* These functions is called when the ipv4-l3-protocol send the packet.
     */
    /* This SEND function is called in the SEND function of ipv4-l3-protocol
     *      And SEND function of ipv4-l3-protocol is called by upper layers
     *      Thus, it is suitable for add Encoding proccess at Source/Sink here.
     */
    void Send(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, uint8_t protocol, Ptr<Ipv4Route>route);

    /*
     * Make CALLBACK for Ipv4L3Protocol::Send2 (be the orignial SEND function of ipv4-l3-protocol)
     */
    typedef Callback<void, Ptr<Packet>, Ipv4Address, Ipv4Address, uint8_t, Ptr<Ipv4Route> > SendCallback;
    void SetSendCallback (SendCallback cb);
    SendCallback GetSendCallback (void) const;

private:
    SendCallback m_Send;

    /*
     * This function is call from Send function if the protocol is TCP.
     */
    void ForTCP_Send(Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route>route);

    //If the processed packet is send as TCP segment, this function is called
    void TCPSend (Ptr<Packet> packet, TcpHeader &header, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route>route,
                  bool SetRetrans=false, bool DelayedAck=false, uint8_t numDelayedAck=1, bool DuplicatedAck=false);

    //If the processed packet is send as NC segment, this function is called
    void NCSend (Ptr<Packet> packet, NcHeader  &header, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route>route);

    //Encoding process
    void DoEncode (Ptr<Packet> packet, SequenceNumber32 sn, Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, uint32_t psize, Ptr<Ipv4Route>route, uint8_t NumDelayedAck);

    //Encoding the retransmission
    bool DoEncodeRetrans (uint8_t gid_index, uint16_t gid,
                          uint8_t numretrans, uint8_t numretransgid,
                          std::vector<SequenceNumber32> sn, std::vector<uint16_t> size,
                          SequenceNumber32 base, uint16_t sport, uint16_t dport,
                          Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route, uint8_t NumDelayedAck);

    //Dependence retransmission
    struct DependenceReturn {
        bool     retransmit;
        bool     force;
        uint16_t Gid;
    };
    DependenceReturn DependenceRetransmit(uint16_t sport, uint16_t dport, SequenceNumber32 base, SequenceNumber32 sn, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route>route, uint8_t NumDelayedAck);

    // Forward retransmission
    bool ForwardRetransmit(Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, SequenceNumber32 sn, SequenceNumber32 base, bool DependRetrans, bool ForceDependRetrans, uint16_t DependentGid, Ptr<Ipv4Route>route, uint8_t NumDelayedAck);

    static void SendBackToTcp (Ptr<IpL4Protocol> protocol, Ptr<Packet> p, Ipv4Header h, Ptr<Ipv4Interface> i);

    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



    //EventId SendScheduleEvent;
    //bool    SendScheduleEnd = false;
    void AckRetransmisionSchedule (NcSocketID::SocketInfo *SockInfo, Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route);

    struct ReturnDelayedAckInfo {
        uint16_t Counter;
        uint64_t PreviousTime;
        std::vector<EventId> Event;
    };

    void DelayedAckSchedule (Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, uint8_t pronum, Ptr<Ipv4Route> route,
                             uint16_t delay_pkt, uint16_t delay_time, bool DuplicatedAck);

    void SendAckWhenScheduleExpired (Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, uint8_t pronum, Ptr<Ipv4Route> route);


    /*
     * ===================================================
     * Make other CALLBACK from ipv4-l3-protocol
     * ===================================================
     */
public:
    typedef Callback<Ptr<IpL4Protocol>, int, int32_t>                                                GetProtocolCallback;
    typedef Callback<Ptr<Ipv4Interface>, uint32_t>                                                   GetInterfaceCallback;
    typedef Callback<int32_t, Ipv4Address>                                                           GetInterfaceForAddressCallback;
    typedef Callback<Ipv4Header, Ipv4Address,Ipv4Address, uint8_t, uint16_t, uint8_t, uint8_t, bool> BuildHeaderCallback;
    typedef Callback<Ptr<NetDevice>, uint32_t>                                                       GetNetDeviceCallback;
    typedef Callback<Ptr<Node> >                                                                     GetNodeCallback;
    //TCP/NC Tunnel:
    typedef Callback<void, Ptr<Packet>, Ipv4Address, Ipv4Address, uint16_t, uint16_t>                SendInformationToHandleCallback;
    typedef Callback<Ptr<NetDevice> >                                                                GetInInterfaceCallback;

    void SetGetProtocolCallback            (GetProtocolCallback cb);
    void SetGetInterfaceCallback           (GetInterfaceCallback cb);
    void SetGetInterfaceForAddressCallback (GetInterfaceForAddressCallback cb);
    void SetBuildHeaderCallback            (BuildHeaderCallback cb);
    void SetGetNetDeviceCallback           (GetNetDeviceCallback cb);
    void SetGetNodeCallback                (GetNodeCallback cb);
    //TCP/NC Tunnel:
    void SetSendInformationToHandleCallback(SendInformationToHandleCallback cb);
    void SetGetInInterfaceCallback(GetInInterfaceCallback cb);

private:
    GetProtocolCallback            m_GetProtocol;
    GetInterfaceCallback           m_GetInterface;
    GetInterfaceForAddressCallback m_GetInterfaceForAddress;
    BuildHeaderCallback            m_BuildHeader;
    GetNetDeviceCallback           m_GetNetDevice;
    GetNodeCallback                m_GetNode;
    //TCP/NC Tunnel:
    SendInformationToHandleCallback m_SendInformationToHandle;
    GetInInterfaceCallback m_GetInInterface;

    // Some support functions ========================================================================
private:
    //Create a route from ipheader and output interface
    Ptr<Ipv4Route> CreateRoute (Ptr<Packet> p, Ipv4Header ipheader, Ipv4Address saddr);

    //LOG the SN and the packet contain to compare the NC process
    void LogPacket (std::string filename, Ptr<Packet> packet, SequenceNumber32 sn, uint32_t psize);

    //Create LOG
    void LogOverwrite (std::string filename,std::string content);
    void LogNonOverwrite(std::string filename,std::string content);


    //Add remove Null to/from packet
    Ptr<Packet> Add_Null_to_Packet(Ptr<Packet> packet, uint16_t Protocol_ID, float R, uint8_t recovery_capacity);
    Ptr<Packet> Rem_Null_to_Packet(Ptr<Packet> packet, uint16_t Protocol_ID, float R, uint8_t recovery_capacity);
    Ptr<Packet> Add_Null_to_ACK (Ptr<Packet> packet);
    Ptr<Packet> Rem_Null_to_ACK (Ptr<Packet> packet);


    // Create NC functions
    //NcBuffer                                    *NC_Buffer;
    //NcEncode                                    NC_Encode;
    NcEncode::Encode_Return                     Encode_Return;

    //NcDecode                                    NC_Decode;
    NcCheckControlPacket                        /*Check_Control, */Check_Control_ForTunnel;
    //NcCombiBuffer                               NC_Combine_Buffer;
    //NcLossEstimation                            NC_LossEstimation;
    NcLossEstimation::ChannelInfoReturn         ChannelInfoReturn;
    NcLossEstimation::NcParaEstimationReturn    NcParaEstimationReturn;
    NcCreateHeader                              CreateHeader;

    //NcUnorderingEstimation                      NC_UnorderingEstimation;

    NcSocketID              NC_SocketID;
    NcSocketID::SocketInfo  NcSocketInfo;

    NcSeenUnseen::LastestUnseenReturn          TxLastestUnseenInfo;


    // Control variable:==============================================================================================
    bool m_control_send;                  // indicate the sending behaviour (0=encode data; 1=control data (ACK, FIN,...)
    //bool m_control_receive;               // indicate the receive behaviour (0=encode data; 1=control data (ACK, FIN,...)

    bool m_ControlPacketStatus;
    uint8_t m_GWtype;   //0: Intermediate Gateway; 1: Sender side Border Gateway; 2 Receiver side Borfer Gateway
    //Ptr<NetDevice> m_GlobalInt;


    // Others:
    std::stringstream LogContent;

private:
    // Parameter Setting:=============================================================================================
    bool        m_NC_Enable;              // Enable/Disable NC function
    uint64_t    m_buffer_size;            // NC buffer size (Incoming and Outgoing)

    uint16_t    m_Duration_Estimation;    // Duration for counting error packets.
    bool        m_AutoTunningR;           // Enable/Disable auto-tunning Redundancy Factor
    bool        m_AutoTunningK;           // Enable/Disable auto-tunning CW recovery capacity
    long double m_RedundancyFactor;       // The default Redundancy Factor when establish connection
    bool        m_ForwardRetransmission;  // Enable forward retransmission
    bool        m_EncodeRetransmission;               // Enable Recoding for retransmitted packets

    bool        m_UsingNKtable;           // Using N, K table which is calculated based on A(n,k) and B(n,k)
    //bool        m_usingNS3orGilbert;      // Using B[NS3](n,k) or B[Gilbert](n,k)
    uint8_t     m_l_method;               // 0: l = max [s*prob];  1: l = L*2-1;
    bool        m_LOG_lr_l_and_L;         // Log Link loss rate, l and L.

    bool        m_LOG_UnOrdering;         // Log UnOrdering.

//    long double Significance_ofR;
    uint8_t     m_Max_k, m_Max_n;
    uint8_t     m_k_initial,m_n_initial;

    bool        m_ForceMN;
    uint8_t     m_PreSetN, m_PreSetM;

    bool        m_LOG_Packet;             // Enable/Disable Log packet (Send, Receive, after encode/decode, before encode/decode)
    bool        m_LogNumSentReceived;
    uint32_t    num_packet_sent, num_packet_received;

    uint8_t     m_NumOf_SMA;
    uint32_t    m_DurationEstimationLoss;
    uint16_t    m_RangeEstimationLoss;
//    bool        m_DurationOrRangeEstimationLoss;

    bool        m_SetSameOverhead;
    bool        m_DispInfoRNM;
    bool        m_UnorderEst;
    bool        m_DupAckGen, m_AckRetran;
    bool        m_DelAck;

    // For Proxy Gateway Case ========================= ************************************** ======================
    bool m_proxy_enable;
    bool m_lognm;

    std::vector <SequenceNumber32> ACKatProxy_Proxytable;
    std::vector <SequenceNumber32> ACKatSource_Proxytable;
    std::vector <Ipv4Address>      sip_Proxytable;        // IP of the source
    std::vector <Ipv4Address>      dip_Proxytable;        // IP of the sink

    void Update_Proxytable (SequenceNumber32 ACKnumAtProxy, SequenceNumber32 ACKnumAtSource, Ipv4Address sip, Ipv4Address dip);


    SequenceNumber32 PreviousACKnum;
    uint16_t         PreviousACKid;

public:
    uint8_t     GetNcPara_K (Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, bool InOut);
    long double GetNcPara_R (Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, bool InOut);



};

} // Namespace ns3

#endif
