/*
 * ===================== GoDataForCreate.h ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.12.02
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#ifndef TPR_GO_DATA_FOR_CREATE_H
#define TPR_GO_DATA_FOR_CREATE_H

//--- glm - 0.9.9.5 ---
#include "glm_no_warnings.h"


//------------------- CPP --------------------//
#include <vector>
#include <memory>
#include <variant>
#include <optional>

//------------------- Engine --------------------//
#include "tprAssert.h"
#include "GameObjType.h"
#include "AnimLabel.h"
#include "NineDirection.h"
#include "BrokenLvl.h"
#include "FloorGoType.h"
#include "animSubspeciesId.h"
#include "IntVec.h"
#include "AnimActionEName.h"
#include "GoAltiRange.h"
#include "GoAssemblePlan.h"
#include "AnimLabel.h"



// 生成一个go实例，需要的基本数据
// - 在 ecoobj 生成阶段，此数据被创建，并永久存储在 ecoobj 中
// - 以 const 指针 的形式，被传递到 job chunk/filed, 以及 具象go类中
// 所以，不用担心此数据的 容量
class GoDataForCreate{
public:
    GoDataForCreate()=default;

    class GoMeshBase;
    class GoMeshByLink;
    class GoMeshByHand; 

    
    //========== Self Vals ==========//
    static std::unique_ptr<GoDataForCreate> assemble_new_goDataForCreate(  
                                                IntVec2             mpos_,             
                                                const glm::dvec2    &dpos_, // 让外部计算好
                                                goSpeciesId_t       goSpeciesId_,
                                                goLabelId_t         goLabelId_,
                                                NineDirection       direction_, // 未来支持从 GoSpecFromJson 中提取默认值
                                                BrokenLvl           brokenLvl_ // 未来支持从 GoSpecFromJson 中提取默认值
                                                );

    static std::unique_ptr<GoDataForCreate> assemble_new_floorGoDataForCreate(  
                                                IntVec2             mpos_,             
                                                const glm::dvec2    &dpos_, // 让外部计算好
                                                goSpeciesId_t       goSpeciesId_,
                                                goLabelId_t         goLabelId_,
                                                NineDirection       direction_ // 未来支持从 GoSpecFromJson 中提取默认值
                                                );

    //---
    goSpeciesId_t       goSpeciesId {};
    goLabelId_t         goLabelId   {};
    glm::dvec2          dpos      {}; // go 绝对 dpos

    NineDirection       direction {NineDirection::Center};  //- 角色 动画朝向
    GoAltiRangeLabel    goAltiRangeLabel {};

    BrokenLvl           brokenLvl   {};

    size_t              uWeight     {}; // base on mpos


    const ColliDataFromJson *colliDataFromJsonPtr {nullptr};
    
    //---

    //bool            isNeedWind    {}; // 是否需要生成 风吹值,暂时 始终为 true

    std::vector<std::unique_ptr<GoMeshBase>> goMeshEntUPtrs {};
};




// interface
class GoDataForCreate::GoMeshBase{
public:
    GoMeshBase()=default;
    virtual ~GoMeshBase()=default;
    //---
    virtual const std::string   &get_goMeshName()const =0;
    virtual const std::string   &get_animFrameSetName()const =0;
    virtual glm::dvec2           get_dposOff()const =0;
    virtual double               get_zOff()const =0;
    virtual AnimLabel            get_animLabel()const =0;
    virtual AnimActionEName      get_animActionEName()const =0;
    virtual RenderLayerType      get_renderLayerType()const =0;
    virtual ShaderType           get_shaderType()const =0;
    virtual bool                 get_isVisible()const =0;
    //---
    virtual animSubspeciesId_t   get_subspeciesId()const =0;
    virtual size_t               get_windDelayIdx()const =0;
    virtual size_t               get_uWeight()const =0;
    virtual FloorGoLayer         get_floorGoLayer()const =0;
};


// 推荐的方案
// 当需要的 gomesh 数据，直接可以从 GoAssemblePlanSet::GoMeshEnt 中取得时，
// 使用此方案。
class GoDataForCreate::GoMeshByLink : public GoDataForCreate::GoMeshBase{
public:
    GoMeshByLink( const GoAssemblePlanSet::GoMeshEnt  *goMeshEntPtr_,
                size_t uWeight_ ):
        goMeshEntPtr(goMeshEntPtr_),
        uWeight(uWeight_)
        {
            tprAssert( this->goMeshEntPtr );
            this->init_subspeciesId();
        }

    //------- set -------//
    inline void set_windDelayIdx( size_t windDelayIdx_ )noexcept{ this->windDelayIdx = windDelayIdx_; }

    //---- get vals by link -----//
    inline const std::string    &get_goMeshName()const override{ return this->goMeshEntPtr->goMeshName;}
    inline const std::string    &get_animFrameSetName()const override{ return this->goMeshEntPtr->animFrameSetName; }
    inline glm::dvec2           get_dposOff()const override{ return this->goMeshEntPtr->dposOff; }
    inline double               get_zOff()const override{ return this->goMeshEntPtr->zOff; }
    inline AnimLabel            get_animLabel()const override{ return this->goMeshEntPtr->animLabel; }
    inline AnimActionEName      get_animActionEName()const override{ return this->goMeshEntPtr->animActionEName; }
    inline RenderLayerType      get_renderLayerType()const override{ return this->goMeshEntPtr->renderLayerType; }
    inline ShaderType           get_shaderType()const override{ return this->goMeshEntPtr->shaderType; }
    inline bool                 get_isVisible()const override{ return this->goMeshEntPtr->isVisible; }
    
    //------- real optional vals -------//
    inline FloorGoLayer         get_floorGoLayer()const override{ 
        tprAssert( this->goMeshEntPtr->floorGoLayer.has_value() );
        return this->goMeshEntPtr->floorGoLayer.value();
    }

    //--- vals not in goMeshEntPtr -----//
    inline animSubspeciesId_t   get_subspeciesId()const override{ return this->subspeciesId; }
    inline size_t               get_windDelayIdx()const override{ return this->windDelayIdx; }
    inline size_t               get_uWeight()const override{ return this->uWeight; }

private:
    void init_subspeciesId()noexcept;

    const GoAssemblePlanSet::GoMeshEnt *goMeshEntPtr {nullptr};

    //--- vals not in goMeshEntPtr -----//
    animSubspeciesId_t  subspeciesId    {};
    size_t              windDelayIdx    {}; // only used in windClock
    size_t              uWeight         {}; // goMPos.uWeight + 一组有序的素数（按照 gomesh 排序）
};



// 手动装填的方案
// 适合动态 组建一些 gomesh，而不是用现成的。
// 比如用于 GroundGo 时
class GoDataForCreate::GoMeshByHand : public GoDataForCreate::GoMeshBase{
public:

    // auto 
    GoMeshByHand(   const GoAssemblePlanSet::GoMeshEnt  *goMeshEntPtr_,
                    size_t                              uWeight_ ):
        goMeshEntPtr(goMeshEntPtr_),
        uWeight(uWeight_)
        {
            tprAssert( this->goMeshEntPtr );
            this->init_subspeciesId(this->goMeshEntPtr->animFrameSetName,
                                    this->goMeshEntPtr->animLabel,
                                    this->uWeight );
        }

    // 需要手动指定 afsName,animLabel 的版本
    GoMeshByHand(   const GoAssemblePlanSet::GoMeshEnt  *goMeshEntPtr_,
                    const std::string                   &animFrameSetName_,
                    AnimLabel                           animLabel_,
                    size_t                              uWeight_ ):
        goMeshEntPtr(goMeshEntPtr_),
        animFrameSetName( {animFrameSetName_} ),
        animLabel( {animLabel_} ),
        uWeight(uWeight_)
        {
            tprAssert( this->goMeshEntPtr );
            this->init_subspeciesId(animFrameSetName_,
                                    animLabel_,
                                    uWeight_ );
        }

    //------- set -------//
    inline void set_windDelayIdx( size_t windDelayIdx_ )noexcept{ this->windDelayIdx = windDelayIdx_; }

    //----- customized vals ----------//
    inline const std::string    &get_goMeshName()const override{ 
        return  this->goMeshName.has_value() ?
                        this->goMeshName.value() :
                        this->goMeshEntPtr->goMeshName;
    }
    inline const std::string    &get_animFrameSetName()const override{ 
        return this->animFrameSetName.has_value() ?
                    this->animFrameSetName.value() :
                    this->goMeshEntPtr->animFrameSetName;
    }
    inline glm::dvec2           get_dposOff()const override{ 
        return this->dposOff.has_value() ?
                    this->dposOff.value() :
                    this->goMeshEntPtr->dposOff;
    }
    inline double               get_zOff()const override{ 
        return this->zOff.has_value() ?
                    this->zOff.value() :
                    this->goMeshEntPtr->zOff;
    }
    inline AnimLabel            get_animLabel()const override{ 
        return this->animLabel.has_value() ?
                    this->animLabel.value() :
                    this->goMeshEntPtr->animLabel;
    }
    inline AnimActionEName      get_animActionEName()const override{ 
        return this->animActionEName.has_value() ?
                    this->animActionEName.value() :
                    this->goMeshEntPtr->animActionEName;
    }
    inline RenderLayerType      get_renderLayerType()const override{ 
        return this->renderLayerType.has_value() ?
                    this->renderLayerType.value() :
                    this->goMeshEntPtr->renderLayerType;
    }
    inline ShaderType           get_shaderType()const override{ 
        return this->shaderType.has_value() ?
                    this->shaderType.value() :
                    this->goMeshEntPtr->shaderType;
    }
    inline bool                 get_isVisible()const override{ 
        return this->isVisible.has_value() ?
                    this->isVisible.value() :
                    this->goMeshEntPtr->isVisible;
    }
    
    //------- real optional vals -------//
    inline FloorGoLayer         get_floorGoLayer()const override{ 
        if( this->floorGoLayer.has_value() ){
            return this->floorGoLayer.value();
        }else{
            tprAssert( this->goMeshEntPtr->floorGoLayer.has_value() );
            return this->goMeshEntPtr->floorGoLayer.value();
        }
    }

    //--- vals not in goMeshEntPtr --
    inline animSubspeciesId_t   get_subspeciesId()const override{ return this->subspeciesId; }
    inline size_t               get_windDelayIdx()const override{ return this->windDelayIdx; }
    inline size_t               get_uWeight()const override{ return this->uWeight; }

    //===== values =====//
    //----- customized vals ----------//
    std::optional<std::string>             goMeshName {}; // 索引key
    //===
    std::optional<glm::dvec2>              dposOff {}; // gomesh-dposoff based on go-dpos
    std::optional<double>                  zOff    {};
    std::optional<AnimActionEName>         animActionEName {};
    std::optional<RenderLayerType>         renderLayerType {};
    std::optional<ShaderType>              shaderType   {};
    std::optional<bool>                    isVisible {};
    std::optional<bool>                    isAutoInit {};
    std::optional<NineDirection>           default_dir {};
    std::optional<BrokenLvl>               default_brokenLvl {}; 

    //------- real optional vals -------//
    std::optional<FloorGoLayer> floorGoLayer    { std::nullopt }; // only for FloorGo 

private:
    void init_subspeciesId( const std::string   &animFrameSetName_,
                            AnimLabel           label_,
                            size_t              uWeight_ )noexcept;

    //===== values =====//
    const GoAssemblePlanSet::GoMeshEnt *goMeshEntPtr {nullptr};

    //----- customized vals ----------//
    std::optional<std::string>             animFrameSetName {};
    std::optional<AnimLabel>               animLabel {};


    //--- vals not in goMeshEntPtr -----//
    animSubspeciesId_t      subspeciesId {};
    size_t                  windDelayIdx {}; // only used in windClock
    size_t                  uWeight     {}; // goMPos.uWeight + 一组有序的素数（按照 gomesh 排序）
};




#endif 

