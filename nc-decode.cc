#include "nc-decode.h"
#include "nc-buffer.h"
#include "nc-packet-calculator.h"
#include "nc-Create-Header.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include <fstream>

namespace ns3 {

NcDecode::NcDecode(){
    //    store_combi_additional_id = 0;
    LastDecoded_LastDecodedTable = (SequenceNumber32)0;
    NC_SeenUnseen = new NcSeenUnseen;
}


NcHeaderInfo NcDecode::NcHeader2NcHeaderInfo(NcHeader header){
    NcHeaderInfo returnvalue;

    uint8_t m_N = header.GetN();
    returnvalue.SetN(m_N);

    for (uint8_t i=0; i<m_N; i++){
        SequenceNumber32 t_sn = (SequenceNumber32)header.GetStart1();
        if (i>0)
            t_sn = t_sn + header.GetStartN(i);

        returnvalue.SetSN  (t_sn);
        returnvalue.SetSize(header.GetEnd(i));
        returnvalue.SetAlpha(header.GetAlpha(i));
    }

    return returnvalue;
}

NcHeaderInfo NcDecode::CreateNcHeaderInfo(uint8_t N, SequenceNumber32 * SN, uint16_t * size, uint8_t* Alpha){
    NcHeaderInfo returnvalue;

    returnvalue.SetN(N);

    for (uint8_t i=0; i<N; i++) {
        returnvalue.SetSN   (SN[i]);
        returnvalue.SetSize (size[i]);
        returnvalue.SetAlpha(Alpha[i]);
    }

    return returnvalue;
}

//return true if Dependence in New Header
NcDecode::DependenceIsSeenAgain NcDecode::AddUnseenToTable(NcHeader header){
    NcDecode::DependenceIsSeenAgain returnValue;
    returnValue.SeenAgain = false;

    uint8_t  t_N = header.GetN();
    uint32_t SN1 = header.GetStart1();
    uint32_t SNi = SN1;
    for (uint8_t i=0; i<=t_N; i++){
        if (i==t_N) {
            SNi = SNi + header.GetEnd(i-1); // Next packet
        }
        else if (i>0) {
            SNi = SN1 + header.GetStartN(i);
        }

//        if (SNi == (uint32_t)29683681)
//            std::cout << header.GetPacketID() << " - " << Simulator::Now().GetSeconds() << "\n";

        NC_SeenUnseen->AddUnSeenWithOrder(0,(SequenceNumber32)SNi);

        if (Get_Dependent().have) {
            if (SNi == Get_Dependent().dependent_sn.GetValue()) {
                returnValue.SeenAgain = true;
                returnValue.SN        = (SequenceNumber32)SNi;
            }
        }
    }

    return returnValue;
}

bool IsRedundancy;

//bool mm_LOG;

// MAIN DECODE FUNCTION
uint8_t NcDecode::Decode(Ptr<Packet> packet, NcHeader header, NcBuffer *Nc_Buffer){
    std::cout << "NcDecode::Decode  -  " << this << "\n";

    /* Get unseen for dependent case --------------------------------------
     * We need to write code in here, because,
     * When Unseen SN is greater than the lagrest SN in Window coding,
     * Source will clear buffer including dependend packet.
     * Led to, can not retransmit dependend packet.
     */
//    SequenceNumber32 unseen_SN = Get_UnSeen();

    //erase data
    store_combi_id.clear();
    store_combi_id2.clear();
    store_NcHeaderInfo.clear();
    store_Variable.clear();
    store__VarSize.clear();
    store_Decoded.clear();

    t_store_Decoded.clear();
    t_store_NcHeaderInfo.clear();
    t_store_combi_id.clear();
    store_AlreadyDecoded_id.clear();

    num_Dependend = 0;
    Dependend_SN.clear();

    Dependend = false;

//    //---------------------------------------


//    uint8_t h_N = header.GetN();

//    SequenceNumber32 seen    = (SequenceNumber32)0;
//    SequenceNumber32 unseen  = (SequenceNumber32)0;
//    SequenceNumber32 nextsn  = (SequenceNumber32)0;

//    // Erase Decoded_Table using base field========================================================================================
    Update_Decoded_Table((SequenceNumber32)0, false, header.GetBase());
//    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//    // update next seen and unseen and next========================================================================================

//    // In dependence case, if sink receives combiantion carried depdendence packet.
//    // This packet seem seen again (when dependence happened, this packet need to unseen to retransmit again (In system don't set unseen, just impact)
//    // If check to SNi=depedendent SN and no new seen until this time => dendendence SN seem seen again.

//    bool t_new_seen   = false;
//    bool t_new_unseen = false;
//    bool t_new_next   = false;

//    bool DependendSeenAgain = false;

//    for (uint8_t i=0; i<h_N; i++){
//        SequenceNumber32 t_sn = (SequenceNumber32)header.GetStart1();
//        if (i>0) t_sn = t_sn + (SequenceNumber32)header.GetStartN(i);


//        // Check including depedended?
//        bool dependend_included = false;
//        if (Check_Dependent(t_sn)) {
//            dependend_included = true;
//        }


//        if (t_new_seen==true && t_new_unseen==false){
//            if (Check_Seen(t_sn)==false  &&  Check_Decoded(t_sn)==false){
//                unseen = t_sn;
//                t_new_unseen = true;
//                nextsn = unseen + (SequenceNumber32)header.GetEnd(i);
//            }
//        }

//        if (t_new_seen==false) {
//            if ((Check_Seen(t_sn)==false  &&  Check_Decoded(t_sn)==false) || dependend_included==true) {
//                seen = t_sn;
//                t_new_seen = true;
//                nextsn = seen + (SequenceNumber32)header.GetEnd(i);

//                if (dependend_included) {
//                    // If no packet is seen before check this SN, this dependent packet (seem) seen again.
//                    // Else, dependent packet is not seen => keep staying in dependent status.
//                    if (Check_Seen(t_sn)) // It has 2 cases: seen or unseen. Seen => dont set seen again; Unseen => process as normal case => set new seen
//                        DependendSeenAgain = true;

//                    /* Ex:
//                     * 1	2
//                     * 1	2	3
//                     * 1	2	3	4
//                     *      2	3	4	5
//                     *          3	4	5	6
//                     *              4	5	6	7
//                     *                  5	6	7	8
//                     *                      6
//                     * Dependent is p8
//                     *
//                     * Next, receive:
//                     *                      6	7	8	9
//                     * 8 is unseen before => need to set to seen.
//                     */


//                    // Update Dependence status:
//                    // If dependent packet is (seem) seen again => dependent packet is retransmitted => remove dependent from list.
//                    Del_Dependent(t_sn);
//                }
//            }
//        }

//        if (i == h_N-1){
//            if (t_new_seen==false && t_new_unseen==false) {
//                nextsn = t_sn + (SequenceNumber32)header.GetEnd(i);

//                //                std::cout << "nextsn=" << nextsn << std::endl;
//            }


//            if (    Check_Seen   (nextsn) == false &&
//                    Check_Decoded(nextsn) == false &&
//                    Check_NextSN (nextsn) == false &&
//                    Check_UnSeen (nextsn) == false)
//                t_new_next = true;
//        }
//    }

//    if (DependendSeenAgain) {
//        t_new_seen = false;
//    }

//    if (t_new_next)   Add_to_Table("nextsn", nextsn);
//    if (t_new_unseen) Add_to_Table("unseen", unseen);
//    if (t_new_seen)   Add_to_Table("seen"  , seen  );
//    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


    //mm_LOG = (header.GetStart1()>=35238785 && header.GetStart1()<=35259689);
    //mm_LOG = false;




    ////////////////////////////////
    SequenceNumber32 LastUnseenSN = NC_SeenUnseen->GetLastestUnseen(true).SN;

    NcDecode::DependenceIsSeenAgain IncludeDependenceInfo = AddUnseenToTable (header);
    if (IncludeDependenceInfo.SeenAgain) {
        Last_Dependence = IncludeDependenceInfo.SN;
        Last_LastUnseen = Get_Dependent().lastunseen_sn;
        //
        //
        //if (mm_LOG)
        //std::cout << "Del_Dependent  1   -   " << IncludeDependenceInfo.SN << "\n";
        Del_Dependent(IncludeDependenceInfo.SN);
        IsRedundancy = false;
    }
    else {
        IsRedundancy = NC_SeenUnseen->Update_SeenUnseen_Table(header,false,(SequenceNumber32)0,(SequenceNumber32)0,header.GetBase());
    }

    NC_SeenUnseen->RemoveSeenUnseen(header.GetBase());









    // Check and find information (Alpha...) if can be decoded; =================================================================
    uint8_t value_num;
    value_num = Can_Decoded(NcHeader2NcHeaderInfo(header), packet, Nc_Buffer);
    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    //    NS_LOG_UNCOND ("value_num = " << (int)value_num);

    // Decode process ===========================================================================================================
    if (value_num > 0   &&   value_num != 255){
        NcPacketCalculator PacketCalulator;
        PacketCalulator.Equations_resolve(store_combi, store_Alpha, value_num, Decode_Status);      // decode packet duoc return tro lai "store_combi"

        // Number of Failure and All_Zero (dependent, need to retransmit)
        uint8_t num_fail = 0;
        num_Dependend    = 0;

        for (uint8_t i=0; i<value_num; i++){
            switch (Decode_Status[i]) {
            case 0:
                num_Dependend++;
                Dependend = true;
                Dependend_SN.push_back(store_SN[i]);
                break;
            case 1:
                break;
            default:
                num_fail++;

                // Store Combination failure decode
                SequenceNumber32 *SN_t = new SequenceNumber32 [Decode_Status[i]];
                uint8_t *alpha_t       = new uint8_t [Decode_Status[i]];
                uint16_t *size_t       = new uint16_t [Decode_Status[i]];

                uint8_t dem = 0;

                for (uint8_t j=i; j<value_num; j++){
                    if (store_Alpha[i][j] > 0){
                        SN_t[dem]    = store_SN[j];
                        alpha_t[dem] = store_Alpha[i][j];
                        size_t[dem]  = store_Sz[j];
                        dem++;
                    }
                }

                // In some cases, the highest SN - smallest SN > 65535; It will be excceded the SN field in NC Header (only 2bytes)

                NcHeaderInfo NC_HeaderInfo_t = CreateNcHeaderInfo(dem, SN_t, size_t, alpha_t);
                Store_Combination (store_combi[i], NC_HeaderInfo_t);

                delete []SN_t;
                delete []alpha_t;
                delete []size_t;
            }
        }

        // Remove packets which already decoded before
        uint8_t t_counter = 0;
        for (uint8_t i=0; i<store_AlreadyDecoded_id.size(); i++){
            for (uint8_t j=store_AlreadyDecoded_id[i]-t_counter; j<value_num-1-t_counter; j++){
                store_SN[j] = store_SN[j+1];
                store_Sz[j] = store_Sz[j+1];
                store_combi[j] = store_combi[j+1];
                Decode_Status[j] = Decode_Status[j+1];
            }

            std::cout << "Remove packets which already decoded before  -  i=" << (int)i << "\n";

            t_counter++;
        }
        value_num = value_num - store_AlreadyDecoded_id.size();

        std::cout << "value_num=" << (int)value_num << "\n";


        // Remove packet which is failure or dependet
        t_counter = 0;
        for (uint8_t i=0; i<value_num; i++){
            if (Decode_Status[t_counter] != 1) {
                for (uint8_t j=t_counter; j<value_num-1; j++) {
                    store_SN[j] = store_SN[j+1];
                    store_Sz[j] = store_Sz[j+1];
                    store_combi[j] = store_combi[j+1];
                    Decode_Status[j] = Decode_Status[j+1];
                }

                std::cout << "Remove packet which is failure or dependet  -  i=" << (int)i << "\n";
            }
            else
                t_counter++;
        }

        value_num = value_num - num_fail - num_Dependend;

        std::cout << "value_num=" << (int)value_num << "\n";

        // Add to Decoded table;
        for (uint8_t i=0; i<value_num; i++) {
            Add_to_Table("decoded", store_SN[i]);

            std::cout << "store_SN=" << store_SN[i] << "  -  Size=" << store_combi[i]->GetSize() << "\n";
        }


        // Remove Combination already use for decoding
        while (store_combi_id2.size() != 0){
            uint8_t  index = 0;
            uint64_t t_max = 0;

            for (uint8_t i=0; i<store_combi_id2.size(); i++){
                if (store_combi_id2[i] > t_max){
                    t_max = store_combi_id2[i];
                    index = i;
                }
            }

            store_combi_id2.erase(store_combi_id2.begin() + index);
            Erase_Combination(t_max);
        }
    }


    // Special Case: One packet decoded only === No more....
    if (value_num==255 && header.GetN()==1){
        store_combi = new Ptr<Packet>[1];
        store_SN = new SequenceNumber32[1];
        store_Sz = new uint16_t[1];

        store_combi[0] = packet;
        store_SN[0] = (SequenceNumber32)header.GetStart1();
        store_Sz[0] = header.GetEnd(0);

        Add_to_Table("decoded", store_SN[0]);

        return 1;
    }


    // Cannot decode yet, Store this combination packet
    if (value_num == 255){
        Store_Combination(packet, NcHeader2NcHeaderInfo(header));

        return 255;
    }

    /* In here. we have special case:
     * EX:   p1  p1p2  p1p2p3  p2p3p4  p3p4  p4
     * lost: x     x      x              x    x
     * TCP will retransmit: p1, p2 (because of timeout, even p2 is seen), p3, p4
     *
     * When receive p3 => all packet is decoded => when p4 come, it's ignored
     * We need check; whether p5 is the new next?
     */
//    if (/*t_new_next == false  &&  */value_num != 0){
//        nextsn = store_SN[value_num-1] + (SequenceNumber32)(store_combi[value_num-1]->GetSize());
//        if (Check_Decoded(nextsn)==false  &&  Check_Seen(nextsn)==false  &&  Check_UnSeen(nextsn)==false){
//            Add_to_Table("nextsn", nextsn);
//        }
//    }


    // Check decoded packets include dependent packet or not
    for (uint16_t i=0; i<value_num; i++){
        if (Check_Dependent(store_SN[i])) {
            //if (mm_LOG)
            //    std::cout << "Del_Dependent  1   -   ";
            Del_Dependent(store_SN[i]);
        }
    }


    // Add new dependent if have
    if (Dependend) {
        for (uint16_t i=0; i<num_Dependend; i++){
            if (Check_Dependent(Dependend_SN[i]) == false) {
                if (Dependend_SN[i]!=Last_Dependence) {
                    //Set_Dependent(Dependend_SN[i], unseen_SN);
                    Set_Dependent(Dependend_SN[i], LastUnseenSN);
                }
                else {
                    Set_Dependent(Dependend_SN[i], Last_LastUnseen);
                }
            }
        }
    }


    for (uint8_t i=0; i<value_num; i++) {
        std::cout << "[2] store_SN=" << store_SN[i] << "  -  Size=" << store_combi[i]->GetSize() << "   -   " << this << "\n";
    }

    return value_num;

    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

void NcDecode::UpdateLastDecoded (SequenceNumber32 sn){
    LastDecoded_LastDecodedTable = sn;
}

SequenceNumber32 NcDecode::GetLastDecoded(){
    return LastDecoded_LastDecodedTable;
}

// DECODED TABLE:
void NcDecode::Update_Decoded_Table (SequenceNumber32 decoded, bool add_remove, SequenceNumber32 base){
    if (add_remove == true){
        Decoded_DecodedTable.push_back(decoded);
    }
    else{
        SequenceNumber32 t_lastdecoded = GetLastDecoded();
        uint32_t t_size  = Decoded_DecodedTable.size();
        uint32_t t_index = 0;
        for (uint32_t i=0; i<t_size; i++){
            if (Decoded_DecodedTable[t_index] < base){
                if (t_lastdecoded < Decoded_DecodedTable[t_index]) {
                    t_lastdecoded = Decoded_DecodedTable[t_index];
                }
                //
                Decoded_DecodedTable.erase(Decoded_DecodedTable.begin() + t_index);
            }
            else
                t_index++;
        }

        UpdateLastDecoded (t_lastdecoded);
    }
}

bool NcDecode::Check_Decoded (SequenceNumber32 sn){
    if (sn <= GetLastDecoded()) {
        return true;
    }

    for (uint32_t i=0; i<Decoded_DecodedTable.size(); i++){
        if (Decoded_DecodedTable[i].GetValue() == sn.GetValue()){
            return true;
        }
    }

    return false;
}

void NcDecode::Add_to_Table (std::string name, SequenceNumber32 sn){
    if (name == "decoded"){
        if (Check_Decoded(sn)){
            return;
        }
        else {
            Update_Decoded_Table(sn,true,(SequenceNumber32) 0);
        }
    }
}

uint8_t NcDecode::Can_Decoded(NcHeaderInfo HeaderInfo, Ptr<Packet> packet, NcBuffer *Nc_Buffer){
    // Check Can-Be_Decoded Condition: All SN in this combiantion must be seen or decoded;
    bool All_Decoded             = true;

    /* In here. we have special case:
     * EX:   p1  p1p2  p1p2p3  p2p3p4  p3p4  p4
     * lost: x     x                     x
     * When receive p4 => return value will be 255
     * We need check; if return value is 0 but N=1 => mean one packet decode;
     */

    for (uint8_t i=0; i<HeaderInfo.GetN(); i++){
        SequenceNumber32 t_sn = HeaderInfo.GetSN(i);

        // Check SN
        if (Check_Decoded(t_sn) == false){
            All_Decoded = false;
        }
    }

    if (All_Decoded) return 0; // redundancy packet;
    // ====================================================================================



    store_combi_id    .push_back((uint64_t)(0-1));
    store_NcHeaderInfo.push_back(HeaderInfo);

    for (uint8_t i=0; i<store_combi_id.size(); i++){
        NcHeaderInfo t_NcHeaderInfo = store_NcHeaderInfo[i];

        for (uint8_t j=0; j<t_NcHeaderInfo.GetN(); j++){
            SequenceNumber32 t_sn = t_NcHeaderInfo.GetSN(j);

            if (Check_store_Variable(t_sn) == false){
                store_Variable.push_back (t_sn);                // increase variable
                store__VarSize.push_back (t_NcHeaderInfo.GetSize(j));

                if (Nc_Buffer->CheckAlreadyWrite(t_sn, true)){
                    store_Decoded.push_back(t_sn);
                }

                Check_SN_in_Combination(t_sn);
            }
        }
    }


    /* In here. we have special case:
         * EX:   p1  p1p2  p1p2p3  p2p3p4  p3p4  p4
         * lost: x     x             x
         * TCP will retransmit: p1 (but lost in second time), p3 (because of timeout, even p3 is seen)
         *
         * When receive p3 => size of store_Variable = 4; size of store_NcHeaderInfo = 3; size of store_Decoded = 1;
         * Decode state is true => But can not decode.
         * ==> Fixed by check again in CreateAlpha fuction, if can create at row i => can decode, else is can not
         */

    uint8_t size = store_Variable.size();

    // Reorder SN;
    for (uint8_t i=0; i<size; i++){
        SequenceNumber32 t_min_sn = (SequenceNumber32)(0-1);
        uint8_t index = 0;

        for (uint8_t j=i; j<size; j++){
            if(store_Variable[j].GetValue() < t_min_sn.GetValue()){
                t_min_sn = store_Variable[j];
                index = j;
            }
        }

        store_Variable[index] = store_Variable[i];
        store__VarSize[index] = store__VarSize[i];
        store_Variable[i]     = t_min_sn;
    }



//    int numDec = store_Decoded.size();
//    int numCom = store_NcHeaderInfo.size();
//    int numVar = store_Variable.size();

//    if (mm_LOG) {
//        std::cout << "numDec=" << numDec << "  -  numCom=" << numCom << "  -  numVar=" << numVar << "\n   Var: ";

//        for (int i=0; i<numVar; i++) {
//            std::cout << store_Variable[i] << " ; ";
//        }
//        std::cout << "\n   Dec: ";

//        for (int i=0; i<numDec; i++) {
//            std::cout << store_Decoded[i] << " ; ";
//        }
//        std::cout << "\n   Com:\n   ";
//        for (int i=0; i<numCom; i++) {
//            for (int ii=0; ii<store_NcHeaderInfo[i].GetN(); ii++) {
//                std::cout << store_NcHeaderInfo[i].GetSN(ii) << " ; ";
//            }
//            std::cout << "\n   ";
//        }
//        std::cout << "\n";
//    }




    bool t_CreateAlpha = false;

    while (t_CreateAlpha == false){
        uint8_t t_size = store_Variable.size();


        t_store_NcHeaderInfo.clear();
        t_store_combi_id.clear();
        t_store_Decoded.clear();

        for (uint8_t i=0; i<store_NcHeaderInfo.size(); i++){
            t_store_NcHeaderInfo.push_back (store_NcHeaderInfo[i]);
            t_store_combi_id.push_back (store_combi_id[i]);
        }

        for (uint8_t i=0; i<store_Decoded.size(); i++){
            t_store_Decoded.push_back (store_Decoded[i]);
        }


        // Create Alpha;
        store_Alpha = new uint8_t*[size];
        // And
        // Create Packet Pointer;
        store_combi = new Ptr<Packet>[size];
        // And
        // Create Decoded Status;
        Decode_Status = new uint8_t [size];


        t_CreateAlpha   = true;
        uint8_t t_index = 0;


        for (t_index=0; t_index<t_size; t_index++){
            if (CreateAlpha(store_Variable[t_index], t_index, packet, Nc_Buffer) == false) {
                t_CreateAlpha = false;
                break;
            }
        }


        if (t_CreateAlpha == false) {
            for (uint8_t j=0; j<=t_index; j++){
                delete []store_Alpha[j];
            }
            delete []store_Alpha;
            delete []store_combi;
            delete []Decode_Status;

            store_AlreadyDecoded_id.clear();
            store_combi_id2.clear();


            for (int j=0; j<(int)store_NcHeaderInfo.size(); j++){
                for (uint8_t k=0; k<store_NcHeaderInfo[j].GetN(); k++){
                    SequenceNumber32 t_sn = store_NcHeaderInfo[j].GetSN(k);

                    if (t_sn == store_Variable[t_index]){
                        store_NcHeaderInfo.erase(store_NcHeaderInfo.begin() + j);
                        store_combi_id.erase(store_combi_id.begin() + j);
                        j--;
                        break;
                    }
                }
            }

            store_Variable.erase(store_Variable.begin() + t_index);
            store__VarSize.erase(store__VarSize.begin() + t_index);
        }
    }

    size = store_Variable.size();


    if (size == store_Decoded.size())
        return 255;


    // Create SN;
    store_SN = new SequenceNumber32[size];
    store_Sz = new uint16_t        [size];
    for (uint8_t i=0; i<size; i++){
        store_SN[i] = store_Variable[i];
        store_Sz[i] = store__VarSize[i];
    }

    return size;
}

// Kiem tra SN co trong Combination nao, tru cac combination da kiem tra roi (bo qua)
void NcDecode::Check_SN_in_Combination (SequenceNumber32 sn){
    for (uint64_t i=0; i<NcHeaderInfo_CombinationStoreTable.size(); i++){
        bool t_continue = true;

        //tru cac combination da kiem tra roi (bo qua)
        for (uint8_t k=0; k<store_combi_id.size(); k++){
            if (i == store_combi_id[k]){
                t_continue = false;
                break;
            }
        }

        //Kiem tra SN co trong Combination nao
        if (t_continue) {
            NcHeaderInfo t_nc_header_info = NcHeaderInfo_CombinationStoreTable[i];

            // Check this Combination is already decoded yet?

            /* Special Case: Original: p1  p1p2  p1p2p3  p2p3p4  p3p4p5  p4p5p6  p5p6p7  p6p7p8  p7p8p9  p8p9p10  p9p10  p10
                 *               Lost:           x                      x               x
                 *               Retransmit cause by 3-ACKs:   p5
                 *               Continue loss:                x
                 *               Retransmit cause by Timeout:  p6 p7 p8
                 *               Receive Order: p1  p1p2p3  p2p3p4  p4p5p6  p6p7p8  p7p8p9  p8p9p10  p9p10  p10  p6  p7  p8  p5
                 *
                 * When receive p5, decode only p1, p2, p3, p4, p5 because when read combination packet p6p7p8, all packet are decoded => erase this => don't call combination
                 * p7p8p9, p8p9p10, p9p10 => p9 can not decode; => Must improve decoded. Khi tao Alpha, dua vao Alpha bo bot hang cot de giai ma truoc cac goi tin co the giai ma truoc
                 */

            bool t_already = true;
            for (uint8_t j=0; j<t_nc_header_info.GetN(); j++){
                SequenceNumber32 t_sn = t_nc_header_info.GetSN(j);

                if (Check_Decoded(t_sn) == false){
                    t_already = false;
                }
            }

            if (t_already == true){
                Erase_Combination(i);

                for (uint8_t k=0; k<store_combi_id.size(); k++){
                    if (store_combi_id[k] != (uint64_t)(0-1)){
                        if (i < store_combi_id[k]){
                            store_combi_id[k] = store_combi_id[k] - 1;
                        }
                    }
                }

                i--;
            }
            else {
                for (uint8_t j=0; j<t_nc_header_info.GetN(); j++){
                    SequenceNumber32 t_sn = t_nc_header_info.GetSN(j);

                    if (t_sn == sn){
                        store_NcHeaderInfo.push_back(t_nc_header_info);
                        store_combi_id.push_back(i);
                    }
                }
            }
        }
    }
}

bool NcDecode::Check_store_Variable (SequenceNumber32 sn){
    for (uint8_t i=0; i<store_Variable.size(); i++){
        if (store_Variable[i] == sn){
            return true;
        }
    }
    return false;
}

bool NcDecode::CreateAlpha(SequenceNumber32 sn, uint8_t id, Ptr<Packet> packet, NcBuffer *Nc_Buffer){
    // true: can create;
    // false: cannot create => Cannot decode;

    store_Alpha[id] = new uint8_t[store_Variable.size()];
    Ptr<Packet> t_combin;

    //    if (sn >= (SequenceNumber32)71600001 && sn <= (SequenceNumber32)71649001) {
    //        NS_LOG_UNCOND ("CreateAlpha - sn = " << sn);
    //    }

    for (uint8_t i=0; i<t_store_Decoded.size(); i++){
        if (sn == t_store_Decoded[i]){

            store_AlreadyDecoded_id.push_back(id);
            t_store_Decoded.erase(t_store_Decoded.begin()+i);

            for (uint8_t j=0; j<store_Variable.size(); j++){
                if (j==id)
                    store_Alpha[id][j] = 1;
                else
                    store_Alpha[id][j] = 0;
            }

            t_combin = Nc_Buffer->Read(sn, true);
            store_combi[id] = t_combin;
            return true;
        }
    }

    bool return_value = false;

    SequenceNumber32 t_snN_min = (SequenceNumber32)(0-1);
    SequenceNumber32 t_sn1_min = (SequenceNumber32)(0-1);;
    uint8_t t_index = 0;

    for (uint8_t i=0; i<t_store_NcHeaderInfo.size(); i++){
        SequenceNumber32 t_first_sn = (SequenceNumber32)(t_store_NcHeaderInfo[i].GetSN(0));
        SequenceNumber32 t_last_sn  = (SequenceNumber32)(t_store_NcHeaderInfo[i].GetSN(t_store_NcHeaderInfo[i].GetN() - 1));

        if (sn.GetValue() >= t_first_sn.GetValue() && sn.GetValue() <= t_last_sn.GetValue()) {
            bool sn_inside = false;

            for (uint8_t j=0; j<t_store_NcHeaderInfo[i].GetN(); j++){
                SequenceNumber32 t_sn = t_store_NcHeaderInfo[i].GetSN(j);

                if (sn == t_sn) {
                    sn_inside = true;
                    break;
                }
            }


            if (sn_inside == true && t_last_sn.GetValue() <= t_snN_min.GetValue()){
                for (uint8_t j=0; j<t_store_NcHeaderInfo[i].GetN(); j++){
                    SequenceNumber32 t_sn = t_store_NcHeaderInfo[i].GetSN(j);

                    if (sn == t_sn) {
                        t_snN_min = t_last_sn;
                        t_sn1_min = t_first_sn;
                        t_index  = i;

                        uint8_t t_counter = 0;

                        for (uint8_t k=0; k<store_Variable.size(); k++){
                            if (t_counter < t_store_NcHeaderInfo[i].GetN()) {
                                SequenceNumber32 t_sn2 = t_store_NcHeaderInfo[i].GetSN(t_counter);

                                if (t_sn2 == store_Variable[k]){
                                    store_Alpha[id][k] = t_store_NcHeaderInfo[i].GetAlpha(t_counter);
                                    t_counter++;
                                }
                                else
                                    store_Alpha[id][k] = 0;
                            }
                            else {
                                store_Alpha[id][k] = 0;
                            }
                        }

                        return_value = true;
                    }
                }
            }
        }
    }

    if (return_value == true){
        // Erase old header=============================================
        if (t_store_combi_id[t_index] != (uint64_t)(0-1)){
            store_combi[id] = Read_Combination(t_store_combi_id[t_index]);
            store_combi_id2.push_back(t_store_combi_id[t_index]);
        }
        else {
            store_combi[id] = packet;
        }

        t_store_NcHeaderInfo.erase(t_store_NcHeaderInfo.begin()+t_index);
        t_store_combi_id.erase(t_store_combi_id.begin()+t_index);
    }

    return return_value;
}


void NcDecode::Store_Combination (Ptr<Packet> packet, NcHeaderInfo HeaderInfo){
    NcHeaderInfo_CombinationStoreTable. push_back(HeaderInfo);

    uint8_t  t_packet[packet->GetSize()];
    uint8_t *t_packet_pointer;
    t_packet_pointer = &t_packet[0];
    packet->CopyData(t_packet_pointer, packet->GetSize());

    std::vector<uint8_t> _combiData;
    for (uint16_t i=0; i<packet->GetSize(); i++){
        _combiData.push_back(t_packet[i]);
    }
    CombiData_CombinationStoreTable.push_back(_combiData);
}

void NcDecode::Erase_Combination (uint64_t id){
//    if (mm_LOG) {
//        std::cout << "DEL: ";
//        for (int i=0; i<NcHeaderInfo_CombinationStoreTable[id].GetN(); i++) {
//            std::cout << NcHeaderInfo_CombinationStoreTable[id].GetSN(i) << " ; ";
//        }
//        std::cout << "\n";
//    }

    NcHeaderInfo_CombinationStoreTable.erase(NcHeaderInfo_CombinationStoreTable.begin() + id);
    CombiData_CombinationStoreTable.erase(CombiData_CombinationStoreTable.begin() + id);
}

Ptr<Packet> NcDecode::Read_Combination (uint64_t id){
    uint16_t size = CombiData_CombinationStoreTable[id].size();
    uint8_t  t_packet[size];
    uint8_t *t_packet_pointer;
    t_packet_pointer = &t_packet[0];

    for (uint16_t i=0; i<size; i++){
        t_packet[i] = CombiData_CombinationStoreTable[id][i];
    }

    return Create<Packet>(t_packet_pointer, size);
}


SequenceNumber32 NcDecode::Get_UnSeen (){
    SequenceNumber32 LastUnseenSN = NC_SeenUnseen->GetLastestUnseen(true).SN;

    NcDecode::Get_Dependent_return Get_Dependent_Return;
    Get_Dependent_Return =  Get_Dependent();

    if (Get_Dependent_Return.have) {
        if (Get_Dependent_Return.lastunseen_sn.GetValue() < LastUnseenSN.GetValue()) {
            return Get_Dependent_Return.lastunseen_sn;
        }
    }

//    if (Simulator::Now().GetSeconds()>1397.97 && Simulator::Now().GetSeconds()<1397.99) {
//        std::cout << " 1--LastUnseenSN=" << LastUnseenSN << "\n";
//    }
    return LastUnseenSN;
}


Ptr<Packet> NcDecode::GetPacket (uint8_t i) {
    std::cout << "NcDecode::GetPacket  -  i=" << (int)i << "   -   " << this << "\n";

    uint16_t size = store_combi[i]->GetSize();
    uint8_t buffer[size];
    uint8_t *buffer_pointer;
    buffer_pointer = &buffer[0];
    store_combi[i]->CopyData(buffer_pointer,size);
    //
    Ptr<Packet> p_return = Create<Packet>(buffer_pointer,store_Sz[i]);

    return p_return;
    //return store_combi[i];
}

SequenceNumber32 NcDecode::GetSN(uint8_t i){
    return store_SN[i];
}


void NcDecode::Set_Dependent (SequenceNumber32 sn, SequenceNumber32 last_unseen){
    //std::cout << "Set_Dependent - SN=" << sn << "\n";
    SN_DependentTable        .push_back(sn);
    LastUnseen_DependentTable.push_back(last_unseen);
}


NcDecode::Get_Dependent_return NcDecode::Get_Dependent(){
    NcDecode::Get_Dependent_return returnvalue;
    returnvalue.have = false;

    bool first = true;
    SequenceNumber32 smallest, last_unseen;

    for (uint16_t i=0; i<SN_DependentTable.size(); i++){
        if (first) {
            smallest    = SN_DependentTable[i];
            last_unseen = LastUnseen_DependentTable[i];
            first       = false;
            returnvalue.have = true;
        }
        else {
            if (SN_DependentTable[i] < smallest){
                smallest    = SN_DependentTable[i];
                last_unseen = LastUnseen_DependentTable[i];
            }
        }
    }

    returnvalue.dependent_sn  = smallest;
    returnvalue.lastunseen_sn = last_unseen;
    return returnvalue;
}


bool NcDecode::Check_Dependent (SequenceNumber32 sn){
    for (uint16_t i=0; i<SN_DependentTable.size(); i++){
        if (sn == SN_DependentTable[i])
            return true;
    }

    return false;
}


void NcDecode::Del_Dependent (SequenceNumber32 sn){
    for (uint16_t i=0; i<SN_DependentTable.size(); i++){
        if (sn == SN_DependentTable[i]){
            SN_DependentTable        .erase(SN_DependentTable.begin() + i);
            LastUnseen_DependentTable.erase(LastUnseen_DependentTable.begin() + i);

//            if (mm_LOG)
//                std::cout << "Del_Dependent - SN=" << sn << "\n";
        }
    }
}

bool NcDecode::IsRedundancyAck (){
    return IsRedundancy;
}


}// end namespace
