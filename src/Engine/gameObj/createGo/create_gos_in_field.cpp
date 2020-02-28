/*
 * ================= create_gos_in_field.cpp ======================
 *                          -- tpr --
 *                                        CREATE -- 2019.09.27
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */



//-------------------- Engine --------------------//
#include "tprAssert.h"
#include "esrc_field.h"
#include "config.h"
#include "chunkKey.h"
#include "dyParams.h"
#include "GameObj.h"
#include "create_goes.h"
#include "GoSpecFromJson.h"

#include "esrc_field.h"
#include "esrc_ecoObj.h"
#include "esrc_gameObj.h" 
#include "esrc_time.h" 
#include "esrc_chunk.h" 
#include "esrc_job_chunk.h"



/* only called in chunkCreate
 * all kinds of gos 
 */
void create_gos_in_field(   fieldKey_t      fieldKey_, 
                            const Chunk     &chunkRef_,
                            const Job_Chunk &job_chunkRef_ ){

    const auto &fieldRef = esrc::get_field(fieldKey_);
    const auto *job_fieldPtr = job_chunkRef_.get_job_fieldPtr(fieldKey_);

    //----- ground go ------//
    if( auto retOpt = job_fieldPtr->get_groundGoDataPtr(); retOpt.has_value() ){
        gameObjs::create_go_from_goDataForCreate( retOpt.value() );
    }

    //----- fieldRim go [-DEBUG-] ------//
    //  显示 map 坐标框
    bool isFieldRimGoCreate { false };
    if( isFieldRimGoCreate ){

        auto goDataUPtr = GoDataForCreate::assemble_new_goDataForCreate(  
                                                    fieldRef.get_midMPos(),
                                                    fieldRef.get_midDPos(),
                                                    GoSpecFromJson::str_2_goSpeciesId("fieldRim"),
                                                    GoAssemblePlanSet::str_2_goLabelId(""),
                                                    NineDirection::Center,
                                                    BrokenLvl::Lvl_0
                                                );
        gameObjs::create_go_from_goDataForCreate( goDataUPtr.get() );
    }
    

    //----- bioSoup ------//    
    for( const auto goDataPtr : job_fieldPtr->get_bioSoupGoDataPtrs() ){
        gameObjs::create_go_from_goDataForCreate( goDataPtr );
    }

    //----- land majorGo in blueprint -----//
    for( const auto goDataPtr : job_fieldPtr->get_majorGoDataPtrs() ){
        gameObjs::create_go_from_goDataForCreate( goDataPtr );
    }

    
    for( const auto goDataPtr : job_fieldPtr->get_floorGoDataPtrs() ){
        gameObjs::create_go_from_goDataForCreate( goDataPtr );
    }
    
}


