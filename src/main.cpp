#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/OptionsLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude; 
namespace fs = std::filesystem;


class $modify(LoadingLayer) {
    // load saved graphics setting instead of medium graphics on startup
    bool init(bool p0) {
        GameManager::sharedState()->m_texQuality = Mod::get()->getSavedValue<int64_t>("quality");
        if (GameManager::sharedState()->m_texQuality == 1) {
                CCDirector::get()->updateContentScale(TextureQuality::kTextureQualityLow);
        }
        if (GameManager::sharedState()->m_texQuality == 3) {
            CCDirector::get()->updateContentScale(TextureQuality::kTextureQualityHigh);
        }
        
        return LoadingLayer::init(p0);
    }
};

class $modify(CCDirector) {
    // force the game to use high graphics
    void updateContentScale(TextureQuality p0) {
        CCDirector::updateContentScale(p0);
        if (p0 == TextureQuality::kTextureQualityHigh) {
            this->m_fContentScaleFactor = 4.f;
            this->m_eTextureQuality = TextureQuality::kTextureQualityHigh;
        }
    }
};

class $modify(AltOptionsLayer, OptionsLayer) {
    void customSetup() {
        OptionsLayer::customSetup();

        // get the menu containing the buttons
        CCArrayExt<CCNode*> children = typeinfo_cast<CCLayer*>(this->getChildren()->objectAtIndex(0))->getChildren();
        CCMenu* menu = nullptr;

        for (auto* child: children) {
            if (child->getChildrenCount() == 6 || child->getChildrenCount() == 7) {
                menu = typeinfo_cast<CCMenu*>(child);
                break;
            }
        }

        float accountX;
        float helpX;
        float optionsY;
        CCSize optionsSize;

        // waiting for nodeids
        CCArrayExt<CCMenuItemSpriteExtra*> buttons = menu->getChildren();
        for (auto* button: buttons) {
            auto sprite = typeinfo_cast<ButtonSprite*>(button->getChildren()->objectAtIndex(0));

            if (strcmp(getChildOfType<CCLabelBMFont>(sprite, 0)->getString(), "Account") == 0) {
                accountX = button->getPositionX();
            } else if (strcmp(getChildOfType<CCLabelBMFont>(sprite, 0)->getString(), "How To Play") == 0)  {
                helpX = button->getPositionX();
            } else if (strcmp(getChildOfType<CCLabelBMFont>(sprite, 0)->getString(), "Options") == 0) {
                button->setPositionX(accountX);
                optionsY = button->getPositionY();
                optionsSize = button->getContentSize();
            }
        }

        auto videoSprite = ButtonSprite::create("Graphics",130, 0, 1.0, true, "goldFont.fnt", "GJ_button_01.png", 0.0);
        auto videoButton = CCMenuItemSpriteExtra::create(videoSprite, this, menu_selector(AltOptionsLayer::onVideo));
        
        videoButton->setContentSize(optionsSize);
        videoButton->setPosition({helpX, optionsY});
        menu->addChild(videoButton);
    }

    void onVideo(CCObject* sender) {
        VideoOptionsLayer::create()->show();    
    }
};

bool highLoaded = false;
bool correctPack = false;
bool hasWarned = false;

class $modify (MenuLayer) {
    // popup for if you havent applied the high-textures pack
    bool init() {
        if (!MenuLayer::init()) return false;

        for (auto& p : CCFileUtils::sharedFileUtils()->getSearchPaths()) {

            auto path = fs::path(p.c_str()).parent_path();

            if (strcmp(path.parent_path().filename().string().c_str(), "packs") == 0) {

                if (fs::exists(path / "pack.json")) {

                    std::ifstream packjson((path / "pack.json").string());
                    std::string content( 
                        (std::istreambuf_iterator<char>(packjson) ),
                        (std::istreambuf_iterator<char>(        ) )
                    );

                    if (strcmp(matjson::parse(content)["id"].as_string().c_str(), "weebify.high-textures") == 0) {
                        highLoaded = true;
                        if (strcmp(matjson::parse(content)["version"].as_string().c_str(), "1.0.1") == 0) {
                            correctPack = true;
                            break;
                        }
                    }
                }
            }
        }


        if (!highLoaded) {
            if (!hasWarned) {
                hasWarned = true;
                geode::Loader::get()->queueInMainThread([] {
                    if (!Mod::get()->getSettingValue<bool>("disable-popup")) {
                        geode::createQuickPopup("Woops!",
                            "It looks like you haven't loaded the <cr>default high graphics textures</c> yet. If you haven't, please download the high textures zip file <cl>using the download button below</c>, <co>unzip it</c> and <cr>load it using Texture Loader</c>. You can disable this popup in the mod's setting page",
                            "CANCEL", "DOWNLOAD",
                            [](auto, bool btn2) {
                                if (btn2) {
                                    geode::utils::web::openLinkInBrowser("https://drive.google.com/drive/folders/1kLSaVvQuGQvvI_hT0p6doAHAtTrVy_uM?usp=sharing");
                                }
                            }
                        );
                    }
                });
            }
        } else if (!correctPack) {
            if (!hasWarned) {
                hasWarned = true;
                geode::Loader::get()->queueInMainThread([] {
                    if (!Mod::get()->getSettingValue<bool>("disable-popup")) {
                        geode::createQuickPopup("Woops!",
                            "It looks like you haven't loaded the <cj>correct high graphics textures for the current GD version (2.206)</c> yet. If you haven't, please download the high textures zip file <cy>using the download button below</c>, <co>unzip it</c> and <cr>load it using Texture Loader</c>. You can disable this popup in the mod's setting page",
                            "CANCEL", "DOWNLOAD",
                            [](auto, bool btn2) {
                                if (btn2) {
                                    geode::utils::web::openLinkInBrowser("https://drive.google.com/drive/folders/1kLSaVvQuGQvvI_hT0p6doAHAtTrVy_uM?usp=sharing");
                                }
                            }
                        );
                    }
                });
            }
        }

        return true;
    }
};

$on_mod(DataSaved) {
    log::info("Saving texture packs");
    Mod::get()->setSavedValue("quality", GameManager::sharedState()->m_texQuality);
}
