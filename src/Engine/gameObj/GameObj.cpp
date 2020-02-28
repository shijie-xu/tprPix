/*
 * ========================= GameObj.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2018.11.24
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "GameObj.h" 

//-------------------- CPP --------------------//
#include <functional>

//-------------------- Engine --------------------//
#include "Chunk.h"
#include "GoSpecFromJson.h"


#include "esrc_chunk.h"
#include "esrc_shader.h"
#include "esrc_coordinate.h"

using namespace std::placeholders;

#include "tprDebug.h" //- tmp


//============== static ===============//
ID_Manager  GameObj::id_manager { ID_TYPE::U64, 0};


/* ===========================================================
 *                   init_for_regularGo
 * -----------------------------------------------------------
 */
void GameObj::init_for_regularGo( const glm::dvec2 &dpos_ ){
    
    //-----------------------//
    //    choose goPos
    //-----------------------//
    this->goPosVUPtr = std::make_unique<GameObjPos>();
    auto vPtr = std::get_if<std::unique_ptr<GameObjPos>>( &this->goPosVUPtr );
    tprAssert( vPtr );
    GameObjPos *goPosPtr = vPtr->get();
    goPosPtr->init( dpos_ );

    //-- bind functors --
    this->accum_dpos            = std::bind( &GameObjPos::accum_dpos, goPosPtr, _1 );
    this->set_pos_alti          = std::bind( &GameObjPos::set_alti, goPosPtr,  _1 );
    this->set_pos_lAltiRange    = std::bind( &GameObjPos::set_lAltiRange, goPosPtr,  _1 );
    this->get_dpos              = std::bind( &GameObjPos::get_dpos, goPosPtr );
    this->get_pos_alti          = std::bind( &GameObjPos::get_alti, goPosPtr );
    this->get_pos_lAltiRange    = std::bind( &GameObjPos::get_lAltiRange, goPosPtr );

    //-----------------------//
    //    collision
    //-----------------------//
    this->collisionUPtr = ((this->family==GameObjFamily::Major) || (this->family==GameObjFamily::BioSoup)) ?
                                std::make_unique<Collision>(*this) :
                                nullptr;

    //-----------------------//
    //         oth
    //-----------------------//
    this->currentChunkKey = anyMPos_2_chunkKey( dpos_2_mpos(this->get_dpos()) );

    //...
}

/* ===========================================================
 *                   init_for_uiGo
 * -----------------------------------------------------------
 */
void GameObj::init_for_uiGo(const glm::dvec2 &basePointProportion_,
                            const glm::dvec2 &offDPos_ ){

    //-----------------------//
    //    choose uiGoPos
    //-----------------------//
    this->goPosVUPtr = std::make_unique<UIAnchor>();
    auto vPtr = std::get_if<std::unique_ptr<UIAnchor>>( &this->goPosVUPtr );
    UIAnchor *uiGoPosPtr = vPtr->get();
    uiGoPosPtr->init( basePointProportion_, offDPos_ );

    //-- bind functors --
    this->accum_dpos            = std::bind( &UIAnchor::accum_dpos, uiGoPosPtr, _1 );
    this->set_pos_alti          = std::bind( &UIAnchor::set_alti, uiGoPosPtr, _1 );
    this->get_dpos              = std::bind( &UIAnchor::get_dpos, uiGoPosPtr );
    this->get_pos_alti          = std::bind( &UIAnchor::get_alti, uiGoPosPtr );

    // uiGo 不应该被 调用 lAltiRange 相关的 函数
    this->set_pos_lAltiRange    = [](GoAltiRange new_){ tprAssert(0); };
    this->get_pos_lAltiRange    = [](){ tprAssert(0); return GoAltiRange{}; };

    //-----------------------//
    //    collision
    //-----------------------//
    this->collisionUPtr = nullptr;

    //-----------------------//
    //         oth
    //-----------------------//
    //...
}


/* ===========================================================
 *                     creat_new_goMesh
 * -----------------------------------------------------------
 * -- 通过一组参数来实现 gomesh 的初始化。
 */
GameObjMesh &GameObj::creat_new_goMesh( const std::string &name_,
                                    animSubspeciesId_t     subspeciesId_,
                                    AnimActionEName     actionEName_,
                                    RenderLayerType     layerType_,
                                    ShaderType          shaderType_,
                                    const glm::vec2     pposOff_,
                                    double              zOff_,
                                    bool                isVisible_ ){

    auto [insertIt, insertBool] = this->goMeshs.insert({name_, std::make_unique<GameObjMesh>( *this, pposOff_, zOff_,isVisible_ ) }); 
    tprAssert( insertBool );

    GameObjMesh &gmesh = *(this->goMeshs.at(name_));    

    //-- bind_animAction --//
    //-- 确保提前设置好了 go.direction ！！！
    gmesh.set_animSubspeciesId( subspeciesId_ );
    gmesh.set_animActionEName( actionEName_ );
    gmesh.bind_animAction(); // Must Before Everything!!!

    //----- init -----//
    gmesh.set_pic_renderLayer( layerType_ ); 
    gmesh.set_pic_shader_program( &esrc::get_shaderRef(shaderType_) );
    if( gmesh.isHaveShadow ){
        gmesh.set_shadow_shader_program( &esrc::get_shaderRef( ShaderType::Shadow ) ); //- 暂时自动选配 tmp
    }
    
    //-- rootColliEntHeadPtr --//
    /*
    if( name_ == std::string{"root"} ){
        this->rootAnimActionPosPtr = &gmesh.get_currentAnimActionPos();
    }
    */

    return gmesh;
}

/* ===========================================================
 *                    init_check
 * -----------------------------------------------------------
 * 收尾工作，杂七杂八
 */
void GameObj::init_check(){

    tprAssert( this->colliDataFromJsonPtr );

    //---
    if( (this->family==GameObjFamily::Major) || (this->family==GameObjFamily::BioSoup) ){

        // MUST NOT EMPTY !!! 
        tprAssert( (this->goPosVUPtr.index()!=0) && (this->goPosVUPtr.index()!=std::variant_npos) );

        //- 参与 moveCollide 的仅有 majorGo:Cir 
        if( this->get_colliderType() == ColliderType::Circular  ){
            //-- 主动调用，init signINMapEnts --- MUST!!!
            this->collisionUPtr->init_signInMapEnts_circle( this->get_dpos(),
                        std::bind( &ColliDataFromJson::get_colliPointDPosOffs, this->colliDataFromJsonPtr ) );
        }
    }

    //-- 在检测完毕后，可以直接无视这些 flags 的混乱，仅用 go自身携带的 flag 来做状态判断 ...
}


/* ===========================================================
 *           rebind_rootAnimActionPosPtr
 * -----------------------------------------------------------
 *          
 * 
 *  未完工。。。
 */
void GameObj::rebind_rootAnimActionPosPtr(){
    //-- 仅在 root gomesh 切换 action 时才被调用 ！！！
    // do nothing 
}


/* ===========================================================
 *                    reCollect_chunkKeys     [ IMPORTANT !!! ]
 * -----------------------------------------------------------
 * 遍历当前 goPos 中每个 colliEnt，收集其所在 chunkKeys
 * ---
 * 此函数 只是单纯记录 本go相关的所有 chunk key 信息。
 * 此过程中并不访问 chunk实例 本身。所有，就算相关 chunk 尚未创建，也不影响本函数的执行。
 */
size_t GameObj::reCollect_chunkKeys(){

    tprAssert( (this->family==GameObjFamily::Major) || (this->family==GameObjFamily::BioSoup) );
    this->chunkKeys.clear();

    auto colliType = this->get_colliderType();
    if( colliType == ColliderType::Circular ){
        // only MajorGo
        for( const auto &mpos : this->get_collisionRef().get_current_signINMapEnts_circle_ref() ){
            this->chunkKeys.insert( anyMPos_2_chunkKey(mpos) ); // maybe
        }

    }else if( colliType == ColliderType::Square ){
        // MajorGo / BioSoupGo
        IntVec2 goRootMPos = dpos_2_mpos(this->get_dpos());
        for( const auto &mposOff : this->get_signInMapEnts_square_ref().get_all_mapEntOffs() ){
            this->chunkKeys.insert( anyMPos_2_chunkKey( goRootMPos + mposOff ) ); // maybe
        }

    }else{
        tprAssert(0);
    }
    //===
    return this->chunkKeys.size();
}



void GameObj::debug(){

    auto mapEntPair = esrc::getnc_memMapEntPtr( dpos_2_mpos(this->get_dpos()) );
    tprAssert( mapEntPair.first == ChunkMemState::Active );

    MemMapEnt &mpRef = *mapEntPair.second;

    cout << "mapEnt.lvl: " << mpRef.get_mapAlti().lvl
        << "; val: " << mpRef.get_mapAlti().val
        << "; mp-ecoKey: " << mpRef.get_ecoObjKey()
        << endl;

    

    

    /*
    cout << "sizeof(go) = " << sizeof( *this )
        << "; sizeof(GO) = " << sizeof( GameObj )
        << endl;
    */

}

