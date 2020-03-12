//#include "../../../../build/ns3/integer.h"
//#include "../../../../build/ns3/log.h"
//#include "../../../../build/ns3/simulator.h"
//#include "../../../../build/ns3/packet.h"
//#include "../../../../build/ns3/pointer.h"
//#include "../tcp-option-ts.h"
//#include "../tcp-option.h"
//#include "../ipv4.h"

#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/tcp-option-ts.h"
#include "ns3/tcp-option.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"

#include "nc-l4-protocol.h"
#include "nc-ACK-NC-Header.h"
#include "nc-Create-NewACK.h"
#include "nc-MakeSchedule.h"


#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NcL4Protocol");

NS_OBJECT_ENSURE_REGISTERED (NcL4Protocol);

TypeId
NcL4Protocol::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::NcL4Protocol")
            .SetParent<Object> ()
            .SetGroupName ("Internet")
            .AddConstructor<NcL4Protocol> ()
            .AddAttribute ("IsNcEnable", "Enable Network Coding?",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_NC_Enable),
                           MakeBooleanChecker ())
            .AddAttribute ("SizeOfNcBuffer", "The size of NC Buffer",
                           UintegerValue (100),
                           MakeUintegerAccessor (&NcL4Protocol::m_buffer_size),
                           MakeUintegerChecker<uint64_t> ())
            //.AddAttribute ("RedundancyFactor", "Redundancy Factor Value",
            //               DoubleValue ((long double)1.2),
            //               MakeDoubleAccessor (&NcL4Protocol::m_RedundancyFactor),
            //               MakeDoubleChecker<long double> ())
            .AddAttribute ("Duration_Estimation", "Duration for Estimation",
                           UintegerValue (1000),
                           MakeUintegerAccessor (&NcL4Protocol::m_Duration_Estimation),
                           MakeUintegerChecker<uint16_t> ())
            //
            //.AddAttribute ("Significance", "Significance of R: Ex 0.05 for 1.05; 1.10; 1.15, ...",
            //               DoubleValue ((float)0.05),
            //               MakeDoubleAccessor (&NcL4Protocol::Significance_ofR),
            //               MakeDoubleChecker<float> ())
            //.AddAttribute ("max_k", "The maximum of reovery lost packet",
            //               UintegerValue (10),
            //               MakeUintegerAccessor (&NcL4Protocol::m_Max_k),
            //               MakeUintegerChecker<uint8_t> ())
            //.AddAttribute ("max_n", "The maximum of coding window + k",
            //               UintegerValue (40),
            //               MakeUintegerAccessor (&NcL4Protocol::m_Max_n),
            //               MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("k_initial", "initial of n",
                           UintegerValue (5),
                           MakeUintegerAccessor (&NcL4Protocol::m_k_initial),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("n_initial", "initial of k",
                           UintegerValue (20),
                           MakeUintegerAccessor (&NcL4Protocol::m_n_initial),
                           MakeUintegerChecker<uint8_t> ())
            //
            .AddAttribute ("ForceMN", "Pre-set M and N instead of R",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_ForceMN),
                           MakeBooleanChecker ())
            .AddAttribute ("PreSetN", "Pre-set N",
                           UintegerValue (1),
                           MakeUintegerAccessor (&NcL4Protocol::m_PreSetN),
                           MakeUintegerChecker<uint8_t> ())
            .AddAttribute ("PreSetM", "Pre-set M",
                           UintegerValue (1),
                           MakeUintegerAccessor (&NcL4Protocol::m_PreSetM),
                           MakeUintegerChecker<uint8_t> ())
            //
            .AddAttribute ("ForwardRetransmission", "Enable burst retransmit function",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_ForwardRetransmission),
                           MakeBooleanChecker ())
            .AddAttribute ("EncodeRetranmission", "Enable recoding for retransmitted packets?",
                           BooleanValue (true),
                           MakeBooleanAccessor (&NcL4Protocol::m_EncodeRetransmission),
                           MakeBooleanChecker ())
            .AddAttribute ("AutoTunningR", "Auto Tunning NC Parameter R?",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_AutoTunningR),
                           MakeBooleanChecker ())
            .AddAttribute ("AutoTunningK", "Auto Tunning NC Parameter K?",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_AutoTunningK),
                           MakeBooleanChecker ())
//            .AddAttribute ("UsingNS3orGilbert", "Using B[NS3](n,k) or B[Gilbert](n,k)",
//                           BooleanValue (true),
//                           MakeBooleanAccessor (&NcL4Protocol::m_usingNS3orGilbert),
//                           MakeBooleanChecker ())
            .AddAttribute ("UnorderingEstimation", "Enable Unordering Estimation and Adaptation",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_UnorderEst),
                           MakeBooleanChecker ())
            .AddAttribute ("DupAckGenerate", "Enable Duplicate ACK Generation",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_DupAckGen),
                           MakeBooleanChecker ())
            .AddAttribute ("AckRetransmit", "Enable ACK Retransmission",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_AckRetran),
                           MakeBooleanChecker ())
            .AddAttribute ("DelayedAck", "Enable Delayed ACK",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_DelAck),
                           MakeBooleanChecker ())
            //
            //.AddAttribute ("EstimationLossMode", "Which mode do you want to use. If using Duration mode, Range will be the minimum of packet need to sending. Is smaller, need to wait",
            //               BooleanValue (true),
            //               MakeBooleanAccessor (&NcL4Protocol::m_DurationOrRangeEstimationLoss),
            //               MakeBooleanChecker ())
            .AddAttribute ("EstimationLossDuration", "Duration for Estimation Loss Rate and Burst Loss (ms)",
                           UintegerValue (2000),
                           MakeUintegerAccessor (&NcL4Protocol::m_DurationEstimationLoss),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("EstimationLossRange", "Range for Estimation Loss Rate and Burst Loss (packet)",
                           UintegerValue (20),
                           MakeUintegerAccessor (&NcL4Protocol::m_RangeEstimationLoss),
                           MakeUintegerChecker<uint16_t> ())
            .AddAttribute ("NumOf_SMA", "How many probability will use for calculate average",
                           UintegerValue (5),
                           MakeUintegerAccessor (&NcL4Protocol::m_NumOf_SMA),
                           MakeUintegerChecker<uint8_t> ())
            //
            .AddAttribute ("SetSameOverhead", "Enable all null data to payload to set all packets have same size",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_SetSameOverhead),
                           MakeBooleanChecker ())
            //
            .AddAttribute ("DispInfoRNM", "Logging N and M",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_DispInfoRNM),
                           MakeBooleanChecker ())
            //
            .AddAttribute ("LogOriginalPacket", "Log Original and Decoded packets (for comparing)?",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_LOG_Packet),
                           MakeBooleanChecker ())
            .AddAttribute ("LogNumSentReceived", "Log Num Sent Received",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_LogNumSentReceived),
                           MakeBooleanChecker ())
            .AddAttribute ("LogLr_l_L", "Log Link loss rate, l and L",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_LOG_lr_l_and_L),
                           MakeBooleanChecker ())
            .AddAttribute ("LogNM", "Logging N and M",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_lognm),
                           MakeBooleanChecker ())
            .AddAttribute ("Log_UnOrdering", "Log UnOrdering",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_LOG_UnOrdering),
                           MakeBooleanChecker ())
            //TCP Tunneling
            .AddAttribute ("TCPProxyEnable", "Enable TCP Proxy",
                           BooleanValue (false),
                           MakeBooleanAccessor (&NcL4Protocol::m_proxy_enable),
                           MakeBooleanChecker ())
            .AddAttribute ("GatewayType", "0: Intermediate GW; 1: Sender-side GW; 2: Receiver-side GW",
                           UintegerValue (0),
                           MakeUintegerAccessor (&NcL4Protocol::m_GWtype),
                           MakeUintegerChecker<uint8_t> ())
            //.AddAttribute ("GlobalInterface", "Indicate Global Interface ID of Gateway",
            //ObjectVectorValue (),
            //MakeObjectVectorAccessor (&NcL4Protocol::m_GlobalInt),
            //MakeObjectVectorChecker<NetDevice> ())

            //PointerValue (),
            //MakePointerAccessor (&NcL4Protocol::m_GlobalInt),
            //MakePointerChecker<NetDevice>())
            ;
    return tid;
}

NcL4Protocol::NcL4Protocol ()
{
    //NC_Buffer = new NcBuffer;
    num_packet_sent     = 0;
    num_packet_received = 0;
    PreviousACKnum      = (SequenceNumber32)0;
    PreviousACKid = 65535;
}


NcL4Protocol::~NcL4Protocol ()
{
    NS_LOG_FUNCTION (this);
}


void NcL4Protocol::RegGlobalInt (Ptr<NetDevice> nd){
    m_GlobalInt.push_back(nd);
}

/*=============================================
     *                                           ||
     * FOR RECEIVING A PACKET FROM LOWER LAYER   ||
     *                                           ||
     * ===========================================VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
     */

void NcL4Protocol::nc_IpForward (Ptr<Ipv4Route> rtentry, Ptr<const Packet> packet, const Ipv4Header &header)
{
    //NS_LOG_UNCOND ("nc_IpForward - " << Simulator::Now().GetMilliSeconds());

    Ptr<Packet> p = packet->Copy ();

    if (m_NC_Enable==true){
        switch (header.GetProtocol())
        {
        case ProNumTCP:   //TCP
        case ProNumNC:    //NC
            ForTCP_UnicastForward(rtentry,p,header);
            break;
        case ProNumUDP: //UDP
            //ForUDP(packet, source, destination, protocol, route);
            break;
        default:
            // Do nothing;
            break;
        }

        return;
    }

    m_IpForward (rtentry,packet,header);
}

void NcL4Protocol::nc_IpMulticastForward (Ptr<Ipv4MulticastRoute> mrtentry, Ptr<const Packet> packet, Ipv4Header const&header)
{
    //NS_LOG_UNCOND ("nc_IpMulticastForward - " << Simulator::Now().GetMilliSeconds());
    m_IpMulticastForward (mrtentry,packet,header);
}

void NcL4Protocol::nc_LocalDeliver(Ptr<const Packet> packet, Ipv4Header const&ip, uint32_t iif)
{
    //NS_LOG_UNCOND ("nc_LocalDeliver - " << Simulator::Now().GetMilliSeconds());
    Ptr<Packet> p = packet->Copy ();

    if (m_NC_Enable==true){
        switch (ip.GetProtocol())
        {
        case ProNumTCP:   //TCP
        case ProNumNC:    //NC
            ForTCP_LocalReceive(p,ip,iif);
            break;
        case ProNumUDP: //UDP
            //ForUDP(packet, source, destination, protocol, route);
            break;
        default:
            // Do nothing;
            break;
        }

        return;
    }

    if (m_SetSameOverhead) {
        // Remove N null bytes from ACK packet;
        packet = Rem_Null_to_ACK (p);

        if (packet->GetSize() > TcpHeaderNormalSize) {
            // Remove N null bytes from original packet;
            packet = Rem_Null_to_Packet(p, ProNumTCP, m_RedundancyFactor, m_Max_k);
        }
    }

    ForwardToLocalDeliver(p,ip,iif);
}

void NcL4Protocol::ForTCP_LocalReceive(Ptr<Packet> packet, Ipv4Header const&ip, uint32_t iif){
    Ipv4Address saddr = ip.GetSource();
    Ipv4Address daddr = ip.GetDestination();
    uint8_t    pronum = ip.GetProtocol();

    Ipv4Header t_ip = ip;
    t_ip.SetProtocol(ProNumTCP);

//    if (Simulator::Now().GetSeconds()>3.2828)
    std::cout << "RECEIVE at: " << Simulator::Now().GetSeconds() << " sip=" << saddr << " dip=" << daddr
              << " proto=" << (int)pronum << " Psz=" << packet->GetSize()  << "\n";

    if (pronum == ProNumTCP){             // Normal Packet / Control Packet
        TcpHeader tcpHeader_temp;
        packet->PeekHeader(tcpHeader_temp);

        uint16_t         sport    = tcpHeader_temp.GetSourcePort();
        uint16_t         dport    = tcpHeader_temp.GetDestinationPort();
        uint8_t          flag     = tcpHeader_temp.GetFlags();
        SequenceNumber32 sn       = tcpHeader_temp.GetSequenceNumber();
        SequenceNumber32 ack      = tcpHeader_temp.GetAckNumber();
        uint16_t         headsize = tcpHeader_temp.GetLength()*4;

        // Get Socket Info;
        NcSocketInfo = NC_SocketID.GetSocketID(saddr, daddr, sport, dport);

        //bool check = Check_Control.Check(saddr, daddr, sport, dport, flag, sn, ack, packet->GetSize()-headsize, false);
        bool check = NcSocketInfo.CheckControl->Check(flag, sn, ack, packet->GetSize()-headsize);

        //std::cout << "check=" << (int)check  << "\n";

        // THIS IS A ACK PACKET
        if (check == false){
            if ((int)tcpHeader_temp.GetFlags()==TcpFlag_ACK && packet->GetSize()>TcpHeaderNormalSize){      // When NC Layer receive ACK packet
                TcpHeader TCP_Header_t;
                packet->RemoveHeader(TCP_Header_t);
                NcAckHeader NC_ACK_Header_t;
                packet->RemoveHeader(NC_ACK_Header_t);
                //
                uint16_t t_Pid                  = NC_ACK_Header_t.GetPid();
                //uint16_t t_Gid                  = NcSocketInfo.CombiBuffer->Get_GidofPid(t_Pid);
                bool t_Redundancy               = NC_ACK_Header_t.GetRedundancy();
                bool t_Dependent                = NC_ACK_Header_t.GetDependent();
                SequenceNumber32 t_DependentSn  = NC_ACK_Header_t.GetSnDependent();
                uint8_t t_IsUpdate              = NC_ACK_Header_t.Get_UpdateUnOrderingLength();
                uint8_t t_UnOrderLength         = NC_ACK_Header_t.GetUnOrderingLength();
                bool    t_LongJumpRatio         = NC_ACK_Header_t.GetLongJumpRatio();

                NcHeader Hch_tem;

                if (m_AckRetran) {
                    //Ignore Duplication of Retransmited ACK
                    if (NcSocketInfo.Buffer->UpdatePidofReceivedAck(t_Pid)) {
                        //std::cout << "Dupplicated Pid=" << t_Pid << "\n";
                        return;
                    }
                }

                if (m_DelAck){
                    if (PreviousACKid == NC_ACK_Header_t.Get_AckId()) {
                        return;
                    }
                    else {
                        PreviousACKid = NC_ACK_Header_t.Get_AckId();
                    }
                }

                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                SequenceNumber32 expect_ack = NcSocketInfo.Buffer->GetExpectAck(ack/*, 10*/);
                //                SequenceNumber32 last_ack   = NcSocketInfo.Buffer->GetLastACK  ();

                // Retransmit Dependent packets;
                SequenceNumber32 stored_dependent_SN = NcSocketInfo.Buffer->GetDependent();
                bool             have_dependent      = NcSocketInfo.Buffer->GetDependentStatus();

                if (t_Dependent) {
//                    std::cout << "[Dependent] -- SN=" << t_DependentSn << "  -  Pid=" << t_Pid
//                              << "  -  ACK=" << ack
//                              << "  -  Current BASE=" << NcSocketInfo.Buffer->GetBase()
//                              << "  -  t=" << Simulator::Now().GetSeconds()
//                              << "\n";

                    if (stored_dependent_SN!=t_DependentSn || !have_dependent) {
                        if (m_ForwardRetransmission) {
                            if (/*NcSocketInfo.SeenUnseen->CheckSeen(t_DependentSn) &&*/ !NcSocketInfo.SeenUnseen->CheckDependent(t_DependentSn)){
                                // Get group ID
                                uint16_t t_gid = NcSocketInfo.SeenUnseen->Get_GidofSN(t_DependentSn);

                                // Set dependent packet is unseen
                                NcSocketInfo.SeenUnseen->SetSeen2Dependent(t_DependentSn);

                                // Increase number of lost
                                if (NcSocketInfo.SeenUnseen->CheckSeen(t_DependentSn)) {
                                    NcSocketInfo.CombiBuffer->UpdateNumLost(t_gid);
                                }
                            }
                        }
                        NcSocketInfo.Buffer->Update_DependentTable(t_DependentSn, ack);
                    }
                }
                else {
                    if (have_dependent) {
                        NcSocketInfo.Buffer->Clean_DependentTable();
                        NcSocketInfo.SeenUnseen->ClearDependent();
                    }
                }


                //                if (saddr=="172.16.3.1") {
//                if (t_Gid == 2909) {
//                if (Simulator::Now().GetSeconds() > 1210.0) {
//                if (Simulator::Now().GetSeconds()>3.2828) {
                std::cout << "RECEIVE ACK  -  ack=" << ack  << " - AckId=" << NC_ACK_Header_t.Get_AckId() << "     - redun=" << (int)t_Redundancy <<
                             " - depen=" << (int)t_Dependent << " - dsn=" << t_DependentSn <<
                             " - pid=" << t_Pid <<
                             "  -  t=" << Simulator::Now().GetSeconds()  << "\n";

//                    std::cout << "NC_ACK_Header_t.GetLossSequence()=" << NC_ACK_Header_t.GetLossSequence()  << "\n";
//                }

                // Dynamic R and Bursty retransmit


                if (m_ForwardRetransmission==true || (m_AutoTunningK==true || m_AutoTunningR==true)){
                    if ((m_AutoTunningK==true || m_AutoTunningR==true)){
                        ChannelInfoReturn = NcSocketInfo.LossEst->UpdateChannelCondition(t_Pid, NC_ACK_Header_t.GetLossSequence(), NC_ACK_Header_t.Get_AckId(),
                                                                                         m_DurationEstimationLoss, m_RangeEstimationLoss, m_NumOf_SMA,
                                                                                         m_AutoTunningR, m_AutoTunningK,
                                                                                         m_LOG_lr_l_and_L, daddr,
                                                                                         m_UnorderEst, t_IsUpdate, t_UnOrderLength, t_LongJumpRatio);


                        /////////////////////////
                        //Estimate SRTT
                        if (m_DelAck) {
                            uint64_t t_SendingTime      = NcSocketInfo.CombiBuffer->GetSendingTime(t_Pid,true);
                            uint16_t t_PacketOnFlight   = NcSocketInfo.CombiBuffer->GetPacketOnFlight();
                            uint64_t t_CurrentTime      = Simulator::Now().GetMilliSeconds();
                            uint16_t t__rtt             = t_CurrentTime - t_SendingTime;
                            uint16_t t_srrt             = 0;
                            uint16_t t_PktOn1Rtt        = 0;

                            if (t_SendingTime > 0) {
                                t_srrt      = NcSocketInfo.SmoothCal->SmaRTT_Cal        (t__rtt          ,0.7);
                                t_PktOn1Rtt = NcSocketInfo.SmoothCal->SmaPktOn1Rtt_Cal  (t_PacketOnFlight,0.7);
                                NcSocketInfo.LossEst->SetNumPktOnFlight(t_PktOn1Rtt,t_srrt);
                            }
                        }
                    }
                }

                std::vector<Ptr<Packet>> FakeAckPacket;
                SequenceNumber32 FakeAck = ack;

                if (m_ForwardRetransmission){
                    //                    for (uint8_t i=0; i<NcSocketInfo.CombiBuffer->GetPacketStatus(Socket_ID);i++){
                    //                        uint16_t t_gid      = NcSocketInfo.CombiBuffer->GetRetransGID(i);
                    //                        uint8_t  t_num_lost = NcSocketInfo.CombiBuffer->GetNumLost (Socket_ID, t_gid);
                    //                        uint8_t  t_capacity = NcSocketInfo.CombiBuffer->GetCapacity(Socket_ID, t_gid);

                    //                        if (Simulator::Now().GetSeconds() > 1997)
                    //                            std::cout << " retrans list   gid=" << t_gid << "  t_num_lost=" << (int)t_num_lost << "  t_capacity=" << (int)t_capacity <<  std::endl;
                    //                    }

                    //Determine NC Header of this Pid & //Update "seen" status
                    //SequenceNumber32 DependentSN     = NcSocketInfo.Buffer->GetDependent();
                    //bool             DependentStatus = NcSocketInfo.Buffer->GetDependentStatus();

                    uint16_t t_numInListPid = ChannelInfoReturn.ListOfPid.size();

                    if (ChannelInfoReturn.IncludeLargestPid==true){
                        if (t_numInListPid==0) {
                            return;
                            std::cout << "Error at nc-l4-protocol.cc  -  t_numInListPid==0";
                            exit(-1);
                        }
                    }

//                    bool bPid = (t_Pid>55228 && t_Pid<=55259);
//                    if (bPid)
//                    if (Simulator::Now().GetSeconds()>3.2828)
//                        std::cout << "\n\nt_numInListPid=" << (int)t_numInListPid << " - t_Pid=" << t_Pid << "  -  t_Dependent=" << (int)t_Dependent
//                                  << " - t=" << Simulator::Now().GetSeconds()  << " - " << this << "\n";

                    for (uint16_t iii=0; iii<t_numInListPid; iii++) {
                        uint16_t              old_pid, new_pid;//, new_gid;
                        uint8_t               num_lost_packet;

                        new_pid = ChannelInfoReturn.ListOfPid[t_numInListPid-1-iii];
                        //new_gid = NcSocketInfo.CombiBuffer->Get_GidofPid(new_pid);
                        if (new_pid != ChannelInfoReturn.OldPid) {
                            old_pid = ChannelInfoReturn.OldPid;

                            if (iii>0 && (old_pid < ChannelInfoReturn.ListOfPid[t_numInListPid-iii])) {
                                old_pid = ChannelInfoReturn.ListOfPid[t_numInListPid-iii];
                            }

                            num_lost_packet = new_pid - old_pid - 1;

                            if (new_pid < old_pid) {
                                num_lost_packet = 255;
                            }

                            //                                if (Simulator::Now().GetSeconds() > 1997) {
                            //                            if (num_lost_packet > 1) {
                            //                                std::cout << "!!!!" ;
                            //                            }
                            //                            std::cout << "!  num_lost_packet=" << (int)num_lost_packet <<" - old_pid=" << old_pid << " - new_pid=" << new_pid << "\n";
                            //                                }

                            Hch_tem = NcSocketInfo.CombiBuffer->GetNcHeader(new_pid);

//                            if (bPid) {
//                                std::cout << "   Call Update_SeenUnseen_Table" << " - new_pid=" << new_pid << " - t_numInListPid=" << (int)t_numInListPid << "\n";
//                            }

                            // Update Seen packets
                            bool tt_redundancy = false;
                            if (iii == t_numInListPid-1) {
                                tt_redundancy = !NcSocketInfo.SeenUnseen->Update_SeenUnseen_Table (Hch_tem,t_Dependent, t_DependentSn, ack, expect_ack);
                            }
                            else {
                                tt_redundancy = !NcSocketInfo.SeenUnseen->Update_SeenUnseen_Table (Hch_tem,t_Dependent, t_DependentSn, (SequenceNumber32)0, (SequenceNumber32)0);
                            }

                            TxLastestUnseenInfo = NcSocketInfo.SeenUnseen->GetLastestUnseen(true); //Get To update Lastest Unseen;
                            //if (Simulator::Now().GetSeconds() > 544.0 && Simulator::Now().GetSeconds() < 546.0)
                            //    std::cout << "TxLastestUnseenInfo.SN=" << TxLastestUnseenInfo.SN << "\n";
                            //bool GetUnseenWithDuplicateValue = NcSocketInfo.CombiBuffer->GetLastestUnseen_isDuplicate(TxLastestUnseenInfo.SN,Hch_tem.GetPacketStatus());
//                            FakeAck = TxLastestUnseenInfo.SN;

//                            if (t_Dependent) {
//                                FakeAck = ack;
//                            }
//                            if (!t_Dependent && !TxLastestUnseenInfo.Have) {
//                                FakeAck = ack;
//                            }                            

                            if (m_DupAckGen) {
                                if (t_numInListPid>1 && iii<t_numInListPid-1) {
                                    FakeAck = TxLastestUnseenInfo.SN;

                                    if(!tt_redundancy) {
                                        //FakeAck = TxLastestUnseenInfo.SN;

                                        bool SendFakeACK = false;
                                        //
                                        if (t_Dependent) {
                                            if (FakeAck>ack) {
                                                FakeAck = ack;
                                                SendFakeACK = true;
                                            }
                                        }
                                        else {
                                            if (!TxLastestUnseenInfo.Have) {
                                                FakeAck = ack;
                                                std::cout << "!t_Dependent && !TxLastestUnseenInfo.Have --- t=" << Simulator::Now().GetSeconds();
                                                exit(-1);
                                            }
                                            if (FakeAck==ack){
                                                SendFakeACK = true;
                                            }
                                        }

                                        if (SendFakeACK) {
                                            NcSocketInfo.Buffer->Update_FinTable(FakeAck,true);

                                            TcpHeader FakeTcpHeader = TCP_Header_t;
                                            //
                                            FakeTcpHeader.SetAckNumber(FakeAck);
                                            //
                                            Ptr<Packet> FakeAckPkt  = Create<Packet> ();
                                            FakeAckPkt->AddHeader(FakeTcpHeader);
                                            //
                                            FakeAckPacket.push_back(FakeAckPkt);

                                            //
                                            //    LogContent << FakeAck << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy << " , " << "Fake" << " , " << "Fowarded" << '\n';
                                            //    LogNonOverwrite("ACK_Source_woTime1.csv",LogContent.str());
                                        }
                                        //else {
                                        //    LogContent << FakeAck << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy << " , " << "Fake" << '\n';
                                        //    LogNonOverwrite("ACK_Source_woTime1.csv",LogContent.str());
                                        //}
                                        //
                                        //LogContent << FakeAck << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy  << '\n';
                                        //LogNonOverwrite("ACK_Source_woTime2.csv",LogContent.str());
                                    }

                                    //else {
                                    //    LogContent << FakeAck << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy << " , " << "Fake" << '\n';
                                    //    LogNonOverwrite("ACK_Source_woTime1.csv",LogContent.str());
                                    //    //
                                    //    LogContent << FakeAck << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy  << '\n';
                                    //    LogNonOverwrite("ACK_Source_woTime2.csv",LogContent.str());
                                    //}

                                    //                                LogContent << FakeAck << " , " << new_pid << " , " << (int)tt_redundancy  << '\n';
                                    //                                LogNonOverwrite("ACK_Source_woTime.csv",LogContent.str());

                                    //                                LogContent << Simulator::Now().GetSeconds() << "," << FakeAck << " , " << new_pid << " , " << (int)tt_redundancy << "  , A" << '\n';
                                    //                                LogNonOverwrite("ACK_Source_wTime.csv",LogContent.str());
                                    //                                LogContent << FakeAck << " , " << new_pid << " , " << (int)tt_redundancy /*<< "  , A"*/ << '\n';
                                    //                                LogNonOverwrite("ACK_Source_woTime.csv",LogContent.str());

                                    //                                LogContent << Simulator::Now().GetSeconds() << "," << FakeAck << " , " << new_pid  << " , " << new_gid << " , " << (int)tt_redundancy
                                    //                                           << " , N=" << (int)Hch_tem.GetN() << " , PktSt=" << (int)Hch_tem.GetPacketStatus() << " , " ;
                                    //                                for (uint8_t ttt=0; ttt<Hch_tem.GetN(); ttt++) {
                                    //                                    if (ttt==0)
                                    //                                        LogContent<< Hch_tem.GetStart1();
                                    //                                    else
                                    //                                        LogContent<< "-" << Hch_tem.GetStart1() + Hch_tem.GetStartN(ttt);
                                    //                                }
                                    //                                LogContent<< " , A\n";

                                    //                                LogNonOverwrite("ACK_Source_wTime2.csv",LogContent.str());
                                }
                                //else {
                                //                                LogContent << ack << " , " << new_pid << " , " << (int)tt_redundancy  << '\n';
                                //                                LogNonOverwrite("ACK_Source_woTime.csv",LogContent.str());
                                //                                //
                                //LogContent << ack << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy  << '\n';
                                //LogNonOverwrite("ACK_Source_woTime1.csv",LogContent.str());
                                //
                                //LogContent << ack << " , " << new_pid /*<< " , " << new_gid*/ << " , " << (int)tt_redundancy  << '\n';
                                //LogNonOverwrite("ACK_Source_woTime2.csv",LogContent.str());
                                //}
                                //
                                //
                            } //endif (m_DupAckGen)


                            // Determine Group ID of lost packet and count the lost packet of each Gid
                            uint16_t t_lost_pid = new_pid;
                            uint16_t t_lost_gid = 0;
                            if (num_lost_packet==255) {
                                t_lost_gid = NcSocketInfo.CombiBuffer->Get_GidofPid(t_lost_pid);
                                NcSocketInfo.CombiBuffer->DecreaseNumLost(t_lost_gid, 1);
                            }
                            else {
                                std::vector <uint16_t> t_vector2_Groupid;
                                std::vector <uint8_t>  t_vector2_numlost;
                                uint8_t  t_vector2_id  = 0;

                                for (uint8_t i=0; i<num_lost_packet; i++){
                                    t_lost_pid = new_pid - (i+1);
                                    t_lost_gid = NcSocketInfo.CombiBuffer->Get_GidofPid(t_lost_pid);

                                    NcSocketInfo.CombiBuffer->UpdateNumLost(t_lost_gid);
                                    uint8_t  t_numlost  = NcSocketInfo.CombiBuffer->GetNumLost(t_lost_gid);

                                    if (i == 0){
                                        t_vector2_Groupid.push_back(t_lost_gid);
                                        t_vector2_numlost.push_back(t_numlost );
                                    }
                                    else {
                                        if (t_vector2_Groupid[t_vector2_id] != t_lost_gid){
                                            t_vector2_id ++;
                                            t_vector2_Groupid.push_back(t_lost_gid);
                                            t_vector2_numlost.push_back(t_numlost );
                                        }
                                        else {
                                            t_vector2_numlost[t_vector2_id] = t_numlost;
                                        }
                                    }
                                }

                                for (uint8_t i=0; i<t_vector2_Groupid.size(); i++){
                                    if (t_vector2_numlost[i] > NcSocketInfo.CombiBuffer->GetCapacity(t_vector2_Groupid[i])){
                                        NcSocketInfo.CombiBuffer->UpdateRetransList(t_vector2_Groupid[i]);
                                    }
                                }
                            }

                            //Remove NC Header of this Pid and lost Pid also.
                            //NcSocketInfo.CombiBuffer->Remove_BufferCombi (new_pid, /*num_lost_packet,*/ expect_ack);
                            //NcSocketInfo.CombiBuffer->Remove_OneBufferCombi (Socket_ID, new_pid);

                            if (iii < t_numInListPid-1)
                                NcSocketInfo.CombiBuffer->Remove_BufferCombi_one(new_pid);
                            else
                                NcSocketInfo.CombiBuffer->Remove_BufferCombi (new_pid, NcSocketInfo.SeenUnseen->Get_GidofSN(expect_ack));
                        }
                        else {
                            std::cout << "Error at nc-l4-protocol.cc  -  new_pid == ChannelInfoReturn.OldPid \n";
                            exit(-1);
                        }
                        //                        }
                    }

                    //Clear no need retransmit Gid (numlost <= k);
                    for (uint8_t i=0; i<NcSocketInfo.CombiBuffer->GetPacketStatus();i++){
                        uint16_t t_gid      = NcSocketInfo.CombiBuffer->GetRetransGID(i);
                        uint8_t  t_num_lost = NcSocketInfo.CombiBuffer->GetNumLost (t_gid);
                        uint8_t  t_capacity = NcSocketInfo.CombiBuffer->GetCapacity(t_gid);

                        if (t_num_lost <= t_capacity) {
                            NcSocketInfo.CombiBuffer->RemovefromRetransList(i);
                            i--;
                        }
                        //                        else {
                        //if (Simulator::Now().GetSeconds() > 1997)
                        //                        std::cout << " retrans list   gid=" << t_gid << "  t_num_lost=" << (int)t_num_lost << "  t_capacity=" << (int)t_capacity <<  std::endl;
                        //                        }
                    }
                } // m_ForwardRetransmission

                if(!t_Redundancy && t_Dependent && NcSocketInfo.Buffer->IsFirstDependence()){
                    NcSocketInfo.Buffer->Reset_IsFirstDependence();
                    return;
                }

                NcSocketInfo.Buffer->Update_FinTable(ack,true);

//                LogContent << Simulator::Now().GetSeconds() << "," << ack << " , " << t_Pid  << " , " << (int)t_Redundancy << '\n';
//                LogNonOverwrite("ACK_Source_wTime.csv",LogContent.str());
//                LogContent << ack << " , " << t_Pid  << " , " << (int)t_Redundancy  << '\n';
//                LogNonOverwrite("ACK_Source_woTime.csv",LogContent.str());

//                LogContent << Simulator::Now().GetSeconds() << "," << ack << " , " << t_Pid  << " , " << t_Gid << " , " << (int)t_Redundancy
//                           << " , N=" << (int)Hch_tem.GetN() << " , PktSt=" << (int)Hch_tem.GetPacketStatus() << " , " ;
//                for (uint8_t ttt=0; ttt<Hch_tem.GetN(); ttt++) {
//                    if (ttt==0)
//                        LogContent<< Hch_tem.GetStart1();
//                    else
//                        LogContent<< "-" << Hch_tem.GetStart1() + Hch_tem.GetStartN(ttt);
//                }
//                LogContent<< " , \n";

//                LogNonOverwrite("ACK_Source_wTime2.csv",LogContent.str());


                //if (TCP_Header_t.GetAckNumber() != ack) {
                //    std::cout << "Faillllllllllllllllllllllllllllll \n";
                //    exit(-1);
                //}

                //TCP_Header_t.SetAckNumber(FakeAck);


                //if (t_Gid==3662) {
                //    std::cout << Simulator::Now().GetSeconds() << "\n\n";
                //}


                if (t_Redundancy == true){
                    //return IpL4Protocol::RX_OK;
                    return;
                }
                else {
                    //NcSocketInfo.CombiBuffer->UpdateNumAckReceiv (t_Gid);

                    /// Eraser Decoded packet in ENCODE BUFFER. Base on Expect ACK.
                    // Erase Seen Table of Combin Buffer
                    NcSocketInfo.SeenUnseen->RemoveSeenUnseen(expect_ack);

                    uint32_t t_buffSize = NcSocketInfo.Buffer->EraseManyPacket(expect_ack, false);
                    NC_SocketID.UpdateNcBufferSize(NcSocketInfo.socketID,t_buffSize,false);

                    packet->AddHeader(TCP_Header_t);
                    //
                    if (m_UnorderEst) {
                        bool t_IsDelayed = NC_ACK_Header_t.Get_DelayedAckIndicate();
                        //                        if (Simulator::Now().GetSeconds() > 1080)
                        //                            std::cout << "ACK=" << ack << "  -  Is Delayed ACK = " << (int)t_IsDelayed << "\n";
                        NcSocketInfo.UnorderEst->AddToDelayedAckBuffer(200,t_IsDelayed,m_LocalDeliver,ack,packet,t_ip,iif);
                        if (t_IsDelayed) return;
                    }
                }

                if (m_DupAckGen) {
                    for (uint8_t i=0; i<FakeAckPacket.size(); i++) {
                        ForwardToLocalDeliver (FakeAckPacket[i], t_ip, iif);
                    }
                }
            }
        } // if (check == false)
        else {
            if (m_SetSameOverhead) {
                // Remove N null bytes from ACK packet;
                packet = Rem_Null_to_ACK (packet);
            }
        }
    }
    else { // Data packet
        if (m_LogNumSentReceived){
            LogContent << num_packet_received++;
            LogOverwrite("Num_packet_received.csv",LogContent.str());
        }

        if (m_SetSameOverhead) {
            // Remove N null bytes from original packet;
            packet = Rem_Null_to_Packet(packet, ProNumNC, m_RedundancyFactor, m_Max_k);
        }

        NcHeader ncHeader_t;
        packet->RemoveHeader(ncHeader_t);
        SequenceNumber32 base_t   = ncHeader_t.GetBase();
        uint16_t         sport_t  = ncHeader_t.GetSourcePort();
        uint16_t         dport_t  = ncHeader_t.GetDestinationPort();
        uint32_t         Start1_t = ncHeader_t.GetStart1();
        uint16_t         PktID_t  = ncHeader_t.GetPacketID();
        //uint8_t          NumDelAck= ncHeader_t.GetNumDelAck();

        //if (0)
        //std::cout << "NumDelAck=" << (int)NumDelAck << "\n";

        //Get Socket Information
        NcSocketInfo = NC_SocketID.GetSocketID(saddr, daddr, sport_t, dport_t);

        if (saddr == Ipv4Address("1.0.1.1")) {
            LogContent << PktID_t << " , " << Simulator::Now().GetSeconds() << "\n";
            LogNonOverwrite("AtRx.csv",LogContent.str());
        }

        //        std::cout << "[AtRx]  Pid=" << PktID_t << std::endl;



//        if (Start1_t>=35238785 && Start1_t<=35259689)
        std::cout << "RECEIVE DATA  -  Start1_t=" << Start1_t  << " - Base=" << base_t << " - N=" << (int)ncHeader_t.GetN() <<
                     " - PktID_t=" << PktID_t <<
                     "  -  PktStatus=" << (int)ncHeader_t.GetPacketStatus() << "  -  GetSerializedSize=" << ncHeader_t.GetSerializedSize() <<
                     "  -  t=" << Simulator::Now().GetSeconds()  << "\n";


        // Remove packet from NC-Buffer
        uint32_t t_buffSize = NcSocketInfo.Buffer->EraseManyPacket(base_t, true);
        NC_SocketID.UpdateNcBufferSize(NcSocketInfo.socketID,t_buffSize,true);

        //uint16_t SocketID = NcSocketInfo.Buffer->GetSocketId(saddr, daddr, sport_t, dport_t,true);

        //NcDecode::SeenInfo SeenInfomation = NcSocketInfo.DeCode->Decode2(ncHeader_t);
        // LogContent << SeenInfomation.SmallestUnseenSN << " , " << PktID_t << " , " << (int)SeenInfomation.Redundancy << '\n';
        //LogNonOverwrite("ACK_Sink_woTime2.csv",LogContent.str());

        // Check duplicate packet VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
        if (ncHeader_t.GetN()==1){
            if (NcSocketInfo.Buffer->CheckAlreadyWrite((SequenceNumber32)Start1_t, true) == true)
            {
                //                std::cout << "  AlreadyWrite   \n";
                SequenceNumber32 ACK_num = NcSocketInfo.DeCode->Get_UnSeen();
                uint16_t WinSize = NcSocketInfo.Buffer->GetWinSize(true);

                //
                bool    t_IsDelayedAck  = false;
                uint8_t t_unOrderLeng   = 0;
                uint8_t t_IsUpdate      = 0;
                bool    t_LongJumpRatio = false;
                //
                if (m_UnorderEst) {
                    t_IsDelayedAck  = NcSocketInfo.UnorderEst->IsDelayedAck      (ACK_num,m_DurationEstimationLoss,m_NumOf_SMA,true,m_LOG_UnOrdering);
                    t_unOrderLeng   = NcSocketInfo.UnorderEst->GetUnOrderLengthRx(PktID_t,m_DurationEstimationLoss,m_NumOf_SMA,m_LOG_UnOrdering);
                    t_LongJumpRatio = NcSocketInfo.UnorderEst->GetLongJumpRatio  ();
                    t_IsUpdate      = NcSocketInfo.UnorderEst->GetUpdateStatus   ();
                }
                //
                //if (t_LongJumpRatio)
                //    std::cout << "   t_LongJumpRatio=" << (int)t_LongJumpRatio << "  -  t_unOrderLeng=" << (int)t_unOrderLeng << "\n";

                NcDecode::Get_Dependent_return Dependent_Infomation = NcSocketInfo.DeCode->Get_Dependent();

                uint32_t loss_seq = NcSocketInfo.LossEst->Update_LossSeqRx(PktID_t,256);

                NcCreateNewACK NC_Create_New_ACK;
                NC_Create_New_ACK.CreateACK(dport_t, sport_t, WinSize, (SequenceNumber32)1, ACK_num,
                                            PktID_t, true, Dependent_Infomation.have, Dependent_Infomation.dependent_sn,
                                            loss_seq,t_unOrderLeng, t_LongJumpRatio, t_IsDelayedAck, t_IsUpdate, 0);

                bool t_DuplicatedAck = false;
                if (m_DelAck) {
                    if (ACK_num == PreviousACKnum) {
                        t_DuplicatedAck = true;
                    }
                    else {
                        PreviousACKnum = ACK_num;
                    }
                }

                Ptr<Packet>    m_ACK_Packet   = NC_Create_New_ACK.GetPacket();
                TcpHeader      TCP_ACK_Header = NC_Create_New_ACK.GetHeader();
                Ipv4Header     ipheader       = CreateHeader.CreateIPv4Header(daddr, saddr, ProNumTCP);
                Ptr<Ipv4Route> t_route        = CreateRoute(m_ACK_Packet, ipheader, daddr);


//                LogContent << Simulator::Now().GetSeconds() << "," << ACK_num << " , " << PktID_t << " , 1" << '\n';
//                LogNonOverwrite("ACK_Sink_wTime.csv",LogContent.str());
//                LogContent << ACK_num << " , " << PktID_t << " , 1" << '\n';
//                LogNonOverwrite("ACK_Sink_woTime.csv",LogContent.str());


                m_control_send = true;

                return TCPSend(m_ACK_Packet,TCP_ACK_Header,daddr,saddr,t_route, m_AckRetran, m_DelAck, ncHeader_t.GetNumDelAck(),t_DuplicatedAck);
            }
        }

        uint8_t decoded = NcSocketInfo.DeCode->Decode(packet, ncHeader_t, NcSocketInfo.Buffer);

//        if (Start1_t>=35238785 && Start1_t<=35259689)
        std::cout << "Num decoded = " << (int)decoded << "\n";

        // return 0 => can not decode or this is a rudundancy packet;
        // return N => N packets are decoded
        // return 255 => 0 packet is decoded, but this is the combination packet

        if (decoded > 0   &&   decoded != 255){
            for (uint16_t i=0; i<decoded; i++) {
                std::cout << &NcSocketInfo << "  -  " << (&NcSocketInfo)->DeCode << "  -  " << NcSocketInfo.DeCode << "\n";

                Ptr<Packet> decoded_packet = NcSocketInfo.DeCode->GetPacket(i);

                uint32_t t_buffSize = NcSocketInfo.Buffer->Write(decoded_packet,NcSocketInfo.DeCode->GetSN(i),true,NcSocketInfo.BufferInSize,m_buffer_size);
                NC_SocketID.UpdateNcBufferSize(NcSocketInfo.socketID, t_buffSize, true);

                // Create TCP header
                TcpHeader New_TCP_header = CreateHeader.CreateTcpHeader(sport_t,dport_t,/*ncHeader_t.GetWindowSize()*/56250,NcSocketInfo.DeCode->GetSN(i),(SequenceNumber32)1,TcpFlag_ACK);

//                if (Start1_t>=35238785 && Start1_t<=35259689) {
                std::cout << "     DECODE SN=" << NcSocketInfo.DeCode->GetSN(i) << " - PktSt=" << ncHeader_t.GetPacketID() << " - t=" << Simulator::Now().GetSeconds() << "\n";
                if (i==decoded-1)
                    std::cout << "\n\n";
//                }

                //std::cout << ncHeader_t.GetWindowSize() << "\n";

                // Move to TCP Layer
                if (i == decoded-1){
                    packet = decoded_packet;
                    packet->AddHeader(New_TCP_header);
                }
                else {
                    decoded_packet->AddHeader(New_TCP_header);
                    ForwardToLocalDeliver(decoded_packet, t_ip, iif);
                }
            }
        }




        SequenceNumber32 ACK_num = NcSocketInfo.DeCode->Get_UnSeen();
        uint16_t         WinSize = NcSocketInfo.Buffer->GetWinSize(true);

        NcCreateNewACK NC_Create_New_ACK;

        NcDecode::Get_Dependent_return  Dependent_Infomation = NcSocketInfo.DeCode->Get_Dependent();
        SequenceNumber32 SN_dependent = Dependent_Infomation.dependent_sn;
        bool dependent                = Dependent_Infomation.have;
        bool redundancy               = !NcSocketInfo.DeCode->IsRedundancyAck();

        //
        bool    t_IsDelayedAck  = false;
        uint8_t t_unOrderLeng   = 0;
        uint8_t t_IsUpdate      = 0;
        bool    t_LongJumpRatio = false;
        //
        if (m_UnorderEst) {
            t_IsDelayedAck  = NcSocketInfo.UnorderEst->IsDelayedAck      (ACK_num,m_DurationEstimationLoss,m_NumOf_SMA,redundancy,m_LOG_UnOrdering);
            t_unOrderLeng   = NcSocketInfo.UnorderEst->GetUnOrderLengthRx(PktID_t,m_DurationEstimationLoss,m_NumOf_SMA,m_LOG_UnOrdering);
            t_LongJumpRatio = NcSocketInfo.UnorderEst->GetLongJumpRatio();
            t_IsUpdate      = NcSocketInfo.UnorderEst->GetUpdateStatus();
        }

        // Create and Send ACK packet
        uint32_t loss_seq = NcSocketInfo.LossEst->Update_LossSeqRx(PktID_t,100);

        NC_Create_New_ACK.CreateACK(dport_t, sport_t, WinSize, (SequenceNumber32)1, ACK_num, PktID_t, redundancy, dependent, SN_dependent,
                                    loss_seq, t_unOrderLeng, t_LongJumpRatio, t_IsDelayedAck, t_IsUpdate, 0);

        bool t_DuplicatedAck = false;
        if (m_DelAck) {
            if (ACK_num == PreviousACKnum) {
                t_DuplicatedAck = true;
            }
            else {
                PreviousACKnum = ACK_num;
            }
        }

        Ptr<Packet>    m_ACK_Packet   = NC_Create_New_ACK.GetPacket();
        TcpHeader      TCP_ACK_Header = NC_Create_New_ACK.GetHeader();
        Ipv4Header     ipheader       = CreateHeader.CreateIPv4Header(daddr, saddr, ProNumTCP);
        Ptr<Ipv4Route> t_route        = CreateRoute(m_ACK_Packet, ipheader, daddr);

        m_control_send = true;

        TCPSend(m_ACK_Packet,TCP_ACK_Header,daddr,saddr,t_route,m_AckRetran,m_DelAck,ncHeader_t.GetNumDelAck(),t_DuplicatedAck);

//        LogContent << Simulator::Now().GetSeconds() << "," << ACK_num << " , " << PktID_t << " , " << (int)redundancy << '\n';
//        LogNonOverwrite("ACK_Sink_wTime.csv",LogContent.str());
//        LogContent << ACK_num << " , " << PktID_t << " , " << (int)redundancy << '\n';
//        LogNonOverwrite("ACK_Sink_woTime.csv",LogContent.str());


//        if (SN_dependent.GetValue()>=35238785 && SN_dependent.GetValue()<=35259689)
//        std::cout << "SEND ACK  -  ack=" << ACK_num  << " - redun=" << (int)redundancy <<
//                     " - depen=" << (int)dependent << " - dsn=" << SN_dependent <<
//                     " - pid=" << PktID_t <<
//                     "  -  t=" << Simulator::Now().GetSeconds()  << "\n";

        if (decoded==0 || decoded==255){
            return;
        }
    }

    ForwardToLocalDeliver (packet, t_ip, iif);
}

//uint16_t num_AckRetran = 0;

void NcL4Protocol::AckRetransmisionSchedule (NcSocketID::SocketInfo *SockInfo, Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route){
    if (*(SockInfo->AckRetransEventFin)==true) {
        SockInfo->AckRetransEvent->Cancel();
        return;
    }
    //
    Ptr<Packet> packet_copy = packet->Copy();
    m_Send(packet,saddr,daddr,ProNumTCP,route);

    //num_AckRetran++;
    //LogContent << Simulator::Now().GetSeconds() << "," << num_AckRetran << "\n";
    //
    *(SockInfo->AckRetransEvent) = Simulator::Schedule(MilliSeconds(200),&NcL4Protocol::AckRetransmisionSchedule,this,SockInfo,packet_copy,saddr,daddr,route);
}

void NcL4Protocol::DelayedAckSchedule (Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, uint8_t pronum, Ptr<Ipv4Route> route,
                                       uint16_t delay_pkt, uint16_t delay_time, bool DuplicatedAck){
    if (!DuplicatedAck) {
        NcMakeSchedule::ReturnValue DelayedAckInfo;

        DelayedAckInfo = NcSocketInfo.MakeSchedule->GetDelayedAckInfo();

        //std::cout << "delay_pkt=" << delay_pkt << "\n";
        if (DelayedAckInfo.Counter < delay_pkt) {
            NcSocketInfo.MakeSchedule->IncreasePktCounter();
            NcSocketInfo.MakeSchedule->AddEvent(Simulator::Schedule(MilliSeconds(delay_time),&NcL4Protocol::SendAckWhenScheduleExpired,this,packet->Copy(),saddr,daddr,pronum,route));
            return;
        }
    }

    NcSocketInfo.MakeSchedule->IncreasePktCounter(true);
    NcSocketInfo.MakeSchedule->CancelAllEvents();

    SendAckWhenScheduleExpired(packet,saddr,daddr,pronum,route);

}

void  NcL4Protocol::SendAckWhenScheduleExpired (Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, uint8_t pronum, Ptr<Ipv4Route> route){
    TcpHeader Tcp_Header;
    packet->RemoveHeader(Tcp_Header);
    NcAckHeader NcAck_Header;
    packet->RemoveHeader(NcAck_Header);
    uint16_t t_AckId = NcSocketInfo.LossEst->GetAckId();
    NcAck_Header.SetAckId(t_AckId);
    packet->AddHeader(NcAck_Header);
    packet->AddHeader(Tcp_Header);

    m_Send(packet,saddr,daddr,pronum,route);

    //std::cout << "Send  -  " << t_AckId << "\n\n";
}


void NcL4Protocol::ForwardToLocalDeliver (Ptr<const Packet> packet, Ipv4Header const&ip, uint32_t iif){
    if (m_LOG_Packet && m_NC_Enable){
        if (packet->GetSize() > TcpHeaderNormalSize){
            Ptr<Packet> p = packet->Copy();
            TcpHeader tcpheader;
            p->RemoveHeader(tcpheader);

            LogPacket("1-Result/De-Coded.csv",p,tcpheader.GetSequenceNumber(),p->GetSize());
        }
    }

    m_LocalDeliver(packet, ip, iif);
}

void NcL4Protocol::ForTCP_UnicastForward(Ptr<Ipv4Route> rtentry, Ptr<Packet> packet, const Ipv4Header &header){
    if (m_proxy_enable){
        if (m_GWtype == 1) {  // At Sender GW1
            Ptr<NetDevice> t_iif = m_GetInInterface();
            Ptr<NetDevice> t_oif = rtentry->GetOutputDevice();

            bool t_iif_is_global = false;
            bool t_oif_is_global = false;
            for (uint8_t i=0; i<m_GlobalInt.size(); i++){
                if (m_GlobalInt[i] == t_iif){
                    t_iif_is_global = true;
                }
                if (m_GlobalInt[i] == t_oif){
                    t_oif_is_global = true;
                }
            }

            if (!t_iif_is_global  &&  t_oif_is_global) {
                Ptr<Packet> p_copy = packet->Copy ();
                Ipv4Header t_l3_header = header;
                TcpHeader  t_l4_header;
                p_copy->RemoveHeader(t_l4_header);

                m_ControlPacketStatus = Check_Control_ForTunnel.Check(t_l3_header.GetSource(),t_l3_header.GetDestination(),
                                                                      t_l4_header.GetSourcePort(),t_l4_header.GetDestinationPort(),
                                                                      t_l4_header.GetFlags(),t_l4_header.GetSequenceNumber(),
                                                                      t_l4_header.GetAckNumber(),p_copy->GetSize(),true);

                if (!m_ControlPacketStatus) {
                    //Drop packet => Move to TCP handle to send into TCP/NC tunnel
                    packet->AddHeader(header);
                    m_SendInformationToHandle (packet,
                                               t_l3_header.GetSource(),t_l3_header.GetDestination(),
                                               t_l4_header.GetSourcePort(),t_l4_header.GetDestinationPort());
                    return;
                }
            }
        }

        m_IpForward (rtentry,packet,header);
    }
    else {
        m_IpForward (rtentry,packet,header);
    }
}

bool NcL4Protocol::GetControlPacketStatus (){
    return m_ControlPacketStatus;
}

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^




/*=============================================
     *                                           ||
     * FOR RECEIVING A PACKET FROM UPPER LAYER   ||
     *                                           ||
     * ============================================VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
     */
void NcL4Protocol::Send(Ptr<Packet> packet, Ipv4Address saddr,Ipv4Address daddr, uint8_t protocol, Ptr<Ipv4Route>route){
    if (m_NC_Enable==true){
        switch (protocol)
        {
        case ProNumTCP:  //TCP
            ForTCP_Send(packet, saddr, daddr, route);
            break;
            //case ProNumUDP: //UDP
            //    ForUDP(packet, source, destination, protocol, route);
            //    break;
        default:
            // Do nothing;
            m_Send(packet, saddr, daddr, protocol, route);
            break;
        }
    }
    else {
        m_Send(packet, saddr, daddr, protocol, route);
    }
}


void NcL4Protocol::ForTCP_Send(Ptr<Packet> packet, Ipv4Address saddr,Ipv4Address daddr, Ptr<Ipv4Route>route){
    TcpHeader tcpHeader;
    packet->RemoveHeader(tcpHeader);
    uint16_t         sport = tcpHeader.GetSourcePort();
    uint16_t         dport = tcpHeader.GetDestinationPort();
    SequenceNumber32 sn    = tcpHeader.GetSequenceNumber();
    SequenceNumber32 ackn  = tcpHeader.GetAckNumber();
    uint8_t          flag  = tcpHeader.GetFlags();
    uint32_t         psize = packet->GetSize();

    //
    NcSocketInfo = NC_SocketID.GetSocketID(saddr, daddr, sport, dport);
    //

    if (flag == TcpFlag_FIN || flag == TcpFlag_FINACK) {
//        std::cout << "*NcSocketInfo.AckRetransEventFin = true\n";

        NcSocketInfo.AckRetransEvent->Cancel();
        *NcSocketInfo.AckRetransEventFin = true;
    }


//    if (Simulator::Now().GetSeconds() > 2306)
//    if (Simulator::Now().GetSeconds()>3.2828) {
//    std::cout << "Send at: " << Simulator::Now().GetSeconds() << " sip=" << saddr << " dip=" << daddr << " Psz=" << packet->GetSize()
//              << " sn=" << sn << " ackn=" << ackn << " flag" << (int)flag;
//    if ((sn.GetValue()-1)%536>0) {
//        std::cout << "no536\n";
//    }
//    else {
//        std::cout << "\n";
//    }
//    }
    //    std::cout << "         (sn-1) mod 536 > 0 = " << (int)((sn.GetValue()-1)%536>0) << "\n";

    /*
     * Indicate the sending behaviour
     *      0 = encode data;
     *      1 = control data (ACK, FIN,...)
     */
    //m_control_send = Check_Control.Check(saddr, daddr, sport, dport, flag, sn, ackn, psize, true);
    m_control_send = NcSocketInfo.CheckControl->Check(flag, sn, ackn, psize);
    //
    if (m_control_send==true){
        /*
         * The FIN Packet have a payload.
         * Should check whether this packet is a retransmission or not.
         * If YES (means this FIN is duplicated)
         *      Should replace the payload (wrong) by an needed payload.
         * Else (NO)
         *      Store the info of this FIN packet to database
         */
        if (flag == TcpFlag_FINACK  &&  psize != 0){
            //uint16_t Socket_ID = NcSocketInfo.Buffer->GetSocketId(saddr, daddr, sport, dport, false);

            bool Dulicate_Fin = NcSocketInfo.Buffer->DupilcateFin ();

            if (Dulicate_Fin){ //YES==============================================
                SequenceNumber32 resent_sn     = NcSocketInfo.Buffer->GetLastACK ();
                Ptr<Packet>      resent_packet = NcSocketInfo.Buffer->Read(resent_sn, false);
                SequenceNumber32 base          = NcSocketInfo.Buffer->GetBase();
                uint16_t t_pid                 = NcSocketInfo.Buffer->GetPid_countup(true);


                // Get Number of Delayed ACK
                uint8_t NumDelayedAck = 1;
                if (m_DelAck) {
                    NumDelayedAck = NcSocketInfo.LossEst->GetNumDelayedAck(m_NumOf_SMA);
                    //std::cout << "    NumDelayedAck=" << (int)NumDelayedAck << "\n";
                }


                // Create NC Header;
                NcHeader m_NC_Header = CreateHeader.CreateNcHeader1(sport,dport,t_pid,base,(uint8_t)0,resent_sn.GetValue(),resent_packet->GetSize(),
                                                                    NumDelayedAck/*,NcSocketInfo.Buffer->GetWinSize(false)*/);

                m_control_send = false;

                if (m_ForwardRetransmission){
                    uint16_t t_gid = NcSocketInfo.CombiBuffer->Get_GidofPid(t_pid);
                    NcSocketInfo.CombiBuffer->Add_BufferCombi (t_gid, t_pid, m_NC_Header);
                }

                NCSend(resent_packet,m_NC_Header,saddr,daddr,route);
            }
            else { //NO=============================================================
                NcSocketInfo.Buffer->Update_FinTable((SequenceNumber32)1, false);
                //
                uint32_t t_buffSize = NcSocketInfo.Buffer->Write(packet, sn, false, NcSocketInfo.BufferOutSize, m_buffer_size);
                NC_SocketID.UpdateNcBufferSize(NcSocketInfo.socketID, t_buffSize, false);
                //
                TCPSend(packet,tcpHeader,saddr,daddr,route);
            }
        }
        else {
            TCPSend(packet,tcpHeader,saddr,daddr,route);
        }
    } // if (m_control_send==true)
    else{
        if (psize != 0){
            /*
             *  Check Already write; If WROTE => This is retransmitted (dulicate ACK or Time out) from TCP Layer
             *                                   => Don't Encode this; just add NC Header.
             *                       If NOT   => This is a new packet => Encode this.
             */

            //IF NOT WROTE ============================================================================================

            //            if (saddr=="192.168.3.1") {
            //                NS_LOG_UNCOND ("SEND  -  " << sn << "  -  Dup=" << NcSocketInfo.Buffer->CheckAlreadyWrite(sn, false) <<
            //                               "  -  t=" << Simulator::Now().GetSeconds());
            //            }



            //NcSocketInfo.Buffer->SetWinSize(tcpHeader.GetWindowSize(),false);


            // Get Number of Delayed ACK
            uint8_t NumDelayedAck = 1;
            if (m_DelAck) {
                NumDelayedAck = NcSocketInfo.LossEst->GetNumDelayedAck(m_NumOf_SMA);
                //std::cout << "    NumDelayedAck=" << (int)NumDelayedAck << "\n";
            }


            if (NcSocketInfo.Buffer->CheckAlreadyWrite(sn, false) == false) {
                uint32_t t_buffSize = NcSocketInfo.Buffer->Write(packet, sn, false, NcSocketInfo.BufferOutSize, m_buffer_size);
                NC_SocketID.UpdateNcBufferSize(NcSocketInfo.socketID, t_buffSize, false);

                // For Proxy: VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
                if (m_proxy_enable) {
                    Ptr<Packet> t_packet_copy = packet->Copy();
                    Ipv4Header  t_l3_header;
                    TcpHeader   t_l4_header;
                    t_packet_copy->RemoveHeader(t_l3_header);
                    t_packet_copy->RemoveHeader(t_l4_header);

                    Update_Proxytable(sn + psize,
                                      t_l4_header.GetSequenceNumber() + t_packet_copy->GetSize(),
                                      t_l3_header.GetSource(),
                                      t_l3_header.GetDestination());
                }
                //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

                //Store the contain of packet to check
                if (m_LOG_Packet){
                    LogPacket("1-Result/OriginalPacket.csv",packet,sn,psize);
                }

                // NETWORK ENCODING PROCESS
                DoEncode(packet,sn,saddr,daddr,sport,dport,psize,route,NumDelayedAck);

            } // (NcSocketInfo.Buffer->CheckAlreadyWrite...

            //IF WROTE =============================================================================================
            else {
                /// Retransmit or Time Out......
                //uint16_t SocketID       = NcSocketInfo.Buffer->GetSocketId(saddr, daddr, sport, dport, false);
                SequenceNumber32 base   = NcSocketInfo.Buffer->GetBase();

                //??????????????????????? Why needed
                //if (sn >= base)
                //    return;
                if (sn < base)
                    return;

                // ===================================
                // Retransmit dependent packet =======
                // ===================================
                DependenceReturn DependStatus       = DependenceRetransmit(sport,dport,base,sn,saddr,daddr,route,NumDelayedAck);
                bool     Dependent_retransmit       = DependStatus.retransmit;
                bool     Force_Dependent_retransmit = DependStatus.force;
                uint16_t Dependent_Gid              = DependStatus.Gid;

                // ===================================
                // Retransmit Burst Lost Packet ======
                // ===================================
                if (m_ForwardRetransmission==true){
                    if(ForwardRetransmit(saddr,daddr,sport,dport,sn,base,
                                         Dependent_retransmit,Force_Dependent_retransmit,Dependent_Gid,
                                         route,NumDelayedAck)){
                        return;
                    }
                }
                else {
                    uint16_t t_pid = NcSocketInfo.Buffer->GetPid_countup(true);
                    NcHeader m_NC_Header = CreateHeader.CreateNcHeader1(sport,dport,t_pid,base,(uint8_t)0,
                                                                        sn.GetValue(),psize,NumDelayedAck
                                                                        /*NcSocketInfo.Buffer->GetWinSize(false)*/);
                    NCSend(packet,m_NC_Header,saddr,daddr,route);
                    return;
                }
            }
        }
        else{//packet size = 0
//            if (Simulator::Now().GetSeconds() > 34.0)
//                std::cout << " Drop ACK at NC Layer - ACK=" << ackn << "    " << Simulator::Now().GetSeconds() << "\n";

            NcSocketInfo.Buffer->SetWinSize(tcpHeader.GetWindowSize(),true);
        }
    }
}

void NcL4Protocol::TCPSend(Ptr<Packet> packet, TcpHeader &header, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route,
                           bool SetRetrans, bool DelayedAck, uint8_t numDelayedAck, bool DuplicatedAck){
    bool ControlPkt = false;
    if (m_SetSameOverhead) {
        if (packet->GetSize()==0 || packet->GetSize()==NcAckHeaderSize){
            ControlPkt = true;
        }
    }

    if (Node::ChecksumEnabled ())
    {
        header.EnableChecksums ();
        header.InitializeChecksum (saddr, daddr, ProNumTCP);
    }

    packet->AddHeader (header);

    if (m_control_send==true){
        if (m_SetSameOverhead) {
            if (ControlPkt == false) {
                packet = Add_Null_to_Packet(packet,ProNumTCP,m_RedundancyFactor,m_Max_k); // Add 8 null bytes to ACK packet if it is not NC ACK packet;
            }
        }
    }

//    std::cout << "***Send at: " << Simulator::Now().GetSeconds() << " sip=" << saddr << " dip=" << daddr << " Psz=" << packet->GetSize()
//              << " sn=" << header.GetSequenceNumber() << " ackn=" << header.GetAckNumber() << " flag" << (int)header.GetFlags() << "\n";


    if (SetRetrans && !(*NcSocketInfo.AckRetransEventFin)) {
        //num_AckRetran = 0;
        //LogContent << Simulator::Now().GetSeconds() << "," << num_AckRetran << "\n";
        //LogNonOverwrite("1-Result/Log_AckRetransmission.csv",LogContent.str());

        NcSocketInfo.AckRetransEvent->Cancel();
        //
        Ptr<Packet> packet_copy = packet->Copy();
        *NcSocketInfo.AckRetransEvent=Simulator::Schedule(MilliSeconds(200),&NcL4Protocol::AckRetransmisionSchedule,this,&NcSocketInfo,packet_copy,saddr,daddr,route);
    }

    if (DelayedAck) {
        DelayedAckSchedule(packet,saddr,daddr,ProNumTCP,route,numDelayedAck,200,DuplicatedAck);
    }
    else {
        m_Send(packet,saddr,daddr,ProNumTCP,route);
    }
}

void NcL4Protocol::NCSend(Ptr<Packet> packet, NcHeader &header, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route){
    if (m_LogNumSentReceived){
        LogContent << num_packet_sent++;
        LogOverwrite("Num_packet_sent.csv",LogContent.str());
    }

//    if (NcSocketInfo.SeenUnseen->Get_GidofSN((SequenceNumber32)header.GetStart1()) == 2909) {
//    if (Simulator::Now().GetSeconds()>3.2828){
    std::cout << "Send NC data  -  Pid=" << header.GetPacketID() << "   " << Simulator::Now().GetSeconds() << "\n";
    std::cout << "SN1=" << header.GetStart1() << "\n";
    for (uint8_t i=1; i<header.GetN(); i++) {
        std::cout << "SN" << (int)i+1 << "=" << header.GetStart1()+header.GetStartN(i) << "\n";
    }

    std::cout << "\n\n";
//    }
//    }

    if (saddr == Ipv4Address("1.0.1.1")) {
        LogContent << NcSocketInfo.SeenUnseen->Get_GidofSN((SequenceNumber32)header.GetStart1()) << " , " <<  header.GetPacketID() << " , " << header.GetStart1();
        for (uint8_t i=1; i<header.GetN(); i++)
            LogContent << "-" << header.GetStart1()+header.GetStartN(i);
        LogContent << " , " << Simulator::Now().GetSeconds() << "\n";
        LogNonOverwrite("Pid_Gid.csv",LogContent.str());
    }

    packet->AddHeader(header);

    if (m_SetSameOverhead) {
        packet = Add_Null_to_Packet(packet, ProNumNC, m_RedundancyFactor, m_Max_k);  // Add N null bytes to original packet;
    }

    m_Send(packet,saddr,daddr,ProNumNC,route);
}

void NcL4Protocol::DoEncode(Ptr<Packet> packet, SequenceNumber32 sn, Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, uint32_t psize, Ptr<Ipv4Route> route, uint8_t NumDelayedAck){
    //uint16_t         SocketID = NcSocketInfo.Buffer->GetSocketId(saddr, daddr, sport, dport, false);
    SequenceNumber32 base     = NcSocketInfo.Buffer->GetBase();

    //Initial....
    bool InitialR_t = NcSocketInfo.LossEst->IsItInitialState();
    bool ChangeR_t  = false;
    bool ChangeK_t  = false;
    bool ChangeX_t  = false;
    uint8_t n_est   = 0;
    uint8_t m_est   = 0;
    uint8_t k_est   = 0;


    /*
     * If tunning NC-parameter is used
     * => Get new m, n, k, R
     */
    if (m_AutoTunningK==true || m_AutoTunningR==true){
        NcParaEstimationReturn = NcSocketInfo.LossEst->GetNcParameter ();

        if (InitialR_t == false) {
            ChangeR_t          = NcParaEstimationReturn.changeR;
            ChangeK_t          = NcParaEstimationReturn.changek;
            ChangeX_t          = NcParaEstimationReturn.changeX;
            n_est              = NcParaEstimationReturn.n;
            m_est              = NcParaEstimationReturn.m;
            k_est              = NcParaEstimationReturn.k;

            if (ChangeR_t || ChangeK_t) {
                if (ChangeR_t && m_AutoTunningR) {
                    m_RedundancyFactor = NcParaEstimationReturn.R;
                }

                if (ChangeK_t && m_AutoTunningK) {
                    m_Max_k = NcParaEstimationReturn.k;
                }
            }

            //Log n, m
            if (ChangeX_t) {
                if (m_lognm) {
                    LogContent << saddr << "," << Simulator::Now().GetSeconds() << "," << (int)n_est << "," << (int)m_est << "," << (int)k_est << '\n';
                    LogNonOverwrite("Log_NM.csv",LogContent.str());
                }
            }
        }
    }

    uint8_t  t_repeat  = 1;
    uint8_t  N_field   = 0;
    uint16_t t_psize   = psize;
    uint16_t t_counter = 0;

    //    bool t_skip = false;

    //std::cout << "n=" << (int)N_est << "  m=" << (int)M_est << "  k=" << (int)K_est << "\n";

    while (t_repeat > 0){
        if (m_ForceMN) {
            n_est = m_PreSetN;
            m_est = m_PreSetM;
            k_est = m_PreSetM - m_PreSetN;
        }

        //std::cout << "n=" << (int)N_est << "  m=" << (int)M_est << "  k=" << (int)K_est << "\n";

        Encode_Return = NcSocketInfo.EnCode->Encode(packet, sn,
                                                    InitialR_t, m_n_initial, m_k_initial,
                                                    ChangeR_t, ChangeK_t,
                                                    NcSocketInfo.Buffer, n_est, m_est, k_est, m_ForceMN,
                                                    saddr, daddr, m_DispInfoRNM);

        ChangeR_t = false;
        ChangeK_t = false;
        InitialR_t= false;

        t_counter++;

        //std::cout << "sn=" << sn << "  -t=" << Simulator::Now().GetSeconds() << "\n";

        //        if (UnorderedK && t_counter>1) {
        //            if (t_counter==K_est+2) {
        //                t_skip = true;
        //                UnorderedK = false;

        //                NcSocketInfo.Buffer->Update_ExpectAckTable(sn + (SequenceNumber32)t_psize);
        //            }
        //        }


        N_field  = Encode_Return.N;
        t_repeat = Encode_Return.Loop;

        //        if (!t_skip) {
        // Get Information=======================================
        uint8_t *Alpha;
        Alpha = new uint8_t[N_field];
        Alpha = Encode_Return.Alpha;

        uint16_t *t_size;
        t_size = new uint16_t[N_field];
        t_size = NcSocketInfo.EnCode->GetSize();

        SequenceNumber32 *t_SN;
        t_SN = new SequenceNumber32[N_field];
        t_SN = NcSocketInfo.EnCode->GetSequenceNumber();
        //========================================================

        //Increase Packet ID
        uint16_t t_pid = NcSocketInfo.Buffer->GetPid_countup(true);

        NcHeader         m_NC_Header = CreateHeader.CreateNcHeaderN (sport, dport, t_pid, base, (uint8_t)0, N_field,
                                                                     t_SN, t_size, Alpha,NumDelayedAck
                                                                     /*NcSocketInfo.Buffer->GetWinSize(false)*/);
        Ptr<Packet>      packet_temp = Encode_Return.Combination->Copy();

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        NCSend(packet_temp, m_NC_Header, saddr, daddr, route);

        //            std::cout << "sn=" << sn << " - N=" << (int)N_field  << " - t=" << Simulator::Now().GetMilliSeconds() << "\n";
        //            std::cout << "   - SN1=" << m_NC_Header.GetStart1() << "\n";
        //            for (uint8_t j=1; j<N_field; j++) {
        //                std::cout << "   - SN" << (int)j+1 << "=" << m_NC_Header.GetStart1()+m_NC_Header.GetStartN(j) << "\n";
        //            }
        //            std::cout << "\n";

        // Delete
        delete [] Alpha;
        delete [] t_size;
        delete [] t_SN;
        //=========================================================

        //if ((t_repeat == 0 && t_counter > 1) ||  Encode_Return.k == 0){
        if (Encode_Return.LastPktInCW){
            NcSocketInfo.Buffer->Update_ExpectAckTable(sn + (SequenceNumber32)t_psize, n_est);
        }

        // Store NC and Group ID to control Bursty Recovery
        if (m_ForwardRetransmission){
            uint16_t t_gid = 0;

            if (Encode_Return.Step == 1){
                //t_gid = NcSocketInfo.CombiBuffer->Add_GroupID(SocketID, Encode_Return.k, Encode_Return.n);
                t_gid = NcSocketInfo.CombiBuffer->Add_GroupID(Encode_Return.m-Encode_Return.n, Encode_Return.n);
            }
            else {
                t_gid = NcSocketInfo.CombiBuffer->Get_GroupID();
            }

            if (Encode_Return.ResetState == true) {
                //NcSocketInfo.CombiBuffer->Update_GroupID (SocketID, t_gid, Encode_Return.k, Encode_Return.n);
                NcSocketInfo.CombiBuffer->Update_GroupID (t_gid, Encode_Return.m-Encode_Return.n, Encode_Return.n);
            }

            NcSocketInfo.CombiBuffer->Add_BufferCombi (t_gid, t_pid, m_NC_Header);

//            if (sn > (SequenceNumber32)(48986113 - 536*50)) {
//                std::cout << "[Add_BufferCombi]    -    sn=" << sn << "  pid=" << t_pid << "  gid" << t_gid << "  t_repeat=" << (int)t_repeat << "\n";
//            }

            if (t_repeat == 0){  // No mention the redundancy combination packet;
                if (Encode_Return.LastPktInCW) {
                    t_gid++;
                }
                NcSocketInfo.SeenUnseen->AddUnSeen(t_gid, sn, t_psize);
            }
        }
        //        }
    }

    if (m_AutoTunningK==true || m_AutoTunningR==true) {
        NcSocketInfo.LossEst->ResetStateNcParameter();
    }
}

bool NcL4Protocol::DoEncodeRetrans (uint8_t gid_index, uint16_t gid,
                                    uint8_t numretrans, uint8_t numretransgid,
                                    std::vector<SequenceNumber32> sn, std::vector<uint16_t> size,
                                    SequenceNumber32 base, uint16_t sport, uint16_t dport,
                                    Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route, uint8_t NumDelayedAck){
    bool returnvalue = false;

    //uint8_t NumRedundancyPacket = ceil(m_RedundancyFactor * numretrans) - numretrans;  // The number of redundancy combination packets (from ReEncoding_numretrans retransmitted packet)

    NcEncode::GetNM_return current_nm = NcSocketInfo.EnCode->GetNM();
    //    uint8_t NumRedundancyPacket = current_nm.k;

    ///////////
    uint8_t NumRedundancyPacket = 0;

    uint8_t     tt_n = current_nm.n;
    uint8_t     tt_k = current_nm.k;

    if (numretrans == tt_n) {
        NumRedundancyPacket = tt_k;
    }
    else {
        NumRedundancyPacket = NcSocketInfo.EnCode->GetNearest_k(numretrans,tt_n,tt_k);
    }
    ///////////

//    std::cout << "NumRedundancyPacket=" << (int)NumRedundancyPacket << "\n";

    // Get packet
    uint16_t *retrans_size;
    retrans_size = new uint16_t[numretrans];

    SequenceNumber32 *retrans_SN;
    retrans_SN = new SequenceNumber32[numretrans];

    Ptr<Packet> *retrans_packet;
    retrans_packet = new Ptr<Packet>[numretrans];

    for (uint8_t i=0; i<numretrans; i++){
        retrans_size   [i] = size [i];
        retrans_SN     [i] = sn   [i];
        retrans_packet [i] = NcSocketInfo.Buffer->Read(sn[i],false);;
    }


    //Create Alpha
    uint8_t *retrans_Alpha;
    retrans_Alpha = new uint8_t[numretrans];

    //uint8_t **retrans_Alpha;
    //retrans_Alpha = new uint8_t*[numretrans];
    //for (uint8_t k=0; k<numretrans; k++){
    //    retrans_Alpha[k] = new uint8_t[numretrans+NumRedundancyPacket];
    //}

    NcEncode EncodeRetrans;
    //retrans_Alpha = EncodeRetrans.CreateAlpha(numretrans,numretrans+NumRedundancyPacket,NumRedundancyPacket);


    //Retransmission
    NcHeader         m_NC_Header;

    for (uint8_t i=0; i<NumRedundancyPacket; i++){
        //Create Alpha VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
        for (uint8_t ii=0; ii<numretrans; ii++){
            if (numretrans == 1) {
                retrans_Alpha[ii] = (uint8_t)1;
            }
            else {
                Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
                retrans_Alpha[ii] = uint8_t(uv->GetInteger(1,255));
            }
        }
        // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

        uint16_t pid   = NcSocketInfo.Buffer->GetPid_countup(true);

        if (i==NumRedundancyPacket-1  && gid_index==numretransgid-1) {
            m_NC_Header = CreateHeader.CreateNcHeaderN(sport,dport,pid,base,(uint8_t)4,numretrans,retrans_SN,
                                                       retrans_size,retrans_Alpha,NumDelayedAck
                                                       /*NcSocketInfo.Buffer->GetWinSize(false)*/);
        }
        else if (gid_index==numretransgid-1){
            m_NC_Header = CreateHeader.CreateNcHeaderN(sport,dport,pid,base,(uint8_t)3,numretrans,retrans_SN,
                                                       retrans_size,retrans_Alpha,NumDelayedAck
                                                       /*NcSocketInfo.Buffer->GetWinSize(false)*/);
        }
        else {
            m_NC_Header = CreateHeader.CreateNcHeaderN(sport,dport,pid,base,(uint8_t)1,numretrans,retrans_SN,
                                                       retrans_size,retrans_Alpha,NumDelayedAck
                                                       /*NcSocketInfo.Buffer->GetWinSize(false)*/);
        }

        NcSocketInfo.CombiBuffer->Add_BufferCombi (gid, pid, m_NC_Header);


        Ptr<Packet> t_CombinePacket = Create<Packet>();
        t_CombinePacket = EncodeRetrans.Combine(retrans_packet,retrans_Alpha,numretrans)->Copy();

        //        if (Simulator::Now().GetSeconds() > 1997)
        //        std::cout << "RETRASMIT (CODED)  -  " << saddr << "  -  " << (int)i+1 << "/" << (int)NumRedundancyPacket << "\n";

        NCSend(t_CombinePacket,m_NC_Header,saddr,daddr,route);

        if (i==NumRedundancyPacket-1  &&  gid_index==numretransgid-1) {
            returnvalue = true;
        }
    }

    delete [] retrans_packet;
    delete [] retrans_size;
    delete [] retrans_SN;
    //    for (uint8_t k=0; k<numretrans; k++){
    //        delete []retrans_Alpha[k];
    //    }
    delete []retrans_Alpha;

    return returnvalue;
}

NcL4Protocol::DependenceReturn NcL4Protocol::DependenceRetransmit(uint16_t sport, uint16_t dport, SequenceNumber32 base, SequenceNumber32 sn, Ipv4Address saddr, Ipv4Address daddr, Ptr<Ipv4Route> route, uint8_t NumDelayedAck){
    NcL4Protocol::DependenceReturn returnValue;
    returnValue.retransmit = false;
    returnValue.force      = false;
    returnValue.Gid        = 0;

    SequenceNumber32 SN_dependend = NcSocketInfo.Buffer->GetDependent_from_retras_packet(sn);

    if (NcSocketInfo.Buffer->GetDependentStatus()) {
        //if (sn.GetValue() > 99874489-10000)
        //    std::cout << "Retransmit Dependence Packet -- SN=" << SN_dependend << "\n";

        if (m_ForwardRetransmission) {
            uint16_t t_gid     = NcSocketInfo.SeenUnseen->Get_GidofSN(SN_dependend);
            uint8_t m_numlost  = NcSocketInfo.CombiBuffer->GetNumLost(t_gid);
            uint8_t m_capacity = NcSocketInfo.CombiBuffer->GetCapacity(t_gid);

//            std::cout << "    - t_gid=" << t_gid << " - m_numlost=" << (int)m_numlost << " - m_capacity=" << (int)m_capacity << "\n";

            returnValue.Gid = t_gid;

            if (m_numlost <= m_capacity) {
                for (uint8_t m=0; m < m_capacity-m_numlost+1; m++) {
                    NcSocketInfo.CombiBuffer->UpdateNumLost(t_gid);
                }
//                returnValue.force = true;
            }

            // In the case: This Gid is already removed.
            //              Because it has many dependence packets,
            //              The second dependent packets is not known at source but
            //              all packet is seen (but dependent) => this Gid will be removed
            if (NcSocketInfo.CombiBuffer->GetNumLost(t_gid) == 0) {
                returnValue.force = true;
            }

            if (NcSocketInfo.CombiBuffer->GetPacketStatus() == false) {
                NcSocketInfo.CombiBuffer->UpdateRetransList(t_gid);
            }
            else {
                if (NcSocketInfo.CombiBuffer->CheckInRetransList(t_gid) == false) {
                    NcSocketInfo.CombiBuffer->UpdateRetransListandReOrder(t_gid);
                }
            }

            returnValue.retransmit = true;

            // NS_LOG_UNCOND ("     [LOG] Dependent retransmission: Dependent = " << SN_dependend  << "   -   Time=" << Simulator::Now().GetSeconds());
        }
        else {
            uint16_t t_pid = NcSocketInfo.Buffer->GetPid_countup(true);

            Ptr<Packet> dependend_packet = NcSocketInfo.Buffer->Read(SN_dependend, false);
            NcHeader m_NC_Header = CreateHeader.CreateNcHeader1(sport,dport,t_pid,base,(uint8_t)0,
                                                                SN_dependend.GetValue(),dependend_packet->GetSize(),NumDelayedAck
                                                                /*NcSocketInfo.Buffer->GetWinSize(false)*/);

            // NS_LOG_UNCOND ("     [LOG] Dependent retransmission: Dependent = " << SN_dependend  << "   -   Time=" << Simulator::Now().GetSeconds());
            NCSend(dependend_packet,m_NC_Header,saddr,daddr,route);
        }
    }

    return returnValue;
}

bool NcL4Protocol::ForwardRetransmit(Ipv4Address saddr, Ipv4Address daddr,
                                     uint16_t sport, uint16_t dport, SequenceNumber32 sn, SequenceNumber32 base,
                                     bool DependRetrans, bool ForceDependRetrans, uint16_t DependentGid,
                                     Ptr<Ipv4Route>route, uint8_t NumDelayedAck){
    //    if (m_ForwardRetransmission==true){
//    if (sn >= (SequenceNumber32)19617601)
//        std::cout << "m_ForwardRetransmission    - sn=" << sn << "\n";


    /* In case of the packet is Seen and it is not a Dependent retransmission,
         * Return ACK to TCP layer
         */
    //    if (NcSocketInfo.CombiBuffer->CheckSeen(SocketID,sn)==true && DependRetrans==false){
    //Because: with the same socket (In/Out) have the same Socket ID.
    //         We can use outgoing socket.

    //            uint16_t WinSize  = NcSocketInfo.Buffer->GetWinSize();

    //            TcpHeader t_ACKTCP_header;
    //            t_ACKTCP_header.SetSourcePort(dport);
    //            t_ACKTCP_header.SetDestinationPort(sport);
    //            t_ACKTCP_header.SetWindowSize(WinSize);
    //            t_ACKTCP_header.SetSequenceNumber((SequenceNumber32)1);
    //            t_ACKTCP_header.SetAckNumber(NcSocketInfo.Buffer->GetLastACK());
    //            t_ACKTCP_header.SetFlags(TcpFlag_ACK);

    //            Ptr<Packet> t_packet = Create<Packet> ();
    //            t_packet->AddHeader(t_ACKTCP_header);

    //            // Return ACK to TCP layer
    //            uint32_t t_iff = m_GetInterfaceForAddress(saddr);
    //            Ptr<IpL4Protocol> protocol = m_GetProtocol (ProNumTCP,t_iff);
    //            //Ipv4Header ipHeader = m_BuildHeader(daddr,saddr,ProNumTCP,0,128,0,false);
    //            Ipv4Header ipHeader = CreateHeader.CreateIPv4Header(daddr,saddr,ProNumTCP);

    //        std::cout << "     NcSocketInfo.Buffer->GetLastACK()=" << NcSocketInfo.Buffer->GetLastACK() << "\n";

    //            Simulator::Schedule (NanoSeconds(1),SendBackToTcp, protocol, t_packet, ipHeader, m_GetInterface(t_iff));
    //        return true;
    //    }

    if (sn < NcSocketInfo.Buffer->GetLastACK()  && DependRetrans==false){
        uint16_t WinSize  = NcSocketInfo.Buffer->GetWinSize(true);

        TcpHeader t_ACKTCP_header;
        t_ACKTCP_header.SetSourcePort(dport);
        t_ACKTCP_header.SetDestinationPort(sport);
        t_ACKTCP_header.SetWindowSize(WinSize);
        t_ACKTCP_header.SetSequenceNumber((SequenceNumber32)1);
        t_ACKTCP_header.SetAckNumber(NcSocketInfo.Buffer->GetLastACK());
        t_ACKTCP_header.SetFlags(TcpFlag_ACK);

        Ptr<Packet> t_packet = Create<Packet> ();
        t_packet->AddHeader(t_ACKTCP_header);

        // Return ACK to TCP layer
        uint32_t t_iff = m_GetInterfaceForAddress(saddr);
        Ptr<IpL4Protocol> protocol = m_GetProtocol (ProNumTCP,t_iff);
        //Ipv4Header ipHeader = m_BuildHeader(daddr,saddr,ProNumTCP,0,128,0,false);
        Ipv4Header ipHeader = CreateHeader.CreateIPv4Header(daddr,saddr,ProNumTCP);

//        std::cout << "     NcSocketInfo.Buffer->GetLastACK()=" << NcSocketInfo.Buffer->GetLastACK() << "\n";

        //Simulator::Schedule (NanoSeconds(1),SendBackToTcp, protocol, t_packet, ipHeader, m_GetInterface(t_iff));
        Simulator::ScheduleNow (SendBackToTcp, protocol, t_packet, ipHeader, m_GetInterface(t_iff));
        return true;
    }
    else {
        // If Gid of this packet doesn't have in Retransmit list => Add to retransmit list; VVVVVVVVVVVVVVVV
        uint16_t gid_of_this = 0;

        if (DependRetrans == false) {
            gid_of_this  = NcSocketInfo.SeenUnseen->Get_GidofSN(sn);
            bool in_list = NcSocketInfo.CombiBuffer->CheckInRetransList(gid_of_this);

            if (in_list == false) {
                NcSocketInfo.CombiBuffer->UpdateRetransListandReOrder(gid_of_this);
            }
        }
        // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


        // Get number of groups which need retransmit lost packet.
        uint8_t num_retrans_gid  = NcSocketInfo.CombiBuffer->GetPacketStatus();

        //        std::cout << "num_retrans_gid=" << (int)num_retrans_gid << std::endl;


        // bool IsRetransmitted = false;


        if (num_retrans_gid > 0) {
            for (uint8_t i=0; i<num_retrans_gid; i++){
                uint16_t t_Gid        = NcSocketInfo.CombiBuffer->GetRetransGID(i);
                uint8_t  t_numlost    = NcSocketInfo.CombiBuffer->GetNumLost              (t_Gid);
                uint8_t  t_capacity   = NcSocketInfo.CombiBuffer->GetCapacity             (t_Gid);
                uint8_t  t_numunseen  = NcSocketInfo.SeenUnseen->GetNumUnseenAndDependent (t_Gid);
                uint8_t  t_numretrans = 0;

//                if (Simulator::Now().GetSeconds() > 2060)
//                std::cout << " !!  t_numlost=" << (int)t_numlost << " - t_numunseen=" << (int)t_numunseen << " - t_capacity=" << (int)t_capacity << "\n";

                // Determine the number of retransmission of GID i-th VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
                // * NOTED: If number of retransmit packet = 0 => change to 1 ???
                //                if (t_numlost > t_capacity){
                if ((uint8_t)(t_numlost - t_capacity) < (uint8_t)128){      // t_t_numlost can be < 0 (128-255) due to unorderring issue. The source retransmit unnecessary packet.
                    t_numretrans = t_numlost - t_capacity;
                }

//                std::cout << "   - t_numlost - t_capacity=" << (uint16_t)((uint8_t)(t_numlost - t_capacity)) << "  -  t_numretrans=" << (int)t_numretrans << "\n";

                if (t_numretrans > t_numunseen){
                    t_numretrans = t_numunseen;
                }

                bool force_retrans = false;
                if (DependRetrans == false) {
                    if (t_Gid == gid_of_this) {
                        if (t_numretrans == 0) {
                            force_retrans = true;
                            t_numretrans  = 1;
                        }
                    }
                }
                else {
                    if (ForceDependRetrans) {
                        if (t_Gid == DependentGid) {
                            t_numretrans  = 1;
                        }
                    }
                }
                //Determine the number of retransmission ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


                if (force_retrans == false) {
                    NcSocketInfo.CombiBuffer->DecreaseNumLost(t_Gid, t_numretrans);
                }

                std::vector<SequenceNumber32> temp_sn;
                std::vector<uint16_t>         temp_size;

//                if (Simulator::Now().GetSeconds() > 2060)
//                    std::cout << "t_numretrans=" << (int)t_numretrans << " - t_Gid=" << t_Gid << " - t_numlost=" << (int)t_numlost
//                              << " - force_retrans=" << (int)force_retrans << "\n";



                // If sn not in list, add to list.
                std::vector<SequenceNumber32> retrans_sn;

                bool t_inlist = false;
                if (!force_retrans) {
                    for (uint8_t j=0; j<t_numretrans; j++){
                        SequenceNumber32 SN_UnseenOrDepen = NcSocketInfo.SeenUnseen->GetSN_UnseenAndDependent(j);
                        if (sn == SN_UnseenOrDepen) {
                            t_inlist = true;
                        }
                        retrans_sn.push_back(SN_UnseenOrDepen);

//                        std::cout << "t_numretrans=" << (int)t_numretrans << " - SN_UnseenOrDepen=" << SN_UnseenOrDepen << "\n";
                    }

                    if (t_Gid==gid_of_this && !t_inlist) {
                        bool tt_addded = false;
                        for (uint8_t j=0; j<t_numretrans; j++){
                            if (sn < retrans_sn[j]) {
                                retrans_sn.insert(retrans_sn.begin()+j,sn);
                                tt_addded = true;
                                break;
                            }
                        }
                        if (!tt_addded) {
                            retrans_sn.push_back(sn);
                        }

                        t_numretrans ++;

                        //                        if (!(sn<retrans_sn[0])) {
                        //                            retrans_sn.push_back(sn);
                        //                            t_numretrans ++;
                        //                        }
                    }
                }


                for (uint8_t j=0; j<t_numretrans; j++){
                    SequenceNumber32  t_retransmit_SN = (SequenceNumber32)0;
                    if (force_retrans)
                        t_retransmit_SN = sn;
                    else
                        t_retransmit_SN = retrans_sn[j];


                    //if (t_retransmit_SN.GetValue() > 99874489-10000)
                    //    std::cout<<"1 - t_retransmit_SN=" << t_retransmit_SN  << "  -  Base=" << NcSocketInfo.Buffer->GetBase()  << "\n";

                    Ptr<Packet>   t_retransmit_packet = NcSocketInfo.Buffer->Read(t_retransmit_SN,false);
//                    std::cout<<"2\n";
                    uint16_t t_retransmit_packet_size = t_retransmit_packet->GetSize();
//                    std::cout<<"3\n";


                    /*if (t_retransmit_SN == sn)
                        IsRetransmitted = true;

                    if (DependRetrans == false) {
                        if (t_Gid == gid_of_this) {
                            if (j == t_numretrans-1){
                                if (IsRetransmitted == false) {
                                    t_numretrans ++;
                                    NcSocketInfo.CombiBuffer->DecreaseNumLost(SocketID,t_Gid, 1);
                                }
                            }
                        }
                    }*/

                    // Store all retransmitted packet and its SN.
                    //      Because: with the same socket (In/Out) have the same Socket ID.
                    //      We can use outgoing socket.
                    temp_sn    .push_back (t_retransmit_SN);
                    temp_size  .push_back (t_retransmit_packet_size);

                    uint16_t t_pid = NcSocketInfo.Buffer->GetPid_countup(true);
                    uint16_t t_gid = NcSocketInfo.SeenUnseen->Get_GidofSN(t_retransmit_SN);

                    // Create NC Header;
                    NcHeader         m_NC_Header;


                    //                    std::cout << "    j=" << (int)j << "  -  t_numretrans" << (int)t_numretrans << "  -  t=" << Simulator::Now().GetSeconds() << std::endl;

                    if (j==t_numretrans-1){
                        if (i==num_retrans_gid-1) {
                            m_NC_Header = CreateHeader.CreateNcHeader1(sport,dport,t_pid,base,(uint8_t)2,t_retransmit_SN.GetValue(),
                                                                       t_retransmit_packet_size,NumDelayedAck
                                                                       /*NcSocketInfo.Buffer->GetWinSize(false)*/);
                        }
                        else {
                            m_NC_Header = CreateHeader.CreateNcHeader1(sport,dport,t_pid,base,(uint8_t)1,t_retransmit_SN.GetValue(),
                                                                       t_retransmit_packet_size,NumDelayedAck
                                                                       /*NcSocketInfo.Buffer->GetWinSize(false)*/);
                        }

                        NcSocketInfo.CombiBuffer->Add_BufferCombi (t_gid, t_pid, m_NC_Header);
                        NcSocketInfo.CombiBuffer->ClearRetransList();

                        //                            if (Simulator::Now().GetSeconds() > 1997)
//                        std::cout << "RETRASMIT 2 (UNCODED)  -  " << saddr << "  -  " << t_retransmit_SN << "  -  t=" << Simulator::Now().GetSeconds() << std::endl;

                        NCSend(t_retransmit_packet,m_NC_Header,saddr,daddr,route);


                        if (!m_EncodeRetransmission  &&  i==num_retrans_gid-1)
                            return true;


                        if (m_EncodeRetransmission){
                            // COMBINE ALL RETRANSMITTED PACKET.
                            if (DoEncodeRetrans(i, t_gid, t_numretrans, num_retrans_gid,
                                                temp_sn, temp_size, base, sport, dport, saddr, daddr, route, NumDelayedAck))
                                return true;
                        }
                    }
                    else {
                        m_NC_Header = CreateHeader.CreateNcHeader1(sport,dport,t_pid,base,(uint8_t)1,t_retransmit_SN.GetValue(),
                                                                   t_retransmit_packet_size,NumDelayedAck
                                                                   /*NcSocketInfo.Buffer->GetWinSize(false)*/);
                        NcSocketInfo.CombiBuffer->Add_BufferCombi (t_gid, t_pid, m_NC_Header);

                        //                            if (Simulator::Now().GetSeconds() > 1997)
//                        std::cout << "RETRASMIT 1 (UNCODED)  -  " << saddr << "  -  " << t_retransmit_SN << std::endl;

                        NCSend(t_retransmit_packet,m_NC_Header,saddr,daddr,route);
                    }
                }
            }
        } // if (num_retrans_gid > 0)
    } // if (NcSocketInfo.CombiBuffer->CheckSeen(SocketID, outgoing.GetSequenceNumber())==true && Dependent_retransmit == false){...}  ----  else {
    //    } // if (m_ForwardRetransmission==true)

    return false;
}


void NcL4Protocol::SendBackToTcp (Ptr<IpL4Protocol> protocol, Ptr<Packet> p, Ipv4Header h, Ptr<Ipv4Interface> i) {
    protocol->Receive (p, h, i);
}


// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


void NcL4Protocol::SetIpForwardCallback (IpForwardCallback cb)
{
    m_IpForward = cb;
}

void NcL4Protocol::SetIpMulticastForwardCallback (IpMulticastForwardCallback cb)
{
    m_IpMulticastForward = cb;
}

void NcL4Protocol::SetLocalDeliverCallback (LocalDeliverCallback cb)
{
    m_LocalDeliver = cb;
}

void NcL4Protocol::SetSendCallback (SendCallback cb){
    m_Send = cb;
}

void NcL4Protocol::SetGetProtocolCallback (GetProtocolCallback cb){
    m_GetProtocol = cb;
}

void NcL4Protocol::SetGetInterfaceCallback (GetInterfaceCallback cb){
    m_GetInterface = cb;
}

void NcL4Protocol::SetGetInterfaceForAddressCallback (GetInterfaceForAddressCallback cb){
    m_GetInterfaceForAddress = cb;
}

void NcL4Protocol::SetBuildHeaderCallback(BuildHeaderCallback cb){
    m_BuildHeader = cb;
}

void NcL4Protocol::SetGetNetDeviceCallback(GetNetDeviceCallback cb){
    m_GetNetDevice = cb;
}

void NcL4Protocol::SetGetNodeCallback (GetNodeCallback cb){
    m_GetNode = cb;
}

void NcL4Protocol::SetSendInformationToHandleCallback(SendInformationToHandleCallback cb){
    m_SendInformationToHandle = cb;
}

void NcL4Protocol::SetGetInInterfaceCallback(GetInInterfaceCallback cb){
    m_GetInInterface = cb;
}

//=========================================================================
//=========================================================================
//=========================================================================

Ptr<Ipv4Route> NcL4Protocol::CreateRoute  (Ptr<Packet> p, Ipv4Header ipheader, Ipv4Address saddr){
    Socket::SocketErrno errno_;
    Ptr<NetDevice> t_oif = m_GetNetDevice(m_GetInterfaceForAddress(saddr));
    Ptr<Ipv4>      ipv4  = m_GetNode()->GetObject<Ipv4>();
    Ptr<Ipv4Route> route = ipv4->GetRoutingProtocol()->RouteOutput (p, ipheader, t_oif, errno_);
    return route;
}


void NcL4Protocol::LogPacket(std::string filename, Ptr<Packet> packet, SequenceNumber32 sn, uint32_t psize){
    std::ofstream _Ofstream;
    _Ofstream.open (filename, std::ofstream::out | std::ofstream::app);

    uint8_t buffer_packet_t[psize];
    uint8_t *buffer_packet_t_pointer;
    buffer_packet_t_pointer = &buffer_packet_t[0];

    packet->CopyData(buffer_packet_t_pointer,psize);

    _Ofstream << sn << ",";

    for (uint16_t i=0; i<psize; i++){
        _Ofstream << (int)buffer_packet_t[i];
        if (i<psize-1){
            _Ofstream << ",";
        }
    }

    _Ofstream << '\n';
    _Ofstream.close();
}

void NcL4Protocol::LogOverwrite (std::string filename, std::string content){
    std::ofstream _Ofstream;
    _Ofstream.open (filename, std::ofstream::out);
    _Ofstream << content;
    _Ofstream.close();
    LogContent.str("");
}

void NcL4Protocol::LogNonOverwrite(std::string filename,std::string content){
    std::ofstream _Ofstream;
    _Ofstream.open (filename, std::ofstream::out | std::ofstream::app);
    _Ofstream << content;
    //_Ofstream.close();
    LogContent.str("");
}


Ptr<Packet> NcL4Protocol::Add_Null_to_Packet(Ptr<Packet> packet, uint16_t Protocol_ID, float R, uint8_t recovery_capacity){
    uint16_t numAddByte = 0;

    if (Protocol_ID == ProNumNC) {
        NcHeader NC_Header;
        packet->PeekHeader(NC_Header);

        uint8_t N = NC_Header.GetN();

        if (R > 1) {
            if (recovery_capacity+1 >= N) {
                numAddByte = ((recovery_capacity+1) - N) * 5;
            }
            else { // *In case of Re-encoding, Header size maybe larger than normal => need to ignore
                return packet;
            }
        }

        packet->AddPaddingAtEnd(numAddByte);
    }
    else { // Protocol_ID == 6
        if (R==(float)1) {
            numAddByte = 0;
        }
        else {
            numAddByte = (recovery_capacity)*5 - 1;
            packet->AddPaddingAtEnd(numAddByte);
        }
    }

    return packet;
}

Ptr<Packet> NcL4Protocol::Rem_Null_to_Packet(Ptr<Packet> packet, uint16_t Protocol_ID, float R, uint8_t recovery_capacity){
    uint16_t numAddByte = 0;

    if (Protocol_ID == ProNumNC) {
        NcHeader NC_Header;
        packet->PeekHeader(NC_Header);

        uint8_t N = NC_Header.GetN();

        if (R > (float)1) {
            if (recovery_capacity+1 >= N) {
                numAddByte = ((recovery_capacity+1) - N) * 5;
            }
            else { // *In case of Re-encoding, Header size maybe larger than normal => need to ignore
                return packet;
            }
        }

        if (numAddByte > 0) {
            packet->RemoveAtEnd(numAddByte);
        }
    }
    else { // Protocol_ID == 6
        if (R > (float)1) {
            numAddByte = (recovery_capacity)*5 - 1;
        }
        else {
            numAddByte = 0;
            return packet;
        }

        packet->RemoveAtEnd(numAddByte);
    }

    return packet;
}

Ptr<Packet> NcL4Protocol::Add_Null_to_ACK (Ptr<Packet> packet){
    if (packet->GetSize() == 0) {
        packet->AddPaddingAtEnd(NcAckHeaderSize);
    }

    return packet;
}

Ptr<Packet> NcL4Protocol::Rem_Null_to_ACK (Ptr<Packet> packet){
    if (packet->GetSize() == NcAckHeaderSize + TcpHeaderNormalSize){
        packet->RemoveAtEnd(packet->GetSize());
    }

    return packet;
}

//=========================================================================
//=========================================================================
//=========================================================================

void NcL4Protocol::Update_Proxytable (SequenceNumber32 ACKnumAtProxy, SequenceNumber32 ACKnumAtSource, Ipv4Address sip, Ipv4Address dip){
    ACKatProxy_Proxytable.push_back(ACKnumAtProxy);
    ACKatSource_Proxytable.push_back(ACKnumAtSource);
    sip_Proxytable.push_back(sip);
    dip_Proxytable.push_back(dip);
}


uint8_t NcL4Protocol::GetNcPara_K (Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, bool InOut){
    NcSocketInfo = NC_SocketID.GetSocketID(saddr, daddr, sport, dport);
    NcParaEstimationReturn = NcSocketInfo.LossEst->GetNcParameter();
    return NcParaEstimationReturn.k;
}

long double NcL4Protocol::GetNcPara_R (Ipv4Address saddr, Ipv4Address daddr, uint16_t sport, uint16_t dport, bool InOut){
    NcSocketInfo = NC_SocketID.GetSocketID(saddr, daddr, sport, dport);
    NcParaEstimationReturn = NcSocketInfo.LossEst->GetNcParameter();
    if (NcParaEstimationReturn.R > 0)
        return NcParaEstimationReturn.R;

    return 1;
}

} //namespace ns3
