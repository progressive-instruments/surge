/*
** Surge Synthesizer is Free and Open Source Software
**
** Surge is made available under the Gnu General Public License, v3.0
** https://www.gnu.org/licenses/gpl-3.0.en.html
**
** Copyright 2004-2020 by various individuals as described by the Git transaction log
**
** All source at: https://github.com/surge-synthesizer/surge.git
**
** Surge was a commercial product from 2004-2018, with Copyright and ownership
** in that period held by Claes Johanson at Vember Audio. Claes made Surge
** open source in September 2018.
*/

#pragma once
#include "SurgeStorage.h"
#include "SurgeBitmaps.h"
#include "SkinSupport.h"
#include <unordered_map>
#include <vector>
#include <map>

class CSnapshotMenu : public VSTGUI::CControl, public Surge::GUI::SkinConsumingComponent
{
  public:
    CSnapshotMenu(const VSTGUI::CRect &size, VSTGUI::IControlListener *listener, long tag,
                  SurgeStorage *storage);
    virtual ~CSnapshotMenu();
    virtual void draw(VSTGUI::CDrawContext *dc) override;
    virtual void populate();
    virtual void loadSnapshot(int type, TiXmlElement *e, int idx){};
    virtual bool loadSnapshotByIndex(int idx);

    virtual void saveSnapshot(TiXmlElement *e, const char *name){};
    virtual bool canSave();

    virtual VSTGUI::CMouseEventResult onMouseDown(VSTGUI::CPoint &where,
                                                  const VSTGUI::CButtonState &button) override;

    virtual VSTGUI::CMouseEventResult onMouseEntered(VSTGUI::CPoint &where,
                                                     const VSTGUI::CButtonState &buttons) override
    {
        // getFrame()->setCursor( VSTGUI::kCursorHand );
        isHovered = true;
        invalid();
        return VSTGUI::kMouseEventHandled;
    }
    virtual VSTGUI::CMouseEventResult onMouseExited(VSTGUI::CPoint &where,
                                                    const VSTGUI::CButtonState &buttons) override
    {
        // getFrame()->setCursor( VSTGUI::kCursorDefault );
        isHovered = false;
        invalid();
        return VSTGUI::kMouseEventHandled;
    }

    bool onWheel(const VSTGUI::CPoint &where, const float &distance,
                 const VSTGUI::CButtonState &buttons) override
    {
        // TODO - mousehweel support when these things can jog and maintain state
        return true;
    }

    juce::PopupMenu menu;
    bool isHovered = false;
    int selectedIdx = -1;
    std::string selectedName = "";

  protected:
    void populateSubmenuFromTypeElement(TiXmlElement *typeElement, juce::PopupMenu &parent,
                                        int &idx, bool isTopLevel = false);
    virtual void addToTopLevelTypeMenu(TiXmlElement *typeElement, juce::PopupMenu &subMenu,
                                       int &idx)
    {
    }
    virtual void setMenuStartHeader(TiXmlElement *typeElement, juce::PopupMenu &subMenu) {}
    SurgeStorage *storage = nullptr;
    char mtype[16] = {0};
    std::map<int, int> firstSnapshotByType;
    std::vector<std::pair<int, TiXmlElement *>> loadArgsByIndex;
    int maxIdx;

    // The parent class is too chatty with the listener, calling a value changed every time I close
    // which means non-swapping menus like copy and paste do the wrong thing
    VSTGUI::IControlListener *listenerNotForParent;
};

class COscMenu : public CSnapshotMenu
{
  public:
    COscMenu(const VSTGUI::CRect &size, VSTGUI::IControlListener *listener, long tag,
             SurgeStorage *storage, OscillatorStorage *osc, std::shared_ptr<SurgeBitmaps>);
    virtual void draw(VSTGUI::CDrawContext *dc) override;
    virtual void loadSnapshot(int type, TiXmlElement *e, int idx) override;

    virtual bool onWheel(const VSTGUI::CPoint &where, const float &distance,
                         const VSTGUI::CButtonState &buttons) override;

    bool text_allcaps = true;
    int text_hoffset = 0;
    int text_voffset = 0;
    int font_size = 8;
    int font_style;
    VSTGUI::CHoriTxtAlign text_align;

  protected:
    OscillatorStorage *osc = nullptr;
    SurgeStorage *storage = nullptr;
    VSTGUI::CBitmap *bmp = nullptr, *hoverBmp = nullptr;
    bool attemptedHoverLoad = false;
    float accumWheel = 0;

    CLASS_METHODS(COscMenu, VSTGUI::CControl)
};

class CFxMenu : public CSnapshotMenu
{
  public:
    CFxMenu(const VSTGUI::CRect &size, VSTGUI::IControlListener *listener, long tag,
            SurgeStorage *storage, FxStorage *fx, FxStorage *fxbuffer, int slot);
    virtual void draw(VSTGUI::CDrawContext *dc) override;
    virtual bool canSave() override { return true; }
    virtual void loadSnapshot(int type, TiXmlElement *e, int idx) override;
    virtual void saveSnapshot(TiXmlElement *e, const char *name) override;
    virtual void populate() override;

  protected:
    virtual void addToTopLevelTypeMenu(TiXmlElement *typeElement, juce::PopupMenu &subMenu,
                                       int &idx) override;
    FxStorage *fx = nullptr, *fxbuffer = nullptr;
    static std::vector<float>
        fxCopyPaste; // OK this is a crap data structure for now. See the code.
    int slot = 0;

    void copyFX();
    void pasteFX();
    void saveFX();
    void saveFXIn(const std::string &s);

    // We know this runs on the UI thread to populate always so we can use a static for user presets
    struct UserPreset
    {
        std::string file;
        std::string name;
        int type;
        float p[n_fx_params];
        bool ts[n_fx_params], er[n_fx_params], da[n_fx_params];

        UserPreset()
        {
            type = 0;

            for (int i = 0; i < n_fx_params; ++i)
            {
                p[i] = 0.0;
                ts[i] = false;
                er[i] = false;
                da[i] = false;
            }
        }
    };

    void setMenuStartHeader(TiXmlElement *typeElement, juce::PopupMenu &subMenu) override;

    static std::unordered_map<int, std::vector<UserPreset>> userPresets; // from type to presets

  public:
    static bool scanForUserPresets;

  protected:
    void rescanUserPresets();
    void loadUserPreset(const UserPreset &p);

    bool triedToLoadBmp = false;
    CScalableBitmap *pBackground, *pBackgroundHover;

    CLASS_METHODS(CFxMenu, VSTGUI::CControl)
};
