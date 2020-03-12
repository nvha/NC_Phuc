#ifndef NC_DECODE
#define NC_DECODE

#include "nc-header.h"
#include "nc-HeaderInformation.h"
#include "nc-buffer.h"
#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include <string>

#include <vector>

#include "nc-SeenUnseen.h"

namespace ns3 {

class NcDecode
{
public:
    NcDecode();

    struct SeenInfo {
        SequenceNumber32    SmallestUnseenSN;
        bool                Redundancy;
    };

    uint8_t  Decode (Ptr<Packet> packet, NcHeader header, NcBuffer *Nc_Buffer);

    NcHeaderInfo NcHeader2NcHeaderInfo(NcHeader header);
    NcHeaderInfo CreateNcHeaderInfo(uint8_t N, SequenceNumber32 *SN, uint16_t * size, uint8_t* Alpha);

    //uint32_t NC_Buffer_Size (bool source_side);
    //uint32_t NC_BCB_Size (bool source_side);
    //uint32_t NC_BCB_Size_New (bool source_side);



private:
    NcSeenUnseen *NC_SeenUnseen;

    void Add_to_Table (std::string name, SequenceNumber32 sn);
    uint8_t Can_Decoded (NcHeaderInfo HeaderInfo, Ptr<Packet> packet, NcBuffer *Nc_Buffer);

    //
    struct DependenceIsSeenAgain {
        bool SeenAgain;
        SequenceNumber32 SN;
    };
    //
    DependenceIsSeenAgain AddUnseenToTable(NcHeader header);


private:
    // Table: Combination Store table: Store combination packet which is not decode
    std::vector <NcHeaderInfo>          NcHeaderInfo_CombinationStoreTable;
    std::vector <std::vector<uint8_t>>  CombiData_CombinationStoreTable;
    //
    void Store_Combination (Ptr<Packet> packet, NcHeaderInfo HeaderInfo);
    void Erase_Combination (uint64_t id);
    Ptr<Packet> Read_Combination (uint64_t id);
    //======


    // Table: Decoded Packet
    std::vector <SequenceNumber32> Decoded_DecodedTable;
    //
    void Update_Decoded_Table   (SequenceNumber32 decoded, bool add_remove, SequenceNumber32 base);
    bool Check_Decoded          (SequenceNumber32 sn);
    //======


    // Last Base table
    SequenceNumber32  LastDecoded_LastDecodedTable;
    //
    void              UpdateLastDecoded(SequenceNumber32 sn);
    SequenceNumber32  GetLastDecoded ();
    //======


//    // Table: Seen Table
//    std::vector <SequenceNumber32> Seen_SeenTable;
//    //
//    void Update_Seen_Table  (SequenceNumber32 sn, bool add_remove);
//    bool Check_Seen         (SequenceNumber32 sn);
//    //======


//    // Table: UnSeen Table
//    std::vector <SequenceNumber32> UnSeen_UnSeenTable;
//    //
//    void Update_UnSeen_Table    (SequenceNumber32 sn, bool add_remove);
//    bool Check_UnSeen           (SequenceNumber32 sn);
//    SequenceNumber32 Get_Smallest_UnSeen ();
//    //======


//    //Table: Next SN Table
//    std::vector <SequenceNumber32> NextSN_NextSnTable;
//    //
//    void Update_NextSN_Table (SequenceNumber32 sn, bool add_remove);
//    bool Check_NextSN (SequenceNumber32 sn);
//    SequenceNumber32 Get_Smallest_NextSN ();
//    //======

public:
    SequenceNumber32 Get_UnSeen ();
//    SequenceNumber32 Get_UnSeen_dontcaredepend();
//    SequenceNumber32 Get_UnSeen_inside ();

    bool IsRedundancyAck ();


    Ptr<Packet>         GetPacket       (uint8_t i);
    SequenceNumber32    GetSN           (uint8_t i);


    /* Table: Dependent Table
     * -----------------------------------------------
     * | Socket Info | Dependent SN | Last Unseen SN |
     * -----------------------------------------------
     * |             |              |                |
     * -----------------------------------------------
     */
private:
    std::vector <SequenceNumber32> SN_DependentTable;
    std::vector <SequenceNumber32> LastUnseen_DependentTable;

    SequenceNumber32 Last_Dependence;
    SequenceNumber32 Last_LastUnseen;

public:
    struct Get_Dependent_return {
        SequenceNumber32 dependent_sn;
        SequenceNumber32 lastunseen_sn;
        bool             have;
    };
    //
    void                    Set_Dependent   (SequenceNumber32 sn, SequenceNumber32 last_unseen);
    Get_Dependent_return    Get_Dependent   ();
    bool                    Check_Dependent (SequenceNumber32 sn);
    void                    Del_Dependent   (SequenceNumber32 sn);


private:
    /* Table: Decode information Table
     * -----------------------------------------------
     * | Socket Info | Dependent SN | Last Unseen SN |
     * -----------------------------------------------
     * |             |              |                |
     * -----------------------------------------------
     */

private:
    bool Dependend;
    uint8_t num_Dependend;
    std::vector<SequenceNumber32> Dependend_SN;

    // Use for "Can_Decoded" function ===============================================
    std::vector<uint64_t>           store_combi_id;
    std::vector<uint64_t>           store_combi_id2;
    std::vector<NcHeaderInfo>       store_NcHeaderInfo;
    std::vector<SequenceNumber32>   store_Variable;
    std::vector<SequenceNumber32>   store_Decoded;
    std::vector<uint16_t>           store__VarSize;

    NcHeader Check_return_NcHeader;
    uint64_t Check_return_combi_id;

    void Check_SN_in_Combination (SequenceNumber32 sn);
    bool Check_store_Variable    (SequenceNumber32 sn);

    uint8_t **store_Alpha;
    Ptr<Packet> *store_combi;
    uint8_t *Decode_Status;  // 0: dependent + need to retransmit;  1: already decoded;  >1: can not decoded;

    std::vector<SequenceNumber32>   t_store_Decoded;
    std::vector<NcHeaderInfo>       t_store_NcHeaderInfo;
    std::vector<uint64_t>           t_store_combi_id;


    SequenceNumber32 *store_SN;
    uint16_t         *store_Sz;
    std::vector <uint8_t> store_AlreadyDecoded_id;
    bool CreateAlpha(SequenceNumber32 sn, uint8_t id, Ptr<Packet> packet, NcBuffer *NcBuffer);
};

}


#endif
